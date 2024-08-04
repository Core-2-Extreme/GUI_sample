#if !defined(DEF_SAPP4_HPP)
#define DEF_SAPP4_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_ENABLE_SUB_APP4

#define DEF_SAPP4_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP4_ENABLE_ICON
#define DEF_SAPP4_ENABLE_NAME
#define DEF_SAPP4_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP4_NAME			/*(const char*)(*/"Speaker\nsample"/*)*/
#define DEF_SAPP4_VER			/*(const char*)(*/"v0.0.1"/*)*/

bool Sapp4_query_init_flag(void);

bool Sapp4_query_running_flag(void);

void Sapp4_hid(Hid_info key);

void Sapp4_resume(void);

void Sapp4_suspend(void);

uint32_t Sapp4_load_msg(const char* lang);

void Sapp4_init(bool draw);

void Sapp4_exit(bool draw);

void Sapp4_main(void);

#endif //!defined(DEF_SAPP4_HPP)
