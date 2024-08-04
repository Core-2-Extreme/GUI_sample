#if !defined(DEF_SAPP5_HPP)
#define DEF_SAPP5_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_ENABLE_SUB_APP5

#define DEF_SAPP5_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP5_ENABLE_ICON
#define DEF_SAPP5_ENABLE_NAME
#define DEF_SAPP5_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP5_NAME			/*(const char*)(*/"sample 5"/*)*/
#define DEF_SAPP5_VER			/*(const char*)(*/"v0.0.1"/*)*/

bool Sapp5_query_init_flag(void);

bool Sapp5_query_running_flag(void);

void Sapp5_hid(Hid_info key);

void Sapp5_resume(void);

void Sapp5_suspend(void);

uint32_t Sapp5_load_msg(const char* lang);

void Sapp5_init(bool draw);

void Sapp5_exit(bool draw);

void Sapp5_main(void);

#endif //!defined(DEF_SAPP5_HPP)
