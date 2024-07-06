#if !defined(DEF_SAPP4_HPP)
#define DEF_SAPP4_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/types.hpp"
#include "system/util/hid_types.h"

bool Sapp4_query_init_flag(void);

bool Sapp4_query_running_flag(void);

void Sapp4_hid(Hid_info key);

void Sapp4_resume(void);

void Sapp4_suspend(void);

Result_with_string Sapp4_load_msg(std::string lang);

void Sapp4_init(bool draw);

void Sapp4_exit(bool draw);

void Sapp4_main(void);

#endif //!defined(DEF_SAPP4_HPP)
