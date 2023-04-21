#ifndef SAPP0_HPP
#define SAPP0_HPP

#include "system/types.hpp"

bool Sapp0_query_init_flag(void);

bool Sapp0_query_running_flag(void);

void Sapp0_hid(Hid_info key);

void Sapp0_resume(void);

void Sapp0_suspend(void);

Result_with_string Sapp0_load_msg(std::string lang);

void Sapp0_init(bool draw);

void Sapp0_exit(bool draw);

void Sapp0_main(void);

#endif
