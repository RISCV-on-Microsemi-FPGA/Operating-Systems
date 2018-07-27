/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <assert.h>
#include <stddef.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdio.h>

#include "sysflash/sysflash.h"

#include <bsp/bsp.h>

#include <flash_map/flash_map.h>
#include <hal/hal_flash.h>
#include <hal/hal_system.h>
#include <hal/hal_gpio.h>
#include <hal/hal_watchdog.h>

#include <os/endian.h>
#include <os/os.h>
#include <os/os_malloc.h>
#include <os/os_cputime.h>

#include <console/console.h>

#include <tinycbor/cbor.h>
#include <tinycbor/cbor_buf_reader.h>
#include <base64/base64.h>
#include <crc/crc16.h>

#include <bootutil/image.h>

#include "boot_serial/boot_serial.h"
#include "boot_serial_priv.h"

#if MYNEWT_VAL(OS_CPUTIME_TIMER_NUM) < 0
#error "Boot serial needs OS_CPUTIME timer"
#endif

#define BOOT_SERIAL_INPUT_MAX   512
#define BOOT_SERIAL_OUT_MAX     80

static uint32_t curr_off;
static uint32_t img_size;
static struct nmgr_hdr *bs_hdr;

static char bs_obuf[BOOT_SERIAL_OUT_MAX];

static int bs_cbor_writer(struct cbor_encoder_writer *, const char *data,
  int len);
static void boot_serial_output(void);

static struct cbor_encoder_writer bs_writer = {
    .write = bs_cbor_writer
};
static CborEncoder bs_root;
static CborEncoder bs_rsp;

int
bs_cbor_writer(struct cbor_encoder_writer *cew, const char *data, int len)
{
    if (cew->bytes_written + len > sizeof(bs_obuf)) {
        return CborErrorOutOfMemory;
    }

    memcpy(&bs_obuf[cew->bytes_written], data, len);
    cew->bytes_written += len;

    return 0;
}

/*
 * Looks for 'name' from NULL-terminated json data in buf.
 * Returns pointer to first character of value for that name.
 * Returns NULL if 'name' is not found.
 */
char *
bs_find_val(char *buf, char *name)
{
    char *ptr;

    ptr = strstr(buf, name);
    if (!ptr) {
        return NULL;
    }
    ptr += strlen(name);

    while (*ptr != '\0') {
        if (*ptr != ':' && !isspace(*ptr)) {
            break;
        }
        ++ptr;
    }
    if (*ptr == '\0') {
        ptr = NULL;
    }
    return ptr;
}

/*
 * Convert version into string without use of snprintf().
 */
static int
u32toa(char *tgt, uint32_t val)
{
    char *dst;
    uint32_t d = 1;
    uint32_t dgt;
    int n = 0;

    dst = tgt;
    while (val / d >= 10) {
        d *= 10;
    }
    while (d) {
        dgt = val / d;
        val %= d;
        d /= 10;
        if (n || dgt > 0 || d == 0) {
            *dst++ = dgt + '0';
            ++n;
        }
    }
    *dst = '\0';

    return dst - tgt;
}

/*
 * dst has to be able to fit "255.255.65535.4294967295" (25 characters).
 */
static void
bs_list_img_ver(char *dst, int maxlen, struct image_version *ver)
{
    int off;

    off = u32toa(dst, ver->iv_major);
    dst[off++] = '.';
    off += u32toa(dst + off, ver->iv_minor);
    dst[off++] = '.';
    off += u32toa(dst + off, ver->iv_revision);
    dst[off++] = '.';
    off += u32toa(dst + off, ver->iv_build_num);
}

/*
 * List images.
 */
static void
bs_list(char *buf, int len)
{
    CborEncoder images;
    CborEncoder image;
    struct image_header hdr;
    uint8_t tmpbuf[64];
    int i, area_id;
    const struct flash_area *fap;

    cbor_encoder_create_map(&bs_root, &bs_rsp, CborIndefiniteLength);
    cbor_encode_text_stringz(&bs_rsp, "images");
    cbor_encoder_create_array(&bs_rsp, &images, CborIndefiniteLength);
    for (i = 0; i < 2; i++) {
        area_id = flash_area_id_from_image_slot(i);
        if (flash_area_open(area_id, &fap)) {
            continue;
        }

        flash_area_read(fap, 0, &hdr, sizeof(hdr));

        if (hdr.ih_magic != IMAGE_MAGIC ||
          bootutil_img_validate(&hdr, fap, tmpbuf, sizeof(tmpbuf),
                                NULL, 0, NULL)) {
            flash_area_close(fap);
            continue;
        }
        flash_area_close(fap);

        cbor_encoder_create_map(&images, &image, CborIndefiniteLength);
        cbor_encode_text_stringz(&image, "slot");
        cbor_encode_int(&image, i);
        cbor_encode_text_stringz(&image, "version");

        bs_list_img_ver((char *)tmpbuf, sizeof(tmpbuf), &hdr.ih_ver);
        cbor_encode_text_stringz(&image, (char *)tmpbuf);
        cbor_encoder_close_container(&images, &image);
    }
    cbor_encoder_close_container(&bs_rsp, &images);
    cbor_encoder_close_container(&bs_root, &bs_rsp);
    boot_serial_output();
}

