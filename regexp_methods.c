#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "error.h"
#include "regexp.h"
#include "regexp_class.h"
#include "regexp_methods.h"
#include "intl_data.h"
#include "intl_convert.h"
#include "utf8.h"
#include "utf16.h"

#include <zend_exceptions.h>

#define REGEXP_PARSE_VOID_ARGS(reset)                                                                                     \
    do {                                                                                                                  \
        if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Regexp_ce_ptr)) { \
            intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);                                 \
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
        REGEXP_CHECK_STATUS(ro, "string conversion of " #from " to UTF-16 failed");        \
    } while (0);

#define UTF16_TO_UTF8(ro, to, to_len, from, from_len)                                      \
    do {                                                                                   \
        to = NULL;                                                                         \
        to_len = 0;                                                                        \
        intl_convert_utf16_to_utf8(&to, &to_len, from, from_len, REGEXP_ERROR_CODE_P(ro)); \
        REGEXP_CHECK_STATUS(ro, "string conversion of " #from " to UTF-8 failed");         \
    } while (0);

#define REGEXP_SET_UTF16_SUBJECT(ro, usubject, usubject_len)

#define REGEXP_SET_UTF8_SUBJECT(ro, usubject, usubject_len) /* we already have done the translation */

#define REGEXP_GROUP_START(ro, group, l)                                                                                     \
    do {                                                                                                                     \
        l = uregex_start(ro->uregex, group, REGEXP_ERROR_CODE_P(ro));                                                        \
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);                                                          \
        if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {                                                                              \
            intl_errors_setf_custom_msg(REGEXP_ERROR_P(ro) TSRMLS_CC, "error extracting start of group capture #%d", group); \
            goto end;                                                                                                        \
        }                                                                                                                    \
    } while (0);

#define REGEXP_GROUP_END(ro, group, u)                                                                                     \
    do {                                                                                                                   \
        u = uregex_end(ro->uregex, group, REGEXP_ERROR_CODE_P(ro));                                                        \
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);                                                        \
        if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {                                                                            \
            intl_errors_setf_custom_msg(REGEXP_ERROR_P(ro) TSRMLS_CC, "error extracting end of group capture #%d", group); \
            goto end;                                                                                                      \
        }                                                                                                                  \
    } while (0);

static const UChar _UREGEXP_FAKE_USTR[] = { 0 };
#define UREGEXP_FAKE_USTR _UREGEXP_FAKE_USTR, 0

