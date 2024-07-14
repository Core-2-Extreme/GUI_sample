#if !defined(DEF_SAPP2_HPP)
#define DEF_SAPP2_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

bool Sapp2_query_init_flag(void);

bool Sapp2_query_running_flag(void);

void Sapp2_hid(Hid_info key);

void Sapp2_resume(void);

void Sapp2_suspend(void);

uint32_t Sapp2_load_msg(const char* lang);

void Sapp2_init(bool draw);

void Sapp2_exit(bool draw);

void Sapp2_main(void);

#endif //!defined(DEF_SAPP2_HPP)
