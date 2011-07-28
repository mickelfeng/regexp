#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <php.h>

#include <unicode/utypes.h>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

typedef UBool (*filter_func_t)(UChar32);

static void validate(INTERNAL_FUNCTION_PARAMETERS, filter_func_t ff)
{
    UChar32 c;
    zval *zstring = NULL;
    char *string = NULL;
    int string_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zstring)) {
        return;
    }
    if (IS_STRING == Z_TYPE_P(zstring)) {
        int32_t cu;

        if (!Z_STRLEN_P(zstring)) {
            RETURN_FALSE;
        }
        string = Z_STRVAL_P(zstring);
        string_len = Z_STRLEN_P(zstring);

        for (cu = 0; cu < string_len; /* NOP */) {
            U8_NEXT(string, cu, string_len, c);
            if (!ff(c)) {
                RETURN_FALSE;
            }
        }
        RETURN_TRUE;
    } else { /* Looking for a code point */
        if (SUCCESS != unicode_convert_needle_to_cp(zstring, &c TSRMLS_CC)) {
            RETURN_FALSE;
        }
        RETURN_BOOL(ff(c));
    }
}

PHP_FUNCTION(utf8_isalpha)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isalpha);
}

PHP_FUNCTION(utf8_istitle)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_istitle);
}

PHP_FUNCTION(utf8_islower)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_islower);
}

PHP_FUNCTION(utf8_isupper)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isupper);
}

PHP_FUNCTION(utf8_ispunct)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_ispunct);
}

PHP_FUNCTION(utf8_isdigit)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isdigit);
}

PHP_FUNCTION(utf8_ixdigit)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isxdigit);
}

PHP_FUNCTION(utf8_isalnum)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isalnum);
}

PHP_FUNCTION(utf8_isspace)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isspace);
}

PHP_FUNCTION(utf8_isblank)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isblank);
}

PHP_FUNCTION(utf8_iscntrl)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_iscntrl);
}

PHP_FUNCTION(utf8_isgraph)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isgraph);
}

PHP_FUNCTION(utf8_isprint)
{
    validate(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isprint);
}
