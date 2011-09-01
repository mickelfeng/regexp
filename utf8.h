#ifndef INTL_UTF8_H

# define INTL_UTF8_H

#include <unicode/ucol.h>

# include "unicode.h"

# define UTF8_CP_TO_CU(string, string_len, cp_offset, cu_offset)                                              \
    do {                                                                                                      \
        if (0 != cp_offset) {                                                                                 \
            int32_t count_cp = utf8_countChar32((const uint8_t *) string, string_len);                        \
            if (cp_offset < 0) {                                                                              \
                if (cp_offset < -count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                cu_offset = string_len;                                                                       \
                U8_BACK_N((const uint8_t *) string, 0, cu_offset, -cp_offset);                                \
            } else {                                                                                          \
                if (cp_offset >= count_cp) {                                                                  \
                    intl_error_set(NULL, U_INDEX_OUTOFBOUNDS_ERROR, "code point out of bounds", 0 TSRMLS_CC); \
                    goto end;                                                                                 \
                }                                                                                             \
                U8_FWD_N((const uint8_t *) string, cu_offset, string_len, cp_offset);                         \
            }                                                                                                 \
        }                                                                                                     \
    } while (0);

typedef struct {
    int32_t cus_length;
    char cus[U8_MAX_LENGTH + 1];
} U8ReplacementCharData;

typedef enum {
    REPLACE_FORWARD,
    REPLACE_REVERSE
} ReplacementDirection;

int32_t utf8_countChar32(const uint8_t *, int32_t);
UBool utf8_validate(const uint8_t *, int32_t, UErrorCode *);
int utf8_cp_to_cu(const char *, int, int32_t, int32_t *, UErrorCode *status);
void utf8_add_cp_replacement(HashTable *, UChar32, const char *, int32_t);
void utf8_foldcase(char **, int32_t *, const char *, int, const char *, UErrorCode *);
void utf8_replace_len_from_utf16(char **, int *, char *, int, UChar *, int32_t, int32_t, int32_t, ReplacementDirection);
UBool utf8_unescape(const uint8_t *, int32_t, uint8_t **, int32_t *, UErrorCode *);
char *utf8_find(UCollator *ucol, char *, int32_t, char *, int32_t, int32_t, const char *, UBool, UBool, UErrorCode *);
int utf8_region_matches(UCollator *, const char *, int32_t, int32_t, const char *, int32_t, int32_t, int32_t, const char *, UBool, UErrorCode *status);

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
