#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <php.h>
#include "php_intl.h"
#include "intl_data.h"
#include "intl_convert.h"
#include "unicode.h"
#include "utf8.h"
#include "utf16.h"
#include "string.h"

#include <unicode/ubrk.h>
#include <unicode/uset.h>

/* <To remove> */
#ifdef ZEND_DEBUG
# ifdef ZEND_WIN32
#  define DIRECTORY_SEPARATOR '\\'
# else
#  define DIRECTORY_SEPARATOR '/'
# endif /* ZEND_WIN32 */
static const char *ubasename(const char *filename)
{
    const char *c;

    if (NULL == (c = strrchr(filename, DIRECTORY_SEPARATOR))) {
        return filename;
    } else {
        return c + 1;
    }
}

# define debug(format, ...) \
    zend_output_debug_string(0, "%s:%d:" format " in %s()\n", ubasename(__FILE__), __LINE__, ## __VA_ARGS__, __func__)
#else
# define debug(format, ...)
#endif /* ZEND_DEBUG */
/* </To remove> */

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

#define UTF16_CP_TO_CU(ustring, ustring_len, cp_offset, cu_offset)                                            \
    do {                                                                                                      \
        if (0 != cp_offset) {                                                                                 \
            int32_t count_cp = u_countChar32(ustring, ustring_len);                                           \
            if (cp_offset < 0) {                                                                              \
                if (cp_offset < -count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                cu_offset = ustring_len;                                                                      \
                U16_BACK_N(ustring, 0, cu_offset, -cp_offset);                                                \
            } else {                                                                                          \
                if (cp_offset >= count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                U16_FWD_N(ustring, cu_offset, ustring_len, cp_offset);                                        \
            }                                                                                                 \
        }                                                                                                     \
    } while (0);

#define UTF8_CP_TO_CU(string, string_len, cp_offset, cu_offset)                                               \
    do {                                                                                                      \
        if (0 != cp_offset) {                                                                                 \
            int32_t count_cp = u8_countChar32(string, string_len);                                            \
            if (cp_offset < 0) {                                                                              \
                if (cp_offset < -count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                cu_offset = string_len;                                                                       \
                U8_BACK_N(string, 0, cu_offset, -cp_offset);                                                  \
            } else {                                                                                          \
                if (cp_offset >= count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                U8_FWD_N(string, cu_offset, string_len, cp_offset);                                           \
            }                                                                                                 \
        }                                                                                                     \
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

PHP_FUNCTION(utf8_count_chars) // not tested
{
    enum {
        ARRAY_ALL_FREQ      = 0,
        ARRAY_NONNULL_FREQ  = 1,
        ARRAY_NULL_FREQ     = 2,
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
            uresult = emalloc((uresult_len + 1) * sizeof(*uresult));
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

PHP_FUNCTION(utf8_sub)
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
        U8_BACK_N(string, 0, cu_from, -cp_start);
    }
    if (ZEND_NUM_ARGS() <= 2) {
        cu_end = string_len;
    } else {
        if (cp_length > 0) {
            cu_end = cu_from;
            U8_FWD_N(string, cu_end, string_len, cp_length);
        } else if (cp_length < 0) {
            cu_end = string_len;
            U8_BACK_N(string, 0, cu_end, -cp_length);
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
#ifdef UTF16_AS_INTERNAL
    UChar *ustring = NULL;
    int32_t ustring_len = 0;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
#endif /* UTF16_AS_INTERNAL */
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &string, &string_len)) {
        return;
    }
#ifdef UTF16_AS_INTERNAL
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    RETVAL_LONG((long) u_countChar32(ustring, ustring_len));

end:
    if (NULL != ustring) {
        efree(ustring);
    }
#else
    RETVAL_LONG((long) u8_countChar32(string, string_len));
#endif /* UTF16_AS_INTERNAL */
}

PHP_FUNCTION(utf8_ord)
{
    UChar32 c = 0;
    char *string = NULL;
    int string_len = 0;
#ifdef UTF16_AS_INTERNAL
    UChar *ustring = NULL;
    int32_t ustring_len = 0;
#endif /* UTF16_AS_INTERNAL */
    long cp_offset = 0;
    int32_t cu_offset = 0;
#ifdef UTF16_AS_INTERNAL
    UErrorCode status = U_ZERO_ERROR;
#endif /* UTF16_AS_INTERNAL */

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &string, &string_len, &cp_offset)) {
        return;
    }
#ifdef UTF16_AS_INTERNAL
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    UTF16_CP_TO_CU(ustring, ustring_len, cp_offset, cu_offset);
    U16_NEXT(ustring, cu_offset, ustring_len, c);
#else
    UTF8_CP_TO_CU(string, string_len, cp_offset, cu_offset);
    U8_NEXT(string, cu_offset, string_len, c);
#endif /* UTF16_AS_INTERNAL */
    RETVAL_LONG((long) c);

    if (FALSE) {
end:
        RETVAL_LONG(0);
    }
#ifdef UTF16_AS_INTERNAL
    if (NULL != ustring) {
        efree(ustring);
    }
#endif /* UTF16_AS_INTERNAL */
}

PHP_FUNCTION(utf8_word_count) // TODO: tests
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
    UBool isError = FALSE;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &cp)) {
        return;
    }
    s_len = U8_LENGTH(cp);
    s = emalloc((s_len + 1) * sizeof(*s));
    U8_APPEND(s, i, s_len, cp, isError);
    s[s_len] = '\0';

    RETURN_STRINGL(s, s_len, FALSE);
}

typedef int32_t (*func_full_case_mapping_t)(UChar *dest, int32_t destCapacity, const UChar *src, int32_t srcLength, const char *locale, UErrorCode *status);

static int32_t u_strToTitleWithoutBI(UChar *dest, int32_t destCapacity, const UChar *src, int32_t srcLength, const char *locale, UErrorCode *status)
{
    return u_strToTitle(dest, destCapacity, src, srcLength, NULL, locale, status);
}

static void fullcasemapping(INTERNAL_FUNCTION_PARAMETERS, func_full_case_mapping_t func)
{
    UErrorCode status = U_ZERO_ERROR;
    int locale_len = 0;
    char *locale = NULL;
    int string_len = 0;
    char *string = NULL;
    int32_t ustring_len = 0;
    UChar *ustring = NULL;
    int32_t uresult_len = 0;
    int32_t uresult_size = 0;
    UChar *uresult = NULL;
    int32_t result_len = 0;
    char *result = NULL;
    int tries = 0;

    intl_error_reset(NULL TSRMLS_CC);
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &string, &string_len, &locale, &locale_len)) {
        return;
    }
    if (0 == locale_len) {
        locale = INTL_G(default_locale);
    }
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    do { /* Iteration needed: string may be longer than original ! */
        uresult_size = ++tries * ustring_len + 1;
        uresult = erealloc(uresult, uresult_size * sizeof(*uresult));
        uresult_len = func(uresult, uresult_size, ustring, ustring_len, locale, &status);
        if (U_SUCCESS(status)) {
            break;
        }
    } while (U_BUFFER_OVERFLOW_ERROR == status);
    CHECK_STATUS(status, "full case mapping");
    UTF16_TO_UTF8(status, result, result_len, uresult, uresult_len);
    RETVAL_STRINGL(result, result_len, 0);

