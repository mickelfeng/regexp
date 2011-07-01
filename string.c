#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "php_intl.h"
#include "intl_data.h"
#include "intl_convert.h"
#include "string.h"

#include <unicode/ubrk.h>

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


PHP_FUNCTION(utf8_chunck_split) // args: string, length
{
    //
}

PHP_FUNCTION(utf8_count_chars)
{
    //
}

PHP_FUNCTION(utf8_strlen) // arg: string
{
    char *string = NULL;
    int string_len = 0;
    UChar *ustring = NULL;
    int32_t ustring_len = 0;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
    if (1 != ZEND_NUM_ARGS()) {
        WRONG_PARAM_COUNT;
    }
    if (FAILURE == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s", &string, &string_len)) {
        RETURN_LONG(0);
    }
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    RETVAL_LONG((long) u_countChar32(ustring, ustring_len));

end:
    if (NULL != ustring) {
        efree(ustring);
    }
}

PHP_FUNCTION(utf8_substr)
{
    //
}

PHP_FUNCTION(utf8_strpos)
{
    //
}

PHP_FUNCTION(utf8_ord)
{
    UChar32 c = 0;
    char *string = NULL;
    int string_len = 0;
    UChar *ustring = NULL;
    int32_t ustring_len = 0;
    long cp_offset = 0;
    int32_t cu_offset = 0;
    UErrorCode status = U_ZERO_ERROR;

    intl_error_reset(NULL TSRMLS_CC);
    if (2 != ZEND_NUM_ARGS()) {
        WRONG_PARAM_COUNT;
    }
    if (FAILURE == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "sl", &string, &string_len, &cp_offset)) {
        RETURN_FALSE;
    }
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    UTF16_CP_TO_CU(ustring, ustring_len, cp_offset, cu_offset);
    U16_NEXT(ustring, cu_offset, ustring_len, c);
    RETVAL_LONG((long) c);

    if (FALSE) {
end:
        RETVAL_LONG(0);
    }
    if (NULL != ustring) {
        efree(ustring);
    }
}

PHP_FUNCTION(utf8_str_word_count) // args: string, locale, format
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
    int32_t i, s_len;
    UBool isError = FALSE;

    if (1 != ZEND_NUM_ARGS()) {
        WRONG_PARAM_COUNT;
    }
    if (FAILURE == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "l", &cp)) {
        cp = 0;
    }
    s_len = U8_LENGTH(cp);
    s = emalloc((s_len + 1) * sizeof(*s));
    U8_APPEND(s, i, s_len, cp, isError);
    s[s_len] = '\0';

    RETURN_STRINGL(s, s_len, FALSE);
}

enum {
    LOWER,
    UPPER,
    TITLE
};

static void fullcasemapping(INTERNAL_FUNCTION_PARAMETERS, int type)
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
        RETURN_FALSE;
    }
    if (0 == locale_len) {
        locale = INTL_G(default_locale);
    }
    UTF8_TO_UTF16(status, ustring, ustring_len, string, string_len);
    do { /* Iteration needed: string may be longer than original ! */
        uresult_size = ++tries * ustring_len + 1;
        uresult = erealloc(uresult, uresult_size * sizeof(*uresult));
        if (TITLE == type) {
            uresult_len = u_strToTitle(uresult, uresult_size, ustring, ustring_len, NULL, locale, &status);
        } else if (UPPER == type) {
            uresult_len = u_strToUpper(uresult, uresult_size, ustring, ustring_len, locale, &status);
        } else /*if (LOWER == type)*/ {
            uresult_len = u_strToLower(uresult, uresult_size, ustring, ustring_len, locale, &status);
        }
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

PHP_FUNCTION(utf8_strtoupper)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, UPPER);
}

PHP_FUNCTION(utf8_strtolower)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, LOWER);
}

PHP_FUNCTION(utf8_strtotitle)
{
    fullcasemapping(INTERNAL_FUNCTION_PARAM_PASSTHRU, TITLE);
}

// explode : same
// *printf : rewrite
// implode, join : same
// ltrim, rtrim, trim : rewrite
// str_replace : same
// str_ireplace : rewrite
// strchr/strstr : rewrite (offsets)
// strichr/stripos : rewrite
// strnatcasecmp, strnatcmp : use collations?
// strncasecmp, strncmp : rewrite (2 versions : with case folding and full/locale?)
// str*pos : rewrite (for offsets and case insensitivity)
// strtr : rewrite
// substr, substr_compare, substr_replace : rewrite (offsets)
// ucfirst, lcfirst : <no sense>
// wordwrap : ?
// str_shuffle : ?
// str_repeat : same
// ...