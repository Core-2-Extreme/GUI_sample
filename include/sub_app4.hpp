#if !defined(DEF_SAPP4_HPP)
#define DEF_SAPP4_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

bool Sapp4_query_init_flag(void);

bool Sapp4_query_running_flag(void);

void Sapp4_hid(Hid_info key);

void Sapp4_resume(void);

void Sapp4_suspend(void);

uint32_t Sapp4_load_msg(const char* lang);

void Sapp4_init(bool draw);

void Sapp4_exit(bool draw);

void Sapp4_main(void);

#endif //!defined(DEF_SAPP4_HPP)
