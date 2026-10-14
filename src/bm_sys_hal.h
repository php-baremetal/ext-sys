/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-sys                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@aurecchia.com>             |
   +----------------------------------------------------------------------+
*/

/* Platform primitives ext-sys needs; implemented by the platform's sys HAL. */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

void bm_sys_hal_delay_ms(long ms);
int64_t bm_sys_hal_now_us(void);
/* Both end the script the way exit() does; the firmware then restarts, or sleeps until the
 * timer (ms, 0 = none) or the pin (-1 = none) wakes it, which restarts it. */
void bm_sys_hal_restart(void);
void bm_sys_hal_deep_sleep(int64_t ms, int pin, bool wake_low);
/* NULL when pin can wake the chip from deep sleep, otherwise the reason. */
const char *bm_sys_hal_wake_pin_error(int pin);
/* Sleeps in place (ms, 0 = none; pin, -1 = none) and returns what woke it: "timer", "gpio" or "other". */
const char *bm_sys_hal_light_sleep(int64_t ms, int pin, bool wake_low);
/* NULL when pin can wake the chip from light sleep, otherwise the reason. */
const char *bm_sys_hal_light_wake_pin_error(int pin);
/* "none" (power-on or a reset), "timer", "gpio" or "other": what woke the chip from deep sleep. */
const char *bm_sys_hal_wake_cause(void);
const char *bm_sys_hal_reset_reason(void);
const char *bm_sys_hal_chip_model(void);
int bm_sys_hal_cpu_freq_mhz(void);
void bm_sys_hal_mac(uint8_t out[6]);
const char *bm_sys_hal_sdk_version(void);

/* Internal RAM and the optional external PSRAM. */
typedef enum { BM_SYS_MEM_INTERNAL, BM_SYS_MEM_PSRAM } bm_sys_mem_t;
size_t bm_sys_hal_mem_free(bm_sys_mem_t which);
size_t bm_sys_hal_mem_total(bm_sys_mem_t which);
size_t bm_sys_hal_mem_largest(bm_sys_mem_t which);
