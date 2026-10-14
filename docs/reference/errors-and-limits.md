---
eyebrow: 'ext-sys · Reference'
lede:    'The ValueErrors of the two sleep functions, and the fixed sizes and timings behind ext-sys on ESP32.'
see_also:
  - { href: '../usage/restart-and-sleep.md', meta: 'PHP API' }
  - { href: '../usage/timing.md', meta: 'PHP API' }
prev: { label: 'Power-save, executor and runtimes', href: './power-and-runtimes.md' }
next: { label: 'Print a health report', href: '../recipes/health-report.md' }
---

# Errors and limits

ext-sys defines no exception classes. Only `sys_deep_sleep()` and `sys_light_sleep()` validate their arguments,
and they throw `\ValueError` before anything stops or sleeps. Every other function accepts any `int` or takes no
argument, so only PHP's own `\TypeError` and `\ArgumentCountError` apply.

## ValueError messages

PHP prefixes each message with the function and argument, for example
`sys_deep_sleep(): Argument #2 ($wakePin) must be an RTC/LP GPIO (GPIO 40 cannot wake the chip from deep sleep)`.

| Message (after the argument) | Function | Cause |
| --- | --- | --- |
| `must be greater than or equal to 0` | both | `$ms` is negative. |
| `must be greater than 0 without a wake pin` | both | `$ms` is 0 and `$wakePin` is `null`. |
| `must be a GPIO number` | both | `$wakePin` is negative or above 65535. |
| `must be a GPIO of this chip (GPIO n does not exist)` | both | The chip has no such GPIO. |
| `must be an RTC/LP GPIO (GPIO n cannot wake the chip from deep sleep)` | `sys_deep_sleep()` | The GPIO exists but cannot wake the chip from deep sleep. |
| `must be null: this chip cannot wake from deep sleep on a pin` | `sys_deep_sleep()` | A pin was given on a chip without pin wake from deep sleep. |

## Limits

| Limit | Value | Source |
| --- | --- | --- |
| PHP integer size | 32 bits | the firmware's PHP build |
| `sys_micros()` turns negative | after 2^31 µs, about 35.8 minutes | 32-bit truncation |
| `sys_uptime_ms()` turns negative | after 2^31 ms, about 24.8 days | 32-bit truncation |
| Longest sleep `$ms` | 2147483647 ms, about 24.8 days | 32-bit `int` |
| `sys_delay()` resolution | 1 ms (scheduler tick 1000 Hz) | SDK `sdkconfig` |
| Delay between the stop and deep sleep | 100 ms | SDK host |
| Wait for the executor queue (`sys_light_sleep()`) | 1000 ms | esp32-sys-hal |
| Deep sleep wake pins | RTC GPIOs on ESP32-S3, LP GPIOs on ESP32-P4 | ESP-IDF |
| Light sleep wake pins | any valid GPIO, level-triggered | esp32-sys-hal |

If the executor queue stays full for the whole 1000 ms, or the chip refuses to sleep, `sys_light_sleep()`
returns `"other"` without having slept.