end:
    if (NULL != uresult) {
        efree(uresult);
    }
    if (NULL != ustring) {
        efree(ustring);
    }
}

PHP_FUNCTION(utf8_toupper)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_strToUpper);
}

PHP_FUNCTION(utf8_tolower)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_strToLower);
}

PHP_FUNCTION(utf8_totitle)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_strToTitleWithoutBI);
}

static void ncasecmp(INTERNAL_FUNCTION_PARAMETERS, int ncmpbehave)
{
    UErrorCode status = U_ZERO_ERROR;
    char *string1 = NULL;
    int string1_len = 0;
    char *string2 = NULL;
    int string2_len = 0;
    long cp_length = 0;
    UChar *ustring1 = NULL;
    int32_t ustring1_len = 0;
    UChar *ustring2 = NULL;
    int32_t ustring2_len = 0;

    intl_error_reset(NULL TSRMLS_CC);
    if (ncmpbehave) {
        if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl", &string1, &string1_len, &string2, &string2_len, &cp_length)) {
            return;
        }
    } else {
        if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &string1, &string1_len, &string2, &string2_len)) {
            return;
        }
    }
    UTF8_TO_UTF16(status, ustring1, ustring1_len, string1, string1_len);
    UTF8_TO_UTF16(status, ustring2, ustring2_len, string2, string2_len);
    if (ncmpbehave) {
        int32_t cu_length = 0;
        UChar *uref;
        int32_t uref_len;

        if (u_countChar32(ustring1, ustring1_len) >= u_countChar32(ustring2, ustring2_len)) {
            uref = ustring1;
            uref_len = ustring1_len;
        } else {
            uref = ustring2;
            uref_len = ustring2_len;
        }
        U16_FWD_N(uref, cu_length, uref_len, cp_length);
        ustring1_len = MIN(ustring1_len, cu_length);
        ustring2_len = MIN(ustring2_len, cu_length);
    }
    RETVAL_LONG((long) u_strCaseCompare(ustring1, ustring1_len, ustring2, ustring2_len, 0, &status));

    if (FALSE) {
end:
        RETVAL_FALSE;
    }
    if (NULL != ustring1) {
        efree(ustring1);
    }
    if (NULL != ustring2) {
        efree(ustring2);
    }
}

