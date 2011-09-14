ZEND_BEGIN_ARG_INFO_EX(arginfo_chr, 0, 0, 1)
    ZEND_ARG_INFO(0, codepoint)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_string, 0, 0, 1)
    ZEND_ARG_INFO(0, string)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_string_plus_1optional, 0, 0, 1)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, optional)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_count_words, 0, 0, 1)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, format)
    ZEND_ARG_INFO(0, locale)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wordwrap, 0, 0, 1)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, width)
    ZEND_ARG_INFO(0, locale)
    ZEND_ARG_INFO(0, replacement)
    ZEND_ARG_INFO(0, cut_word)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_slice, 0, 0, 2)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, start)
    ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ncmp, 0, 0, 3)
    ZEND_ARG_INFO(0, string1)
    ZEND_ARG_INFO(0, string2)
    ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ncasecmp, 0, 0, 3)
    ZEND_ARG_INFO(0, string1)
    ZEND_ARG_INFO(0, string2)
    ZEND_ARG_INFO(0, length)
    ZEND_ARG_INFO(0, locale)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_casecmp, 0, 0, 2)
    ZEND_ARG_INFO(0, string1)
    ZEND_ARG_INFO(0, string2)
    ZEND_ARG_INFO(0, locale)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_index, 0, 0, 2)
    ZEND_ARG_INFO(0, haystack)
    ZEND_ARG_INFO(0, needle)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, case_insensitive)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_find, 0, 0, 2)
    ZEND_ARG_INFO(0, haystack)
    ZEND_ARG_INFO(0, needle)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, case_insensitive)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tr, 0, 0, 3)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, from)
    ZEND_ARG_INFO(0, to)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_starts_ends_with, 0, 0, 2)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, substring)
    ZEND_ARG_INFO(0, ignore_case)
    ZEND_ARG_INFO(0, locale)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_slice_cmp, 0, 0, 3)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, string_offset)
    ZEND_ARG_INFO(0, substring)
    ZEND_ARG_INFO(0, substring_offset)
    ZEND_ARG_INFO(0, length)
    ZEND_ARG_INFO(0, ignore_case)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_slice_count, 0, 0, 2)
    ZEND_ARG_INFO(0, haystack)
    ZEND_ARG_INFO(0, needle)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_slice_replace, 0, 0, 3)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, replacement)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ireplace, 0, 0, 3)
    ZEND_ARG_INFO(0, string)
    ZEND_ARG_INFO(0, search)
    ZEND_ARG_INFO(0, replacement)
    ZEND_ARG_INFO(0, locale)
    ZEND_ARG_INFO(1, count)
ZEND_END_ARG_INFO()
