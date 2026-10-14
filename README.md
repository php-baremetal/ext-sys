# ext-sys

System functions for the PHP that runs on your board, all global and prefixed `sys_`:

- **timing**: `sys_delay()` (alias `delay()`), `sys_uptime_ms()`, `sys_micros()`;
- **restart and sleep**: `sys_restart()`, `sys_deep_sleep()`, `sys_light_sleep()`, `sys_wake_cause()`,
  `sys_reset_reason()`;
- **identity**: `sys_chip_model()`, `sys_cpu_freq_mhz()`, `sys_mac()`, `sys_idf_version()`;
- **memory**: `sys_heap_free()`, `sys_heap_size()`, `sys_psram_free()`, `sys_psram_size()`,
  `sys_psram_largest_free()`.

ext-sys is platform-neutral: it requires the virtual `php-baremetal/sys-hal`, and conductor adds the HAL for your
SDK platform (`php-baremetal/esp32-sys-hal` on ESP32).

## Install

In a project created with `phpbm init`:

```bash
composer require php-baremetal/ext-sys
```

## Example

```php
<?php
printf("%s @ %d MHz, ESP-IDF %s, MAC %s\n",
    sys_chip_model(), sys_cpu_freq_mhz(), sys_idf_version(), sys_mac());
printf("reset: %s, wake: %s\n", sys_reset_reason(), sys_wake_cause());
printf("internal RAM free %d of %d bytes\n", sys_heap_free(), sys_heap_size());

delay(1000);                                // suspends the PHP task for 1 s

if (sys_wake_cause() === 'none') {
    sys_deep_sleep(60000);                  // ends the program; the board wakes in 60 s and boots again
}
```

## Documentation

- [Overview](docs/overview.md): requirements, install, a first project, the API at a glance.
- [Timing](docs/usage/timing.md), [Restart and sleep](docs/usage/restart-and-sleep.md),
  [Chip and memory](docs/usage/chip-and-memory.md): the PHP API.
- [Power-save, executor and runtimes](docs/reference/power-and-runtimes.md)
- [Errors and limits](docs/reference/errors-and-limits.md)
- Recipes: [Print a health report](docs/recipes/health-report.md),
  [Measure elapsed time](docs/recipes/measure-elapsed-time.md),
  [Sleep between readings](docs/recipes/sleep-between-readings.md)

## License

MIT. The package also ships `AI_USAGE.md`, the project's policy on AI-assisted work.
