#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "regexp_class.h"
#include "regexp.h"

#include <unicode/uregex.h>

void regexp_register_constants(INIT_FUNC_ARGS)
{
    if (!Regexp_ce_ptr) {
        zend_error(E_ERROR, "Regexp class not defined");
        return;
    }

#define REGEXP_EXPOSE_CLASS_CONST(x) \
    zend_declare_class_constant_long(Regexp_ce_ptr, ZEND_STRS(#x) - 1, UREGEX_##x TSRMLS_CC);

    REGEXP_EXPOSE_CLASS_CONST(CANON_EQ);
    REGEXP_EXPOSE_CLASS_CONST(CASE_INSENSITIVE);
    REGEXP_EXPOSE_CLASS_CONST(COMMENTS);
    REGEXP_EXPOSE_CLASS_CONST(DOTALL);
    REGEXP_EXPOSE_CLASS_CONST(LITERAL);
    REGEXP_EXPOSE_CLASS_CONST(MULTILINE);
    REGEXP_EXPOSE_CLASS_CONST(UNIX_LINES);
    REGEXP_EXPOSE_CLASS_CONST(UWORD);
    REGEXP_EXPOSE_CLASS_CONST(ERROR_ON_UNKNOWN_ESCAPES);

#undef REGEXP_EXPOSE_CLASS_CONST

#define REGEXP_FLAG_CLASS_CONST(name) \
    zend_declare_class_constant_long(Regexp_ce_ptr, ZEND_STRS(#name) - 1, name TSRMLS_CC);

    REGEXP_FLAG_CLASS_CONST(OFFSET_CAPTURE);
    REGEXP_FLAG_CLASS_CONST(MATCH_ALL_PATTERN_ORDER);
    REGEXP_FLAG_CLASS_CONST(MATCH_ALL_SET_ORDER);
    REGEXP_FLAG_CLASS_CONST(SPLIT_NO_EMPTY);
    REGEXP_FLAG_CLASS_CONST(SPLIT_DELIM_CAPTURE);

#undef REGEXP_EXPOSE_CLASS_CONST
}
