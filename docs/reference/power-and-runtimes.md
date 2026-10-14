---
eyebrow: 'ext-sys · Reference'
lede:    'Where each call runs, how the power-save option relates to sys_light_sleep() and sys_deep_sleep(), and what a stop does in each runtime.'
see_also:
  - { href: '../usage/restart-and-sleep.md', meta: 'PHP API' }
  - { href: 'package:php-baremetal/esp32-executor', meta: 'Library', label: 'esp32-executor' }
  - { href: 'package:php-baremetal/ext-events', meta: 'Extension', label: 'ext-events' }
prev: { label: 'Chip and memory', href: '../usage/chip-and-memory.md' }
next: { label: 'Errors and limits', href: './errors-and-limits.md' }
---

# Power-save, executor and runtimes

## Where the work runs

| Call | Runs on |
| --- | --- |
| timing, identity, memory | the PHP task, directly |
| `sys_restart()`, `sys_deep_sleep()` | the PHP task records the action and ends the script; `app_main` restarts or sleeps after PHP has shut down and the storage is unmounted |
| `sys_light_sleep()` | the executor task (esp32-executor, core 0); the PHP task waits for it |

The PHP task never restarts or sleeps the chip itself. Its stack lives in PSRAM, and entering light sleep
suspends the cache that PSRAM goes through, so the light sleep is handed to the executor task, whose stack is in
internal RAM.

<!-- @callout variant="warning" title="sys_light_sleep() blocks the executor" -->
For the whole sleep the executor runs nothing else: background I2C samplers stop, and operations that go through
the executor (writes to the NVS store, a `Bus::CORE1` bus) wait until the chip wakes. Keep explicit light sleeps
short when the firmware relies on the executor.
<!-- @endcallout -->

## The power-save option

`power-save` (`off` or `on`, default `off`) is an SDK option set in the project's `extra.baremetal.options`. With
`on`, the firmware enables ESP-IDF power management with automatic light sleep: the CPU clock drops between the
crystal frequency and the default maximum, and the board sleeps by itself whenever every task is waiting
(between events, during `delay()`, between two `loop()` calls with a `loop-interval`). The console prints
`phpbm: power save on (light sleep, …)` at boot.

<!-- @code-block language="json" label="composer.json, extra.baremetal" -->
```json
{
  "sdk": "esp32@^1.0",
  "board": "waveshare-esp32-s3-zero",
  "runtime": "event-driven",
  "storage": "embedded",
  "options": { "power-save": "on" }
}
```
<!-- @endcode-block -->

How it relates to ext-sys:

| Aspect | power-save | `sys_light_sleep()` | `sys_deep_sleep()` |
| --- | --- | --- | --- |
| Who decides | the scheduler, whenever all tasks wait | your script, for `$ms` | your script |
| Program | keeps running | pauses, then continues | stops; waking boots again |
| Wakes on | any task's timer or interrupt | timer or one pin | timer or one RTC/LP pin |
| Needs the option | yes | no | no |

ext-sys works the same with the option on or off. `sys_cpu_freq_mhz()` keeps returning the configured maximum.

<!-- @callout variant="note" title="USB keeps the board awake" -->
With `power-save` on, a connected USB data cable on a board whose console is the USB-Serial-JTAG keeps the chip
awake (the firmware sets this on purpose, otherwise the port goes silent). To measure real sleep current, power the
board from 5 V without the USB data cable. An `init-loop` program with `loop-interval` 0 never sleeps either,
because `loop()` never waits.
<!-- @endcallout -->

ext-events adds `power_hold()` and `power_release()` to keep the board awake around timing-critical work.

## Stops in each runtime

| Runtime | `sys_restart()` / `sys_deep_sleep()` |
| --- | --- |
| `init-loop` | From the script, `setup()` or `loop()`: the program stops; `loop()` is not called again. |
| `event-driven` | From the script or a handler: the reactor stops; pending events are dropped. |
| `web-server` | From the init script: the server never starts. From a request: that request ends like `exit()` and its response is sent, then the server stops; requests that arrive before the restart or sleep get 503. |

`sys_light_sleep()` in `web-server` pauses the request that called it; the server answers it, and the requests
that arrived meanwhile, after the wake.
