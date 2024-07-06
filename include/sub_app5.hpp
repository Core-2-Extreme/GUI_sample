#if !defined(DEF_SAPP5_HPP)
#define DEF_SAPP5_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/types.hpp"
#include "system/util/hid_types.h"

bool Sapp5_query_init_flag(void);

bool Sapp5_query_running_flag(void);

void Sapp5_hid(Hid_info key);

void Sapp5_resume(void);

void Sapp5_suspend(void);

Result_with_string Sapp5_load_msg(std::string lang);

void Sapp5_init(bool draw);

void Sapp5_exit(bool draw);

void Sapp5_main(void);

#endif //!defined(DEF_SAPP5_HPP)
