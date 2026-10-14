---
eyebrow: 'ext-sys · PHP API'
lede:    'Chip model, configured CPU frequency, MAC address and ESP-IDF version; free, total and largest-free figures for internal RAM and PSRAM.'
see_also:
  - { href: '../recipes/health-report.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/esp32-sys-hal', meta: 'HAL', label: 'esp32-sys-hal' }
prev: { label: 'Restart and sleep', href: './restart-and-sleep.md' }
next: { label: 'Power-save, executor and runtimes', href: '../reference/power-and-runtimes.md' }
---

# Chip and memory

None of these functions takes an argument or throws.

## Identity

<!-- @method name="sys_chip_model(): string" returns="string" visibility="public" -->

The chip model: `"ESP32"`, `"ESP32-S2"`, `"ESP32-S3"`, `"ESP32-C3"`, `"ESP32-C2"`, `"ESP32-C6"`, `"ESP32-H2"`,
`"ESP32-P4"`, or `"unknown"`.

<!-- @method name="sys_cpu_freq_mhz(): int" returns="int" visibility="public" -->

The CPU frequency the firmware is configured for, in MHz (`CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ`, e.g. 240). It is
the configured value, not a measurement: with the `power-save` option on, the clock drops while idle and this
number does not change.

<!-- @method name="sys_mac(): string" returns="string" visibility="public" -->

The Wi-Fi station MAC address as 17 lowercase characters, `aa:bb:cc:dd:ee:ff`. It derives from the address
burned in at the factory, so it is unique per chip and stable across restarts: a usable device id, also on
boards without Wi-Fi in use.

<!-- @method name="sys_idf_version(): string" returns="string" visibility="public" -->

The ESP-IDF version the firmware was built with (`esp_get_idf_version()`).

## Memory

All values are bytes, read from the chip's heap allocator (`heap_caps_*`) for two pools: **internal** RAM and
external **PSRAM**. They describe the firmware's heap, not PHP's memory manager: for PHP's own usage,
`memory_get_usage()` is the function.

<!-- @method name="sys_heap_free(): int" returns="int" visibility="public" -->

Free internal RAM now.

<!-- @method name="sys_heap_size(): int" returns="int" visibility="public" -->

Size of the internal RAM heap. Smaller than the chip's SRAM figure: static data and code in RAM are outside the
heap.

<!-- @method name="sys_psram_free(): int" returns="int" visibility="public" -->

Free PSRAM now. 0 on a board without PSRAM.

<!-- @method name="sys_psram_size(): int" returns="int" visibility="public" -->

Size of the PSRAM heap. 0 without PSRAM.

<!-- @method name="sys_psram_largest_free(): int" returns="int" visibility="public" -->

The largest single block that can still be allocated from PSRAM. Much smaller than `sys_psram_free()` means
the pool is fragmented: a large allocation can fail although enough memory is free in total.

## Presence

<!-- @method name="sys_available(): bool" returns="bool" visibility="public" -->

Always `true`. Without ext-sys in the firmware the function does not exist; test with
`function_exists('sys_available')` or `extension_loaded('sys')`.

## Deprecated aliases

Each call raises `E_DEPRECATED`; the return value is that of the `sys_` function.

| Deprecated | Use |
| --- | --- |
| `heap_free(): int` | `sys_heap_free()` |
| `heap_size(): int` | `sys_heap_size()` |
| `psram_free(): int` | `sys_psram_free()` |
| `psram_size(): int` | `sys_psram_size()` |
| `psram_largest_free(): int` | `sys_psram_largest_free()` |
