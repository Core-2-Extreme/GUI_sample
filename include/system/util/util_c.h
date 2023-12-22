#ifndef UTIL_H_
#define UTIL_H_
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Sleep thread.
 * @param ns (in) Time to sleep in us.
 * @note Thread safe
*/
void Util_sleep(int64_t us);

#endif //UTIL_H_
