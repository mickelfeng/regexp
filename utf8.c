#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include "php_intl.h"
#include "intl_error.h"
#include <unicode/utf8.h>
#include <unicode/ucasemap.h>
#include "unicode.h"
#include "utf16.h"
#include "utf8.h"

int32_t u8_countChar32(const uint8_t *string, int32_t length)
{
    int32_t cpcount, cucount;

    if (NULL == string || length < 0) {
        return 0;
    }

    cpcount = 0;
    if (length >= 0) {
        while (length > 0) {
            cucount = U8_COUNT_TRAIL_BYTES(*string) + 1;
            if (length >= cucount) {
                string += cucount;
                length -= cucount;
                cpcount++;
            }
        }
    }

    return cpcount;
}

int utf8_cp_to_cu(const char *string, int string_len, long cp_offset, int32_t *cu_offset TSRMLS_DC)
{
    if (0 != cp_offset) {
        int32_t _cp_count = u8_countChar32(string, string_len);
        if (cp_offset < 0) {
            if (cp_offset < -_cp_count) {
                intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC);
                return FAILURE;
            }
            *cu_offset = string_len;
            U8_BACK_N(string, 0, *cu_offset, -cp_offset);
        } else {
            if (cp_offset >= _cp_count) {
                intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC);
                return FAILURE;
            }
            U8_FWD_N(string, *cu_offset, string_len, cp_offset);
        }
    }

    return SUCCESS;
}

void utf8_foldcase(char **target, int32_t *target_len, const char *src, int src_len, UErrorCode *status)
{
    UCaseMap *cm;

    cm = ucasemap_open("", 0, status);
    if (U_FAILURE(*status)) {
        return;
    }
    *target_len = ucasemap_utf8FoldCase(cm, NULL, 0, src, src_len, status);
    if (U_BUFFER_OVERFLOW_ERROR != *status) {
        return;
    }
    *status = U_ZERO_ERROR;
    *target = emalloc((*target_len + 1) * sizeof(**target));
    /*result_len = */ucasemap_utf8FoldCase(cm, *target, *target_len, src, src_len, status);
    if (U_FAILURE(*status)) {
        efree(*target);
        *target = NULL;
        *target_len = 0;
    } else {
        *target[*target_len] = '\0';
    }
}

void utf8_add_cp_replacement(HashTable *ht, UChar32 cp_from, const char *cu_to, int32_t cu_to_len)
{
    U8ReplacementCharData *tmp;

    if (SUCCESS == zend_hash_index_find(ht, cp_from, (void **) &tmp)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "multiple replacements for %d", cp_from);
        memcpy(tmp->cus, cu_to, cu_to_len);
        tmp->cus[cu_to_len] = 0;
        tmp->cus_length = cu_to_len;
    } else {
        U8ReplacementCharData nullu8rcd = { 0 };

        tmp = &nullu8rcd;
        memcpy(tmp->cus, cu_to, cu_to_len);
        tmp->cus[cu_to_len] = 0;
        tmp->cus_length = cu_to_len;
        zend_hash_index_update(ht, cp_from, (void *) tmp, sizeof(*tmp), NULL);
    }
}

void utf16_add_cp_replacement(HashTable *ht, UChar32 cp_from, UChar32 cp_to)
{
    //
}

void utf8_do_replacement(HashTable *ht, const char *from, int from_len, char **to, int *to_len)
{
    //
}
