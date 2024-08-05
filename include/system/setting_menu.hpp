#if !defined(DEF_SEM_HPP)
#define DEF_SEM_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/hid_types.h"

#define DEF_SEM_UPDATE_DIR_PREFIX		/*(const char*)(*/"/3ds/GUI_sample_ver_"/*)*/
#define DEF_SEM_UPDATE_FILE_PREFIX		/*(const char*)(*/"GUI"/*)*/
#define DEF_SEM_CHECK_UPDATE_URL		/*(const char*)(*/"https://script.google.com/macros/s/AKfycbwvEedP97o8vgfpAG6EzcW6jxZZqFfZaMaqE1V7kCdp9BfuXySfRQ4own5CcFW1JxRBBQ/exec"/*)*/

#define DEF_SEM_NUM_OF_MSG				(uint16_t)(71)
#define DEF_SEM_ENABLE_ICON
//#define DEF_SEM_ENABLE_NAME
#define DEF_SEM_ICON_PATH				(const char*)"romfs:/gfx/draw/icon/sem_icon.t3x"
#define DEF_SEM_NAME					(const char*)"Settings"

#define DEF_SEM_MENU_TOP				(int8_t)(-1)
#define DEF_SEM_MENU_UPDATE				(int8_t)(0)
#define DEF_SEM_MENU_LANGAGES			(int8_t)(1)
#define DEF_SEM_MENU_LCD				(int8_t)(2)
#define DEF_SEM_MENU_CONTROL			(int8_t)(3)
#define DEF_SEM_MENU_FONT				(int8_t)(4)
#define DEF_SEM_MENU_WIFI				(int8_t)(5)
#define DEF_SEM_MENU_ADVANCED			(int8_t)(6)
#define DEF_SEM_MENU_BATTERY			(int8_t)(7)
#define DEF_SEM_MENU_RECORDING			(int8_t)(8)

#define DEF_SEM_RECORD_BOTH				(uint8_t)(0)
#define DEF_SEM_RECORD_TOP				(uint8_t)(1)
#define DEF_SEM_RECORD_BOTTOM			(uint8_t)(2)

#define DEF_SEM_EDTION_NONE				(int8_t)(-1)
#define DEF_SEM_EDTION_3DSX				(int8_t)(0)
#define DEF_SEM_EDTION_CIA				(int8_t)(1)

#define DEF_SEM_SCREEN_AUTO				(uint8_t)(0)
#define DEF_SEM_SCREEN_400PX			(uint8_t)(1)
#define DEF_SEM_SCREEN_800PX			(uint8_t)(2)
#define DEF_SEM_SCREEN_3D				(uint8_t)(3)

