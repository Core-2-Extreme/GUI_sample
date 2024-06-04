#if !defined(SAPP6_HPP)
#define SAPP6_HPP

#include "system/types.hpp"
#include "system/util/hid.hpp"

bool Sapp6_query_init_flag(void);

bool Sapp6_query_running_flag(void);

void Sapp6_hid(Hid_info key);

void Sapp6_resume(void);

void Sapp6_suspend(void);

Result_with_string Sapp6_load_msg(std::string lang);

void Sapp6_init(bool draw);

void Sapp6_exit(bool draw);

void Sapp6_main(void);

#endif //!defined(SAPP6_HPP)