/*
 * Image upload request.
 */
static void
bs_upload(char *buf, int len)
{
    CborParser parser;
    struct cbor_buf_reader reader;
    struct CborValue root_value;
    struct CborValue value;
    uint8_t img_data[512];
    long long int off = UINT_MAX;
    size_t img_blen = 0;
    long long int data_len = UINT_MAX;
    size_t slen;
    char name_str[8];
    /*
    const struct cbor_attr_t attr[4] = {
        [0] = {
            .attribute = "data",
            .type = CborAttrByteStringType,
            .addr.bytestring.data = img_data,
            .addr.bytestring.len = &img_blen,
            .len = sizeof(img_data)
        },
        [1] = {
            .attribute = "off",
            .type = CborAttrUnsignedIntegerType,
            .addr.uinteger = &off,
            .nodefault = true
        },
        [2] = {
            .attribute = "len",
            .type = CborAttrUnsignedIntegerType,
            .addr.uinteger = &data_len,
            .nodefault = true
        }
    };
     */
    const struct flash_area *fap = NULL;
    int rc;

    memset(img_data, 0, sizeof(img_data));

    cbor_buf_reader_init(&reader, (uint8_t *)buf, len);
    cbor_parser_init(&reader.r, 0, &parser, &root_value);

    /*
     * Expected data format.
     * {
     *    "data":<img_data>
     *    "len":<image len>
     *    "off":<current offset of image data>
     * }
     */

    /*
     * Object comes within { ... }
     */
    if (!cbor_value_is_container(&root_value)) {
        goto out_invalid_data;
    }
    if (cbor_value_enter_container(&root_value, &value)) {
        goto out_invalid_data;
    }
    while (cbor_value_is_valid(&value)) {
        /*
         * Decode key.
         */
        if (cbor_value_calculate_string_length(&value, &slen)) {
            goto out_invalid_data;
        }
        if (!cbor_value_is_text_string(&value) ||
            slen >= sizeof(name_str) - 1) {
            goto out_invalid_data;
        }
        if (cbor_value_copy_text_string(&value, name_str, &slen, &value)) {
            goto out_invalid_data;
        }
        name_str[slen] = '\0';
        if (!strcmp(name_str, "data")) {
            /*
             * Image data
             */
            if (value.type != CborByteStringType) {
                goto out_invalid_data;
            }
            if (cbor_value_calculate_string_length(&value, &slen) ||
                slen >= sizeof(img_data)) {
                goto out_invalid_data;
            }
            if (cbor_value_copy_byte_string(&value, img_data, &slen, &value)) {
                goto out_invalid_data;
            }
            img_blen = slen;
        } else if (!strcmp(name_str, "off")) {
            /*
             * Offset of the data.
             */
            if (value.type != CborIntegerType) {
                goto out_invalid_data;
            }
            if (cbor_value_get_int64(&value, &off)) {
                goto out_invalid_data;
            }
            if (cbor_value_advance(&value)) {
                goto out_invalid_data;
            }
        } else if (!strcmp(name_str, "len")) {
            /*
             * Length of the image. This should only be present in the first
             * block of data; when offset is 0.
             */
            if (value.type != CborIntegerType) {
                goto out_invalid_data;
            }
            if (cbor_value_get_int64(&value, &data_len)) {
                goto out_invalid_data;
            }
            if (cbor_value_advance(&value)) {
                goto out_invalid_data;
            }
        } else {
            /*
             * Skip unknown keys.
             */
            if (cbor_value_advance(&value)) {
                goto out_invalid_data;
            }
        }
    }
    if (off == UINT_MAX) {
        /*
         * Offset must be set in every block.
         */
        goto out_invalid_data;
    }

    rc = flash_area_open(flash_area_id_from_image_slot(0), &fap);
    if (rc) {
        rc = MGMT_ERR_EINVAL;
        goto out;
    }

    if (off == 0) {
        curr_off = 0;
        if (data_len > fap->fa_size) {
            goto out_invalid_data;
        }
        rc = flash_area_erase(fap, 0, fap->fa_size);
        if (rc) {
            rc = MGMT_ERR_EINVAL;
            goto out;
        }
        img_size = data_len;
    }
    if (off != curr_off) {
        rc = 0;
        goto out;
    }
    rc = flash_area_write(fap, curr_off, img_data, img_blen);
    if (rc == 0) {
        curr_off += img_blen;
    } else {
out_invalid_data:
        rc = MGMT_ERR_EINVAL;
    }

out:
    cbor_encoder_create_map(&bs_root, &bs_rsp, CborIndefiniteLength);
    cbor_encode_text_stringz(&bs_rsp, "rc");
    cbor_encode_int(&bs_rsp, rc);
    if (rc == 0) {
        cbor_encode_text_stringz(&bs_rsp, "off");
        cbor_encode_uint(&bs_rsp, curr_off);
    }
    cbor_encoder_close_container(&bs_root, &bs_rsp);

    boot_serial_output();
    flash_area_close(fap);
}

