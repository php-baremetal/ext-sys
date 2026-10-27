/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-sys                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@php-baremetal.com>         |
   +----------------------------------------------------------------------+
*/

/* sys_*: timing, restart, deep sleep and reset reason, chip identity, memory. delay() aliases sys_delay(). */
#include "php.h"
#include "php_sys.h"
#include "bm_sys_hal.h"

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sys_delay, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, ms, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sys_int, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sys_string, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sys_bool, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sys_never, 0, 0, IS_NEVER, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sys_deep_sleep, 0, 1, IS_NEVER, 0)
    ZEND_ARG_TYPE_INFO(0, ms, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, wakePin, IS_LONG, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, wakeLow, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sys_light_sleep, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, ms, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, wakePin, IS_LONG, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, wakeLow, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

/* --- timing --- */

static void sys_delay_impl(zend_long ms)
{
    if (ms < 0) {
        ms = 0;
    }
    bm_sys_hal_delay_ms(ms);
}

/* sys_delay(int $ms): void */
PHP_FUNCTION(sys_delay)
{
    zend_long ms;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(ms)
    ZEND_PARSE_PARAMETERS_END();
    sys_delay_impl(ms);
}

/* sys_uptime_ms(): int */
PHP_FUNCTION(sys_uptime_ms)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG((zend_long) (bm_sys_hal_now_us() / 1000));
}

/* sys_micros(): int */
PHP_FUNCTION(sys_micros)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG((zend_long) bm_sys_hal_now_us());
}

/* --- control --- */

/* sys_restart(): never -- stops the program like exit(), then the firmware restarts */
PHP_FUNCTION(sys_restart)
{
    ZEND_PARSE_PARAMETERS_NONE();
    bm_sys_hal_restart();
}

/* sys_deep_sleep(int $ms, ?int $wakePin = null, bool $wakeLow = true): never */
PHP_FUNCTION(sys_deep_sleep)
{
    zend_long ms;
    zend_long pin = -1;
    bool pin_null = true;
    bool wake_low = true;
    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_LONG(ms)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG_OR_NULL(pin, pin_null)
        Z_PARAM_BOOL(wake_low)
    ZEND_PARSE_PARAMETERS_END();

    if (ms < 0) {
        zend_argument_value_error(1, "must be greater than or equal to 0");
        RETURN_THROWS();
    }
    if (pin_null) {
        if (ms == 0) {
            zend_argument_value_error(1, "must be greater than 0 without a wake pin");
            RETURN_THROWS();
        }
        pin = -1;
    } else {
        const char *error = pin < 0 || pin > 0xFFFF ? "must be a GPIO number" : bm_sys_hal_wake_pin_error((int) pin);
        if (error != NULL) {
            zend_argument_value_error(2, "%s", error);
            RETURN_THROWS();
        }
    }
    bm_sys_hal_deep_sleep((int64_t) ms, (int) pin, wake_low);
}

/* sys_light_sleep(int $ms, ?int $wakePin = null, bool $wakeLow = true): string */
PHP_FUNCTION(sys_light_sleep)
{
    zend_long ms;
    zend_long pin = -1;
    bool pin_null = true;
    bool wake_low = true;
    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_LONG(ms)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG_OR_NULL(pin, pin_null)
        Z_PARAM_BOOL(wake_low)
    ZEND_PARSE_PARAMETERS_END();

    if (ms < 0) {
        zend_argument_value_error(1, "must be greater than or equal to 0");
        RETURN_THROWS();
    }
    if (pin_null) {
        if (ms == 0) {
            zend_argument_value_error(1, "must be greater than 0 without a wake pin");
            RETURN_THROWS();
        }
        pin = -1;
    } else {
        const char *error = pin < 0 || pin > 0xFFFF ? "must be a GPIO number" : bm_sys_hal_light_wake_pin_error((int) pin);
        if (error != NULL) {
            zend_argument_value_error(2, "%s", error);
            RETURN_THROWS();
        }
    }
    RETURN_STRING(bm_sys_hal_light_sleep((int64_t) ms, (int) pin, wake_low));
}

/* sys_wake_cause(): string -- "none", "timer", "gpio" or "other" */
PHP_FUNCTION(sys_wake_cause)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_STRING(bm_sys_hal_wake_cause());
}

/* sys_reset_reason(): string */
PHP_FUNCTION(sys_reset_reason)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_STRING(bm_sys_hal_reset_reason());
}

/* --- identity --- */

/* sys_chip_model(): string */
PHP_FUNCTION(sys_chip_model)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_STRING(bm_sys_hal_chip_model());
}

/* sys_cpu_freq_mhz(): int */
PHP_FUNCTION(sys_cpu_freq_mhz)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG(bm_sys_hal_cpu_freq_mhz());
}

