#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <unicode/ubrk.h>
#include <unicode/uset.h>
#include <unicode/unorm.h>

#include "php.h"
#include "../../standard/php_rand.h"
#include "error.h"
#include "intl_data.h"
#include "intl_convert.h"
#include "utf8.h"
#include "utf16.h"
#include "string.h"
#include "../../standard/php_smart_str.h"

#define CHECK_STATUS(status, msg)                           \
    intl_error_set_code(NULL, status TSRMLS_CC);            \
    if (U_FAILURE(status)) {                                \
        intl_errors_set_custom_msg(NULL, msg, 0 TSRMLS_CC); \
        RETVAL_FALSE;                                       \
        goto end;                                           \
    }

#define UTF8_TO_UTF16(status, to, to_len, from, from_len)                        \
    do {                                                                         \
        to = NULL;                                                               \
        to_len = 0;                                                              \
        intl_convert_utf8_to_utf16(&to, &to_len, from, from_len, &status);       \
        CHECK_STATUS(status, "String conversion of " #from " to UTF-16 failed"); \
    } while (0);

#define UTF16_TO_UTF8(status, to, to_len, from, from_len)                       \
    do {                                                                        \
        to = NULL;                                                              \
        to_len = 0;                                                             \
        intl_convert_utf16_to_utf8(&to, &to_len, from, from_len, &status);      \
        CHECK_STATUS(status, "String conversion of " #from " to UTF-8 failed"); \
    } while (0);

PHP_FUNCTION(utf8_split)
{
    char *string = NULL;
    int string_len = 0;
    long length = 1;
    int32_t last = 0, cu_offset = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &string, &string_len, &length)) {
        return;
    }
    if (length < 1) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length of each segment must be greater than zero");
        RETURN_FALSE;
    }
    array_init(return_value);
    while (cu_offset < string_len) {
        U8_FWD_N(string, cu_offset, string_len, length);
        add_next_index_stringl(return_value, string + last, cu_offset - last, TRUE);
        last = cu_offset;
    }
}

PHP_FUNCTION(utf8_count_chars) // TODO: tests
{
    enum {
        ARRAY_ALL_FREQ      = 0, // ask too much memory
        ARRAY_NONNULL_FREQ  = 1,
        ARRAY_NULL_FREQ     = 2, // ask too much memory
        STRING_NONNULL_FREQ = 3,
        STRING_NULL_FREQ    = 4,
        _LAST_MODE
    };

    UChar32 c;
    long mode = ARRAY_ALL_FREQ;
    char *string = NULL;
    int string_len = 0;
    int32_t cu_offset = 0;
    USet *set;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &string, &string_len, &mode)) {
        RETURN_FALSE;
    }
    if (mode < 0 || mode >= _LAST_MODE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown mode");
        RETURN_FALSE;
    }
    set = uset_openEmpty();
    if (mode < STRING_NONNULL_FREQ) {
        array_init(return_value);
    }
    while (cu_offset < string_len) {
        U8_NEXT(string, cu_offset, string_len, c);
        switch (mode) {
            case ARRAY_ALL_FREQ:
                uset_add(set, c);
                /* NO BREAK !!! */
            case ARRAY_NONNULL_FREQ:
            {
                zval **tmp;
                if (zend_hash_index_find(Z_ARRVAL_P(return_value), c, (void **) &tmp) == FAILURE) {
                    zval *data;
                    MAKE_STD_ZVAL(data);
                    ZVAL_LONG(data, 1);
                    zend_hash_index_update(Z_ARRVAL_P(return_value), c, &data, sizeof(data), NULL);
                } else {
                    Z_LVAL_PP(tmp)++;
                }
                break;
            }
            default:
                uset_add(set, c);
        }
    }
    if (ARRAY_ALL_FREQ == mode || ARRAY_NULL_FREQ == mode || STRING_NONNULL_FREQ == mode) {
        uset_complement(set);
    }
    switch (mode) {
        case ARRAY_NONNULL_FREQ:
            // NOP : array already contains wished datas
            break;
        case ARRAY_ALL_FREQ:  // add to array complemented set
        case ARRAY_NULL_FREQ: // build array from set
        {
            int32_t i, size;

            /* Ask lots of memory */
            size = uset_size(set);
            for (i = 0; i < size; i++) {
                c = uset_charAt(set, i);
                add_index_long(return_value, c, 0);
            }

            break;
        }
        case STRING_NULL_FREQ:    // as a set/class
        case STRING_NONNULL_FREQ: // same remark
        {
            char *result;
            int32_t result_len;
            UChar *uresult = NULL;
            int32_t uresult_len = 0;
            UErrorCode status = U_ZERO_ERROR;

            uresult_len = uset_toPattern(set, NULL, 0, TRUE, &status);
            if (U_BUFFER_OVERFLOW_ERROR != status) {
                RETVAL_FALSE;
                break;
            }
            status = U_ZERO_ERROR;
            uresult = mem_new_n(*uresult, uresult_len + 1);
            /*uresult_len = */uset_toPattern(set, uresult, uresult_len, TRUE, &status);
            if (U_FAILURE(status)) {
                efree(uresult);
                RETVAL_FALSE;
                break;
            }
            intl_convert_utf16_to_utf8(&result, &result_len, uresult, uresult_len, &status);
            if (U_FAILURE(status)) {
                efree(uresult);
                RETVAL_FALSE;
                break;
            }
            efree(uresult);
            RETVAL_STRINGL(result, result_len, FALSE);
            break;
        }
    }

    uset_close(set);
}

