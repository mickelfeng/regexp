#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "php_intl.h"
#include "regexp.h"
#include "regexp_class.h"
#include "regexp_methods.h"
#include "intl_data.h"
#include "intl_convert.h"

#include <zend_exceptions.h>


static void intl_errors_setf_custom_msg(intl_error* err TSRMLS_DC, char *format, ...)
{
    char *msg;
    va_list args;

    va_start(args, format);
    vspprintf(&msg, 0, format, args);
    va_end(args);
    intl_errors_set_custom_msg(err, msg, TRUE TSRMLS_CC);
    efree(msg);
}

#define REGEXP_PARSE_VOID_ARGS(name, reset)                                                                               \
    do {                                                                                                                  \
        if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Regexp_ce_ptr)) { \
            intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, name ": bad arguments", 0 TSRMLS_CC);                          \
            RETURN_FALSE;                                                                                                 \
        }                                                                                                                 \
        ro = zend_object_store_get_object(object TSRMLS_CC);                                                              \
        if (NULL == ro) {                                                                                                 \
            RETURN_FALSE;                                                                                                 \
        }                                                                                                                 \
        REGEXP_METHOD_FETCH_OBJECT(reset);                                                                                \
    } while(0);

#define REGEXP_RESET(ro)                                                           \
    do {                                                                           \
        UErrorCode status = U_ZERO_ERROR;                                          \
        uregex_setText(ro->uregex, UREGEXP_FAKE_USTR, &status);                    \
        if (U_FAILURE(status)) {                                                   \
            intl_error_set(NULL, status, "internal resetting error", 0 TSRMLS_CC); \
        }                                                                          \
    } while (0);

#define UTF8_TO_UTF16(ro, to, to_len, from, from_len)                                      \
    do {                                                                                   \
        to = NULL;                                                                         \
        to_len = 0;                                                                        \
        intl_convert_utf8_to_utf16(&to, &to_len, from, from_len, REGEXP_ERROR_CODE_P(ro)); \
        REGEXP_CHECK_STATUS(ro, "String conversion of " #from " to UTF-16 failed");        \
    } while (0);

#define UTF16_TO_UTF8(ro, to, to_len, from, from_len)                                      \
    do {                                                                                   \
        to = NULL;                                                                         \
        to_len = 0;                                                                        \
        intl_convert_utf16_to_utf8(&to, &to_len, from, from_len, REGEXP_ERROR_CODE_P(ro)); \
        REGEXP_CHECK_STATUS(ro, "String conversion of " #from " to UTF-8 failed");         \
    } while (0);

#define REGEXP_SET_UTF16_SUBJECT(ro, usubject, usubject_len)

#define REGEXP_SET_UTF8_SUBJECT(ro, usubject, usubject_len) /* we already have done the translation */

#define REGEXP_GROUP_START(ro, group, l)                                                                                     \
    do {                                                                                                                     \
        l = uregex_start(ro->uregex, group, REGEXP_ERROR_CODE_P(ro));                                                        \
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);                                                          \
        if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {                                                                              \
            intl_errors_setf_custom_msg(REGEXP_ERROR_P(ro) TSRMLS_CC, "Error extracting start of group capture #%d", group); \
            goto end;                                                                                                        \
        }                                                                                                                    \
    } while (0);

#define REGEXP_GROUP_END(ro, group, u)                                                                                     \
    do {                                                                                                                   \
        u = uregex_end(ro->uregex, group, REGEXP_ERROR_CODE_P(ro));                                                        \
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);                                                        \
        if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {                                                                            \
            intl_errors_setf_custom_msg(REGEXP_ERROR_P(ro) TSRMLS_CC, "Error extracting end of group capture #%d", group); \
            goto end;                                                                                                      \
        }                                                                                                                  \
    } while (0);

#define UTF16_CP_TO_CU(ustring, ustring_len, cp_offset, cu_offset)                                            \
    do {                                                                                                      \
        if (0 != cp_offset) {                                                                                 \
            int32_t count_cp = u_countChar32(ustring, ustring_len);                                           \
            if (cp_offset < 0) {                                                                              \
                if (cp_offset < -count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    RETVAL_FALSE;                                                                             \
                    goto end;                                                                                 \
                }                                                                                             \
                cu_offset = ustring_len;                                                                      \
                U16_BACK_N(ustring, 0, cu_offset, -cp_offset);                                                \
            } else {                                                                                          \
                if (cp_offset >= count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    RETVAL_FALSE;                                                                             \
                    goto end;                                                                                 \
                }                                                                                             \
                U16_FWD_N(ustring, cu_offset, ustring_len, cp_offset);                                        \
            }                                                                                                 \
        }                                                                                                     \
    } while (0);