static void regexp_parse_error_to_string(UParseError pe, char *pattern, int32_t pattern_len)
{
    int32_t l;
    char *eol, *from, *to, *end;

    eol = from = pattern;
    to = end = pattern + pattern_len;
    if (1 != pe.line) {
        for (l = 1; l < pe.line; l++) {
            if (NULL == (eol = strchr(eol, 0x0A))) {
                goto end; // prevent "bug"
            }
            eol++;
        }
        from = eol;
    }
    if (NULL != (eol = strchr(eol, 0x0A))) {
        to = --eol;
    }
    if (to > from) { /* normal case: to == from on new/empty line ; anormal: implementation error */
        intl_errors_setf_custom_msg(
            NULL,
            TSRMLS_CC
            "unable to compile ICU regular expression, syntax error at line %d, offset %d:\n%.*s\n%.*s\n%*c\n%.*s\n",
            pe.line,
            pe.offset,
            /* text before */
            (from > pattern && 0x0A == *(from - 1)) ? (from - pattern) - 1 : from - pattern, pattern,
            /* line pointed */
            to - from + 1, from,
            /* offset pointed */
            pe.offset, '^',
            /* text after */
            (to < end && 0x0A == to[1]) ? end - to - 1 : end - to, to + 2 // TODO: to + 2 = UNSAFE ?
        );
    } else {
end:
        intl_errors_setf_custom_msg(
            NULL,
            TSRMLS_CC
            "unable to compile ICU regular expression, syntax error at line %d, offset %d",
            pe.line,
            pe.offset
        );
    }
}

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
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
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
                            intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid modifier", 0 TSRMLS_CC);
                            zval_dtor(object);
                            RETURN_NULL();
                    }
                }
                break;
            }
            default:
                intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
                zval_dtor(object);
                RETURN_NULL();
        }
    }
    ro = (Regexp_object *) zend_object_store_get_object(object TSRMLS_CC);
    intl_convert_utf8_to_utf16(&upattern, &upattern_len, pattern, pattern_len, REGEXP_ERROR_CODE_P(ro));
    INTL_CTOR_CHECK_STATUS(ro, "string conversion of pattern to UTF-16 failed");
    ro->uregex = uregex_open(upattern, upattern_len, flags, &pe, REGEXP_ERROR_CODE_P(ro));
    efree(upattern);
    if (U_FAILURE(REGEXP_ERROR_CODE(ro))) {
        intl_error_set_code(NULL, REGEXP_ERROR_CODE(ro) TSRMLS_CC);
        if (-1 != pe.line) {
            regexp_parse_error_to_string(pe, pattern, pattern_len);
        } else {
            intl_error_set_custom_msg(NULL, "unable to compile ICU regular expression", 0 TSRMLS_CC);
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
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (NULL != subpats) {
        zval_dtor(subpats);
        array_init(subpats);
    }
    if (0 != (flags & ~(OFFSET_CAPTURE))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid flag(s)", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    UTF16_CP_TO_CU(usubject, usubject_len, start_cp_offset, start_cu_offset);
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
    res = uregex_find(ro->uregex, start_cu_offset, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error finding pattern");
    if (res && NULL != subpats) {
        int i;
        char *group;
        int group_len;
        int32_t group_count;
        int32_t l, u;

        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error counting groups");
        for (i = 0; i <= group_count; i++) {
            REGEXP_GROUP_START(ro, i, l);
            REGEXP_GROUP_END(ro, i, u);
            UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
            if (!(flags & OFFSET_CAPTURE)) {
                add_index_stringl(subpats, i, group, group_len, FALSE);
            } else {
                add_index_stringl(subpats, u_countChar32(usubject, l), group, group_len, FALSE);
            }
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
    int i;
    int match_count = 0;
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    zval *subpats = NULL;
    zval **Zres = NULL;
    int32_t start_cu_offset = 0;
    long start_cp_offset = 0;
    int32_t group_count = 0;
    long flags = 0;

    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|zll", &object, Regexp_ce_ptr, &subject, &subject_len, &subpats, &flags, &start_cp_offset)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if (0 != (flags & ~(OFFSET_CAPTURE|MATCH_ALL_PATTERN_ORDER|MATCH_ALL_SET_ORDER))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid flag(s)", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    UTF16_CP_TO_CU(usubject, usubject_len, start_cp_offset, start_cu_offset);
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
    if (NULL != subpats) {
        zval_dtor(subpats);
        array_init(subpats);
        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error counting groups");

        if ((flags & MATCH_ALL_PATTERN_ORDER)) {
            Zres = mem_new_n(*Zres, group_count + 1);
            for (i = 0; i <= group_count; i++) {
                ALLOC_ZVAL(Zres[i]);
                array_init(Zres[i]);
                INIT_PZVAL(Zres[i]);
            }
        }
    }
    if (0 != start_cp_offset) {
        uregex_reset(ro->uregex, start_cu_offset, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error setting start region");
    }
    while (uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
        match_count++;
        if (NULL != subpats) {
            char *group;
            int group_len;
            int32_t l, u;
            zval *match_groups;

            if (!(flags & MATCH_ALL_PATTERN_ORDER)) {
                ALLOC_ZVAL(match_groups);
                array_init(match_groups);
                INIT_PZVAL(match_groups);
            }
            for (i = 0; i <= group_count; i++) {
                REGEXP_GROUP_START(ro, i, l);
                REGEXP_GROUP_END(ro, i, u);
                UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
                if ((flags & MATCH_ALL_PATTERN_ORDER)) {
                    if (!(flags & OFFSET_CAPTURE)) {
                        add_next_index_stringl(Zres[i], group, group_len, FALSE);
                    } else {
                        add_index_stringl(Zres[i], u_countChar32(usubject, l), group, group_len, FALSE);
                    }
                } else {
                    if (!(flags & OFFSET_CAPTURE)) {
                        add_index_stringl(match_groups, i, group, group_len, FALSE);
                    } else {
                        add_index_stringl(match_groups, u_countChar32(usubject, l), group, group_len, FALSE);
                    }
                }
            }
            if (!(flags & MATCH_ALL_PATTERN_ORDER)) {
                zend_hash_next_index_insert(Z_ARRVAL_P(subpats), &match_groups, sizeof(zval *), NULL);
            }
        }
    }
    REGEXP_CHECK_STATUS(ro, "error finding pattern");
    if ((flags & MATCH_ALL_PATTERN_ORDER)) {
        for (i = 0; i <= group_count; i++) {
            zend_hash_next_index_insert(Z_ARRVAL_P(subpats), &Zres[i], sizeof(zval *), NULL);
        }
        efree(Zres);
    }
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

    REGEXP_PARSE_VOID_ARGS(TRUE);

    upattern = uregex_pattern(ro->uregex, &upattern_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error retrieving pattern");

    UTF16_TO_UTF8(ro, pattern, pattern_len, upattern, upattern_len);
    RETURN_STRINGL(pattern, pattern_len, 0);

end:
    RETURN_FALSE;
}

PHP_FUNCTION(regexp_get_flags)
{
    int32_t flags = 0;
    REGEXP_METHOD_INIT_VARS

    REGEXP_PARSE_VOID_ARGS(TRUE);

    flags = uregex_flags(ro->uregex, REGEXP_ERROR_CODE_P(ro));

    RETURN_LONG((long) flags);

end:
    RETURN_FALSE;
}

/**
 * TODO:
 * - eval flag equivalent?
 * - limit + count arguments
 **/
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
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    UTF8_TO_UTF16(ro, ureplacement, ureplacement_len, replacement, replacement_len);

    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");

    uresult_len = uregex_replaceAll(ro->uregex, ureplacement, ureplacement_len, NULL, 0, REGEXP_ERROR_CODE_P(ro));
    if (U_BUFFER_OVERFLOW_ERROR != REGEXP_ERROR_CODE(ro)) {
        intl_errors_set_custom_msg(REGEXP_ERROR_P(ro), "replacing failed", 0 TSRMLS_CC);
        goto end;
    }
    intl_error_reset(REGEXP_ERROR_P(ro) TSRMLS_CC);
    uresult = emalloc((uresult_len + 1) * sizeof(*uresult));
    /*uresult_len = */uregex_replaceAll(ro->uregex, ureplacement, ureplacement_len, uresult, uresult_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error while replacing");

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

PHP_FUNCTION(regexp_replace_callback)
{
    char *subject = NULL;
    int subject_len = 0;
    UChar *usubject = NULL;
    int32_t usubject_len = 0;
    int32_t usubject_cp_len = 0;
    zval **Zcallback;
    char *callback = NULL;
    zval *match_groups = NULL;
    zval **zargs[1];
    zval *retval_ptr;
    char *result = NULL;
    int result_len = 0;
    int32_t group_count = 0;

    REGEXP_METHOD_INIT_VARS

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "OsZ", &object, Regexp_ce_ptr, &subject, &subject_len, &Zcallback)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    REGEXP_METHOD_FETCH_OBJECT(TRUE);
    if (!zend_is_callable(*Zcallback, 0, &callback TSRMLS_CC)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "valid callback required", callback);
        //intl_errors_set_custom_msg(REGEXP_ERROR_P(ro), "regexp_replace_callback: requires a valid callback", 0 TSRMLS_CC);
        efree(callback);
        RETURN_FALSE;
    }
    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);
    usubject_cp_len = u_countChar32(usubject, usubject_len);
    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
    group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error counting groups");
    MAKE_STD_ZVAL(match_groups);
    array_init(match_groups);
    zargs[0] = &match_groups;
    result = estrndup(subject, result_len = subject_len);
    while (/*match_count <= limit && */ uregex_findNext(ro->uregex, REGEXP_ERROR_CODE_P(ro))) {
        int i;
        char *group;
        int group_len;
        int32_t l, u, l0, u0;
        //match_count++;

        REGEXP_GROUP_START(ro, 0, l0);
        REGEXP_GROUP_END(ro, 0, u0);
        UTF16_TO_UTF8(ro, group, group_len, usubject + l0, u0 - l0);
        add_index_stringl(match_groups, 0, group, group_len, FALSE);
        for (i = 1; i <= group_count; i++) {
            REGEXP_GROUP_START(ro, i, l);
            REGEXP_GROUP_END(ro, i, u);
            UTF16_TO_UTF8(ro, group, group_len, usubject + l, u - l);
            add_index_stringl(match_groups, i, group, group_len, FALSE);
        }
        if (SUCCESS == call_user_function_ex(EG(function_table), NULL, *Zcallback, &retval_ptr, 1, zargs, 0, NULL TSRMLS_CC) && NULL != retval_ptr) {
            convert_to_string_ex(&retval_ptr);
            utf8_replace_len_from_utf16(&result, &result_len, Z_STRVAL_P(retval_ptr), Z_STRLEN_P(retval_ptr), usubject, l0, u0 - l0, usubject_cp_len, REPLACE_FORWARD);
            zval_ptr_dtor(&retval_ptr);
        } else {
            if (!EG(exception)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to call custom replacement callback");
            }
            goto end; // is it appropriate?
        }
        zend_hash_clean(Z_ARRVAL_P(match_groups)); // is it better/faster than overwrite/s?
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
    if (NULL != usubject) {
        efree(usubject);
    }
    if (NULL != match_groups) {
        zval_ptr_dtor(&match_groups);
    }
    if (NULL != callback) {
        efree(callback);
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
    int32_t group_count = 0;
    int i;

    REGEXP_METHOD_INIT_VARS

    array_init(return_value);

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ll", &object, Regexp_ce_ptr, &subject, &subject_len, &limit, &flags)) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "bad arguments", 0 TSRMLS_CC);
        goto end;
    }
    if (0 != (flags & ~(OFFSET_CAPTURE|SPLIT_NO_EMPTY|SPLIT_DELIM_CAPTURE))) {
        intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR, "invalid flag(s)", 0 TSRMLS_CC);
        goto end;
    }
    if (limit <= 0) {
        limit = INT_MAX;
    }

    REGEXP_METHOD_FETCH_OBJECT(TRUE);

    UTF8_TO_UTF16(ro, usubject, usubject_len, subject, subject_len);

    uregex_setText(ro->uregex, usubject, usubject_len, REGEXP_ERROR_CODE_P(ro));
    REGEXP_CHECK_STATUS(ro, "error setting text");
    if (0 != (flags & SPLIT_DELIM_CAPTURE)) {
        group_count = uregex_groupCount(ro->uregex, REGEXP_ERROR_CODE_P(ro));
        REGEXP_CHECK_STATUS(ro, "error counting groups");
    }
    /* We don't use uregex_split, it has few "limitations" */
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
        if (0 != (flags & SPLIT_DELIM_CAPTURE)) {
            int j;
            int32_t gu, gl;

            for (j = 1; j <= group_count; j++) {
                REGEXP_GROUP_START(ro, j, gl);
                REGEXP_GROUP_END(ro, j, gu);
                UTF16_TO_UTF8(ro, group, group_len, usubject + gl, gu - gl);
                if (!(flags & OFFSET_CAPTURE)) {
                    add_next_index_stringl(return_value, group, group_len, FALSE);
                } else {
                    add_index_stringl(return_value, u_countChar32(usubject, gl), group, group_len, FALSE);
                }
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
    REGEXP_PARSE_VOID_ARGS(FALSE);

    RETURN_LONG((long) REGEXP_ERROR_CODE(ro));
}

PHP_FUNCTION(regexp_get_error_message)
{
    const char *message = NULL;

    REGEXP_METHOD_INIT_VARS
    REGEXP_PARSE_VOID_ARGS(FALSE);

    message = intl_error_get_message(REGEXP_ERROR_P(ro) TSRMLS_CC);
    RETURN_STRING(message, 0);
}
