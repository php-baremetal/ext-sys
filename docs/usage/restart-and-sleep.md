---
eyebrow: 'ext-sys · PHP API'
lede:    'sys_restart() and sys_deep_sleep() end the program like exit() and let the firmware restart or sleep; sys_light_sleep() sleeps in place and returns; sys_wake_cause() and sys_reset_reason() say how the board came up.'
see_also:
  - { href: '../reference/power-and-runtimes.md', meta: 'Reference' }
  - { href: '../reference/errors-and-limits.md', meta: 'Reference' }
  - { href: '../recipes/sleep-between-readings.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/esp32-sys-hal', meta: 'HAL', label: 'esp32-sys-hal' }
prev: { label: 'Timing', href: './timing.md' }
next: { label: 'Chip and memory', href: './chip-and-memory.md' }
---

# Restart and sleep

## How a stop works

`sys_restart()` and `sys_deep_sleep()` never restart or sleep from inside PHP. They record what the firmware
must do after the program stops, then end the script the way `exit()` does:

- the rest of the script, `loop()` and every pending handler are skipped;
- shutdown functions (`register_shutdown_function()`) and destructors run; `finally` blocks do not, and the
  stop cannot be caught;
- the console prints `--- stopped: restart ---` or `--- stopped: deep sleep 5000 ms ---` instead of the usual
  exit line;
- the firmware then shuts PHP down, unmounts the storage, and restarts at once (no `restart-delay`) or enters
  deep sleep.

This works the same from the top-level script, `setup()`, `loop()`, an event handler, the exception handler of
`set_exception_handler()`, or a web request. The first request wins: a later `exit()` or a second call does not
change it.

## sys_restart()

<!-- @method name="sys_restart(): never" returns="never" visibility="public" -->

Stops the program, then restarts the board immediately. After the restart `sys_reset_reason()` is
`"software"` and `sys_wake_cause()` is `"none"`. Files written and closed before the call are on the storage
after the restart.

## sys_deep_sleep()

<!-- @method name="sys_deep_sleep(int $ms, ?int $wakePin = null, bool $wakeLow = true): never" returns="never" visibility="public" -->

<!-- @params -->
<!-- @param name="$ms" type="int" required -->
Milliseconds to sleep. 0 means no timer: only the pin wakes the board, so `$wakePin` is then required.
<!-- @endparam -->
<!-- @param name="$wakePin" type="?int" default="null" -->
A GPIO that wakes the board, or `null` for timer only. It must be able to wake the chip from deep sleep: an
RTC GPIO on ESP32-S3 (GPIO 0 to 21), an LP GPIO on ESP32-P4.
<!-- @endparam -->
<!-- @param name="$wakeLow" type="bool" default="true" -->
`true` wakes when the pin is low (a button to GND), `false` when it is high. The opposite internal pull is
enabled on the pin during the sleep.
<!-- @endparam -->
<!-- @endparams -->

Stops the program, then puts the chip into deep sleep about 100 ms later (so the last console lines get out).
The console line includes the pin, for example
`--- stopped: deep sleep 60000 ms, wake on GPIO 4 low ---`. Deep sleep powers down the CPUs and RAM: waking up
is a full boot, the firmware prints `phpbm: woke from deep sleep (timer)` (or `gpio`) and the script starts from
the beginning. Then `sys_reset_reason()` is `"deepsleep"` and `sys_wake_cause()` tells which source fired.

Throws `\ValueError` before stopping when `$ms` is negative, when `$ms` is 0 without a pin, or when the pin is
not valid; see [Errors and limits](../reference/errors-and-limits.md). If the firmware cannot set the wake
sources after the stop, it prints a line and restarts instead.

## sys_light_sleep()

<!-- @method name="sys_light_sleep(int $ms, ?int $wakePin = null, bool $wakeLow = true): string" returns="string" visibility="public" -->

<!-- @params -->
<!-- @param name="$ms" type="int" required -->
Milliseconds to sleep. 0 means no timer, and requires `$wakePin`.
<!-- @endparam -->
<!-- @param name="$wakePin" type="?int" default="null" -->
Any valid GPIO of the chip, or `null` for timer only. The wake is on the pin's level, not on an edge.
<!-- @endparam -->
<!-- @param name="$wakeLow" type="bool" default="true" -->
`true` wakes while the pin is low, `false` while it is high.
<!-- @endparam -->
<!-- @endparams -->

Puts the chip into light sleep and returns when it wakes, with what woke it:

| Return | Meaning |
| --- | --- |
| `"timer"` | `$ms` elapsed. |
| `"gpio"` | The pin reached the wake level. Also returned at once, without sleeping, when the pin is already at that level. |
| `"other"` | Any other wake source, or the sleep could not start. |

RAM, PSRAM, PHP state and open files survive; the script simply continues. The uptime keeps counting through the
sleep. The sleep runs on the executor task, which is blocked until the chip wakes: see
[Power-save, executor and runtimes](../reference/power-and-runtimes.md). The USB-Serial-JTAG console drops out
while the chip sleeps.

Throws `\ValueError` for a negative `$ms`, `$ms` 0 without a pin, or a pin the chip does not have.

## sys_wake_cause()

<!-- @method name="sys_wake_cause(): string" returns="string" visibility="public" -->

What ended the deep sleep this boot came from: `"timer"`, `"gpio"`, `"other"`, or `"none"` when the board did not
come from deep sleep (power-on, reset button, `sys_restart()`, a crash). The firmware reads it once at boot, so
`sys_light_sleep()` does not change it.

## sys_reset_reason()

<!-- @method name="sys_reset_reason(): string" returns="string" visibility="public" -->

Why the chip last booted. On ESP32:

| Value | Meaning |
| --- | --- |
| `poweron` | Power was applied. |
| `external` | The external reset pin. |
| `software` | A software restart: `sys_restart()`, or the firmware restarting after the program stopped. |
| `panic` | A crash (exception or abort). |
| `int_wdt` | The interrupt watchdog. |
| `task_wdt` | The task watchdog, typically a task that never yields. |
| `wdt` | Another watchdog. |
| `deepsleep` | Woke from deep sleep. |
| `brownout` | The supply voltage dropped too low. |
| `sdio` | Reset over SDIO. |
| `usb` | Reset by the USB peripheral (for example by the flashing tool). |
| `jtag` | Reset over JTAG. |
| `unknown` | Anything else. |

## Example

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
printf("reset %s, wake %s\n", sys_reset_reason(), sys_wake_cause());

register_shutdown_function(function (): void {
    echo "saving state before the stop\n";   // runs before the firmware sleeps
});

$why = sys_light_sleep(2000);                // 2 s in light sleep, then continue
echo "light sleep ended: $why\n";

sys_deep_sleep(10000, wakePin: 4);           // 10 s, or earlier when GPIO 4 goes low
```
<!-- @endcode-block -->
