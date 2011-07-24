#ifndef INTL_STRING_H

# include <php.h>

PHP_FUNCTION(utf8_len);
PHP_FUNCTION(utf8_chr);
PHP_FUNCTION(utf8_ord);
PHP_FUNCTION(utf8_reverse);

PHP_FUNCTION(utf8_count_words);
PHP_FUNCTION(utf8_count_chars);
PHP_FUNCTION(utf8_split);
PHP_FUNCTION(utf8_slice);
PHP_FUNCTION(utf8_slice_cmp);

PHP_FUNCTION(utf8_ncmp);
PHP_FUNCTION(utf8_ncasecmp);
PHP_FUNCTION(utf8_casecmp);
PHP_FUNCTION(utf8_startswith);
PHP_FUNCTION(utf8_endswith);

PHP_FUNCTION(utf8_toupper);
PHP_FUNCTION(utf8_tolower);
PHP_FUNCTION(utf8_totitle);

PHP_FUNCTION(utf8_firstsub);
PHP_FUNCTION(utf8_firstpos);
PHP_FUNCTION(utf8_lastsub);
PHP_FUNCTION(utf8_lastpos);

PHP_FUNCTION(utf8_tr);

PHP_FUNCTION(utf8_trim);
PHP_FUNCTION(utf8_rtrim);
PHP_FUNCTION(utf8_ltrim);

PHP_FUNCTION(utf8_shuffle);

PHP_FUNCTION(utf8_validate);

#endif /* !INTL_STRING_H */