PHP_FUNCTION(utf8_slice)
{
    char *string = NULL;
    int string_len = 0;
    long cp_length = 0, cp_start = 0;
    long cu_end = 0, cu_from = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|l", &string, &string_len, &cp_start, &cp_length) == FAILURE) {
        return;
    }

    if (cp_start > 0) {
        U8_FWD_N(string, cu_from, string_len, cp_start);
    } else if (cp_start < 0) {
        cu_from = string_len;
        U8_BACK_N((const uint8_t *) string, 0, cu_from, -cp_start);
    }
    if (ZEND_NUM_ARGS() <= 2) {
        cu_end = string_len;
    } else {
        if (cp_length > 0) {
            cu_end = cu_from;
            U8_FWD_N(string, cu_end, string_len, cp_length);
        } else if (cp_length < 0) {
            cu_end = string_len;
            U8_BACK_N((const uint8_t *) string, 0, cu_end, -cp_length);
        }
    }

    if (cu_end > cu_from) {
        RETURN_STRINGL(string + cu_from, cu_end - cu_from, TRUE);
    } else {
        RETURN_EMPTY_STRING();
    }
}

PHP_FUNCTION(utf8_len)
{
    char *string = NULL;
    int string_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &string, &string_len)) {
        return;
    }
    RETVAL_LONG((long) utf8_countChar32((const uint8_t *) string, string_len));
}

PHP_FUNCTION(utf8_ord)
{
    UChar32 c = 0;
    char *string = NULL;
    int string_len = 0;
    long cp_offset = 0;
    int32_t cu_offset = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &string, &string_len, &cp_offset)) {
        return;
    }
    UTF8_CP_TO_CU(string, string_len, cp_offset, cu_offset);
    U8_NEXT(string, cu_offset, string_len, c);
    RETVAL_LONG((long) c);

    if (FALSE) {
end:
        RETVAL_LONG(0);
    }
}

PHP_FUNCTION(utf8_count_words) // TODO: tests
{
    enum {
        COUNT_ONLY    = 0,
        INDEXED_WORDS = 1,
        OFFSET_WORDS  = 2
    };

    zval *ret = NULL;
    char *string = NULL;
    int string_len = 0;
    char *locale = NULL;
    int locale_len = 0;
    UChar *ustring = NULL;
    int32_t ustring_len = 0;
    UBreakIterator *brk = NULL;
    long format = COUNT_ONLY;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|sl", &string, &string_len, &locale, &locale_len, &format)) {
        RETURN_FALSE;
    }
    if (0 == locale_len) {
        locale = INTL_G(default_locale);
    }
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    brk = ubrk_open(UBRK_WORD, locale, ustring, ustring_len, &status);
    if (!intl_error_non_quiet_set_code(status TSRMLS_CC)) {
        goto end;
    }
    if (COUNT_ONLY == format) {
        long words = 0;

        if (UBRK_DONE != ubrk_first(brk)) {
            do {
                if (UBRK_WORD_NONE != ubrk_getRuleStatus(brk)) {
                    words++;
                }
            } while (UBRK_DONE != ubrk_next(brk));
        }
        RETVAL_LONG(words);
    } else {
        char *result;
        int result_len;
        int32_t l = 0, u;

        ALLOC_ZVAL(ret);
        array_init(ret);
        if (UBRK_DONE != (u = ubrk_first(brk))) {
            do {
                if (UBRK_WORD_NONE != ubrk_getRuleStatus(brk)) {
                    UTF16_TO_UTF8(status, result, result_len, ustring + l, u - l);
                    if (INDEXED_WORDS == format) {
                        add_next_index_stringl(ret, result, result_len, FALSE);
                    } else {
                        add_index_stringl(ret, u_countChar32(ustring, l), result, result_len, FALSE);
                    }
                }
                l = u;
            } while (UBRK_DONE != (u = ubrk_next(brk)));
        }
        RETVAL_ZVAL(ret, 0, 1);
    }

    if (FALSE) {
end:
        if (NULL != ret) {
            zval_dtor(ret);
        }
        RETVAL_FALSE;
    }
    if (NULL != ustring) {
        efree(ustring);
    }
    if (NULL != brk) {
        ubrk_close(brk);
    }
}

PHP_FUNCTION(utf8_chr)
{
    long cp;
    char *s;
    int32_t s_len, i = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &cp)) {
        return;
    }
    s_len = U8_LENGTH(cp);
    s = mem_new_n(*s, s_len + 1);
    U8_APPEND_UNSAFE(s, i, cp);
    s[s_len] = '\0';

    RETURN_STRINGL(s, s_len, FALSE);
}

static void fullcasemapping(INTERNAL_FUNCTION_PARAMETERS, UCaseType ct)
{
    UErrorCode status;
    int locale_len = 0;
    char *locale = NULL;
    int string_len = 0;
    char *string = NULL;
    int32_t result_len = 0;
    char *result = NULL;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &string, &string_len, &locale, &locale_len)) {
        return;
    }
    if (0 == string_len) {
        RETURN_EMPTY_STRING();
    }
    if (0 == locale_len) {
        locale = INTL_G(default_locale);
    }
    status = U_ZERO_ERROR;
    utf8_fullcase(&result, &result_len, string, string_len, locale, ct, &status);
    CHECK_STATUS(status, "full case mapping failed");
    RETVAL_STRINGL(result, result_len, FALSE);

    if (FALSE) {
end:
        if (NULL != result) {
            efree(result);
        }
        RETVAL_FALSE;
    }
}

