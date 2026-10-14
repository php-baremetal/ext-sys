---
eyebrow: 'ext-sys · PHP API'
lede:    'sys_delay() and delay() suspend the PHP task; sys_uptime_ms() and sys_micros() read the time since boot, truncated to 32-bit PHP integers.'
see_also:
  - { href: '../recipes/measure-elapsed-time.md', meta: 'Recipe' }
  - { href: '../reference/power-and-runtimes.md', meta: 'Reference' }
prev: { label: 'Overview', href: '../overview.md' }
next: { label: 'Restart and sleep', href: './restart-and-sleep.md' }
---

# Timing

## sys_delay() and delay()

<!-- @method name="sys_delay(int $ms): void" returns="void" visibility="public" -->

<!-- @method name="delay(int $ms): void" returns="void" visibility="public" -->

<!-- @params -->
<!-- @param name="$ms" type="int" required -->
Milliseconds to wait. Negative values count as 0.
<!-- @endparam -->
<!-- @endparams -->

Suspends the PHP task for `$ms` milliseconds (`vTaskDelay` on ESP32). Other tasks run meanwhile and the
watchdog stays fed; it is never a busy-wait. The firmware's scheduler tick is 1 ms, so the wait has 1 ms
resolution. `delay()` is the same function under the Arduino-style name; it is not deprecated.

PHP's own `usleep()` and `sleep()` also work. `sys_delay()` is the one that names milliseconds directly.

With the `power-save` option on, a delay is idle time in which the board can enter light sleep by itself.

## sys_uptime_ms()

<!-- @method name="sys_uptime_ms(): int" returns="int" visibility="public" -->

Milliseconds since boot, from the ESP32 high-resolution timer (`esp_timer_get_time()`). The value is truncated
to a 32-bit PHP integer, so it turns negative after 2^31 ms (about 24.8 days) and keeps counting from there.

## sys_micros()

<!-- @method name="sys_micros(): int" returns="int" visibility="public" -->

Microseconds since boot, from the same timer. It turns negative after 2^31 µs (about 35.8 minutes): use it for
short intervals only, and always through a wrap-safe difference.

<!-- @callout variant="warning" title="Compare differences, not readings" -->
A plain `$now >= $deadline` breaks once the counter wraps. Subtract two readings and fold the result back into
range, as shown in [Measure elapsed time](../recipes/measure-elapsed-time.md).
<!-- @endcallout -->

Both clocks restart from 0 after `sys_restart()` and after a deep sleep, since both boot the board again. A
`sys_light_sleep()` does not reset them: the time asleep is counted.

## Example

<!-- @code-block language="php" label="time a block" -->
```php
<?php
$t0 = sys_micros();
$hash = md5(str_repeat('x', 4096));
$dt = sys_micros() - $t0;
if ($dt < 0) {
    $dt += 4294967296;   // crossed the wrap: add 2^32
}
printf("md5 of 4 KiB: %d us\n", $dt);
```
<!-- @endcode-block -->
