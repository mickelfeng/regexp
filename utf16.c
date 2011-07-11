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
