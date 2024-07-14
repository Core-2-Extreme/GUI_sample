#if !defined(DEF_SAPP3_HPP)
#define DEF_SAPP3_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

bool Sapp3_query_init_flag(void);

bool Sapp3_query_running_flag(void);

void Sapp3_hid(Hid_info key);

void Sapp3_resume(void);

void Sapp3_suspend(void);

uint32_t Sapp3_load_msg(const char* lang);

void Sapp3_init(bool draw);

void Sapp3_exit(bool draw);

void Sapp3_main(void);

#endif //!defined(DEF_SAPP3_HPP)
