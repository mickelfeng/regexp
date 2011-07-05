#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include "php_intl.h"
#include "intl_error.h"
#include <unicode/ustring.h>
#include "unicode.h"
#include "utf16.h"

void utf16_foldcase(UChar **target, int32_t *target_len, const UChar *src, int32_t src_len, UErrorCode *status)
{
    int32_t target_size;
    int tries = 0;

    *target = NULL;
    *target_len = 0;
    *status = U_ZERO_ERROR;
    do { /* Iteration needed: string may be longer than original ! */
        target_size = ++tries * src_len + 1;
        *target = erealloc(*target, target_size * sizeof(*target));
        *target_len = u_strFoldCase(*target, target_size, src, src_len, 0, status);
        if (U_SUCCESS(status)) {
            break;
        }
    } while (U_BUFFER_OVERFLOW_ERROR == *status);
    if (U_FAILURE(*status)) {
        efree(*target);
        *target = NULL;
    } else {
        *target[*target_len] = 0;
    }
}

int utf16_cp_to_cu(const UChar *ustring, int32_t ustring_len, long cp_offset, int32_t *cu_offset TSRMLS_DC)
{
    if (0 != cp_offset) {
        int32_t _cp_count = u_countChar32(ustring, ustring_len);
        if (cp_offset < 0) {
            if (cp_offset < -_cp_count) {
                intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC);
                return FAILURE;
            }
            *cu_offset = ustring_len;
            U16_BACK_N(ustring, 0, *cu_offset, -cp_offset);
        } else {
            if (cp_offset >= _cp_count) {
                intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC);
                return FAILURE;
            }
            U16_FWD_N(ustring, *cu_offset, ustring_len, cp_offset);
        }
    }

    return SUCCESS;
}
