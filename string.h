#ifndef INTL_STRING_H

# include <php.h>

PHP_FUNCTION(utf8_len);
PHP_FUNCTION(utf8_chr);
PHP_FUNCTION(utf8_ord);

PHP_FUNCTION(utf8_word_count);
PHP_FUNCTION(utf8_count_chars);
PHP_FUNCTION(utf8_split);
PHP_FUNCTION(utf8_sub);

PHP_FUNCTION(utf8_toupper);
PHP_FUNCTION(utf8_tolower);
PHP_FUNCTION(utf8_totitle);

#endif /* !INTL_STRING_H */