/*
 * Console echo control. Send empty response, don't do anything.
 */
static void
bs_echo_ctl(char *buf, int len)
{
    boot_serial_output();
}

/*
 * Reset, and (presumably) boot to newly uploaded image. Flush console
 * before restarting.
 */
static int
bs_reset(char *buf, int len)
{
    cbor_encoder_create_map(&bs_root, &bs_rsp, CborIndefiniteLength);
    cbor_encode_text_stringz(&bs_rsp, "rc");
    cbor_encode_int(&bs_rsp, 0);
    cbor_encoder_close_container(&bs_root, &bs_rsp);

    boot_serial_output();
    os_cputime_delay_usecs(250000);
    hal_system_reset();
}

/*
 * Parse incoming line of input from console.
 * Expect newtmgr protocol with serial transport.
 */
void
boot_serial_input(char *buf, int len)
{
    struct nmgr_hdr *hdr;

    hdr = (struct nmgr_hdr *)buf;
    if (len < sizeof(*hdr) ||
      (hdr->nh_op != NMGR_OP_READ && hdr->nh_op != NMGR_OP_WRITE) ||
      (ntohs(hdr->nh_len) < len - sizeof(*hdr))) {
        return;
    }
    bs_hdr = hdr;
    hdr->nh_group = ntohs(hdr->nh_group);

    buf += sizeof(*hdr);
    len -= sizeof(*hdr);

    bs_writer.bytes_written = 0;
    cbor_encoder_init(&bs_root, &bs_writer, 0);

    /*
     * Limited support for commands.
     */
    if (hdr->nh_group == MGMT_GROUP_ID_IMAGE) {
        switch (hdr->nh_id) {
        case IMGMGR_NMGR_ID_STATE:
            bs_list(buf, len);
            break;
        case IMGMGR_NMGR_ID_UPLOAD:
            bs_upload(buf, len);
            break;
        default:
            break;
        }
    } else if (hdr->nh_group == MGMT_GROUP_ID_DEFAULT) {
        switch (hdr->nh_id) {
        case NMGR_ID_CONS_ECHO_CTRL:
            bs_echo_ctl(buf, len);
            break;
        case NMGR_ID_RESET:
            bs_reset(buf, len);
            break;
        default:
            break;
        }
    }
}

static void
boot_serial_output(void)
{
    char *data;
    int len;
    uint16_t crc;
    uint16_t totlen;
    char pkt_start[2] = { SHELL_NLIP_PKT_START1, SHELL_NLIP_PKT_START2 };
    char buf[BOOT_SERIAL_OUT_MAX];
    char encoded_buf[BASE64_ENCODE_SIZE(BOOT_SERIAL_OUT_MAX)];

    data = bs_obuf;
    len = bs_writer.bytes_written;

    bs_hdr->nh_op++;
    bs_hdr->nh_flags = 0;
    bs_hdr->nh_len = htons(len);
    bs_hdr->nh_group = htons(bs_hdr->nh_group);

    crc = crc16_ccitt(CRC16_INITIAL_CRC, bs_hdr, sizeof(*bs_hdr));
    crc = crc16_ccitt(crc, data, len);
    crc = htons(crc);

    console_write(pkt_start, sizeof(pkt_start));

    totlen = len + sizeof(*bs_hdr) + sizeof(crc);
    totlen = htons(totlen);

    memcpy(buf, &totlen, sizeof(totlen));
    totlen = sizeof(totlen);
    memcpy(&buf[totlen], bs_hdr, sizeof(*bs_hdr));
    totlen += sizeof(*bs_hdr);
    memcpy(&buf[totlen], data, len);
    totlen += len;
    memcpy(&buf[totlen], &crc, sizeof(crc));
    totlen += sizeof(crc);
    totlen = base64_encode(buf, totlen, encoded_buf, 1);
    console_write(encoded_buf, totlen);
    console_write("\n", 1);
}

