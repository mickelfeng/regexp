#ifndef REGEXP_METHODS_H
#define REGEXP_METHODS_H

#include <php.h>

PHP_FUNCTION(regexp_create);
PHP_FUNCTION(regexp_match);
PHP_FUNCTION(regexp_match_all);
PHP_FUNCTION(regexp_replace);
PHP_FUNCTION(regexp_replace_callback);
PHP_FUNCTION(regexp_split);
PHP_FUNCTION(regexp_get_pattern);
PHP_FUNCTION(regexp_get_flags);
PHP_FUNCTION(regexp_get_error_code);
PHP_FUNCTION(regexp_get_error_message);

PHP_METHOD(Regexp, __construct);
PHP_METHOD(Regexp, match);
PHP_METHOD(Regexp, matchAll);
PHP_METHOD(Regexp, replace);
PHP_METHOD(Regexp, replaceCallback);
PHP_METHOD(Regexp, split);
PHP_METHOD(Regexp, getErrorCode);
PHP_METHOD(Regexp, getErrorMessage);
PHP_METHOD(Regexp, getPattern);
PHP_METHOD(Regexp, getFlags);

#endif /* #ifndef REGEXP_METHODS_H */
