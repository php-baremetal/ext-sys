# Changelog

## 1.1.0

- Added: ide/ext-sys.stub, the PHP declarations of the package for IDEs (conductor copies it into vendor/php-baremetal/ide-stubs/).

## 1.0.2

- Changed: `sys_light_sleep()` returns `"none"` when the chip did not sleep (with esp32-sys-hal 1.0.2); it
  returned `"other"`, as for a real wake-up.

## 1.0.1

- Changed: the author's contact email.
- Docs: projects use `"sdk": "esp32@^2.0"`.

## 1.0.0

Initial release.
