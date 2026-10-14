---
eyebrow: 'ext-sys · Recipe'
lede:    'Time code, run at a fixed rate and wait with a timeout using sys_micros() and sys_uptime_ms(), correctly across the 32-bit wrap.'
see_also:
  - { href: '../usage/timing.md', meta: 'PHP API' }
  - { href: '../reference/errors-and-limits.md', meta: 'Reference' }
prev: { label: 'Print a health report', href: './health-report.md' }
next: { label: 'Sleep between readings', href: './sleep-between-readings.md' }
---

# Measure elapsed time

PHP integers are 32-bit on the board, and the two clocks are truncated to that size: `sys_micros()` turns
negative after about 35.8 minutes of uptime, `sys_uptime_ms()` after about 24.8 days. Comparing raw readings
fails once across each wrap; a difference folded back into range never does, for any interval shorter than the
wrap.

## The helper

<!-- @code-block language="php" label="wrap-safe difference" -->
```php
<?php
// Ticks from $from to $to, correct across one wrap of a 32-bit counter.
function elapsed(int $from, int $to): int
{
    $d = $to - $from;          // across the wrap PHP returns a negative float
    if ($d < 0) {
        $d += 4294967296;      // 2^32
    }
    return (int) $d;
}
```
<!-- @endcode-block -->

## Time a piece of code

<!-- @code-block language="php" label="time a block" -->
```php
<?php
$t0 = sys_micros();
$json = json_encode(range(1, 500));
printf("json_encode: %d us\n", elapsed($t0, sys_micros()));
```
<!-- @endcode-block -->

## Run at a fixed rate

`delay()` after the work makes the period drift by however long the work took. Subtract it:

<!-- @code-block language="php" label="project-src/index.php, init-loop" -->
```php
<?php
const PERIOD_MS = 250;

function loop(int $tick): void
{
    $start = sys_uptime_ms();

    // ... read sensors, print ...

    $spent = elapsed($start, sys_uptime_ms());
    delay(max(0, PERIOD_MS - $spent));
}
```
<!-- @endcode-block -->

The runtime's `loop-interval` option gives a fixed pause between two `loop()` calls instead, which is simpler
when drift does not matter.

## Wait with a timeout

<!-- @code-block language="php" label="wait for a pin, at most 2 s" -->
```php
<?php
$start = sys_uptime_ms();
while (gpio_read(4) !== 0) {
    if (elapsed($start, sys_uptime_ms()) > 2000) {
        throw new RuntimeException('GPIO 4 did not go low within 2 s');
    }
    delay(5);
}
```
<!-- @endcode-block -->

This one also uses ext-gpio for `gpio_read()`.
