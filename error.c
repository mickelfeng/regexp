#include "error.h"

/* copied from intl_error.c, remove it if merged */
static intl_error* intl_g_error_get(TSRMLS_D)
{
    return &INTL_G(g_error);
}

void intl_errors_setf_custom_msg(intl_error *err TSRMLS_DC, char *format, ...)
{
    char *msg;
    va_list args;

    va_start(args, format);
    vspprintf(&msg, 0, format, args);
    va_end(args);
    intl_errors_set_custom_msg(err, msg, FALSE TSRMLS_CC);
}

int intl_error_non_quiet_set_code(UErrorCode err_code TSRMLS_DC)
{
    intl_error *err;

    if (NULL != (err = intl_g_error_get(TSRMLS_C))) {
        err->code = err_code;
        if (U_FAILURE(err_code)) {
            if (0 != INTL_G(error_level)) {
                php_error_docref(NULL TSRMLS_CC, INTL_G(error_level), "%s(%d)", u_errorName(err_code), err_code);
            }
            return FALSE;
        }
        return TRUE;
    }

    return U_SUCCESS(err_code);
}
