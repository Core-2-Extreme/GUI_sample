#if !defined(DEF_MENU_HPP)
#define DEF_MENU_HPP
#include <stdbool.h>
#include <stdint.h>

bool Menu_query_must_exit_flag(void);

void Menu_set_must_exit_flag(bool flag);

void Menu_resume(void);

void Menu_suspend(void);

uint32_t Menu_load_msg(const char* lang);

void Menu_init(void);

bool Menu_add_worker_thread_callback(void (*callback)(void));

void Menu_remove_worker_thread_callback(void (*callback)(void));

void Menu_exit(void);

void Menu_main(void);

#endif //!defined(DEF_MENU_HPP)
