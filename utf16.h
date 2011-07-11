#ifndef INTL_UTF16_H

# define INT_UTF16_H

void utf16_foldcase(UChar **, int32_t *, const UChar *, int32_t, UErrorCode *);
int utf16_cp_to_cu(const UChar *, int32_t, long, int32_t *, UErrorCode *status);

#endif /* !INTL_UTF16_H */
