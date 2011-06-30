#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "regexp_class.h"
#include "regexp.h"

#include <unicode/uregex.h>

void regexp_register_constants(INIT_FUNC_ARGS)
{
    if (!Regexp_ce_ptr) {
        zend_error(E_ERROR, "Collator class not defined");
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
}