PHP_FUNCTION(utf8_toupper)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, UCASE_UPPER);
}

PHP_FUNCTION(utf8_tolower)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, UCASE_LOWER);
}

PHP_FUNCTION(utf8_totitle)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, UCASE_TITLE);
}

static void cmp(INTERNAL_FUNCTION_PARAMETERS, int ignore_case, int limited_length)
{
    int ret;
    char *string1 = NULL;
    int string1_len = 0;
    char *string2 = NULL;
    int string2_len = 0;
    int length = -1;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
    if (limited_length) {
        if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl", &string1, &string1_len, &string2, &string2_len, &length)) {
            return;
        }
        if (length < 0) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length must be greater than or equal to 0");
            RETURN_FALSE;
        }
    } else {
        if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &string1, &string1_len, &string2, &string2_len)) {
            return;
        }
    }
    ret = utf8_region_matches(
        string1, string1_len, 0,
        string2, string2_len, 0,
        length,
        "" /* locale */, UNORM_NFD /* unused */, ignore_case ? UCASE_FOLD : UCASE_NONE /* unused */,
        &status
    );
    intl_error_set_code(NULL, status TSRMLS_CC);
    if (U_FAILURE(status)) {
        RETURN_FALSE;
    } else {
        RETURN_LONG((long) ret);
    }
}

PHP_FUNCTION(utf8_casecmp)
{
    cmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, FALSE);
}

PHP_FUNCTION(utf8_ncasecmp) // TODO: tests
{
    cmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, TRUE);
}

PHP_FUNCTION(utf8_ncmp)
{
    cmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, TRUE);
}

PHP_FUNCTION(utf8_slice_cmp)
{
    int ret;
    char *string = NULL;
    int string_len = 0;
    long string_cp_offset = 0;
    char *substring = NULL;
    int substring_len = 0;
    long substring_cp_offset = 0;
    long cp_length = -1;
    int32_t string_cp_count;
    int32_t substring_cp_count;
    zend_bool ignore_case = FALSE;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sls|llb", &string, &string_len, &string_cp_offset, &substring, &substring_len, &substring_cp_offset, &cp_length, &ignore_case)) {
        return;
    }
    if (cp_length < 0) {
        string_cp_count = utf8_countChar32((const uint8_t *) string, string_len);
        substring_cp_count = utf8_countChar32((const uint8_t *) substring, substring_len);
        cp_length = MIN(string_cp_count, substring_cp_count);
    }
    ret = utf8_region_matches(
        string, string_len, string_cp_offset,
        substring, substring_len, substring_cp_offset,
        cp_length,
        "" /* locale */, UNORM_NFD /* unused */, ignore_case ? UCASE_FOLD : UCASE_NONE /* unused */,
        &status
    );
    intl_error_set_code(NULL, status TSRMLS_CC);
    if (U_FAILURE(status)) {
        RETURN_FALSE;
    } else {
        RETURN_LONG((long) ret);
    }
}

PHP_FUNCTION(utf8_startswith) // TODO: tests
{
    int ret;
    char *string = NULL;
    int string_len = 0;
    char *substring = NULL;
    int substring_len = 0;
    int32_t string_cp_count;
    int32_t substring_cp_count;
    zend_bool ignore_case = FALSE;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &string, &string_len, &substring, &substring_len, &ignore_case)) {
        return;
    }
    string_cp_count = utf8_countChar32((const uint8_t *) string, string_len);
    substring_cp_count = utf8_countChar32((const uint8_t *) substring, substring_len);
    if (substring_cp_count > string_cp_count) {
        RETURN_FALSE;
    }
    ret = utf8_region_matches(
        string, string_len, 0,
        substring, substring_len, 0,
        substring_cp_count,
        "" /* locale */, UNORM_NFD /* unused */, ignore_case ? UCASE_FOLD : UCASE_NONE /* unused */,
        &status
    );
    intl_error_set_code(NULL, status TSRMLS_CC);
    if (U_FAILURE(status)) {
        RETURN_FALSE;
    } else {
        RETURN_BOOL(0 == ret);
    }
}

PHP_FUNCTION(utf8_endswith) // TODO: tests
{
    int ret;
    char *string = NULL;
    int string_len = 0;
    char *substring = NULL;
    int substring_len = 0;
    int32_t string_cp_count;
    int32_t substring_cp_count;
    zend_bool ignore_case = FALSE;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &string, &string_len, &substring, &substring_len, &ignore_case)) {
        return;
    }
    string_cp_count = utf8_countChar32((const uint8_t *) string, string_len);
    substring_cp_count = utf8_countChar32((const uint8_t *) substring, substring_len);
    if (substring_cp_count > string_cp_count) {
        RETURN_FALSE;
    }
    ret = utf8_region_matches(
        string, string_len, -substring_cp_count,
        substring, substring_len, 0,
        substring_cp_count,
        "" /* locale */, UNORM_NFD /* unused */, ignore_case ? UCASE_FOLD : UCASE_NONE /* unused */,
        &status
    );
    intl_error_set_code(NULL, status TSRMLS_CC);
    if (U_FAILURE(status)) {
        RETURN_FALSE;
    } else {
        RETURN_BOOL(0 == ret);
    }
}

