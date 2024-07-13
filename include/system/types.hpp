#if !defined(DEF_TYPES_HPP)
#define DEF_TYPES_HPP
#include <stdbool.h>
#include <stdint.h>
#include <string>

#include <3ds.h>
#include <citro2d.h>

#include "system_definitions.hpp"

#include "system/util/converter_types.h"
#include "system/util/error_types.h"

enum Keyboard_button
{
	KEYBOARD_BUTTON_INVALID = -1,

	KEYBOARD_BUTTON_NONE,	//No button was pressed.
	KEYBOARD_BUTTON_LEFT,	//Left button (usually cancel) was pressed.
	KEYBOARD_BUTTON_MIDDLE,	//Middle button (usually I forgot) was pressed.
	KEYBOARD_BUTTON_RIGHT,	//Right button (usually confirm) was pressed.

	KEYBOARD_BUTTON_MAX,
};

enum Watch_handle
{
	WATCH_HANDLE_INVALID = -1,

	WATCH_HANDLE_GLOBAL,			//Watch handle for global data.
	WATCH_HANDLE_MAIN_MENU,			//(menu.cpp) Watch handle for main menu.
	WATCH_HANDLE_SETTINGS_MENU,		//(setting_menu.cpp) Watch handle for settings menu.
	WATCH_HANDLE_SUB_APP0,			//(sub_app0.cpp) Watch handle for sub app0.
	WATCH_HANDLE_SUB_APP1,			//(sub_app1.cpp) Watch handle for sub app1.
	WATCH_HANDLE_SUB_APP2,			//(sub_app2.cpp) Watch handle for sub app2.
	WATCH_HANDLE_SUB_APP3,			//(sub_app3.cpp) Watch handle for sub app3.
	WATCH_HANDLE_SUB_APP4,			//(sub_app4.cpp) Watch handle for sub app4.
	WATCH_HANDLE_SUB_APP5,			//(sub_app5.cpp) Watch handle for sub app5.
	WATCH_HANDLE_SUB_APP6,			//(sub_app6.cpp) Watch handle for sub app6.
	WATCH_HANDLE_SUB_APP7,			//(sub_app7.cpp) Watch handle for sub app7.

	WATCH_HANDLE_MAX,
	WATCH_HANDLE_FORCE_8BIT = INT8_MAX,
};

typedef uint16_t Watch_handle_bit;
#define	DEF_WATCH_HANDLE_BIT_NONE			(Watch_handle_bit)(0 << 0)							//No watch handles.
#define	DEF_WATCH_HANDLE_BIT_GLOBAL			(Watch_handle_bit)(1 << WATCH_HANDLE_GLOBAL)		//Watch handle bit for WATCH_HANDLE_GLOBAL.
#define	DEF_WATCH_HANDLE_BIT_MAIN_MENU		(Watch_handle_bit)(1 << WATCH_HANDLE_MAIN_MENU)		//Watch handle bit for WATCH_HANDLE_MAIN_MENU.
#define	DEF_WATCH_HANDLE_BIT_SETTINGS_MENU	(Watch_handle_bit)(1 << WATCH_HANDLE_SETTINGS_MENU)	//Watch handle bit for WATCH_HANDLE_SETTINGS_MENU.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP0		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP0)		//Watch handle bit for WATCH_HANDLE_SUB_APP0.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP1		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP1)		//Watch handle bit for WATCH_HANDLE_SUB_APP1.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP2		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP2)		//Watch handle bit for WATCH_HANDLE_SUB_APP2.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP3		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP3)		//Watch handle bit for WATCH_HANDLE_SUB_APP3.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP4		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP4)		//Watch handle bit for WATCH_HANDLE_SUB_APP4.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP5		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP5)		//Watch handle bit for WATCH_HANDLE_SUB_APP5.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP6		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP6)		//Watch handle bit for WATCH_HANDLE_SUB_APP6.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP7		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP7)		//Watch handle bit for WATCH_HANDLE_SUB_APP7.

struct Watch_data
{
	void* original_address = NULL;				//Original data address.
	void* previous_data = NULL;					//Previous data.
	uint32_t data_length = 0;					//Data length for this data.
	Watch_handle handle = WATCH_HANDLE_INVALID;	//Watch handle.
};

struct Result_with_string
{
	std::string string = "[Success] ";
	std::string error_description = "";
	uint code = DEF_SUCCESS;
};

#endif //!defined(DEF_TYPES_HPP)
