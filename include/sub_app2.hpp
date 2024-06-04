#if !defined(SAPP2_HPP)
#define SAPP2_HPP

#include "system/types.hpp"
#include "system/util/hid.hpp"

bool Sapp2_query_init_flag(void);

bool Sapp2_query_running_flag(void);

void Sapp2_hid(Hid_info key);

void Sapp2_resume(void);

void Sapp2_suspend(void);

Result_with_string Sapp2_load_msg(std::string lang);

void Sapp2_init(bool draw);

void Sapp2_exit(bool draw);

void Sapp2_main(void);

#endif //!defined(SAPP2_HPP)