PHP_FUNCTION(utf8_reverse)
{
    char *string = NULL;
    int string_len = 0;
    char *result, *r;
    int32_t last, cu_offset;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &string, &string_len)) {
        return;
    }
    result = mem_new_n(*result, string_len + 1);
    last = cu_offset = string_len;
    r = result;
    while (cu_offset > 0) {
        U8_BACK_1((const uint8_t *) string, 0, cu_offset);
        memcpy(r, string + cu_offset, last - cu_offset);
        r += last - cu_offset;
        last = cu_offset;
    }
    result[string_len] = '\0';

    RETVAL_STRINGL(result, string_len, FALSE);
}

#if 1
static inline char *memrnstr(char *haystack, char *needle, int needle_len, char *end)
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

static void find_case_sensitive(INTERNAL_FUNCTION_PARAMETERS, int last, int want_only_pos)
{
    char *haystack = NULL;
    int haystack_len = 0;
    zval *needle = NULL;
    zend_bool before = FALSE;
    long start_cp = 0;
    int32_t start_cu = 0;
    char *found = NULL;
    char cus[U8_MAX_LENGTH + 1] = { 0 };
    int cus_length = 0;

    if (want_only_pos) {
        if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|l", &haystack, &haystack_len, &needle, &start_cp)) {
            return;
        }
    } else {
        if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|lb", &haystack, &haystack_len, &needle, &start_cp, &before)) {
            return;
        }
    }
    UTF8_CP_TO_CU(haystack, haystack_len, start_cp, start_cu);
    if (IS_STRING == Z_TYPE_P(needle)) {
        if (!Z_STRLEN_P(needle)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty delimiter");
            goto end;
        }
        if (!last) {
            found = php_memnstr(haystack + start_cu, Z_STRVAL_P(needle), Z_STRLEN_P(needle), haystack + haystack_len);
        } else {
            if (start_cp >= 0) {
                found = memrnstr(haystack + start_cu, Z_STRVAL_P(needle), Z_STRLEN_P(needle), haystack + haystack_len);
            } else {
                found = memrnstr(haystack, Z_STRVAL_P(needle), Z_STRLEN_P(needle), haystack + start_cu);
            }
        }
    } else { // we search a code point (convert needle into long)
        UChar32 c;

        if (SUCCESS != unicode_convert_needle_to_cp(needle, &c TSRMLS_CC)) {
            goto end;
        }
        U8_APPEND_UNSAFE(cus, cus_length, c);
        if (!last) {
            found = php_memnstr(haystack + start_cu, cus, cus_length, haystack + haystack_len);
        } else {
            if (start_cp >= 0) {
                found = memrnstr(haystack + start_cu, cus, cus_length, haystack + haystack_len);
            } else {
                found = memrnstr(haystack, cus, cus_length, haystack + start_cu);
            }
        }
    }
    if (NULL != found) {
        if (want_only_pos) {
            RETURN_LONG((long) utf8_countChar32((const uint8_t *) haystack, found - haystack));
        } else {
            if (before) {
                RETURN_STRINGL(haystack, found - haystack, TRUE);
            } else {
                RETURN_STRINGL(found, haystack_len - (found - haystack), TRUE);
            }
        }
    }

end:
    if (want_only_pos) {
        RETVAL_LONG((long) -1);
    } else {
        RETVAL_FALSE;
    }
}

// TODO: rename firstpos/lastpos to firstIndex/lastIndex ?
PHP_FUNCTION(utf8_firstsub) // TODO: tests
{
    find_case_sensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, FALSE);
}

PHP_FUNCTION(utf8_firstpos) // TODO: tests
{
    find_case_sensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, TRUE);
}

PHP_FUNCTION(utf8_lastsub) // TODO: tests
{
    find_case_sensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, FALSE);
}

PHP_FUNCTION(utf8_lastpos)
{
    find_case_sensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, TRUE);
}
#else
static void utf8_index(INTERNAL_FUNCTION_PARAMETERS, int search_first/*, int want_only_pos*/)
{
    char *found;
    char *haystack = NULL;
    int haystack_len = 0;
    zval *zneedle = NULL;
    char *needle = NULL;
    int needle_len = 0;
    int start_cp_offset = 0;
    UErrorCode status;
    zend_bool ignore_case = FALSE;
    char cus[U8_MAX_LENGTH + 1] = { 0 };
    int cus_length = 0;

    intl_error_reset(NULL TSRMLS_CC);
    // if (want_only_pos)
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|lb", &haystack, &haystack_len, &zneedle, &start_cp_offset, &ignore_case)) {
        return;
    }
    if (IS_STRING == Z_TYPE_P(zneedle)) {
        if (!Z_STRLEN_P(zneedle)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty delimiter");
            RETURN_FALSE;
        }
        needle = Z_STRVAL_P(zneedle);
        needle_len = Z_STRLEN_P(zneedle);
    } else {
        UChar32 c;

        if (SUCCESS != unicode_convert_needle_to_cp(zneedle, &c TSRMLS_CC)) {
            RETURN_FALSE;
        }
        U8_APPEND_UNSAFE(cus, cus_length, c);
        needle = cus;
        needle_len = cus_length;
    }
    found = utf8_find(
        haystack, haystack_len,
        needle, needle_len,
        start_cp_offset, locale, search_first,
        UNORM_NFD /* unused */, ignore_case ? UCASE_FOLD : UCASE_NONE /* unused */,
        &status
    );
    intl_error_set_code(NULL, status TSRMLS_CC);
    if (U_FAILURE(status)) {
        RETURN_FALSE;
    }
    // if (want_only_pos)
    if (NULL == found) {
        RETURN_LONG((long) -1);
    } else {
        RETURN_LONG((long) utf8_countChar32(haystack, found - haystack));
    }
}

