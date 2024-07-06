#if !defined(DEF_SAPP7_HPP)
#define DEF_SAPP7_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/types.hpp"
#include "system/util/hid_types.h"

bool Sapp7_query_init_flag(void);

bool Sapp7_query_running_flag(void);

void Sapp7_hid(Hid_info key);

void Sapp7_resume(void);

void Sapp7_suspend(void);

Result_with_string Sapp7_load_msg(std::string lang);

void Sapp7_init(bool draw);

void Sapp7_exit(bool draw);

void Sapp7_main(void);

#endif //!defined(DEF_SAPP7_HPP)