#ifdef ZEND_DEBUG
# ifdef ZEND_WIN32
#  define DIRECTORY_SEPARATOR '\\'
# else
#  define DIRECTORY_SEPARATOR '/'
# endif /* ZEND_WIN32 */
const char *ubasename(const char *filename)
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

static const UChar _UREGEXP_FAKE_USTR[] = { 0 };
#define UREGEXP_FAKE_USTR _UREGEXP_FAKE_USTR, 0

static void regexp_ctor(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *object;
    Regexp_object *ro;
    char *pattern;
    int32_t pattern_len;
    UChar *upattern = NULL;
    int32_t upattern_len = 0;
    zval *zflags = NULL;
    uint32_t flags = 0;
    UParseError pe = { -1, -1, {0}, {0} };

    intl_error_reset(NULL TSRMLS_CC);
    object = return_value;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &pattern, &pattern_len, &zflags)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_create: bad arguments", 0 TSRMLS_CC);
        zval_dtor(object);
        RETURN_NULL();
    }
    if (NULL != zflags) {
        switch (Z_TYPE_P(zflags)) {
            case IS_LONG:
                flags = (uint32_t) Z_LVAL_P(zflags);
                break;
            case IS_STRING:
            {
                const char *p;

                for (p = Z_STRVAL_P(zflags); '\0' != *p; p++) {
                    switch (*p) {
                        case 'i': flags |= UREGEX_CASE_INSENSITIVE; break;
                        case 'm': flags |= UREGEX_MULTILINE;        break;
                        case 's': flags |= UREGEX_DOTALL;           break;
                        case 'x': flags |= UREGEX_COMMENTS;         break;
                        case 'w': flags |= UREGEX_UWORD;            break;
                        default:
                            intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_create: invalid modifier", 0 TSRMLS_CC);
                            zval_dtor(object);
                            RETURN_NULL();
                    }
                }
                break;
            }
            default:
                intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_create: bad arguments", 0 TSRMLS_CC);
                zval_dtor(object);
                RETURN_NULL();
        }
    }
    ro = (Regexp_object *) zend_object_store_get_object(object TSRMLS_CC);
    intl_convert_utf8_to_utf16(&upattern, &upattern_len, pattern, pattern_len, REGEXP_ERROR_CODE_P(ro));
    INTL_CTOR_CHECK_STATUS(ro, "String conversion of pattern to UTF-16 failed");
    ro->uregex = uregex_open(upattern, upattern_len, flags, &pe, REGEXP_ERROR_CODE_P(ro));
    efree(upattern);
    if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);
        if (-1 == pe.line) {
            intl_error_set_custom_msg(NULL, "regexp_create: unable to compile ICU regular expression, syntax error", 0 TSRMLS_CC);
        } else {
            intl_error_set_custom_msg(NULL, "regexp_create: unable to compile ICU regular expression", 0 TSRMLS_CC);
        }
        zval_dtor(object);
        RETURN_NULL();
    }
}

PHP_FUNCTION(regexp_create)
{
    object_init_ex(return_value, Regexp_ce_ptr);
    regexp_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}