PHP_FUNCTION(utf8_rindex)
{
    utf8_index(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE);
}

PHP_FUNCTION(utf8_lindex)
{
    utf8_index(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE);
}

PHP_FUNCTION(utf8_lfind) // szlbb
{
    //
}

PHP_FUNCTION(utf8_rfind)
{
    //
}
#endif

PHP_FUNCTION(utf8_tr)
{
    HashTable map;
    char *string = NULL;
    int string_len = 0;
    char *from = NULL;
    int from_len = 0;
    char *to = NULL;
    int to_len = 0;
    smart_str result = { 0 };
    UChar32 c;
    int32_t f; // to iterate on from's CU/CP
    int32_t t; // to iterate on to's CU/CP
    int32_t s; // to iterate on string's CU/CP
    int32_t p; // to keep start offset of the previous CP
    U8ReplacementCharData *rep;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &string, &string_len, &from, &from_len, &to, &to_len)) {
        return;
    }
    zend_hash_init(&map, utf8_countChar32((const uint8_t *) from, from_len), NULL, NULL, FALSE);
    for (f = p = t = 0; f < from_len && t < to_len; /* NOP */) {
        U8_NEXT(from, f, from_len, c);
        U8_FWD_1(to, t, to_len);
        utf8_add_cp_replacement(&map, c, to + p, t - p);
        p = t;
    }
    for (s = p = 0; s < string_len; /* NOP */) {
        U8_NEXT(string, s, string_len, c);

        if (SUCCESS == zend_hash_index_find(&map, c, (void **) &rep)) {
            smart_str_appendl(&result, rep->cus, rep->cus_length);
        } else {
            smart_str_appendl(&result, string + p, s - p);
        }
        p = s;
    }
    zend_hash_destroy(&map);

    smart_str_0(&result);
    RETVAL_STRINGL(result.c, result.len, FALSE);
}

enum {
    TRIM_LEFT  = 1,
    TRIM_RIGHT = 2,
    TRIM_BOTH  = 3
};

static void trim(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
    int32_t i, k;
    UChar32 c = 0;
    int32_t start = 0, end;
    char *string = NULL;
    int string_len = 0;
    char *what = NULL;
    int what_len = 0;
    HashTable filter;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &string, &string_len, &what, &what_len)) {
        return;
    }
    if (NULL != what) {
        void *dummy = (void *) 1;

        zend_hash_init(&filter, utf8_countChar32((const uint8_t *) what, what_len), NULL, NULL, FALSE);
        for (i = 0; i < what_len; /* NOP */) {
            U8_NEXT(what, i, what_len, c);
            zend_hash_index_update(&filter, c, &dummy, sizeof(void *), NULL);
        }
    }
    end = string_len;
    if (mode & TRIM_LEFT) {
        for (i = k = 0 ; i < end ; /* NOP */) {
            U8_NEXT(string, k, end, c);
            if (NULL == what) {
                if (FALSE == u_isWhitespace(c)) {
                    break;
                }
            } else {
                if (!zend_hash_index_exists(&filter, c)) {
                    break;
                }
            }
            i = k;
        }
        start = i;
    }
    if (mode & TRIM_RIGHT) {
        for (i = k = end ; i > start ; /* NOP */) {
            U8_PREV(string, 0, k, c);
            if (NULL == what) {
                if (FALSE == u_isWhitespace(c)) {
                    break;
                }
            } else {
                if (!zend_hash_index_exists(&filter, c)) {
                    break;
                }
            }
            i = k;
        }
        end = i;
    }
    if (NULL != what) {
        zend_hash_destroy(&filter);
    }
    if (start < string_len) {
        RETURN_STRINGL(string + start, end - start, TRUE);
    } else {
        RETURN_EMPTY_STRING();
    }
}

PHP_FUNCTION(utf8_rtrim) // TODO: tests
{
    trim(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRIM_RIGHT);
}

PHP_FUNCTION(utf8_ltrim) // TODO: tests
{
    trim(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRIM_LEFT);
}

PHP_FUNCTION(utf8_trim) // TODO: tests
{
    trim(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRIM_BOTH);
}

PHP_FUNCTION(utf8_shuffle)
{
    UChar32 c;
    int i, cp;
    UChar32 *chars;
    char *result;
    long rnd_idx;
    int32_t cp_count;
    char *string = NULL;
    int string_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &string, &string_len)) {
        return;
    }
    cp_count = utf8_countChar32((const uint8_t *) string, string_len);
    if (cp_count <= 1) {
        RETURN_STRINGL(string, string_len, TRUE);
    }
    chars = mem_new_n(*chars, cp_count);
    result = mem_new_n(*result, string_len + 1);
    for (i = cp = 0; i < string_len; cp++) {
        U8_NEXT(string, i, string_len, c);
        chars[cp] = c;
    }
    for (cp = cp_count; --cp; /* NOP */) {
        rnd_idx = php_rand(TSRMLS_C);
        RAND_RANGE(rnd_idx, 0, cp, PHP_RAND_MAX);
        if (cp != rnd_idx) {
            c = chars[cp];
            chars[cp] = chars[rnd_idx];
            chars[rnd_idx] = c;
        }
    }
    for (i = cp = 0; cp < cp_count; cp++) {
        U8_APPEND_UNSAFE(result, i, chars[cp]);
    }
    result[string_len] = '\0';
    efree(chars);

    RETVAL_STRINGL(result, string_len, FALSE);
}

