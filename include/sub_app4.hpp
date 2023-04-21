#ifndef SAPP4_HPP
#define SAPP4_HPP

#include "system/types.hpp"

bool Sapp4_query_init_flag(void);

bool Sapp4_query_running_flag(void);

void Sapp4_hid(Hid_info key);

void Sapp4_resume(void);

void Sapp4_suspend(void);

Result_with_string Sapp4_load_msg(std::string lang);

void Sapp4_init(bool draw);

void Sapp4_exit(bool draw);

void Sapp4_main(void);

#endif
