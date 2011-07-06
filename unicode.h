#ifndef INTL_UNICODE_H

# define INTL_UNICODE_H

#ifdef ZEND_DEBUG
# include <php.h>
static const char *ubasename(const char *filename)
{
    const char *c;

    if (NULL == (c = strrchr(filename, PHP_DIR_SEPARATOR))) {
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

int unicode_convert_needle_to_cp(zval *, UChar32 * TSRMLS_DC);

#endif /* !INTL_UNICODE_H */
