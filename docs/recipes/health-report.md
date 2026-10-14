---
eyebrow: 'ext-sys · Recipe'
lede:    'Print the chip identity, why it booted and how memory develops over time, on the serial console.'
see_also:
  - { href: '../usage/chip-and-memory.md', meta: 'PHP API' }
  - { href: '../usage/restart-and-sleep.md', meta: 'PHP API' }
prev: { label: 'Errors and limits', href: '../reference/errors-and-limits.md' }
next: { label: 'Measure elapsed time', href: './measure-elapsed-time.md' }
---

# Print a health report

A banner at boot, then one line every ten seconds with uptime and free memory. A falling minimum of free
internal RAM, or a PSRAM pool whose largest free block shrinks much faster than its free total, points at a leak or
at fragmentation long before an allocation fails.

## The project

The `composer.json` from the [Overview](../overview.md): `ext-sys` only, runtime `init-loop`, a Waveshare
ESP32-S3-Zero.

## The script

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
printf("%s @ %d MHz, ESP-IDF %s, MAC %s\n",
    sys_chip_model(), sys_cpu_freq_mhz(), sys_idf_version(), sys_mac());
printf("reset %s, wake %s\n", sys_reset_reason(), sys_wake_cause());

if (in_array(sys_reset_reason(), ['panic', 'int_wdt', 'task_wdt', 'wdt', 'brownout'], true)) {
    echo "WARNING: the last run ended abnormally\n";
}

$minFree = PHP_INT_MAX;

function loop(int $tick): void
{
    global $minFree;

    $free = sys_heap_free();
    $minFree = min($minFree, $free);

    printf("up %6d s | RAM %d of %d free (min %d) | PSRAM %d of %d free, largest %d\n",
        intdiv(sys_uptime_ms(), 1000),
        $free, sys_heap_size(), $minFree,
        sys_psram_free(), sys_psram_size(), sys_psram_largest_free());

    delay(10000);
}
```
<!-- @endcode-block -->

## How it works

- The identity functions are constant for the run; print them once.
- `sys_reset_reason()` separates a clean restart (`software`, `deepsleep`, `poweron`) from a crash or a brownout.
- On a board without PSRAM the three `sys_psram_*` values are 0.
