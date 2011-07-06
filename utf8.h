#ifndef INTL_UTF8_H

# define INT_UTF8_H

typedef struct {
    int32_t cus_length;
    char cus[U8_MAX_LENGTH + 1];
} U8ReplacementCharData;

int32_t u8_countChar32(const uint8_t *, int32_t);
int utf8_cp_to_cu(const char *, int, long, int32_t * TSRMLS_DC);
void utf8_add_cp_replacement(HashTable *, UChar32, const char *, int32_t);
void utf8_foldcase(char **, int32_t *, const char *, int, UErrorCode *);

#endif /* !INTL_UTF8_H */
