#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include "php_intl.h"
#include "intl_error.h"
#include <unicode/ustring.h>
#include <unicode/unorm.h>
#include "unicode.h"
#include "utf16.h"

void utf16_foldcase(UChar **target, int32_t *target_len, const UChar *src, int32_t src_len, UErrorCode *status)
{
    utf16_fullcase(target, target_len, src, src_len, "" /* locale not used */, UCASE_FOLD, status);
}

int utf16_cp_to_cu(const UChar *ustring, int32_t ustring_len, long cp_offset, int32_t *cu_offset, UErrorCode *status)
{
    if (0 != cp_offset) {
        int32_t _cp_count = u_countChar32(ustring, ustring_len);
        if (cp_offset < 0) {
            if (cp_offset < -_cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FAILURE;
            }
            *cu_offset = ustring_len;
            U16_BACK_N(ustring, 0, *cu_offset, -cp_offset);
        } else {
            if (cp_offset >= _cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FAILURE;
            }
            U16_FWD_N(ustring, *cu_offset, ustring_len, cp_offset);
        }
    }

    return SUCCESS;
}

void utf16_normalize(UChar **target, int32_t *target_len, const UChar *src, int32_t src_len, UNormalizationMode nm, UErrorCode *status)
{
    *status = U_ZERO_ERROR;
    if (nm < UNORM_NONE || nm >= UNORM_MODE_COUNT) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (UNORM_NONE == nm) {
        *target = (UChar *) src;
        *target_len = src_len;
        return;
    }
    *target_len = unorm_normalize(src, src_len, nm, 0, NULL, 0, status);
    if (U_BUFFER_OVERFLOW_ERROR != *status) {
        return;
    }
    *status = U_ZERO_ERROR;
    *target = mem_new_n(**target, *target_len + 1);
    /* *target_len = */unorm_normalize(src, src_len, nm, 0, *target, *target_len + 1, status);
    if (U_FAILURE(*status)) {
        efree(*target);
        *target = NULL;
        *target_len = 0;
    } else {
        *(*target + *target_len) = '\0';
        assert(*status == U_ZERO_ERROR);
    }
}
