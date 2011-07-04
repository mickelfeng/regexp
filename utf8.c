#include <unicode/utf8.h>
#include "utf8.h"

int32_t u8_countChar32(const uint8_t *string, int32_t length)
{
    int32_t cpcount, cucount;

    if (NULL == string || length < 0) {
        return 0;
    }

    cpcount = 0;
    if (length >= 0) {
        while (length > 0) {
            cucount = U8_COUNT_TRAIL_BYTES(*string) + 1;
            if (length >= cucount) {
                string += cucount;
                length -= cucount;
                cpcount++;
            }
        }
    }

    return cpcount;
}
