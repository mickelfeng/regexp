#ifndef INTL_EXTENDED_ERROR_H

# define INTL_EXTENDED_ERROR_H

# include <php.h>
# include "php_intl.h"
# include <unicode/utypes.h>

void intl_errors_setf_custom_msg(intl_error * TSRMLS_DC, char *, ...);
int intl_error_non_quiet_set_code(UErrorCode TSRMLS_DC);

#endif /* !INTL_EXTENDED_ERROR_H */
