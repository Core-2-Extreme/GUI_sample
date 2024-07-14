#if !defined(DEF_SAPP1_HPP)
#define DEF_SAPP1_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

bool Sapp1_query_init_flag(void);

bool Sapp1_query_running_flag(void);

void Sapp1_hid(Hid_info key);

void Sapp1_resume(void);

void Sapp1_suspend(void);

uint32_t Sapp1_load_msg(const char* lang);

void Sapp1_init(bool draw);

void Sapp1_exit(bool draw);

void Sapp1_main(void);

#endif //!defined(DEF_SAPP1_HPP)
