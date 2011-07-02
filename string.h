#ifndef INTL_STRING_H

# include <php.h>

PHP_FUNCTION(utf8_strlen);
PHP_FUNCTION(utf8_chr);
PHP_FUNCTION(utf8_ord);

PHP_FUNCTION(utf8_str_word_count);
PHP_FUNCTION(utf8_str_count_chars);
PHP_FUNCTION(utf8_str_split);
PHP_FUNCTION(utf8_str_sub);

PHP_FUNCTION(utf8_strtoupper);
PHP_FUNCTION(utf8_strtolower);
PHP_FUNCTION(utf8_strtotitle);

#endif /* !INTL_STRING_H */