#include "error.h"

/* copied from intl_error.c, remove it if merged */
static intl_error* intl_g_error_get(TSRMLS_D)
{
    return &INTL_G(g_error);
}

void intl_errors_setf_custom_msg(intl_error *err TSRMLS_DC, char *format, ...)
{
    va_list args;

    if (NULL == err) {
        if (0 != INTL_G(error_level)) {
            va_start(args, format);
            php_verror(NULL, "", INTL_G(error_level), format, args TSRMLS_CC);
            va_end(args);
        }
    } else {
        char *msg;

        if (NULL == err && NULL == (err = intl_g_error_get(TSRMLS_C))) {
            return;
        }
        intl_free_custom_error_msg(err TSRMLS_CC);
        va_start(args, format);
        vspprintf(&msg, 0, format, args); // check msg ?
        va_end(args);
        err->free_custom_error_message = TRUE;
        err->custom_error_message = msg;
    }
}

int intl_error_non_quiet_set_code(UErrorCode err_code TSRMLS_DC)
{
    intl_error *err;

    if (NULL != (err = intl_g_error_get(TSRMLS_C))) {
        err->code = err_code;
        if (U_FAILURE(err_code)) {
            if (0 != INTL_G(error_level)) {
                php_error_docref(NULL TSRMLS_CC, INTL_G(error_level), "%s (%d)", u_errorName(err_code), err_code);
            }
            return FALSE;
        }
        return TRUE;
    }

    return U_SUCCESS(err_code);
}
