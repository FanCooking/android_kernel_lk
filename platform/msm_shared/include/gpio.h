/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of The Linux Foundation, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#define TLMM_PULL_MASK  0x3
#define TLMM_HDRV_MASK  0x7

/* Current values for tlmm pins */
enum {
	TLMM_CUR_VAL_16MA = 0x7,
	TLMM_CUR_VAL_10MA = 0x4,
} tlmm_drive_config;

enum {
	TLMM_PULL_UP = 0x3,
	TLMM_NO_PULL = 0x0,
} tlmm_pull_values;

/* Bit offsets in the TLMM register */
enum {
	SDC1_DATA_HDRV_CTL_OFF = 0,
	SDC1_CMD_HDRV_CTL_OFF  = 3,
	SDC1_CLK_HDRV_CTL_OFF  = 6,
	SDC1_DATA_PULL_CTL_OFF = 9,
	SDC1_CMD_PULL_CTL_OFF  = 11,
	SDC1_CLK_PULL_CTL_OFF  = 13,
} tlmm_drv_ctrl;

/* Input for the tlmm config function */
struct tlmm_cfgs {
	uint32_t off;  /* Bit offeset in the register */
	uint8_t val;   /* Current value */
	uint8_t mask;  /* Mask for the clk/dat/cmd control */
};

/* APIs: exposed for other drivers */
/* API: Hdrive control for tlmm pins */
void tlmm_set_hdrive_ctrl(struct tlmm_cfgs *, uint8_t);
/* API:  Pull control for tlmm pins */
void tlmm_set_pull_ctrl(struct tlmm_cfgs *, uint8_t);
#endif
