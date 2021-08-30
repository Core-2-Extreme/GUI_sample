#pragma once

//menu
#define DEF_MENU_NUM_OF_MSG 5
#define DEF_MENU_MAIN_STR (std::string)"Menu/Main"
#define DEF_MENU_INIT_STR (std::string)"Menu/Init"
#define DEF_MENU_EXIT_STR (std::string)"Menu/Exit"
#define DEF_MENU_WORKER_THREAD_STR (std::string)"Menu/Worker thread"
#define DEF_MENU_UPDATE_THREAD_STR (std::string)"Menu/Update thread"
#define DEF_MENU_SEND_APP_INFO_STR (std::string)"Menu/Send app info thread"
#define DEF_MENU_CHECK_INTERNET_STR (std::string)"Menu/Check internet thread"
#define DEF_MENU_EXIST_MSG 0
#define DEF_MENU_CONFIRM_MSG 1
#define DEF_MENU_CANCEL_MSG 2
#define DEF_MENU_NEW_VERSION_MSG 3
#define DEF_MENU_HOW_TO_UPDATE_MSG 4

//setting menu
#define DEF_SEM_NUM_OF_MSG 59
#define DEF_SEM_ENABLE_ICON
//#define DEF_SEM_ENABLE_NAME
#define DEF_SEM_ICON_PATH (std::string)"romfs:/gfx/draw/icon/sem_icon.t3x"
#define DEF_SEM_NAME (std::string)"Settings"
#define DEF_SEM_INIT_STR (std::string)"Sem/Init"
#define DEF_SEM_EXIT_STR (std::string)"Sem/Exit"
#define DEF_SEM_WORKER_THREAD_STR (std::string)"Sem/Worker thread"
#define DEF_SEM_UPDATE_THREAD_STR (std::string)"Sem/Update thread"
#define DEF_SEM_ENCODE_THREAD_STR (std::string)"Sem/Encode thread"
#define DEF_SEM_RECORD_THREAD_STR (std::string)"Sem/Record thread"
#define DEF_SEM_MENU_TOP -1
#define DEF_SEM_MENU_UPDATE 0
#define DEF_SEM_MENU_LANGAGES 1
#define DEF_SEM_MENU_LCD 2
#define DEF_SEM_MENU_CONTROL 3
#define DEF_SEM_MENU_FONT 4
#define DEF_SEM_MENU_WIFI 5
#define DEF_SEM_MENU_ADVANCED 6
#define DEF_SEM_MENU_BATTERY 7
#define DEF_SEM_MENU_RECORDING 8
#define DEF_SEM_RECORD_BOTH 0
#define DEF_SEM_RECORD_TOP 1
#define DEF_SEM_RECORD_BOTTOM 2
#define DEF_SEM_EDTION_NONE -1
#define DEF_SEM_EDTION_3DSX 0
#define DEF_SEM_EDTION_CIA 1

//draw
#define DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS 128

//abgr8888 color
#define DEF_DRAW_RED 0xFF0000FF
#define DEF_DRAW_GREEN 0xFF00FF00
#define DEF_DRAW_BLUE 0xFFFF0000
#define DEF_DRAW_BLACK 0xFF000000
#define DEF_DRAW_WHITE 0xFFFFFFFF
#define DEF_DRAW_AQUA 0xFFFFFF00
#define DEF_DRAW_YELLOW 0xFF00C5FF
#define DEF_DRAW_WEAK_RED 0x500000FF
#define DEF_DRAW_WEAK_GREEN 0x5000FF00
#define DEF_DRAW_WEAK_BLUE 0x50FF0000
#define DEF_DRAW_WEAK_BLACK 0x50000000
#define DEF_DRAW_WEAK_WHITE 0x50FFFFFF
#define DEF_DRAW_WEAK_AQUA 0x50FFFF00
#define DEF_DRAW_WEAK_YELLOW 0x5000C5FF
#define DEF_DRAW_NO_COLOR 0x0

