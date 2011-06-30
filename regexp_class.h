/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Gustavo Lopes <cataphract@netcabo.pt>                       |
   +----------------------------------------------------------------------+
 */

#ifndef TRANSLITERATOR_CLASS_H
#define TRANSLITERATOR_CLASS_H

#include <php.h>

#include "intl_common.h"
#include "intl_error.h"

#include <unicode/uregex.h>

typedef struct {
    zend_object zo;

    //  error handling
    intl_error err;

    // ICU Regular Expression
    URegularExpression* uregex;
} Regexp_object;

#define REGEXP_ERROR(ro)   (ro)->err
#define REGEXP_ERROR_P(ro) &(REGEXP_ERROR(ro))

#define REGEXP_ERROR_CODE(ro)   INTL_ERROR_CODE(REGEXP_ERROR(ro))
#define REGEXP_ERROR_CODE_P(ro) &(INTL_ERROR_CODE(REGEXP_ERROR(ro)))

#define REGEXP_METHOD_INIT_VARS              INTL_METHOD_INIT_VARS(Regexp, ro)
#define REGEXP_METHOD_FETCH_OBJECT_NO_CHECK  INTL_METHOD_FETCH_OBJECT(Regexp, ro)
#define REGEXP_METHOD_FETCH_OBJECT           \
    do {                                     \
        REGEXP_METHOD_FETCH_OBJECT_NO_CHECK; \
        if (NULL == ro->uregex) {            \
            intl_errors_set(&ro->err, U_ILLEGAL_ARGUMENT_ERROR, "Found unconstructed regular expression", 0 TSRMLS_CC ); \
            RETURN_FALSE;                    \
        }                                    \
    } while (0);

#define REGEXP_CHECK_STATUS(ro, msg)                                          \
    do {                                                                      \
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);           \
        if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {                               \
            intl_errors_set_custom_msg(REGEXP_ERROR_P(ro), msg, 0 TSRMLS_CC); \
            goto end;                                                         \
        }                                                                     \
    } while (0);

int REGEXP_object_construct(zval *object, URegularExpression *uregex, UErrorCode *status TSRMLS_DC);

void regexp_register_Regexp_class(TSRMLS_D);

extern zend_class_entry *Regexp_ce_ptr;
extern zend_object_handlers Regexp_handlers;

#endif /* #ifndef TRANSLITERATOR_CLASS_H */
