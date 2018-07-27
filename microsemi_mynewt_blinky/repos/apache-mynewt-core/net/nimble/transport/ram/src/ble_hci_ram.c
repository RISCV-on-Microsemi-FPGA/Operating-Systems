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
#include <errno.h>
#include <stddef.h>
#include "syscfg/syscfg.h"
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "mem/mem.h"
#include "nimble/ble.h"
#include "nimble/ble_hci_trans.h"
#include "transport/ram/ble_hci_ram.h"

static ble_hci_trans_rx_cmd_fn *ble_hci_ram_rx_cmd_hs_cb;
static void *ble_hci_ram_rx_cmd_hs_arg;

static ble_hci_trans_rx_cmd_fn *ble_hci_ram_rx_cmd_ll_cb;
static void *ble_hci_ram_rx_cmd_ll_arg;

static ble_hci_trans_rx_acl_fn *ble_hci_ram_rx_acl_hs_cb;
static void *ble_hci_ram_rx_acl_hs_arg;

static ble_hci_trans_rx_acl_fn *ble_hci_ram_rx_acl_ll_cb;
static void *ble_hci_ram_rx_acl_ll_arg;

static struct os_mempool ble_hci_ram_evt_hi_pool;
static void *ble_hci_ram_evt_hi_buf;
static struct os_mempool ble_hci_ram_evt_lo_pool;
static void *ble_hci_ram_evt_lo_buf;

static uint8_t *ble_hci_ram_hs_cmd_buf;
static uint8_t ble_hci_ram_hs_cmd_buf_alloced;

void
ble_hci_trans_cfg_hs(ble_hci_trans_rx_cmd_fn *cmd_cb,
                     void *cmd_arg,
                     ble_hci_trans_rx_acl_fn *acl_cb,
                     void *acl_arg)
{
    ble_hci_ram_rx_cmd_hs_cb = cmd_cb;
    ble_hci_ram_rx_cmd_hs_arg = cmd_arg;
    ble_hci_ram_rx_acl_hs_cb = acl_cb;
    ble_hci_ram_rx_acl_hs_arg = acl_arg;
}

void
ble_hci_trans_cfg_ll(ble_hci_trans_rx_cmd_fn *cmd_cb,
                     void *cmd_arg,
                     ble_hci_trans_rx_acl_fn *acl_cb,
                     void *acl_arg)
{
    ble_hci_ram_rx_cmd_ll_cb = cmd_cb;
    ble_hci_ram_rx_cmd_ll_arg = cmd_arg;
    ble_hci_ram_rx_acl_ll_cb = acl_cb;
    ble_hci_ram_rx_acl_ll_arg = acl_arg;
}

int
ble_hci_trans_hs_cmd_tx(uint8_t *cmd)
{
    int rc;

    assert(ble_hci_ram_rx_cmd_ll_cb != NULL);

    rc = ble_hci_ram_rx_cmd_ll_cb(cmd, ble_hci_ram_rx_cmd_ll_arg);
    return rc;
}

int
ble_hci_trans_ll_evt_tx(uint8_t *hci_ev)
{
    int rc;

    assert(ble_hci_ram_rx_cmd_hs_cb != NULL);

    rc = ble_hci_ram_rx_cmd_hs_cb(hci_ev, ble_hci_ram_rx_cmd_hs_arg);
    return rc;
}

int
ble_hci_trans_hs_acl_tx(struct os_mbuf *om)
{
    int rc;

    assert(ble_hci_ram_rx_acl_ll_cb != NULL);

    rc = ble_hci_ram_rx_acl_ll_cb(om, ble_hci_ram_rx_acl_ll_arg);
    return rc;
}

int
ble_hci_trans_ll_acl_tx(struct os_mbuf *om)
{
    int rc;

    assert(ble_hci_ram_rx_acl_hs_cb != NULL);

    rc = ble_hci_ram_rx_acl_hs_cb(om, ble_hci_ram_rx_acl_hs_arg);
    return rc;
}