//decoder 
#define DEF_DECODER_MAX_AUDIO_TRACKS 8
#define DEF_DECODER_MAX_VIDEO_TRACKS 2
#define DEF_DECODER_MAX_SESSIONS 2
#define DEF_DECODER_THREAD_TYPE_NONE 0
#define DEF_DECODER_THREAD_TYPE_FRAME 1
#define DEF_DECODER_THREAD_TYPE_SLICE 2
#define DEF_DECODER_THREAD_TYPE_AUTO 3
#define STB_IMAGE_IMPLEMENTATION

//encoder
#define DEF_ENCODER_MAX_SESSIONS 2

//error num
#define DEF_ERR_SUMMARY 0
#define DEF_ERR_DESCRIPTION 1
#define DEF_ERR_PLACE 2
#define DEF_ERR_CODE 3

//error code
#define DEF_ERR_OTHER 0xFFFFFFFF
#define DEF_ERR_OUT_OF_MEMORY 0xFFFFFFFE
#define DEF_ERR_OUT_OF_LINEAR_MEMORY 0xFFFFFFFD
#define DEF_ERR_GAS_RETURNED_NOT_SUCCESS 0xFFFFFFFC
#define DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS 0xFFFFFFFB
#define DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS 0xFFFFFFFA
#define DEF_ERR_INVALID_ARG 0xFFFFFFF9
#define DEF_ERR_JSMN_RETURNED_NOT_SUCCESS 0xFFFFFFF8

#define DEF_ERR_OTHER_STR (std::string)"[Error] Something went wrong. "
#define DEF_ERR_OUT_OF_MEMORY_STR (std::string)"[Error] Out of memory. "
#define DEF_ERR_OUT_OF_LINEAR_MEMORY_STR (std::string)"[Error] Out of linear memory. "
#define DEF_ERR_GAS_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] Google apps script returned NOT success. "
#define DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] stb image returned NOT success. "
#define DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] ffmpeg returned NOT success. "
#define DEF_ERR_INVALID_ARG_STR (std::string)"[Error] Invalid arg. "
#define DEF_ERR_JSMN_RETURNED_NOT_SUCCESS_STR "[Error] jsmn returned NOT success. "

//explorer
#define DEF_EXPL_MAX_FILES 256
#define DEF_EXPL_INIT_STR (std::string)"Expl/Init"
#define DEF_EXPL_EXIT_STR (std::string)"Expl/Exit"
#define DEF_EXPL_READ_DIR_THREAD_STR (std::string)"Expl/Read dir thread"

//external font
#define DEF_EXFONT_NUM_OF_FONT_NAME 50
#define DEF_EXFONT_INIT_STR (std::string)"Exfont/Init"
#define DEF_EXFONT_EXIT_STR (std::string)"Exfont/Exit"
#define DEF_EXFONT_LOAD_FONT_THREAD_STR (std::string)"Exfont/Load font thread"

//fake pthread
#define _POSIX_THREADS

//hid
#define DEF_HID_INIT_STR (std::string)"Hid/Init"
#define DEF_HID_EXIT_STR (std::string)"Hid/Exit"
#define DEF_HID_SCAN_THREAD_STR (std::string)"Hid/Scan thread"

//log
#define DEF_LOG_BUFFER_LINES 512
#define DEF_LOG_DISPLAYED_LINES 23
#define DEF_LOG_COLOR 0xFFBBBB00

//muxer
#define DEF_MUXER_MAX_SESSIONS 2

//speaker
#define DEF_SPEAKER_MAX_BUFFERS 64

//swkbd
#define DEF_SWKBD_MAX_DIC_WORDS 128

//thread
#define DEF_STACKSIZE (64 * 1024)
#define DEF_INACTIVE_THREAD_SLEEP_TIME 100000
#define DEF_ACTIVE_THREAD_SLEEP_TIME 50000
#define DEF_THREAD_WAIT_TIME 10000000000//10s
//0x18~0x3F
#define DEF_THREAD_PRIORITY_IDLE 0x36
#define DEF_THREAD_PRIORITY_LOW 0x25
#define DEF_THREAD_PRIORITY_NORMAL 0x24
#define DEF_THREAD_PRIORITY_HIGH 0x23
#define DEF_THREAD_PRIORITY_REALTIME 0x18
