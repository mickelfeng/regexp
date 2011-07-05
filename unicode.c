#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include "php_intl.h"
#include "intl_error.h"
#include <unicode/utypes.h>
#include "unicode.h"

int unicode_convert_needle_to_cp(zval *needle, UChar32 *cp TSRMLS_DC)
{
    switch (Z_TYPE_P(needle)) {
        case IS_LONG:
        case IS_BOOL:
            *cp = (UChar32) Z_LVAL_P(needle);
            return SUCCESS;
        case IS_NULL:
            *cp = 0;
            return SUCCESS;
        case IS_DOUBLE:
            *cp = (UChar32) Z_DVAL_P(needle);
            return SUCCESS;
        case IS_OBJECT:
        {
            zval holder = *needle;
            zval_copy_ctor(&(holder));
            convert_to_long(&(holder));
            if (IS_LONG != Z_TYPE(holder)) {
                return FAILURE;
            }
            *cp = (UChar32) Z_LVAL(holder);
            return SUCCESS;
        }
        default:
        {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "needle is not a string or an integer");
            return FAILURE;
        }
    }
}