uint8_t *
ble_hci_trans_buf_alloc(int type)
{
    uint8_t *buf;

    switch (type) {
    case BLE_HCI_TRANS_BUF_EVT_HI:
        buf = os_memblock_get(&ble_hci_ram_evt_hi_pool);
        if (buf == NULL) {
            /* If no high-priority event buffers remain, try to grab a
             * low-priority one.
             */
            buf = ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_EVT_LO);
        }
        break;

    case BLE_HCI_TRANS_BUF_EVT_LO:
        buf = os_memblock_get(&ble_hci_ram_evt_lo_pool);
        break;

    case BLE_HCI_TRANS_BUF_CMD:
        assert(!ble_hci_ram_hs_cmd_buf_alloced);
        ble_hci_ram_hs_cmd_buf_alloced = 1;
        buf = ble_hci_ram_hs_cmd_buf;
        break;

    default:
        assert(0);
        buf = NULL;
    }

    return buf;
}

void
ble_hci_trans_buf_free(uint8_t *buf)
{
    int rc;

    if (buf == ble_hci_ram_hs_cmd_buf) {
        assert(ble_hci_ram_hs_cmd_buf_alloced);
        ble_hci_ram_hs_cmd_buf_alloced = 0;
    } else if (os_memblock_from(&ble_hci_ram_evt_hi_pool, buf)) {
        rc = os_memblock_put(&ble_hci_ram_evt_hi_pool, buf);
        assert(rc == 0);
    } else {
        assert(os_memblock_from(&ble_hci_ram_evt_lo_pool, buf));
        rc = os_memblock_put(&ble_hci_ram_evt_lo_pool, buf);
        assert(rc == 0);
    }
}

static void
ble_hci_ram_free_mem(void)
{
    free(ble_hci_ram_evt_hi_buf);
    ble_hci_ram_evt_hi_buf = NULL;

    free(ble_hci_ram_evt_lo_buf);
    ble_hci_ram_evt_lo_buf = NULL;

    free(ble_hci_ram_hs_cmd_buf);
    ble_hci_ram_hs_cmd_buf = NULL;
    ble_hci_ram_hs_cmd_buf_alloced = 0;
}

int
ble_hci_trans_reset(void)
{
    /* No work to do.  All allocated buffers are owned by the host or
     * controller, and they will get freed by their owners.
     */
    return 0;
}

/**
 * Initializes the RAM HCI transport module.
 *
 * @param cfg                   The settings to initialize the HCI RAM
 *                                  transport with.
 *
 * @return                      0 on success;
 *                              A BLE_ERR_[...] error code on failure.
 */
int
ble_hci_ram_init(void)
{
    int rc;

    ble_hci_ram_free_mem();

    rc = mem_malloc_mempool(&ble_hci_ram_evt_hi_pool,
                            MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT),
                            MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                            "ble_hci_ram_evt_hi_pool",
                            &ble_hci_ram_evt_hi_buf);
    if (rc != 0) {
        rc = ble_err_from_os(rc);
        goto err;
    }

    rc = mem_malloc_mempool(&ble_hci_ram_evt_lo_pool,
                            MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT),
                            MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                            "ble_hci_ram_evt_lo_pool",
                            &ble_hci_ram_evt_lo_buf);
    if (rc != 0) {
        rc = ble_err_from_os(rc);
        goto err;
    }

    ble_hci_ram_hs_cmd_buf = malloc(BLE_HCI_TRANS_CMD_SZ);
    if (ble_hci_ram_hs_cmd_buf == NULL) {
        rc = BLE_ERR_MEM_CAPACITY;
        goto err;
    }

    return 0;

err:
    ble_hci_ram_free_mem();
    return rc;
}

void
ble_hci_ram_pkg_init(void)
{
    int rc;

    /* Ensure this function only gets called by sysinit. */
    SYSINIT_ASSERT_ACTIVE();

    rc = ble_hci_ram_init();
    SYSINIT_PANIC_ASSERT(rc == 0);
}
