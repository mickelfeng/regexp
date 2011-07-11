#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include "php_intl.h"
#include "intl_data.h"
#include "intl_convert.h"
#include "intl_error.h"
#include <unicode/utf8.h>
#include <unicode/utf16.h>
#include <unicode/uchar.h>
#include <unicode/ucasemap.h>
#include <unicode/ustring.h>
#include <unicode/unorm.h>
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

int utf8_cp_to_cu(const char *string, int string_len, int32_t cp_offset, int32_t *cu_offset, UErrorCode *status)
{
    if (0 != cp_offset) {
        int32_t _cp_count = u8_countChar32((const uint8_t *) string, string_len);
        if (cp_offset < 0) {
            if (cp_offset < -_cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FAILURE;
            }
            *cu_offset = string_len;
            U8_BACK_N((const uint8_t *) string, 0, *cu_offset, -cp_offset);
        } else {
            if (cp_offset >= _cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FAILURE;
            }
            U8_FWD_N(string, *cu_offset, string_len, cp_offset);
        }
    }

    return SUCCESS;
}

void utf8_foldcase(char **target, int32_t *target_len, const char *src, int src_len, UErrorCode *status)
{
    utf8_fullcase(target, target_len, src, src_len, "" /* locale not used */, UCASE_FOLD, status);
}

static inline char *utf8_binary_memrnstr(char *haystack, char *needle, int needle_len, char *end)
{
    char *p;
    char ne = needle[needle_len - 1];

    if (1 == needle_len) {
        return (char *) memrchr(haystack, *needle, end - haystack);
    }

    if (needle_len > end - haystack) {
        return NULL;
    }

    p = end - needle_len;
    while (p >= haystack) {
        if ((p = (char *) memrchr(haystack, *needle, p - haystack + 1)) && ne == p[needle_len - 1]) {
            if (!memcmp(needle, p, needle_len - 1)) {
                return p;
            }
        }

        if (NULL == p) {
            return NULL;
        }

        p--;
    }

    return NULL;
}

/**
 * TODO: Tests with composed/decomposed strings
 **/

/**
 * Helper for strr?i?pos and strr?i?chr
 * --
 * Return a pointer on the, first or last, occurence of needle in haystack from specified code point
 * or NULL if needle is not found in this haystack substring
 * --
 * Caller should:
 * - handle needle conversion to code point if it is not a string
 * - handle/set ICU error
 **/
char *utf8_find( /* /!\ may be changed in the future /!\ */
    char *haystack, int32_t haystack_len, /* length in CU */
    char *needle, int32_t needle_len,     /* length in CU */
    int32_t start_cp_offset,              /* offset in CP ; may be negative to search from end */
    const char *locale,
    UBool search_first,
    UNormalizationMode nm, UCaseType ct, /* /!\ may be removed for a default/specific value (ICU ones) /!\ */
    UErrorCode *status
) {
    *status = U_ZERO_ERROR;
    if (UNORM_NONE == nm) { // it is possible to directly work in UTF-8
        char *found;
        char *cased_haystack, *cased_needle;
        int32_t start_cu_offset;
        int32_t cased_haystack_len, cased_needle_len;

        found = cased_haystack = cased_needle = NULL;
        if (!utf8_cp_to_cu(haystack, haystack_len, start_cp_offset, &start_cu_offset, status)) {
            return NULL;
        }
        // TODO: ne convertir que la partie où la recherche a lieu
        utf8_fullcase(&cased_haystack, &cased_haystack_len, haystack, haystack_len, locale, ct, status);
        if (U_FAILURE(*status)) {
            if (UCASE_NONE != ct) {
                if (NULL != cased_haystack) {
                    efree(cased_haystack);
                }
            }
            return NULL;
        }
        // TODO: ne convertir que la partie où la recherche a lieu
        utf8_fullcase(&cased_needle, &cased_needle_len, needle, needle_len, locale, ct, status);
        if (U_FAILURE(*status)) {
            if (UCASE_NONE != ct) {
                if (NULL != cased_haystack) {
                    efree(cased_haystack);
                }
                if (NULL != cased_needle) {
                    efree(cased_needle);
                }
            }
            return NULL;
        }
        if (search_first) {
            found = php_memnstr(haystack + start_cu_offset, needle, needle_len, haystack + haystack_len);
        } else {
            if (start_cp_offset >= 0) {
                found = utf8_binary_memrnstr(haystack + start_cu_offset, needle, needle_len, haystack + haystack_len);
            } else {
                found = utf8_binary_memrnstr(haystack, needle, needle_len, haystack + start_cu_offset);
            }
        }

        return found;
    } else { // impossible here, we need to work in UTF-16
        UChar *uhaystack = NULL;
        UChar *uneedle = NULL;
        int32_t uhaystack_len = 0;
        int32_t uneedle_len = 0;

        // TODO: ne convertir que la partie où la recherche a lieu
        intl_convert_utf8_to_utf16(&uhaystack, &uhaystack_len, haystack, haystack_len, status);
        if (U_FAILURE(status)) {
            if (NULL != uhaystack) {
                efree(uhaystack);
            }
            return NULL;
        }
        // TODO: ne convertir que la partie où la recherche a lieu
        intl_convert_utf8_to_utf16(&uneedle, &uneedle_len, needle, needle_len, status);
        if (U_FAILURE(status)) {
            if (NULL != uhaystack) {
                efree(uhaystack);
            }
            if (NULL != uneedle) {
                efree(uneedle);
            }
            return NULL;
        }
        // normaliser, appliquer la casse, (re)normaliser, chercher
        // "recalculer" le pointeur UTF-16 => UTF-8
        if (NULL != uhaystack) {
            efree(uhaystack);
        }
        if (NULL != uneedle) {
            efree(uneedle);
        }
    }

    return NULL;
}