PHP_FUNCTION(utf8_casecmp) // TODO: tests
{
    ncasecmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE);
}

PHP_FUNCTION(utf8_ncasecmp) // TODO: tests
{
    ncasecmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE);
}

PHP_FUNCTION(utf8_ncmp)
{
    char *string1 = NULL;
    int string1_len = 0;
    char *string2 = NULL;
    int string2_len = 0;
    long cp_length = 0;
    int32_t cu_length = 0;
    char *ref;
    int32_t ref_len;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl", &string1, &string1_len, &string2, &string2_len, &cp_length)) {
        return;
    }
    if (cp_length < 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length must be greater than or equal to 0");
        RETURN_FALSE;
    }
    /* result were wrong if the shorter string (in CP) was used as reference */
    if (u8_countChar32(string1, string1_len) >= u8_countChar32(string2, string2_len)) {
        ref = string1;
        ref_len = string1_len;
    } else {
        ref = string2;
        ref_len = string2_len;
    }
    /* if length (in CP) is lesser than cp_length, there is no matter : ICU macro's iteration will be stopped */
    U8_FWD_N(ref, cu_length, ref_len, cp_length);
    RETURN_LONG(zend_binary_strncmp(string1, string1_len, string2, string2_len, cu_length));
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
    result = emalloc((string_len + 1) * sizeof(*result));
    last = cu_offset = string_len;
    r = result;
    while (cu_offset > 0) {
        U8_BACK_1(string, 0, cu_offset);
        memcpy(r, string + cu_offset, last - cu_offset);
        r += last - cu_offset;
        last = cu_offset;
    }
    result[string_len] = '\0';

    RETVAL_STRINGL(result, string_len, FALSE);
}

static void findFirst_CaseSensitive(INTERNAL_FUNCTION_PARAMETERS, int want_only_pos)
{
    char *haystack = NULL;
    int haystack_len = 0;
    zval *needle = NULL;
    zend_bool before = FALSE;
    long start_cp = 0;
    int32_t start_cu = 0;
    char *found = NULL;

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
        found = php_memnstr(haystack + start_cu, Z_STRVAL_P(needle), Z_STRLEN_P(needle), haystack + haystack_len);
    } else { // we search a code point (convert needle into long)
        UChar32 c;
        char cus[U8_MAX_LENGTH + 1] = { 0 };
        char cus_length = 0;

        if (SUCCESS != unicode_convert_needle_to_cp(needle, &c TSRMLS_CC)) {
            goto end;
        }
        U8_APPEND_UNSAFE(cus, cus_length, c);
        found = php_memnstr(haystack + start_cu, cus, cus_length, haystack + haystack_len);
    }
    if (NULL != found) {
        if (want_only_pos) {
            RETURN_LONG((long) u8_countChar32(haystack, found - haystack));
        } else {
            if (before) {
                RETURN_STRINGL(haystack, found - haystack, 1);
            } else {
                RETURN_STRINGL(found, haystack_len - (found - haystack), 1);
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
    findFirst_CaseSensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE);
}

PHP_FUNCTION(utf8_firstpos) // TODO: tests
{
    findFirst_CaseSensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE);
}

static void findLast_CaseSensitive(INTERNAL_FUNCTION_PARAMETERS, int want_only_pos)
{
    //
}

PHP_FUNCTION(utf8_lastsub)
{
    findLast_CaseSensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE);
}

PHP_FUNCTION(utf8_lastpos)
{
    findLast_CaseSensitive(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE);
}

#include "../../standard/php_smart_str.h"

PHP_FUNCTION(utf8_tr) // TODO: tests
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
    zend_hash_init(&map, u8_countChar32(from, from_len), NULL, NULL, FALSE);
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
    RETVAL_STRINGL(result.c, result.len, 0);
}

// add: startswith, endswith (version cs and ci)

// ltrim, rtrim, trim : rewrite
// strchr/strstr : rewrite (offsets)
// str*pos : rewrite (for offsets and case insensitivity)

// explode : same
// *printf : rewrite
// implode, join : same
// str_replace : same
// strcmp : same

// str_ireplace : rewrite
// strichr/stripos : rewrite
// strnatcasecmp, strnatcmp : use collations?
// strncasecmp, strcasecmp : rewrite (2 versions : with case folding and full/locale?)
// strtr : rewrite
// substr_compare, substr_replace : rewrite (offsets)
// ucfirst, lcfirst : no sense ?
// wordwrap : ?
// str_shuffle : ?
// str_repeat : same
// ...