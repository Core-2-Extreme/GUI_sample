#if !defined(DEF_SAPP7_HPP)
#define DEF_SAPP7_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_SAPP7_ENABLE

#define DEF_SAPP7_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP7_ENABLE_ICON
#define DEF_SAPP7_ENABLE_NAME
#define DEF_SAPP7_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP7_NAME			/*(const char*)(*/"sample 7"/*)*/
#define DEF_SAPP7_VER			/*(const char*)(*/"v0.0.1"/*)*/

bool Sapp7_query_init_flag(void);

bool Sapp7_query_running_flag(void);

void Sapp7_hid(Hid_info key);

void Sapp7_resume(void);

void Sapp7_suspend(void);

uint32_t Sapp7_load_msg(const char* lang);

void Sapp7_init(bool draw);

void Sapp7_exit(bool draw);

void Sapp7_main(void);

#endif //!defined(DEF_SAPP7_HPP)