PHP_FUNCTION(utf8_validate) // TODO: tests
{
    UBool ret = FALSE;
    char *string = NULL;
    int string_len = 0;
    zend_bool quiet = TRUE;
    UErrorCode status = U_ZERO_ERROR;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &string, &string_len, &quiet)) {
        return;
    }
    intl_error_reset(NULL TSRMLS_CC);
    ret = utf8_validate((const uint8_t *) string, string_len, &status);
    if (quiet) {
        intl_error_set_code(NULL, status TSRMLS_CC);
    } else {
        intl_error_non_quiet_set_code(status TSRMLS_CC);
    }

    RETURN_BOOL(ret);
}

PHP_FUNCTION(utf8_slice_count)
{
    char *haystack = NULL;
    int haystack_len = 0;
    zval *zneedle = NULL;
    char *needle = NULL;
    int needle_len = 0;
    long cp_start = 0;
    int32_t cu_start = 0;
    long cp_length = 0;
    int count = 0;
    char *end, *p;
    char cus[U8_MAX_LENGTH + 1] = { 0 };
    int cus_length = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|ll", &haystack, &haystack_len, &zneedle, &cp_start, &cp_length)) {
        return;
    }
    if (IS_STRING == Z_TYPE_P(zneedle)) {
        if (0 == Z_STRLEN_P(zneedle)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty substring");
            RETURN_FALSE;
        }
        needle = Z_STRVAL_P(zneedle);
        needle_len = Z_STRLEN_P(zneedle);
    } else {
        UChar32 c;

        if (SUCCESS != unicode_convert_needle_to_cp(zneedle, &c TSRMLS_CC)) {
            RETURN_FALSE;
        }
        U8_APPEND_UNSAFE(cus, cus_length, c);
        needle = cus;
        needle_len = cus_length;
    }
    if (cp_start > 0) {
        U8_FWD_N(haystack, cu_start, haystack_len, cp_start);
    } else if (cp_start < 0) {
        cu_start = haystack_len;
        U8_BACK_N((const uint8_t *) haystack, 0, cu_start, -cp_start);
    }
    if (4 == ZEND_NUM_ARGS()) {
        int32_t cu_end = 0;

        if (cp_length > 0) {
            cu_end = cu_start;
            U8_FWD_N(haystack, cu_end, haystack_len, cp_length);
        } else if (cp_length < 0) {
            cu_end = haystack_len;
            U8_BACK_N((const uint8_t *) haystack, 0, cu_end, -cp_length);
        }
        end = haystack + cu_end;
        if (end < haystack + cu_start) {
            RETURN_FALSE;
        }
    } else {
        end = haystack + haystack_len;
    }
    p = haystack + cu_start;
    if (1 == needle_len) {
        char cmp = needle[0];

        while ((p = memchr(p, cmp, end - p))) {
            count++;
            p++;
        }
    } else {
        while (NULL != (p = php_memnstr(p, needle, needle_len, end))) {
            p += needle_len;
            count++;
        }
    }

    RETURN_LONG((long) count);
}

PHP_FUNCTION(utf8_slice_replace)
{
    char *string = NULL;
    int string_len = 0;
    char *replacement = NULL;
    int replacement_len = 0;
    long cp_length = 0;
    int32_t cu_end = 0;
    long cp_from = 0;
    int32_t cu_from = 0;
    char *result = NULL;
    int result_len = 0;
    int replaced_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl|l", &string, &string_len, &replacement, &replacement_len, &cp_from, &cp_length)) {
        return;
    }
    if (cp_from > 0) {
        U8_FWD_N(string, cu_from, string_len, cp_from);
    } else if (cp_from < 0) {
        cu_from = string_len;
        U8_BACK_N((const uint8_t *) string, 0, cu_from, -cp_from);
    }
    if (4 == ZEND_NUM_ARGS()) {
        if (cp_length > 0) {
            cu_end = cu_from;
            U8_FWD_N(string, cu_end, string_len, cp_length);
        } else if (cp_length < 0) {
            cu_end = string_len;
            U8_BACK_N((const uint8_t *) string, 0, cu_end, -cp_length);
        }
    } else {
        cu_end = string_len;
    }
    if (cu_from > cu_end) {
        RETURN_FALSE;
    }
    replaced_len = cu_end - cu_from;
    result_len = string_len + replacement_len - replaced_len;
    result = mem_new_n(*result, result_len + 1);

    memcpy(result, string, cu_from);
    memcpy(result + cu_from, replacement, replacement_len);
    memcpy(result + cu_from + replacement_len, string + cu_from + replaced_len, string_len - cu_from - replaced_len);
    result[result_len] = '\0';

    RETVAL_STRINGL(result, result_len, FALSE);
}

#include <unicode/ucol.h>
#include <unicode/usearch.h>

/**
 * TODO: move to Collator (as Collator::replace) (and remove ucol + locale variables and associated)
 * - locale were already handled
 * - permits all UCollator options (strength, attributes, etc)
 **/