/*
 * Returns 1 if full packet has been received.
 */
static int
boot_serial_in_dec(char *in, int inlen, char *out, int *out_off, int maxout)
{
    int rc;
    uint16_t crc;
    uint16_t len;

    if (*out_off + base64_decode_len(in) >= maxout) {
        return -1;
    }
    rc = base64_decode(in, &out[*out_off]);
    if (rc < 0) {
        return -1;
    }
    *out_off += rc;

    if (*out_off > sizeof(uint16_t)) {
        len = ntohs(*(uint16_t *)out);

        len = min(len, *out_off - sizeof(uint16_t));
        out += sizeof(uint16_t);
        crc = crc16_ccitt(CRC16_INITIAL_CRC, out, len);
        if (crc || len <= sizeof(crc)) {
            return 0;
        }
        *out_off -= sizeof(crc);
        out[*out_off] = '\0';

        return 1;
    }
    return 0;
}

/*
 * Task which waits reading console, expecting to get image over
 * serial port.
 */
void
boot_serial_start(int max_input)
{
    int rc;
    int off;
    char *buf;
    char *dec;
    int dec_off;
    int full_line;
#ifdef BOOT_SERIAL_REPORT_PIN
    uint32_t tick;
#endif

#if 0
    /*
     * This is commented out, as it includes divide operation, bloating
     * the bootloader 10%.
     * Note that there are calls to hal_watchdog_tickle() in the subsequent
     * code.
     */
    rc = hal_watchdog_init(MYNEWT_VAL(WATCHDOG_INTERVAL));
    assert(rc == 0);
#endif
#ifdef BOOT_SERIAL_REPORT_PIN
    /*
     * Configure GPIO line as output. This is a pin we toggle at the
     * given frequency.
     */
    hal_gpio_init_out(BOOT_SERIAL_REPORT_PIN, 0);
    tick = os_cputime_get32();
#endif

    rc = console_init(NULL);
    assert(rc == 0);
    console_echo(0);

    buf = os_malloc(max_input);
    dec = os_malloc(max_input);
    assert(buf && dec);

    off = 0;
    while (1) {
        hal_watchdog_tickle();
#ifdef BOOT_SERIAL_REPORT_PIN
        if (os_cputime_get32() - tick > BOOT_SERIAL_REPORT_FREQ) {
            hal_gpio_toggle(BOOT_SERIAL_REPORT_PIN);
            tick = os_cputime_get32();
        }
#endif
        rc = console_read(buf + off, max_input - off, &full_line);
        if (rc <= 0 && !full_line) {
            continue;
        }
        off += rc;
        if (!full_line) {
            if (off == max_input) {
                /*
                 * Full line, no newline yet. Reset the input buffer.
                 */
                off = 0;
            }
            continue;
        }
        if (buf[0] == SHELL_NLIP_PKT_START1 &&
          buf[1] == SHELL_NLIP_PKT_START2) {
            dec_off = 0;
            rc = boot_serial_in_dec(&buf[2], off - 2, dec, &dec_off, max_input);
        } else if (buf[0] == SHELL_NLIP_DATA_START1 &&
          buf[1] == SHELL_NLIP_DATA_START2) {
            rc = boot_serial_in_dec(&buf[2], off - 2, dec, &dec_off, max_input);
        }
        if (rc == 1) {
            boot_serial_input(&dec[2], dec_off - 2);
        }
        off = 0;
    }
}

/*
 * os_init() will not be called with bootloader, so we need to initialize
 * devices created by hal_bsp_init() here.
 */
void
boot_serial_os_dev_init(void)
{
    os_dev_initialize_all(OS_DEV_INIT_PRIMARY);
    os_dev_initialize_all(OS_DEV_INIT_SECONDARY);

    /*
     * Configure GPIO line as input. This is read later to see if
     * we should stay and keep waiting for input.
     */
    hal_gpio_init_in(BOOT_SERIAL_DETECT_PIN, BOOT_SERIAL_DETECT_PIN_CFG);
}

void
boot_serial_pkg_init(void)
{
    /*
     * Configure a GPIO as input, and compare it against expected value.
     * If it matches, await for download commands from serial.
     */
    if (hal_gpio_read(BOOT_SERIAL_DETECT_PIN) == BOOT_SERIAL_DETECT_PIN_VAL) {
        boot_serial_start(BOOT_SERIAL_INPUT_MAX);
        assert(0);
    }
}
