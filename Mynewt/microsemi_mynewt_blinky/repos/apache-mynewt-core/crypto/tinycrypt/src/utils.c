/* utils.c - TinyCrypt platform-dependent run-time operations */

/*
 *  Copyright (C) 2015 by Intel Corporation, All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *    - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *    - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    - Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <tinycrypt/utils.h>
#include <tinycrypt/constants.h>

#include <string.h>

#define MASK_MOST_SIG_BIT 0x80
#define MASK_TWENTY_SEVEN 0x1b

uint32_t _copy(uint8_t *to, uint32_t to_len,
	       const uint8_t *from, uint32_t from_len)
{
	if (from_len <= to_len) {
		(void)memcpy(to, from, from_len);
		return from_len;
	} else {
		return TC_CRYPTO_FAIL;
	}
}

void _set(void *to, uint8_t val, uint32_t len)
{
	(void)memset(to, val, len);
}

/*
 * Doubles the value of a byte for values up to 127. Original 'return
 * ((a<<1) ^ ((a>>7) * 0x1b))' re-written to avoid extra multiplication which
 * the compiler won't be able to optimize
 */
uint8_t _double_byte(uint8_t a)
{
	return (a & MASK_MOST_SIG_BIT) ?
		((a << 1) ^ MASK_TWENTY_SEVEN) : (a << 1);
}

int32_t _compare(const uint8_t *a, const uint8_t *b, size_t size)
{
	const uint8_t *tempa = a;
	const uint8_t *tempb = b;
	uint8_t result = 0;

	for (uint32_t i = 0; i < size; i++) {
		result |= tempa[i] ^ tempb[i];
	}
	return result;
}
