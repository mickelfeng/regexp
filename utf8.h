#ifndef INTL_UTF8_H

# define INT_UTF8_H

typedef struct {
    int32_t cus_length;
    char cus[U8_MAX_LENGTH + 1];
} U8ReplacementCharData;

int32_t utf8_countChar32(const uint8_t *, int32_t);
UBool utf8_validate(const uint8_t *, int32_t, UErrorCode *);
int utf8_cp_to_cu(const char *, int, int32_t, int32_t *, UErrorCode *status);
void utf8_add_cp_replacement(HashTable *, UChar32, const char *, int32_t);
void utf8_foldcase(char **, int32_t *, const char *, int, UErrorCode *);
int utf8_region_matches(const char *, int32_t, int32_t, const char *, int32_t, int32_t, int32_t, const char *, UNormalizationMode, UCaseType, UErrorCode *);
void utf8_replace_len_from_utf16(char *, int *, char *, int, UChar *, int32_t, int32_t);

# ifdef ZEND_DEBUG
#  define UTF8_DEBUG_STRING(string, string_len)        \
    do {                                               \
        debug("tmp1 = :%s: (%d)", string, string_len); \
        UChar32 c;                                     \
        int cu, cp;                                    \
                                                       \
        for (cp = cu = 0; cu < string_len; cp++) {     \
            U8_NEXT(string, cu, string_len, c);        \
            debug("%d => 0x%08X", cp, c);              \
        }                                              \
    } while (0);
# else
#  define UTF8_DEBUG_STRING(string, string_len)
# endif /* ZEND_DEBUG */

#endif /* !INTL_UTF8_H */
