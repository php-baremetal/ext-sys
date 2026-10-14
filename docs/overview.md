---
eyebrow: 'Package · Extension'
lede:    'Timing, restart, deep and light sleep, wake cause, reset reason, chip identity and memory figures, as global sys_* functions. Neutral extension; conductor adds the sys HAL for your platform.'
see_also:
  - { href: './usage/restart-and-sleep.md', meta: 'PHP API' }
  - { href: './reference/power-and-runtimes.md', meta: 'Reference' }
  - { href: 'package:php-baremetal/esp32-sys-hal', meta: 'HAL', label: 'esp32-sys-hal' }
  - { href: 'package:php-baremetal/esp32-executor', meta: 'Library', label: 'esp32-executor' }
prev: { label: 'No previous page', href: '#' }
next: { label: 'Timing', href: './usage/timing.md' }
---

# System functions

ext-sys gives the script on your board what it needs from the system around it: wait without busy-looping,
measure time since boot, restart the board or put it into deep or light sleep, find out why it booted or woke,
and read the chip's identity and free memory. Everything is a global function; there are no classes and no
constants.

## Requirements

| Requires | Why |
| --- | --- |
| `php-baremetal/sys-hal` (virtual) | The platform side. On ESP32 it is `php-baremetal/esp32-sys-hal` (platform `esp32`). |

You never require the HAL yourself. Conductor reads the platform from `extra.baremetal.sdk` (`esp32@^1.0` gives
`esp32`) and adds `php-baremetal/esp32-sys-hal` while Composer resolves. That HAL brings
[esp32-executor](package:php-baremetal/esp32-executor), which runs the light sleep, and links against the SDK
host component `phpbm`, which performs the restart and the deep sleep after the program stops. The extension's
PHP module is `sys`.

## Install

<!-- @steps -->
- **Install conductor once per machine**

  `composer global require php-baremetal/conductor`

- **Create the project**

  `phpbm init my-node` writes `composer.json` with conductor, the package registry, and the SDK, board, runtime
  and storage you pick.

- **Require the extension**

  `composer require php-baremetal/ext-sys`

- **Build and flash**

  `phpbm flash`, then `phpbm monitor` for the console.
<!-- @endsteps -->

The resulting `composer.json`, for a Waveshare ESP32-S3-Zero:

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/my-node",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/ext-sys": "^1.0"
  },
  "config": {
    "allow-plugins": { "php-baremetal/conductor": true },
    "platform": { "php": "8.5.11" }
  },
  "extra": {
    "baremetal": {
      "sdk": "esp32@^1.0",
      "board": "waveshare-esp32-s3-zero",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

## A first script

`init-loop`: the script runs once, then `loop()` repeatedly. This one prints a status line every second and
restarts the board when internal RAM runs low.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
printf("%s, reset %s, wake %s\n", sys_chip_model(), sys_reset_reason(), sys_wake_cause());

function loop(int $tick): void
{
    printf("up %d s, internal RAM free %d bytes\n", intdiv(sys_uptime_ms(), 1000), sys_heap_free());

    if (sys_heap_free() < 20000) {
        echo "low memory, restarting\n";
        sys_restart();                  // never returns
    }
    delay(1000);
}
```
<!-- @endcode-block -->

## PHP API at a glance

| Function | Returns | Meaning |
| --- | --- | --- |
| `sys_delay(int $ms)`, `delay(int $ms)` | `void` | Suspend the PHP task for `$ms` ms. |
| `sys_uptime_ms()`, `sys_micros()` | `int` | Time since boot in ms or µs (32-bit, wraps). |
| `sys_restart()` | `never` | End the program like `exit()`, then restart at once. |
| `sys_deep_sleep(int $ms, ?int $wakePin = null, bool $wakeLow = true)` | `never` | End the program, then deep sleep until the timer or the pin; waking boots again. |
| `sys_light_sleep(int $ms, ?int $wakePin = null, bool $wakeLow = true)` | `string` | Sleep in place; returns `"timer"`, `"gpio"` or `"other"`. |
| `sys_wake_cause()` | `string` | `"none"`, `"timer"`, `"gpio"` or `"other"`: what ended the last deep sleep. |
| `sys_reset_reason()` | `string` | Why the chip booted: `"poweron"`, `"software"`, `"deepsleep"`, `"panic"`, … |
| `sys_chip_model()`, `sys_cpu_freq_mhz()`, `sys_mac()`, `sys_idf_version()` | `string`/`int` | Chip identity. |
| `sys_heap_free()`, `sys_heap_size()` | `int` | Internal RAM, bytes. |
| `sys_psram_free()`, `sys_psram_size()`, `sys_psram_largest_free()` | `int` | PSRAM, bytes; 0 without PSRAM. |
| `sys_available()` | `bool` | Always `true`. |

Five deprecated aliases (`heap_free()`, `heap_size()`, `psram_free()`, `psram_size()`, `psram_largest_free()`)
still work and raise `E_DEPRECATED`. Pages: [Timing](./usage/timing.md),
[Restart and sleep](./usage/restart-and-sleep.md), [Chip and memory](./usage/chip-and-memory.md).

## Dependencies

ext-sys requires only the virtual `php-baremetal/sys-hal`, provided on ESP32 by
[esp32-sys-hal](package:php-baremetal/esp32-sys-hal). It declares no registry and registers nothing.

## Notes

- `sys_restart()` and `sys_deep_sleep()` stop the program the way `exit()` does: shutdown functions and
  destructors run, `finally` blocks do not, and storage is unmounted before the board restarts or sleeps. In the
  `web-server` runtime they stop the whole server, not just the request.
- `sys_light_sleep()` runs on the executor task and blocks it for the whole sleep. The `power-save` option is a
  separate, automatic light sleep. See [Power-save, executor and runtimes](./reference/power-and-runtimes.md).
- PHP integers are 32-bit on the board, so `sys_micros()` wraps after about 35.8 minutes and `sys_uptime_ms()`
  after about 24.8 days. See [Measure elapsed time](./recipes/measure-elapsed-time.md).
- Invalid sleep arguments throw `\ValueError`; nothing else in ext-sys throws. See
  [Errors and limits](./reference/errors-and-limits.md).