PHP_FUNCTION(utf8_ireplace)
{
    char *result = NULL;
    int result_len = 0;
    char *locale = NULL;
    int locale_len = 0;
    char *search = NULL;
    int search_len = 0;
    char *replace = NULL;
    int replace_len = 0;
    char *subject = NULL;
    int subject_len = 0;
    zval *zcount = NULL;
    long count = 0;
    UChar *usearch = NULL;
    int32_t usearch_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    int32_t usubject_cp_len = 0;
    UStringSearch *uss = NULL;
    UCollator *ucol = NULL;
    UErrorCode status = U_ZERO_ERROR;
    int32_t l;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|sz", &subject, &subject_len, &search, &search_len, &replace, &replace_len, &locale, &locale_len, &zcount)) {
        return;
    }
    UTF8_TO_UTF16(status, usearch, usearch_len, search, search_len);
    UTF8_TO_UTF16(status, usubject, usubject_len, subject, subject_len);
    if (0 == locale_len) {
        locale = INTL_G(default_locale);
    }
    ucol = ucol_open(locale, &status);
    if (!intl_error_non_quiet_set_code(status TSRMLS_CC)) {
        goto end;
    }
    ucol_setStrength(ucol, UCOL_PRIMARY); // it's just for testing
    uss = usearch_openFromCollator(usearch, usearch_len, usubject, usubject_len, ucol, NULL, &status);
    if (!intl_error_non_quiet_set_code(status TSRMLS_CC)) {
        goto end;
    }
    usubject_cp_len = u_countChar32(usubject, usubject_len);
    result = estrndup(subject, result_len = subject_len);
    for (l = usearch_first(uss, &status); U_SUCCESS(status) && USEARCH_DONE != l; l = usearch_next(uss, &status), count++) {
        utf8_replace_len_from_utf16(&result, &result_len, replace, replace_len, usubject, l, usearch_getMatchedLength(uss), usubject_cp_len, REPLACE_FORWARD);
    }
    if (!intl_error_non_quiet_set_code(status TSRMLS_CC)) {
        goto end;
    }
    result[result_len] = '\0';
    RETVAL_STRINGL(result, result_len, FALSE);

    if (FALSE) {
end:
        if (NULL != result) {
            efree(result);
        }
        RETVAL_FALSE;
    }
    if (NULL != usearch) {
        efree(usearch);
    }
    if (NULL != usubject) {
        efree(usubject);
    }
    if (NULL != usearch) {
        usearch_close(uss);
    }
    if (NULL != ucol) {
        ucol_close(ucol);
    }
    if (ZEND_NUM_ARGS() > 4) {
        zval_dtor(zcount);
        ZVAL_LONG(zcount, count);
    }
}

#include "SAPI.h"

#define MAY_BE_TRIMMED(c) \
    (u_isspace(c) || U_CONTROL_CHAR == u_charType(c) || U_NON_SPACING_MARK == u_charType(c))

PHP_FUNCTION(utf8_wordwrap) // TODO: tests
{
    char *string = NULL;
    int string_len = 0;
    char *replace = NULL;
    int replace_len = 0;
    char *locale = NULL;
    int locale_len = 0;
    long width = 75;
    UChar *ustring = NULL;
    int32_t ustring_len = 0;
    int32_t ustring_cp_len = 0;
    zend_bool cut = FALSE;
    UErrorCode status = U_ZERO_ERROR;
    UBreakIterator *brk = NULL;
    char *result = NULL;
    int result_len = 0;
    int32_t i, l = 0, u = 0;
    int32_t current_line_cu_offset = 0, current_line_cp_length = 0;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lssb", &string, &string_len, &width, &locale, &locale_len, &replace, &replace_len, &cut)) {
        return;
    }
    if (0 == string_len) {
        RETURN_EMPTY_STRING();
    }
    if (width <= 0) {
        intl_errors_set_custom_msg(NULL, "length of segment must be greater than 0", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (0 == locale_len) {
        locale = INTL_G(default_locale);
    }
    if (0 == replace_len) {
        if (!strcmp(sapi_module.name, "cli")) {
            replace = "\n";
            replace_len = sizeof("\n") - 1;
        } else {
            replace = "<br />\n";
            replace_len = sizeof("<br />\n") - 1;
        }
    }
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    ustring_cp_len = u_countChar32(ustring, ustring_len);
    result = estrndup(string, result_len = string_len);
    brk = ubrk_open(UBRK_LINE, locale, ustring, ustring_len, &status);
    if (!intl_error_non_quiet_set_code(status TSRMLS_CC)) {
        goto end;
    }
    {
        UChar32 c;
        int32_t utf16_cu_offset_l, utf16_cu_offset_u, utf16_cu_offset_b; // lower, upper, break offsets
        int32_t utf16_cu_offset_p; // previous (after while, this is the last non trimmable code point)
        int32_t utf16_cu_offset_ru; // upper reajusted (to trim)

        for (utf16_cu_offset_l = 0; utf16_cu_offset_l < ustring_len; /* none */) {
            utf16_cu_offset_u = utf16_cu_offset_l;
            U16_FWD_N(ustring, utf16_cu_offset_u, ustring_len, width);
            /*for (i = 0; utf16_cu_offset_u < ustring_len && i < width; ) {
                U16_NEXT(ustring, utf16_cu_offset_u, ustring_len, c);
                i += !MAY_BE_TRIMMED(c);
            }*/
            utf16_cu_offset_b = ubrk_preceding(brk, utf16_cu_offset_u + 1);
            if (utf16_cu_offset_b <= utf16_cu_offset_l) {
                if (cut) {
                    int32_t utf16_cu_offset_p;
                    utf16_cu_offset_p = utf16_cu_offset_ru = utf16_cu_offset_u;
                    while (utf16_cu_offset_ru < ustring_len) {
                        utf16_cu_offset_p = utf16_cu_offset_ru;
                        U16_NEXT(ustring, utf16_cu_offset_ru, ustring_len, c);
                        if (!MAY_BE_TRIMMED(c)) {
                            break;
                        }
                    }
                    utf8_replace_len_from_utf16(&result, &result_len, replace, replace_len, ustring, utf16_cu_offset_u, utf16_cu_offset_p - utf16_cu_offset_u, ustring_cp_len, REPLACE_FORWARD);
                }
                utf16_cu_offset_l = utf16_cu_offset_u;
            } else if (utf16_cu_offset_b >= ustring_len) {
                break;
            } else {
                utf16_cu_offset_u = utf16_cu_offset_b;
                while (utf16_cu_offset_u >= utf16_cu_offset_l) {
                    utf16_cu_offset_p = utf16_cu_offset_u;
                    U16_PREV(ustring, 0, utf16_cu_offset_u, c);
                    if (!MAY_BE_TRIMMED(c)) {
                        break;
                    }
                }
                if (utf16_cu_offset_p < utf16_cu_offset_b) {
                    utf8_replace_len_from_utf16(&result, &result_len, replace, replace_len, ustring, utf16_cu_offset_p, utf16_cu_offset_b - utf16_cu_offset_p, ustring_cp_len, REPLACE_FORWARD);
                } else {
                    utf8_replace_len_from_utf16(&result, &result_len, replace, replace_len, ustring, utf16_cu_offset_b, 0, ustring_cp_len, REPLACE_FORWARD);
                }
                utf16_cu_offset_l = utf16_cu_offset_b;
            }
        }
    }
    result[result_len] = '\0';
    RETVAL_STRINGL(result, result_len, FALSE);

    if (FALSE) {
end:
        RETVAL_FALSE;
        if (NULL != result) {
            efree(result);
        }
    }
    if (NULL != ustring) {
        efree(ustring);
    }
    if (NULL != brk) {
        ubrk_close(brk);
    }
}

PHP_FUNCTION(utf8_unescape) // TODO: tests
{
    UBool ret;
    char *string = NULL;
    int string_len = 0;
    char *result = NULL;
    int32_t result_len = 0;
    zend_bool quiet = TRUE;
    UErrorCode status = U_ZERO_ERROR;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &string, &string_len, &quiet)) {
        return;
    }
    intl_error_reset(NULL TSRMLS_CC);
    ret = utf8_unescape((const uint8_t *) string, string_len, (uint8_t **) &result, &result_len, &status);
    if (quiet) {
        intl_error_set_code(NULL, status TSRMLS_CC);
    } else {
        intl_error_non_quiet_set_code(status TSRMLS_CC);
    }
    if (!ret) {
        if (NULL != result) {
            efree(result);
        }
        RETURN_FALSE;
    }

    RETURN_STRINGL(result, result_len, FALSE);
}

