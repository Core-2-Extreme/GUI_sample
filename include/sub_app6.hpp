#if !defined(DEF_SAPP6_HPP)
#define DEF_SAPP6_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/types.hpp"
#include "system/util/hid_types.h"

bool Sapp6_query_init_flag(void);

bool Sapp6_query_running_flag(void);

void Sapp6_hid(Hid_info key);

void Sapp6_resume(void);

void Sapp6_suspend(void);

Result_with_string Sapp6_load_msg(std::string lang);

void Sapp6_init(bool draw);

void Sapp6_exit(bool draw);

void Sapp6_main(void);

#endif //!defined(DEF_SAPP6_HPP)