#define DEF_SEM_UPDATE_MSG					(uint16_t)(0)
#define DEF_SEM_LANGAGES_MSG				(uint16_t)(1)
#define DEF_SEM_LCD_MSG						(uint16_t)(2)
#define DEF_SEM_CONTROL_MSG					(uint16_t)(3)
#define DEF_SEM_FONT_MSG					(uint16_t)(4)
#define DEF_SEM_WIFI_MSG					(uint16_t)(5)
#define DEF_SEM_ADVANCED_MSG				(uint16_t)(6)
#define DEF_SEM_BATTERY_MSG					(uint16_t)(7)
#define DEF_SEM_CHECK_UPDATE_MSG			(uint16_t)(8)
#define DEF_SEM_ENGLISH_MSG					(uint16_t)(9)
#define DEF_SEM_JAPANESE_MSG				(uint16_t)(10)
#define DEF_SEM_NIGHT_MODE_MSG				(uint16_t)(11)
#define DEF_SEM_ON_MSG						(uint16_t)(12)
#define DEF_SEM_OFF_MSG						(uint16_t)(13)
#define DEF_SEM_FLASH_MSG					(uint16_t)(14)
#define DEF_SEM_BRIGHTNESS_MSG				(uint16_t)(15)
#define DEF_SEM_LCD_OFF_TIME_0_MSG			(uint16_t)(16)
#define DEF_SEM_LCD_OFF_TIME_1_MSG			(uint16_t)(17)
#define DEF_SEM_SCROLL_SPEED_MSG			(uint16_t)(18)
#define DEF_SEM_JAPANESE_FONT_MSG			(uint16_t)(19)
#define DEF_SEM_CHINESE_FONT_MSG			(uint16_t)(20)
#define DEF_SEM_KOREAN_FONT_MSG				(uint16_t)(21)
#define DEF_SEM_TAIWANESE_FONT_MSG			(uint16_t)(22)
#define DEF_SEM_LOAD_ALL_FONT_MSG			(uint16_t)(23)
#define DEF_SEM_UNLOAD_ALL_FONT_MSG			(uint16_t)(24)
#define DEF_SEM_SEND_INFO_MODE_MSG			(uint16_t)(25)
#define DEF_SEM_ALLOW_MSG					(uint16_t)(26)
#define DEF_SEM_DENY_MSG					(uint16_t)(27)
#define DEF_SEM_DEBUG_MODE_MSG				(uint16_t)(28)
#define DEF_SEM_ECO_MODE_MSG				(uint16_t)(29)
#define DEF_SEM_BACK_MSG					(uint16_t)(30)
#define DEF_SEM_CHECKING_UPDATE_MSG			(uint16_t)(31)
#define DEF_SEM_CHECKING_UPDATE_FAILED_MSG	(uint16_t)(32)
#define DEF_SEM_UP_TO_DATE_MSG				(uint16_t)(33)
#define DEF_SEM_NEW_VERSION_AVAILABLE_MSG	(uint16_t)(34)
#define DEF_SEM_CLOSE_UPDATER_MSG			(uint16_t)(35)
#define DEF_SEM_BACK_TO_PATCH_NOTE_MSG		(uint16_t)(35)	//Same as CLOSE_UPDATER_MSG.
#define DEF_SEM_SELECT_EDITION_MSG			(uint16_t)(36)
#define DEF_SEM_3DSX_MSG					(uint16_t)(37)
#define DEF_SEM_CIA_MSG						(uint16_t)(38)
#define DEF_SEM_FILE_PATH_MSG				(uint16_t)(39)
#define DEF_SEM_DOWNLOADING_MSG				(uint16_t)(40)
#define DEF_SEM_INSTALLING_MSG				(uint16_t)(41)
#define DEF_SEM_SUCCESS_MSG					(uint16_t)(42)
#define DEF_SEM_FAILURE_MSG					(uint16_t)(43)
#define DEF_SEM_RESTART_MSG					(uint16_t)(44)
#define DEF_SEM_DL_INSTALL_MSG				(uint16_t)(45)
#define DEF_SEM_CLOSE_APP_MSG				(uint16_t)(46)
#define DEF_SEM_WIFI_MODE_MSG				(uint16_t)(47)
#define DEF_SEM_CONNECTED_SSID_MSG			(uint16_t)(48)
#define DEF_SEM_RECORDING_MSG				(uint16_t)(49)
#define DEF_SEM_RECORD_BOTH_LCD_MSG			(uint16_t)(50)
#define DEF_SEM_RECORD_TOP_LCD_MSG			(uint16_t)(51)
#define DEF_SEM_RECORD_BOTTOM_LCD_MSG		(uint16_t)(52)
#define DEF_SEM_STOP_RECORDING_MSG			(uint16_t)(53)
#define DEF_SEM_LCD_MODE_MSG				(uint16_t)(54)
#define DEF_SEM_CANNOT_RECORD_MSG			(uint16_t)(55)
#define DEF_SEM_800PX_MSG					(uint16_t)(56)
#define DEF_SEM_3D_MSG						(uint16_t)(57)
#define DEF_SEM_400PX_MSG					(uint16_t)(58)
#define DEF_SEM_HUNGARIAN_MSG				(uint16_t)(59)
#define DEF_SEM_CHINESE_MSG					(uint16_t)(60)
#define DEF_SEM_ITALIAN_MSG					(uint16_t)(61)
#define DEF_SEM_FAKE_MODEL_MSG				(uint16_t)(62)
#define DEF_SEM_SPANISH_MSG					(uint16_t)(63)
#define DEF_SEM_ROMANIAN_MSG				(uint16_t)(64)
#define DEF_SEM_POLISH_MSG					(uint16_t)(65)
#define DEF_SEM_CPU_USAGE_MONITOR_MSG		(uint16_t)(66)
#define DEF_SEM_DUMP_LOGS_MSG				(uint16_t)(67)
#define DEF_SEM_RYUKYUAN_MSG				(uint16_t)(68)
#define DEF_SEM_AUTO_MSG					(uint16_t)(69)
#define DEF_SEM_SLEEP_TIME_MSG				(uint16_t)(70)

//You need to enable DEF_CONVERTER_SW_API_ENABLE **and** DEF_ENCODER_VIDEO_AUDIO_API_ENABLE as well to use screen recorder.
#define DEF_SEM_ENABLE_SCREEN_RECORDER		/*(bool)(*/true/*)*/
//You need to enable DEF_HTTPC_API_ENABLE **or** DEF_CURL_API_ENABLE as well to use updater.
#define DEF_SEM_ENABLE_UPDATER				/*(bool)(*/true/*)*/

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

#endif //!defined(DEF_SEM_HPP)
