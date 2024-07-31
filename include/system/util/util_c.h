#if !defined(DEF_UTIL_H)
#define DEF_UTIL_H
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Sleep thread.
 * @param ns (in) Time to sleep in us.
 * @note Thread safe
*/
void Util_sleep(uint64_t us);

#endif //!defined(DEF_UTIL_H)
