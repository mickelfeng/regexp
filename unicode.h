#ifndef INTL_UNICODE_H

# define INTL_UNICODE_H

# ifdef ZEND_DEBUG
#  ifdef HAVE_ASSERT_H
#   include <assert.h>
#  else
#   define assert(expr)
#  endif /* HAVE_ASSERT_H */
#  include <php.h>
#  include <unicode/ustdio.h>
static const inline char *ubasename(const char *filename)
{
    const char *c;

    if (NULL == (c = strrchr(filename, PHP_DIR_SEPARATOR))) {
        return filename;
    } else {
        return c + 1;
    }
}

#  define debug(format, ...) \
    do {                                                                                                           \
        UFILE *ustderr = u_finit(stderr, NULL, NULL);                                                              \
        u_fprintf(ustderr, "%s:%d:" format " in %s()\n", ubasename(__FILE__), __LINE__, ## __VA_ARGS__, __func__); \
    } while (0);
# else
#  define assert(expr)
#  define debug(format, ...)
# endif /* ZEND_DEBUG */

# ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((unused))
# else
#  define UNUSED
# endif /* UNUSED */

# define mem_new(type)           emalloc((sizeof(type)))
# define mem_new_n(type, n)      emalloc((sizeof(type) * (n)))
# define mem_renew(ptr, type, n) erealloc((ptr), (sizeof(type) * (n)))

# define DEFAULT_NORMALIZATION UNORM_NFKC

typedef enum {
    UCASE_NONE,
    UCASE_FOLD,
    UCASE_LOWER,
    UCASE_UPPER,
    UCASE_TITLE,
    UCASE_COUNT
} UCaseType;

uint32_t get_option_from_locale(const char *locale);
int unicode_convert_needle_to_cp(zval *, UChar32 * TSRMLS_DC);
void utf8_fullcase(char **, int32_t *, const char *, int, const char *, UCaseType, UErrorCode *);
void utf16_fullcase(UChar **, int32_t *, const UChar *, int, const char *, UCaseType, UErrorCode *);

#endif /* !INTL_UNICODE_H */