PHP_METHOD(Regexp, __construct)
{
    return_value = getThis();
    regexp_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_FUNCTION(regexp_match)
{
    UBool res = FALSE;
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    zval *subpats = NULL;
    int32_t start_cu_offset = 0;
    long start_cp_offset = 0;
    long flags = 0;

    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|zll", &object, Regexp_ce_ptr, &subject, &subject_len, &subpats, &flags, &start_cp_offset)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_match: bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (0 != (flags & ~(OFFSET_CAPTURE))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_match: invalid flag(s)", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    UTF16_CP_TO_CU(usubject, usubject_len, start_cp_offset, start_cu_offset);
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "Error setting text");
    res = uregex_find(ro->uregex, start_cu_offset, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "Error finding pattern");
    if (res && NULL != subpats) {
        int i;
        char *group;
        int group_len;
        int32_t group_count;
        int32_t l, u;

        zval_dtor(subpats);
        array_init(subpats);

        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "Error counting groups");
        for (i = 0; i <= group_count; i++) {
            REGEXP_GROUP_START(ro, i, l);
            REGEXP_GROUP_END(ro, i, u);
            UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
            add_index_stringl(subpats, i, group, group_len, FALSE);
            // For PREG_OFFSET_CAPTURE:
            // add_index_stringl(subpats, u_countChar32(usubject, l), group, group_len, FALSE);
        }
    }

    if (FALSE) {
end:
        if (NULL != subpats) {
            zval_dtor(subpats);
            array_init(subpats);
        }
    }
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
    RETURN_BOOL(res);
}

PHP_FUNCTION(regexp_match_all)
{
    int match_count = 0;
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    zval *subpats = NULL;
    int32_t start_cu_offset = 0;
    long start_cp_offset = 0;
    int32_t group_count = 0;
    long flags = 0;

    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|zll", &object, Regexp_ce_ptr, &subject, &subject_len, &subpats, &flags, &start_cp_offset)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_match_all: bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (0 != (flags & ~(OFFSET_CAPTURE|MATCH_ALL_PATTERN_ORDER|MATCH_ALL_SET_ORDER))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_match_all: invalid flag(s)", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    UTF16_CP_TO_CU(usubject, usubject_len, start_cp_offset, start_cu_offset);
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "Error setting text");
    if (NULL != subpats) {
        zval_dtor(subpats);
        array_init(subpats);
        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "Error counting groups");
    }
    if (0 != start_cp_offset) {
        uregex_reset(ro->uregex, start_cu_offset, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "Error setting start region");
    }
    while (uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
        match_count++;
        if (NULL != subpats) {
            int i;
            char *group;
            int group_len;
            int32_t l, u;
            zval *match_groups;

            ALLOC_ZVAL(match_groups);
            array_init(match_groups);
            INIT_PZVAL(match_groups);

            for (i = 0; i <= group_count; i++) {
                REGEXP_GROUP_START(ro, i, l);
                REGEXP_GROUP_END(ro, i, u);
                UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
                add_index_stringl(match_groups, i, group, group_len, FALSE);
                // For PREG_OFFSET_CAPTURE:
                // add_index_stringl(match_groups, u_countChar32(usubject, l), group, group_len, FALSE);
            }
            zend_hash_next_index_insert(Z_ARRVAL_P(subpats), &match_groups, sizeof(zval *), NULL);
        }
    }
    REGEXP_CHECK_STATUS(ro, "Error finding pattern");
    RETVAL_LONG(match_count);

    if (FALSE) {
end:
        if (NULL != subpats) {
            zval_dtor(subpats);
            array_init(subpats);
        }
        RETVAL_FALSE;
    }
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
}

PHP_FUNCTION(regexp_get_pattern)
{
    char *pattern = NULL;
    int pattern_len = 0;
    const UChar *upattern;
    int32_t upattern_len = 0;
    REGEXP_METHOD_INIT_VARS

    REGEXP_PARSE_VOID_ARGS("regexp_get_pattern", TRUE);

    upattern = uregex_pattern(ro->uregex, &upattern_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "Error retrieving pattern");

    UTF16_TO_UTF8(ro, pattern, pattern_len, upattern, upattern_len);
    RETURN_STRINGL(pattern, pattern_len, 0);

end:
    RETURN_FALSE;
}

PHP_FUNCTION(regexp_get_flags)
{
    int32_t flags = 0;
    REGEXP_METHOD_INIT_VARS

    REGEXP_PARSE_VOID_ARGS("regexp_get_flags", TRUE);

    flags = uregex_flags(ro->uregex, REGEXP_ERROR_CODE_P(ro));

    RETURN_LONG((long) flags);

end:
    RETURN_FALSE;
}

