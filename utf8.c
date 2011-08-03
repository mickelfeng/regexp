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
#include "utf16.h"
#include "utf8.h"

#define UTF8_3_4_IS_ILLEGAL(b) (((b) < 0x80) || ((b) > 0xBF))

#undef I
#undef R

#define I -1 /* Illegal */
#define R -2 /* Out of range */

static const int8_t utf8_count_bytes[256] = {
    /*      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
    /* 0 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 1 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 2 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 4 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 6 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 8 */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* 9 */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* A */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* B */ I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    /* C */ I, I, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* D */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* E */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* F */ 4, 4, 4, 4, R, R, R, R, R, R, R, R, R, R, R, R
};

#undef I
#undef R

static const uint8_t utf8_min_second_byte_value[256] = {
    /*      0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,    E,    F */
    /* 0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 1 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 2 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 3 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 4 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 5 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 6 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 9 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* A */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* B */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* C */ 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* D */ 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* E */ 0xA0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    /* F */ 0x90, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t utf8_max_second_byte_value[256] = {
    /*      0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,    E,    F */
    /* 0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 1 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 2 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 3 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 4 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 5 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 6 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 9 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* A */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* B */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* C */ 0x00, 0x00, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF,
    /* D */ 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF,
    /* E */ 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0x9F, 0xBF, 0xBF,
    /* F */ 0xBF, 0xBF, 0xBF, 0xBF, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int32_t utf8_countChar32(const uint8_t *string, int32_t length)
{
    int32_t cpcount, cucount;

    if (NULL == string || length < 0) {
        return 0;
    }

    cpcount = 0;
    while (length > 0) {
        cucount = utf8_count_bytes[*string];
        if (length >= cucount) {
            string += cucount;
            length -= cucount;
            cpcount++;
        }
    }

    return cpcount;
}

UBool utf8_validate(const uint8_t *string, int32_t string_len, UErrorCode *status)
{
    int i;
    uint8_t *end, *p;
    int8_t cplen;

    if (NULL == string) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return FALSE;
    }

    if (string_len < 0) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return FALSE;
    }

    p = (uint8_t *) string;
    end = ((uint8_t *) string) + string_len;
    while (end > p) {
        cplen = utf8_count_bytes[*p];
        if (cplen < 0 || cplen > U8_MAX_LENGTH) {
            *status = U_ILLEGAL_CHAR_FOUND;
            return FALSE;
        }
        if (end - p < cplen) {
            *status = U_TRUNCATED_CHAR_FOUND;
            return FALSE;
        }
        if (1 == cplen) {
            p++;
        } else {
            /**
             * TODO: (3.5) check non character code point [0xfdd0-0xfdef] + !0x*fff[e-f] + < 0x10ffff
             * U_INVALID_CHAR_FOUND
             **/
            if (p[1] < utf8_min_second_byte_value[*p] || p[1] > utf8_max_second_byte_value[*p]) {
                *status = U_ILLEGAL_CHAR_FOUND;
                return FALSE;
            }
            for (i = 2; i < cplen; i++) {
                if (UTF8_3_4_IS_ILLEGAL(p[i])) {
                    *status = U_ILLEGAL_CHAR_FOUND;
                    return FALSE;
                }
            }
            p += cplen;
        }
    }

    return TRUE;
}

