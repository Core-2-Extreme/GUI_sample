#pragma once
#include "system/types.hpp"

bool Sapp6_query_init_flag(void);

bool Sapp6_query_running_flag(void);

void Sapp6_hid(Hid_info key);

void Sapp6_resume(void);

void Sapp6_suspend(void);

Result_with_string Sapp6_load_msg(std::string lang);

void Sapp6_init(bool draw);

void Sapp6_exit(bool draw);

void Sapp6_main(void);