// longueur/position + décomposition = kaboum ?

/*enum {
    LESSER  = -1,
    EQUAL   = 0,
    GREATER = 1
};*/

/**
 * Helper for strn?(?:case)?cmp
 * --
 * Return :
 * # <0 if string1[string1_offset, match_length] < string2[string2_offset, match_length]
 * # =0 if string1[string1_offset, match_length] == string2[string2_offset, match_length]
 * # >0 if string1[string1_offset, match_length] > string2[string2_offset, match_length]
 * --
 * Caller should:
 * - handle/set ICU error
 **/
int utf8_region_matches( /* /!\ may be changed in the future /!\ */
    const char *string1, int32_t string1_len, int32_t string1_offset, /* length and offset in CP */
    const char *string2, int32_t string2_len, int32_t string2_offset, /* length and offset in CP */
    int32_t match_length,                                             /* length in CP, < 0 for all string (str(?:case)?cmp behavior) */
    const char *locale,
    UNormalizationMode nm, UCaseType ct, /* /!\ may be removed for a default/specific value (ICU ones) /!\ */
    UErrorCode *status
) {
    int ret = 0;
    int32_t string1_cu_offset = 0;
    int32_t string2_cu_offset = 0;

    *status = U_ZERO_ERROR;
    if (string1_offset != 0) {
        if (!utf8_cp_to_cu(string1, string1_len, string1_offset, &string1_cu_offset, status)) {
            return 0; /* caller should consider status first */
        }
    }
    if (string2_offset != 0) {
        if (!utf8_cp_to_cu(string2, string2_len, string2_offset, &string2_cu_offset, status)) {
            return 0; /* caller should consider status first */
        }
    }
    if (UNORM_NONE == nm) {
        char *cased_string1 = NULL;
        int32_t cased_string1_len = 0;
        char *cased_string2 = NULL;
        int32_t cased_string2_len = 0;
        int32_t substring1_len;
        int32_t substring2_len;

        utf8_fullcase(&cased_string1, &cased_string1_len, string1 + string1_offset, string1_len - string1_offset, locale, ct, status);
        if (U_FAILURE(*status)) {
            if (UCASE_NONE != ct) {
                if (NULL != cased_string1) {
                    efree(cased_string1);
                }
            }
            return 0; /* caller should consider status first */
        }
        utf8_fullcase(&cased_string2, &cased_string2_len, string2 + string2_offset, string2_len - string2_offset, locale, ct, status);
        if (U_FAILURE(*status)) {
            if (UCASE_NONE != ct) {
                if (NULL != cased_string1) {
                    efree(cased_string1);
                }
                if (NULL != cased_string2) {
                    efree(cased_string2);
                }
            }
            return 0; /* caller should consider status first */
        }
        substring1_len = u8_countChar32((const uint8_t *) string1 + string1_offset, string1_len - string1_offset);
        substring2_len = u8_countChar32((const uint8_t *) string2 + string2_offset, string2_len - string2_offset);
        // TODO: binary utf8 comparaison
        // ret = ?
    } else {
        // ct (Case Type) et nm (Normalization Mode) ignored
        UChar *usubstring1 = NULL;
        UChar *usubstring2 = NULL;
        int32_t usubstring1_len = 0;
        int32_t usubstring2_len = 0;

// debug("%.*s", string1_len - string1_cu_offset, string1);
// debug("%.*s", string2_len - string2_cu_offset, string2);
        intl_convert_utf8_to_utf16(&usubstring1, &usubstring1_len, string1 + string1_cu_offset, string1_len - string1_cu_offset, status);
        if (U_FAILURE(*status)) {
            if (NULL != usubstring1) {
                efree(usubstring1);
            }
            return 0; /* caller should consider status first */
        }
        intl_convert_utf8_to_utf16(&usubstring2, &usubstring2_len, string2 + string2_cu_offset, string2_len - string2_cu_offset, status);
        if (U_FAILURE(*status)) {
            if (NULL != usubstring1) {
                efree(usubstring1);
            }
            if (NULL != usubstring2) {
                efree(usubstring2);
            }
            return 0; /* caller should consider status first */
        }
        // TODO: handle match_length (here or before ?)
        // TODO: case insensitive use case folding
        ret = unorm_compare(usubstring1, usubstring1_len, usubstring2, usubstring2_len, UCASE_NONE == ct ? U_FOLD_CASE_DEFAULT : U_COMPARE_IGNORE_CASE, status);
        // we don't need to check *status here
        if (NULL != usubstring1) {
            efree(usubstring1);
        }
        if (NULL != usubstring2) {
            efree(usubstring2);
        }
    }

    return ret;
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

#if 0
void utf16_add_cp_replacement(HashTable *ht, UChar32 cp_from, UChar32 cp_to)
{
    //
}

void utf8_do_replacement(HashTable *ht, const char *from, int from_len, char **to, int *to_len)
{
    //
}
#endif