int utf8_cp_to_cu(const char *string, int string_len, int32_t cp_offset, int32_t *cu_offset, UErrorCode *status)
{
    if (0 != cp_offset) {
        int32_t _cp_count = utf8_countChar32((const uint8_t *) string, string_len);
        if (cp_offset < 0) {
            if (cp_offset < -_cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FALSE;
            }
            *cu_offset = string_len;
            U8_BACK_N((const uint8_t *) string, 0, *cu_offset, -cp_offset);
        } else {
            if (cp_offset >= _cp_count) {
                *status = U_INDEX_OUTOFBOUNDS_ERROR;
                return FALSE;
            }
            U8_FWD_N(string, *cu_offset, string_len, cp_offset);
        }
    }

    return TRUE;
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
    int32_t start_cu_offset = 0;

    *status = U_ZERO_ERROR;
    if (!utf8_cp_to_cu(haystack, haystack_len, start_cp_offset, &start_cu_offset, status)) {
        return NULL;
    }
    if (UNORM_NONE == nm) { // it is possible to directly work in UTF-8
        char *found;
        char *cased_haystack, *cased_needle;
        int32_t cased_haystack_len, cased_needle_len;

        found = cased_haystack = cased_needle = NULL;
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
        int32_t uhaystack_len = 0;
        UChar *uneedle = NULL;
        int32_t uneedle_len = 0;
        UChar *unormalized_haystack = NULL;
        int32_t unormalized_haystack_len = 0;
        UChar *unormalized_needle = NULL;
        int32_t unormalized_needle_len = 0;
        UChar *ucased_haystack = NULL;
        int32_t ucased_haystack_len = 0;
        UChar *ucased_needle = NULL;
        int32_t ucased_needle_len = 0;

        // TODO: ne convertir que la partie où la recherche a lieu
        intl_convert_utf8_to_utf16(&uhaystack, &uhaystack_len, haystack, haystack_len, status);
        if (U_FAILURE(*status)) {
            goto end;
        }
        // TODO: ne convertir que la partie où la recherche a lieu
        intl_convert_utf8_to_utf16(&uneedle, &uneedle_len, needle, needle_len, status);
        if (U_FAILURE(*status)) {
            goto end;
        }
        utf16_normalize(&unormalized_haystack, &unormalized_haystack_len, uhaystack, uhaystack_len, nm, status);
        if (U_FAILURE(*status)) {
            goto end;
        }
        efree(uhaystack); uhaystack = NULL; // we don't need it anymore
        utf16_normalize(&unormalized_needle, &unormalized_needle_len, uneedle, uneedle_len, nm, status);
        if (U_FAILURE(*status)) {
            goto end;
        }
        efree(uneedle); uneedle = NULL; // we don't need it anymore

        if (UCASE_NONE == ct) {
            ucased_haystack = unormalized_haystack;
            ucased_haystack_len = unormalized_haystack_len;
            ucased_needle = unormalized_needle;
            ucased_needle_len = unormalized_needle_len;
        } else {
            utf16_fullcase(&ucased_haystack, &ucased_haystack_len, unormalized_haystack, unormalized_haystack_len, locale, ct, status);
            if (U_FAILURE(*status)) {
                goto end;
            }
            efree(unormalized_haystack); unormalized_haystack = NULL; // we don't need it anymore
            utf16_fullcase(&ucased_needle, &ucased_needle_len, unormalized_needle, unormalized_needle_len, locale, ct, status);
            if (U_FAILURE(*status)) {
                goto end;
            }
            efree(unormalized_needle); unormalized_needle = NULL; // we don't need it anymore
        }
        // "recalculer" le pointeur UTF-16 => UTF-8

end:
        if (NULL != uhaystack) {
            efree(uhaystack);
        }
        if (NULL != uneedle) {
            efree(uneedle);
        }
        if (NULL != unormalized_haystack) {
            efree(unormalized_haystack);
        }
        if (NULL != unormalized_needle) {
            efree(unormalized_needle);
        }
        if (NULL != ucased_haystack && UCASE_NONE != ct) {
            efree(ucased_haystack);
        }
        if (NULL != ucased_needle && UCASE_NONE != ct) {
            efree(ucased_needle);
        }
    }

    return NULL;
}

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
 * - check for status/error before considering return value
 * - check match_length >= 0
 **/
