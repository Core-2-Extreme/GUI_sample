#if !defined(DEF_SAPP0_H)
#define DEF_SAPP0_H
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_SAPP0_ENABLE

#define DEF_SAPP0_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP0_ENABLE_ICON
#define DEF_SAPP0_ENABLE_NAME
#define DEF_SAPP0_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP0_NAME			/*(const char*)(*/"draw image\nsample"/*)*/
#define DEF_SAPP0_VER			/*(const char*)(*/"v0.0.1"/*)*/

bool Sapp0_query_init_flag(void);

bool Sapp0_query_running_flag(void);

void Sapp0_hid(Hid_info key);

void Sapp0_resume(void);

void Sapp0_suspend(void);

uint32_t Sapp0_load_msg(const char* lang);

void Sapp0_init(bool draw);

void Sapp0_exit(bool draw);

void Sapp0_main(void);

#endif //!defined(DEF_SAPP0_H)
