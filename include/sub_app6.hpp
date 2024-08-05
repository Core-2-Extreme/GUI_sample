#if !defined(DEF_SAPP6_HPP)
#define DEF_SAPP6_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_SAPP6_ENABLE

#define DEF_SAPP6_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP6_ENABLE_ICON
#define DEF_SAPP6_ENABLE_NAME
#define DEF_SAPP6_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP6_NAME			/*(const char*)(*/"sample 6"/*)*/
#define DEF_SAPP6_VER			/*(const char*)(*/"v0.0.1"/*)*/

bool Sapp6_query_init_flag(void);

bool Sapp6_query_running_flag(void);

void Sapp6_hid(Hid_info key);

void Sapp6_resume(void);

void Sapp6_suspend(void);

uint32_t Sapp6_load_msg(const char* lang);

void Sapp6_init(bool draw);

void Sapp6_exit(bool draw);

void Sapp6_main(void);

#endif //!defined(DEF_SAPP6_HPP)
