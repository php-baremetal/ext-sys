---
eyebrow: 'ext-sys · Recipe'
lede:    'A node that wakes, does its work and sleeps again: deep sleep with a timer and a button, or light sleep when the program must keep its state.'
see_also:
  - { href: '../usage/restart-and-sleep.md', meta: 'PHP API' }
  - { href: '../reference/power-and-runtimes.md', meta: 'Reference' }
prev: { label: 'Measure elapsed time', href: './measure-elapsed-time.md' }
next: { label: 'No next page', href: '#' }
---

# Sleep between readings

A Waveshare ESP32-S3-Zero with a push button between GPIO 4 and GND. GPIO 4 is an RTC GPIO, so it can wake the
chip from deep sleep too.

## The project

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/sleepy-node",
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

## Deep sleep: the lowest current

The script runs from the top on every wake. It reports why it is running, does its work and goes back to sleep
for a minute, or until the button is pressed. Nothing survives in PHP between two wakes: keep state on the
storage if you need it.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
const BUTTON = 4;

$why = match (sys_wake_cause()) {
    'timer' => 'scheduled wake',
    'gpio'  => 'button pressed',
    'none'  => 'first boot (' . sys_reset_reason() . ')',
    default => 'other wake',
};
echo "awake: $why\n";

// ... take the reading, send it, write it to storage ...
printf("reading done after %d ms awake\n", sys_uptime_ms());

sys_deep_sleep(60000, wakePin: BUTTON, wakeLow: true);
```
<!-- @endcode-block -->

`loop()` is not defined: the program never gets there, because `sys_deep_sleep()` ends it. The console shows
`--- stopped: deep sleep 60000 ms, wake on GPIO 4 low ---`, then nothing until the wake, then the normal boot
lines followed by `phpbm: woke from deep sleep (timer)` or `(gpio)`.

## Light sleep: keep the program running

When the program must keep its variables, open connections or a counter, sleep in place instead:

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
const BUTTON = 4;

$readings = 0;

function loop(int $tick): void
{
    global $readings;

    $readings++;
    printf("reading %d at %d ms\n", $readings, sys_uptime_ms());

    $woke = sys_light_sleep(10000, wakePin: BUTTON);   // "timer" or "gpio"
    if ($woke === 'gpio') {
        echo "button\n";
        delay(300);                                      // let go of the button
    }
}
```
<!-- @endcode-block -->

Light sleep wakes on the pin's level, so a button still held after the wake would return `"gpio"` at once on the
next call; the short `delay()` covers that.

## Notes

- Measure current on external 5 V without the USB data cable. While the USB-Serial-JTAG console is connected, the
  console drops during each sleep and the host may log a disconnect.
- Deep sleep restarts the whole firmware on wake, which costs the boot time each cycle. Light sleep resumes in
  milliseconds but draws more while asleep.
