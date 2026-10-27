/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-sys                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@php-baremetal.com>         |
   +----------------------------------------------------------------------+
*/

/* sys_*: system helpers. */
#ifndef PHP_SYS_H
#define PHP_SYS_H

#include "php.h"

extern zend_module_entry sys_module_entry;
#define phpext_sys_ptr (&sys_module_entry)

#endif /* PHP_SYS_H */