int utf8_region_matches( /* /!\ may be changed in the future /!\ */
    const char *string1, int32_t string1_len, int32_t string1_offset, /* length in CU ; offset in CP */
    const char *string2, int32_t string2_len, int32_t string2_offset, /* length in CU ; offset in CP */
    int32_t match_length,                                             /* length in CP, < 0 for all string (str(?:case)?cmp behavior) */
    const char *locale,
    UNormalizationMode nm, UCaseType ct, /* /!\ may be removed for a default/specific value (ICU ones) /!\ */
    UErrorCode *status
) {
    int ret = 0;
    int32_t string1_start_cu_offset;
    int32_t string2_start_cu_offset;
    int32_t string1_end_cu_offset;
    int32_t string2_end_cu_offset;

    *status = U_ZERO_ERROR;
    string1_start_cu_offset = string2_start_cu_offset = 0;
    if (!utf8_cp_to_cu(string1, string1_len, string1_offset, &string1_start_cu_offset, status)) {
        return 0;
    }
    if (!utf8_cp_to_cu(string2, string2_len, string2_offset, &string2_start_cu_offset, status)) {
        return 0;
    }
    if (match_length < 0) {
        string1_end_cu_offset = string1_len;
        string2_end_cu_offset = string2_len;
    } else {
        string1_end_cu_offset = string1_start_cu_offset;
        U8_FWD_N(string1, string1_end_cu_offset, string1_len, match_length);
        string2_end_cu_offset = string2_start_cu_offset;
        U8_FWD_N(string2, string2_end_cu_offset, string2_len, match_length);
// debug("string1_start_cu_offset = %d, string1_end_cu_offset = %d", string1_start_cu_offset, string1_end_cu_offset);
// debug("string2_start_cu_offset = %d, string2_end_cu_offset = %d", string2_start_cu_offset, string2_end_cu_offset);
    }
    if (UNORM_NONE == nm) {
        char *cased_string1 = NULL;
        int32_t cased_string1_len = 0;
        char *cased_string2 = NULL;
        int32_t cased_string2_len = 0;
        int32_t substring1_len;
        int32_t substring2_len;

        utf8_fullcase(&cased_string1, &cased_string1_len, string1 + string1_start_cu_offset, string1_end_cu_offset - string1_start_cu_offset, locale, ct, status);
        if (U_FAILURE(*status)) {
            if (UCASE_NONE != ct) {
                if (NULL != cased_string1) {
                    efree(cased_string1);
                }
            }
            return 0;
        }
        utf8_fullcase(&cased_string2, &cased_string2_len, string2 + string2_start_cu_offset, string2_end_cu_offset - string2_start_cu_offset, locale, ct, status);
        if (U_FAILURE(*status)) {
            if (UCASE_NONE != ct) {
                if (NULL != cased_string1) {
                    efree(cased_string1);
                }
                if (NULL != cased_string2) {
                    efree(cased_string2);
                }
            }
            return 0;
        }
        substring1_len = utf8_countChar32((const uint8_t *) string1 + string1_start_cu_offset, string1_len - string1_start_cu_offset);
        substring2_len = utf8_countChar32((const uint8_t *) string2 + string2_start_cu_offset, string2_len - string2_start_cu_offset);
        // TODO: binary utf8 comparison
        //ret = memcmp(string1, string2, LENGTH);
        if (UCASE_NONE != ct) {
            if (NULL != cased_string1) {
                efree(cased_string1);
            }
            if (NULL != cased_string2) {
                efree(cased_string2);
            }
        }
    } else {
        // ct (Case Type) et nm (Normalization Mode) ignored
        UChar *usubstring1 = NULL;
        UChar *usubstring2 = NULL;
        int32_t usubstring1_len = 0;
        int32_t usubstring2_len = 0;

// debug("(sub)string1 = %*s (%d)\n", string1_end_cu_offset - string1_start_cu_offset, string1 + string1_start_cu_offset, string1_end_cu_offset - string1_start_cu_offset);
// debug("(sub)string2 = %*s (%d)\n", string2_end_cu_offset - string2_start_cu_offset, string2 + string2_start_cu_offset, string2_end_cu_offset - string2_start_cu_offset);
        // TODO: make a binary comparison first to avoid potential uneeded conversions/normalizations ?
        intl_convert_utf8_to_utf16(&usubstring1, &usubstring1_len, string1 + string1_start_cu_offset, string1_end_cu_offset - string1_start_cu_offset, status);
        if (U_FAILURE(*status)) {
            if (NULL != usubstring1) {
                efree(usubstring1);
            }
            return 0;
        }
        intl_convert_utf8_to_utf16(&usubstring2, &usubstring2_len, string2 + string2_start_cu_offset, string2_end_cu_offset - string2_start_cu_offset, status);
        if (U_FAILURE(*status)) {
            if (NULL != usubstring1) {
                efree(usubstring1);
            }
            if (NULL != usubstring2) {
                efree(usubstring2);
            }
            return 0;
        }
// debug("usubstring1 = %.*S (%d)", usubstring1_len, usubstring1, usubstring1_len);
// debug("usubstring2 = %.*S (%d)", usubstring2_len, usubstring2, usubstring2_len);
        ret = unorm_compare(usubstring1, usubstring1_len, usubstring2, usubstring2_len, UCASE_NONE == ct ? U_FOLD_CASE_DEFAULT : U_COMPARE_IGNORE_CASE, status);
        // we don't need to check *status here: same case
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
void utf8_do_replacement(HashTable *ht, const char *from, int from_len, char **to, int *to_len)
{
    //
}
#endif

void utf8_replace_len_from_utf16(
    char **string, int *string_len,
    char *replacement, int replacement_len,
    UChar *ustring, int32_t utf16_cu_start_match_offset, int32_t utf16_cu_length,
    int32_t utf16_cp_length,
    ReplacementDirection direction
) {
    int i;
    int32_t diff_len;
    int32_t utf8_match_cu_length = 0;
    int32_t utf8_cu_start_match_offset = 0;
    int32_t utf16_cp_start_match_offset = u_countChar32(ustring, utf16_cu_start_match_offset);

    if (0 == utf16_cu_length) {
        utf8_match_cu_length = 0;
    } else {
        int32_t utf16_cu_end_match_offset = utf16_cu_start_match_offset + utf16_cu_length;

        for (i = utf16_cu_start_match_offset; i < utf16_cu_end_match_offset; i++) {
            UChar32 c;

            U16_NEXT(ustring, utf16_cu_start_match_offset, utf16_cu_end_match_offset, c);
            utf8_match_cu_length += U8_LENGTH(c); // TODO: handle surrogates(forbidden)?
        }
    }
    /* <NOTE> */
    /**
     * String is altered from start to end
     * But a replacement of a different length, create a gap. We should consider it, when there is multiple replacements
     * So, we should consider the part of string we haven't yet modified: we should recalculate offsets from end of string
     * instead its start
     * (there is many way: but it'll introduce, at least, a new parameter: an int or a pointer - last one may be "dangerous" because of realloc)
     **/
    if (REPLACE_REVERSE == direction) {
        utf8_cu_start_match_offset = 0;
        U8_FWD_N(*string, utf8_cu_start_match_offset, *string_len, utf16_cp_start_match_offset);
    } else {
        utf8_cu_start_match_offset = *string_len;
        U8_BACK_N(*string, 0, utf8_cu_start_match_offset, utf16_cp_length - utf16_cp_start_match_offset);
    }
    /* </NOTE> */
    diff_len = replacement_len - utf8_match_cu_length;
    if (diff_len > 0) {
        *string = mem_renew(*string, **string, *string_len + diff_len + 1); // reference may no longer be valid from this point
    }
    if (replacement_len != utf8_match_cu_length) {
        memmove(*string + utf8_cu_start_match_offset + utf8_match_cu_length + diff_len, *string + utf8_cu_start_match_offset + utf8_match_cu_length, *string_len - utf8_cu_start_match_offset - utf8_match_cu_length);
    }
    memcpy(*string + utf8_cu_start_match_offset, replacement, replacement_len);
    *string_len += diff_len;
}
