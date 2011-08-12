#include "error.h"

/* copied from intl_error.c, remove it if merged */
static intl_error* intl_g_error_get(TSRMLS_D)
{
    return &INTL_G(g_error);
}

#define INTIFY_VERSION(M, m, p) ((M) * 10000 + (m) * 100 + (p))
#define ICU_VER INTIFY_VERSION(U_ICU_VERSION_MAJOR_NUM, U_ICU_VERSION_MINOR_NUM, U_ICU_VERSION_PATCHLEVEL_NUM)

#define ICU_VER_LT(M, m, p) (ICU_VER < INTIFY_VERSION(M, m, p))
#define ICU_VER_GT(M, m, p) (ICU_VER > INTIFY_VERSION(M, m, p))
#define ICU_VER_LE(M, m, p) (ICU_VER <= INTIFY_VERSION(M, m, p))
#define ICU_VER_GE(M, m, p) (ICU_VER >= INTIFY_VERSION(M, m, p))

static const char *intl_error_desc(UErrorCode err_code)
{
    switch (err_code) {
        case U_ZERO_ERROR:
            return "no error, no warning";
        case U_ILLEGAL_ARGUMENT_ERROR:
            return "illegal or inappropriate argument";
        case U_MISSING_RESOURCE_ERROR:
            return "the requested resource cannot be found";
        case U_INVALID_FORMAT_ERROR:
            return "data format is not what is expected";
        case U_FILE_ACCESS_ERROR:
            return "the requested file cannot be found";
        case U_INTERNAL_PROGRAM_ERROR:
            return "bug in ICU's code";
        case U_MESSAGE_PARSE_ERROR:
            return "unable to parse a message (message format)";
        case U_MEMORY_ALLOCATION_ERROR:
            return "memory allocation error";
        case U_INDEX_OUTOFBOUNDS_ERROR:
            return "trying to access the index that is out of bounds";
        case U_PARSE_ERROR:
            return "unexpected error while parsing";
        case U_INVALID_CHAR_FOUND:
            return "unmappable input sequence or invalid character";
        case U_TRUNCATED_CHAR_FOUND:
            return "incomplete input sequence";
        case U_ILLEGAL_CHAR_FOUND:
            return "illegal input sequence/combination of input units";
        case U_INVALID_TABLE_FORMAT:
            return "conversion table file found, but corrupted";
        case U_INVALID_TABLE_FILE:
            return "conversion table file not found";
        case U_BUFFER_OVERFLOW_ERROR:
            return "a result would not fit in the supplied buffer";
        case U_UNSUPPORTED_ERROR:
            return "requested operation not supported in current context";
        case U_RESOURCE_TYPE_MISMATCH:
            return "an operation is requested over a resource that does not support it";
        case U_ILLEGAL_ESCAPE_SEQUENCE:
            return "ISO-2022 illlegal escape sequence";
        case U_UNSUPPORTED_ESCAPE_SEQUENCE:
            return "ISO-2022 unsupported escape sequence";
        case U_NO_SPACE_AVAILABLE:
            return "no space available for in-buffer expansion for Arabic shaping";
        case U_CE_NOT_FOUND_ERROR:
            return "currently used only while setting variable top, but can be used generally";
        case U_PRIMARY_TOO_LONG_ERROR:
            return "user tried to set variable top to a primary that is longer than two bytes";
        case U_STATE_TOO_OLD_ERROR:
            return "ICU cannot construct a service from this state, as it is no longer supported";
        case U_TOO_MANY_ALIASES_ERROR:
            return "there are too many aliases in the path to the requested resource. It is very possible that a circular alias definition has occured";
        case U_ENUM_OUT_OF_SYNC_ERROR:
            return "UEnumeration out of sync with underlying collection";
        case U_INVARIANT_CONVERSION_ERROR:
            return "unable to convert a UChar* string to char* with the invariant converter";
        case U_INVALID_STATE_ERROR:
            return "requested operation can not be completed with ICU in its current state";
        case U_COLLATOR_VERSION_MISMATCH:
            return "collator version is not compatible with the base version";
        case U_USELESS_COLLATOR_ERROR:
            return "collator is options only and no base is specified";
        case U_NO_WRITE_PERMISSION:
            return "attempt to modify read-only or constant data";
#if 0
        case :
            return "";
#endif
        default:
            return "bogus error code";
    }
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
                php_error_docref(NULL TSRMLS_CC, INTL_G(error_level), "%s (%s/%d)", intl_error_desc(err_code), u_errorName(err_code), err_code);
            }
            return FALSE;
        }
        return TRUE;
    }

    return U_SUCCESS(err_code);
}
