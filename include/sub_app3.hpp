#if !defined(DEF_SAPP3_HPP)
#define DEF_SAPP3_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_SAPP3_ENABLE

#define DEF_SAPP3_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP3_ENABLE_ICON
#define DEF_SAPP3_ENABLE_NAME
#define DEF_SAPP3_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP3_NAME			/*(const char*)(*/"Camera\nand mic\nsample"/*)*/
#define DEF_SAPP3_VER			/*(const char*)(*/"v0.0.1"/*)*/

bool Sapp3_query_init_flag(void);

bool Sapp3_query_running_flag(void);

void Sapp3_hid(Hid_info key);

void Sapp3_resume(void);

void Sapp3_suspend(void);

uint32_t Sapp3_load_msg(const char* lang);

void Sapp3_init(bool draw);

void Sapp3_exit(bool draw);

void Sapp3_main(void);

#endif //!defined(DEF_SAPP3_HPP)
