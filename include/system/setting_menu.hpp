#if !defined(DEF_SETTING_MENU_HPP)
#define DEF_SETTING_MENU_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

bool Sem_query_init_flag(void);

bool Sem_query_running_flag(void);

void Sem_resume(void);

void Sem_suspend(void);

uint32_t Sem_load_msg(const char* lang);

void Sem_init(void);

void Sem_draw_init(void);

void Sem_exit(void);

void Sem_main(void);

void Sem_hid(Hid_info key);

#endif //!defined(DEF_SETTING_MENU_HPP)
