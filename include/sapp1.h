#if !defined(DEF_SAPP1_H)
#define DEF_SAPP1_H
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_SAPP1_ENABLE

#define DEF_SAPP1_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP1_ENABLE_ICON
#define DEF_SAPP1_ENABLE_NAME
#define DEF_SAPP1_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP1_NAME			/*(const char*)(*/"file explorer\nsample"/*)*/
#define DEF_SAPP1_VER			/*(const char*)(*/"v0.0.1"/*)*/

bool Sapp1_query_init_flag(void);

bool Sapp1_query_running_flag(void);

void Sapp1_hid(Hid_info key);

void Sapp1_resume(void);

void Sapp1_suspend(void);

uint32_t Sapp1_load_msg(const char* lang);

void Sapp1_init(bool draw);

void Sapp1_exit(bool draw);

void Sapp1_main(void);

#endif //!defined(DEF_SAPP1_H)