PHP_FUNCTION(regexp_replace)
{
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;

    char *replacement = NULL;
    int replacement_len = 0;
    UChar *ureplacement = NULL;
    int32_t ureplacement_len = 0;

    char *result = NULL;
    int result_len = 0;
    UChar *uresult = NULL;
    int32_t uresult_len = 0;

    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss", &object, Regexp_ce_ptr, &subject, &subject_len, &replacement, &replacement_len)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_replace: bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    UTF8_TO_UTF16(ro, ureplacement, ureplacement_len, replacement, replacement_len);

    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "Error setting text");

    uresult_len = uregex_replaceAll(ro->uregex, ureplacement, ureplacement_len, NULL, 0, REGEXP_ERROR_CODE_P(ro));
    if (U_BUFFER_OVERFLOW_ERROR != REGEXP_ERROR_CODE(ro)) {
        intl_errors_set_custom_msg(REGEXP_ERROR_P(ro), "regexp_replace: replacing failed", 0 TSRMLS_CC);
        goto end;
    }
    intl_error_reset(REGEXP_ERROR_P(ro) TSRMLS_CC);
    uresult = emalloc((uresult_len + 1) * sizeof(*uresult));
    /*uresult_len = */uregex_replaceAll(ro->uregex, ureplacement, ureplacement_len, uresult, uresult_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "Error while replacing");

    UTF16_TO_UTF8(ro, result, result_len, uresult, uresult_len);

end:
    if (NULL != ureplacement) {
        efree(ureplacement);
    }
    if (NULL != usubject) {
        REGEXP_RESET(ro);
        efree(usubject);
    }
    if (NULL != uresult) {
        efree(uresult);
    }
    if (NULL == result) {
        RETVAL_FALSE;
    } else {
        RETVAL_STRINGL(result, result_len, 0);
    }
}

PHP_FUNCTION(regexp_split)
{
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    long limit = INT_MAX;
    int32_t last = 0;
    char *group = NULL;
    int group_len = 0;
    long flags = 0;
    int i;

    REGEXP_METHOD_INIT_VARS

    array_init(return_value);

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ll", &object, Regexp_ce_ptr, &subject, &subject_len, &limit, &flags)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_split: bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (0 != (flags & ~(OFFSET_CAPTURE|SPLIT_NO_EMPTY|SPLIT_DELIM_CAPTURE))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "regexp_split: invalid flag(s)", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (limit <= 0) {
        limit = INT_MAX;
    } else if (1 == limit) {
        //
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);

    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "Error setting text");
    /* We don't use uregex_split, it has a few "limitations" */
    for (i = 1; i < limit && uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro)); i++) {
        int32_t l, u;

        REGEXP_GROUP_START(ro, 0, l);
        REGEXP_GROUP_END(ro, 0, u);
        if (!(flags & SPLIT_NO_EMPTY) || last < l) {
            UTF16_TO_UTF8(ro, group, group_len, usubject + last, l - last);
            if (!(flags & OFFSET_CAPTURE)) {
                add_next_index_stringl(return_value, group, group_len, FALSE);
            } else {
                add_index_stringl(return_value, u_countChar32(usubject, last), group, group_len, FALSE);
            }
        }
        last = u;
    }
    if (!(flags & SPLIT_NO_EMPTY) || last < usubject_len) {
        UTF16_TO_UTF8(ro, group, group_len, usubject + last, usubject_len - last);
        if (!(flags & OFFSET_CAPTURE)) {
            add_next_index_stringl(return_value, group, group_len, FALSE);
        } else {
            add_index_stringl(return_value, u_countChar32(usubject, last), group, group_len, FALSE);
        }
    }

    if (FALSE) {
end:
        zval_dtor(return_value);
        RETVAL_FALSE;
    }
    if (NULL != usubject) {
        efree(usubject);
        REGEXP_RESET(ro);
    }
}

PHP_FUNCTION(regexp_get_error_code)
{
    REGEXP_METHOD_INIT_VARS
    REGEXP_PARSE_VOID_ARGS("regexp_get_error_code", FALSE);

    RETURN_LONG((long) REGEXP_ERROR_CODE(ro));
}

PHP_FUNCTION(regexp_get_error_message)
{
    const char *message = NULL;

    REGEXP_METHOD_INIT_VARS
    REGEXP_PARSE_VOID_ARGS("regexp_get_error_message", FALSE);

    message = intl_error_get_message(REGEXP_ERROR_P(ro) TSRMLS_CC);
    RETURN_STRING(message, 0);
}
