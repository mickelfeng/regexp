#include "regexp_class.h"
#include "php_intl.h"
#include "regexp_methods.h"
#include "intl_error.h"
#include "intl_convert.h"
#include "intl_data.h"

#include <unicode/uregex.h>

zend_class_entry *Regexp_ce_ptr = NULL;

zend_object_handlers Regexp_handlers;


static void regexp_object_init(Regexp_object* ro TSRMLS_DC )
{
    if (NULL == ro) {
        return;
    }

    intl_error_init(REGEXP_ERROR_P(ro) TSRMLS_CC);
}

static void regexp_object_destroy(Regexp_object* ro TSRMLS_DC )
{
    if (NULL == ro) {
        return;
    }

    if (NULL != ro->uregex) {
        uregex_close(ro->uregex);
        ro->uregex = NULL;
    }

    intl_error_reset(REGEXP_ERROR_P(ro) TSRMLS_CC);
}

static void Regexp_objects_dtor(void *object, zend_object_handle handle TSRMLS_DC )
{
    zend_objects_destroy_object(object, handle TSRMLS_CC);
}

static void Regexp_objects_free(zend_object *object TSRMLS_DC)
{
    Regexp_object* ro = (Regexp_object*) object;

    zend_object_std_dtor(&ro->zo TSRMLS_CC);

    regexp_object_destroy(ro TSRMLS_CC);

    efree(ro);
}

static zend_object_value Regexp_object_create(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    Regexp_object* intern;

    intern = ecalloc(1, sizeof(Regexp_object));
    intl_error_init(REGEXP_ERROR_P(intern) TSRMLS_CC);
    zend_object_std_init(&intern->zo, ce TSRMLS_CC);

    retval.handle = zend_objects_store_put(intern, Regexp_objects_dtor, (zend_objects_free_object_storage_t) Regexp_objects_free, NULL TSRMLS_CC);
    retval.handlers = &Regexp_handlers;

    return retval;
}

/*static zend_object_value Regexp_clone_obj(zval *object TSRMLS_DC)
{
    Regexp_object *to_orig, *to_new;
    zend_object_value ret_val;
    intl_error_reset(NULL TSRMLS_CC);

    to_orig = zend_object_store_get_object(object TSRMLS_CC);
    intl_error_reset(INTL_DATA_ERROR_P(to_orig) TSRMLS_CC);
    ret_val = Regexp_ce_ptr->create_object(Regexp_ce_ptr TSRMLS_CC);
    to_new = zend_object_store_get_object_by_handle(ret_val.handle TSRMLS_CC);

    zend_objects_clone_members(&to_new->zo, ret_val, &to_orig->zo, Z_OBJ_HANDLE_P(object) TSRMLS_CC);

    if (NULL != to_orig->uregex) {
        URegularExpression *uregex = NULL;
        zval tempz;

        uregex = uregex_clone(to_orig->uregex, REGEXP_ERROR_CODE_P(to_orig));

        if (U_FAILURE(REGEXP_ERROR_CODE(to_orig))) {
            goto err;
        }

        Z_OBJVAL(tempz) = ret_val;
        regexp_object_construct(&tempz, uregex, REGEXP_ERROR_CODE_P(to_orig) TSRMLS_CC);

        if (U_FAILURE(REGEXP_ERROR_CODE(to_orig))) {
            char *err_msg;
err:

            if (NULL != uregex) {
                regexp_object_destroy(to_new TSRMLS_CC);
            }

            intl_error_set_code(NULL, INTL_DATA_ERROR_CODE(to_orig) TSRMLS_CC);
            intl_errors_set_custom_msg(REGEXP_ERROR_P(to_orig), "Could not clone regular expression", 0 TSRMLS_CC);

            err_msg = intl_error_get_message(REGEXP_ERROR_P(to_orig) TSRMLS_CC);
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "%s", err_msg);
            efree(err_msg);
        }
    } else {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cloning unconstructed regular expression.");
    }

    return ret_val;
}*/

ZEND_BEGIN_ARG_INFO_EX(ainfo_regexp_void, 0, 0, 0 )
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_regexp_create, 0, 0, 1)
    ZEND_ARG_INFO(0, pattern)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_regexp_match, 0, 0, 2)
    ZEND_ARG_INFO(0, subject)
    ZEND_ARG_INFO(1, match)
    ZEND_ARG_INFO(0, start_offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_regexp_replace, 0, 0, 2)
    ZEND_ARG_INFO(0, subject)
    ZEND_ARG_INFO(0, replacement)
    //ZEND_ARG_INFO(0, limit)
    //ZEND_ARG_INFO(1, count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_regexp_split, 0, 0, 1)
    ZEND_ARG_INFO(0, subject)
    ZEND_ARG_INFO(0, limit)
    //ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

zend_function_entry Regexp_class_functions[] = {
    PHP_ME(Regexp, __construct, ainfo_regexp_void, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME_MAPPING(create, regexp_create, ainfo_regexp_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME_MAPPING(match, regexp_match, ainfo_regexp_match, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(matchAll, regexp_match_all, ainfo_regexp_match, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(replace, regexp_replace, ainfo_regexp_replace, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(split, regexp_split, ainfo_regexp_split, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getPattern, regexp_get_pattern, ainfo_regexp_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getFlags, regexp_get_flags, ainfo_regexp_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getErrorCode, regexp_get_error_code, ainfo_regexp_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getErrorMessage, regexp_get_error_message, ainfo_regexp_void, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

void regexp_register_Regexp_class(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Regexp", Regexp_class_functions);
    ce.create_object = Regexp_object_create;
    Regexp_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&Regexp_handlers, zend_get_std_object_handlers(), sizeof(Regexp_handlers));
//     Regexp_handlers.clone_obj = Regexp_clone_obj;

    if (!Regexp_ce_ptr) {
        zend_error(E_ERROR, "Regexp: attempt to create properties on a non-registered class.");
        return;
    }
}