/* sys_mac(): string -- aa:bb:cc:dd:ee:ff */
PHP_FUNCTION(sys_mac)
{
    ZEND_PARSE_PARAMETERS_NONE();
    uint8_t mac[6] = {0};
    bm_sys_hal_mac(mac);
    char buf[18];
    snprintf(buf, sizeof buf, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    RETURN_STRING(buf);
}

/* sys_idf_version(): string */
PHP_FUNCTION(sys_idf_version)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_STRING(bm_sys_hal_sdk_version());
}

/* --- memory --- */

/* sys_psram_free(): int */
PHP_FUNCTION(sys_psram_free)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG((zend_long) bm_sys_hal_mem_free(BM_SYS_MEM_PSRAM));
}

/* sys_psram_size(): int */
PHP_FUNCTION(sys_psram_size)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG((zend_long) bm_sys_hal_mem_total(BM_SYS_MEM_PSRAM));
}

/* sys_psram_largest_free(): int */
PHP_FUNCTION(sys_psram_largest_free)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG((zend_long) bm_sys_hal_mem_largest(BM_SYS_MEM_PSRAM));
}

/* sys_heap_free(): int */
PHP_FUNCTION(sys_heap_free)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG((zend_long) bm_sys_hal_mem_free(BM_SYS_MEM_INTERNAL));
}

/* sys_heap_size(): int */
PHP_FUNCTION(sys_heap_size)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG((zend_long) bm_sys_hal_mem_total(BM_SYS_MEM_INTERNAL));
}

/* sys_available(): bool */
PHP_FUNCTION(sys_available)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_TRUE;
}

/* Deprecated aliases (E_DEPRECATED on use). */
PHP_FUNCTION(psram_free)         { ZEND_PARSE_PARAMETERS_NONE(); RETURN_LONG((zend_long) bm_sys_hal_mem_free(BM_SYS_MEM_PSRAM)); }
PHP_FUNCTION(psram_size)         { ZEND_PARSE_PARAMETERS_NONE(); RETURN_LONG((zend_long) bm_sys_hal_mem_total(BM_SYS_MEM_PSRAM)); }
PHP_FUNCTION(psram_largest_free) { ZEND_PARSE_PARAMETERS_NONE(); RETURN_LONG((zend_long) bm_sys_hal_mem_largest(BM_SYS_MEM_PSRAM)); }
PHP_FUNCTION(heap_free)          { ZEND_PARSE_PARAMETERS_NONE(); RETURN_LONG((zend_long) bm_sys_hal_mem_free(BM_SYS_MEM_INTERNAL)); }
PHP_FUNCTION(heap_size)          { ZEND_PARSE_PARAMETERS_NONE(); RETURN_LONG((zend_long) bm_sys_hal_mem_total(BM_SYS_MEM_INTERNAL)); }

static const zend_function_entry sys_functions[] = {
    PHP_FE(sys_delay,             arginfo_sys_delay)
    PHP_FE(sys_uptime_ms,         arginfo_sys_int)
    PHP_FE(sys_micros,            arginfo_sys_int)
    PHP_FE(sys_restart,           arginfo_sys_never)
    PHP_FE(sys_deep_sleep,        arginfo_sys_deep_sleep)
    PHP_FE(sys_light_sleep,       arginfo_sys_light_sleep)
    PHP_FE(sys_wake_cause,        arginfo_sys_string)
    PHP_FE(sys_reset_reason,      arginfo_sys_string)
    PHP_FE(sys_chip_model,        arginfo_sys_string)
    PHP_FE(sys_cpu_freq_mhz,      arginfo_sys_int)
    PHP_FE(sys_mac,               arginfo_sys_string)
    PHP_FE(sys_idf_version,       arginfo_sys_string)
    PHP_FE(sys_psram_free,        arginfo_sys_int)
    PHP_FE(sys_psram_size,        arginfo_sys_int)
    PHP_FE(sys_psram_largest_free, arginfo_sys_int)
    PHP_FE(sys_heap_free,         arginfo_sys_int)
    PHP_FE(sys_heap_size,         arginfo_sys_int)
    PHP_FE(sys_available,         arginfo_sys_bool)
    PHP_FALIAS(delay, sys_delay,  arginfo_sys_delay)
    /* deprecated */
    PHP_DEP_FE(psram_free,         arginfo_sys_int)
    PHP_DEP_FE(psram_size,         arginfo_sys_int)
    PHP_DEP_FE(psram_largest_free, arginfo_sys_int)
    PHP_DEP_FE(heap_free,          arginfo_sys_int)
    PHP_DEP_FE(heap_size,          arginfo_sys_int)
    PHP_FE_END
};

zend_module_entry sys_module_entry = {
    STANDARD_MODULE_HEADER,
    "sys",
    sys_functions,
    NULL,   /* MINIT */
    NULL,   /* MSHUTDOWN */
    NULL,   /* RINIT */
    NULL,   /* RSHUTDOWN */
    NULL,   /* MINFO */
    "1.0",
    STANDARD_MODULE_PROPERTIES
};