/**
 * TEST:
 * - [lr]?trim
 * - pos
 * - rpos
 * - str
 * - count_chars
 * - word_count
 * - validate
 * - wordwrap
 * - unescape
 **/

/**
 * locale support on case folding ?
 * Length have more sense with a NFC normalization ?
 **/

/**
 * Equivalents :
 *
 * substr => utf8_slice
 * substr_compare => utf8_slice_cmp
 * strtolower => utf8_tolower
 * strtoupper => utf8_toupper
 * ucwords => utf8_totitle (not strictly, others chars are lowered)
 * trim => utf8_trim (TODO: tests)
 * ltrim => utf8_ltrim (TODO: tests)
 * rtrim => utf8_rtrim (TODO: tests)
 * chr => utf8_chr
 * ord => utf8_ord
 * strtr => utf8_tr
 * str_shuffle => utf8_shuffle
 * str_reverse => utf8_reverse
 * str_split => utf8_split
 * strlen => utf8_len
 * str_word_count => utf8_count_words (TODO: tests)
 * count_chars => utf8_count_chars (TODO: some modes ask too much memory ; tests)
 * strcasecmp => utf8_casecmp
 * strncasecmp => utf8_ncasecmp
 * strncmp => utf8_ncmp
 * strpos => utf8_firstpos (TODO: rename ?)
 * strrpos => utf8_lastpos (TODO: rename ?)
 * strstr/strchr => utf8_firstsub (TODO: rename ?)
 * strrchr => utf8_lastsub (TODO: rename ?)
 * substr_count => utf8_slice_count
 * substr_replace => utf8_slice_replace
 * str_ireplace => utf8_ireplace (TODO: move to Collator)
 * wordwrap => utf8_wordwrap (TODO: tests)
 **/

/**
 * "Ideas":
 * - Make comparisons highly customable for fast/full (normalization or not), cs/ci by optionnal flags (of constants)?
 * eg : utf8_lindex($string, $substring, UTF8_IGNORE_CASE | UTF8_FULL_CMP)
 * - Pass strings by reference for normalization?
 **/

// stri[r?pos|r?chr] : rewrite (depends on locale support with case folding)

// explode : same
// implode, join : same
// str_replace : same
// strcmp : same
// str_repeat : same

// strnatcasecmp, strnatcmp : use collations?
// strncasecmp, strcasecmp : rewrite (2 versions : with case folding and full/locale?)
// ucfirst, lcfirst : no sense ?
// ...

/**
 * strn(case)?cmp: length doesn't really make sense ?
 * str(case)cmp : use collations for implementation ?
 **/
