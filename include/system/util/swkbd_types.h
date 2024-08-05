#if !defined(DEF_KEYBOARD_TYPES_H)
#define DEF_KEYBOARD_TYPES_H
#include <stdbool.h>
#include <stdint.h>

#define DEF_KEYBOARD_API_ENABLE					/*(bool)(*/true/*)*/	//Enable software keyboard API.

#define DEF_KEYBOARD_MAX_DIC_WORDS				(uint16_t)(128)

typedef uint8_t Keyboard_features_bit;
#define KEYBOARD_FEATURES_BIT_NONE				(Keyboard_features_bit)(0 << 0)
#define KEYBOARD_FEATURES_BIT_DARKEN_SCREEN		(Keyboard_features_bit)(1 << 0)	//Darken the top screen when swkbd is shown.
#define KEYBOARD_FEATURES_BIT_PREDICTIVE_INPUT	(Keyboard_features_bit)(1 << 1)	//Enable predictive input (so that users can also type kanji).
#define KEYBOARD_FEATURES_BIT_MULTILINE			(Keyboard_features_bit)(1 << 2)	//Enable multiline input.
#define KEYBOARD_FEATURES_BIT_ALLOW_HOME		(Keyboard_features_bit)(1 << 3)	//Allow HOME button.
#define KEYBOARD_FEATURES_BIT_ALL				(Keyboard_features_bit)(KEYBOARD_FEATURES_BIT_DARKEN_SCREEN \
| KEYBOARD_FEATURES_BIT_PREDICTIVE_INPUT | KEYBOARD_FEATURES_BIT_MULTILINE | KEYBOARD_FEATURES_BIT_ALLOW_HOME)

typedef enum
{
	KEYBOARD_TYPE_INVALID = -1,

	KEYBOARD_TYPE_NORMAL,		//Normal keyboard.
	KEYBOARD_TYPE_QWERTY,		//QWERTY only keyboard.
	KEYBOARD_TYPE_NUMPAD,     	//Number pad only keyboard.
	KEYBOARD_TYPE_WESTERN,		//Same as TYPE_NORMAL except no Japanese support.

	KEYBOARD_TYPE_MAX,
} Keyboard_type;

typedef enum
{
	KEYBOARD_ACCEPTABLE_INPUT_INVALID = -1,

	KEYBOARD_ACCEPTABLE_INPUT_ANY,			//Any inputs will be accepted.
	KEYBOARD_ACCEPTABLE_INPUT_NO_EMPTY,		//Empty input will NOT be accepted.

	KEYBOARD_ACCEPTABLE_INPUT_MAX,
} Keyboard_acceptable_input;

typedef enum
{
	KEYBOARD_DISPLAY_BUTTON_INVALID = -1,

	KEYBOARD_DISPLAY_BUTTON_MIDDLE,				//Only middle button (confirm) is displayed.
	KEYBOARD_DISPLAY_BUTTON_LEFT_MIDDLE,		//Left (cancel) and middle button (confirm) are displayed.
	KEYBOARD_DISPLAY_BUTTON_LEFT_MIDDLE_RIGHT,	//Left (cancel), middle (confirm) and right (I forgot) are displayed.

	KEYBOARD_DISPLAY_BUTTON_MAX,
} Keyboard_display_button;

typedef enum
{
	KEYBOARD_BUTTON_INVALID = -1,

	KEYBOARD_BUTTON_NONE,	//No button was pressed.
	KEYBOARD_BUTTON_LEFT,	//Left button (cancel) was pressed.
	KEYBOARD_BUTTON_MIDDLE,	//Middle button (I forgot) was pressed.
	KEYBOARD_BUTTON_RIGHT,	//Right button (confirm) was pressed.

	KEYBOARD_BUTTON_MAX,
} Keyboard_button;

typedef enum
{
	KEYBOARD_PASSWORD_MODE_INVALID = -1,

	KEYBOARD_PASSWORD_MODE_OFF,			//Inputs will NOT be hidden.
	KEYBOARD_PASSWORD_MODE_ON_DELAY,	//Inputs will be hidden after certain period of time.
	KEYBOARD_PASSWORD_MODE_ON,			//Inputs will be hidden immediately.

	KEYBOARD_PASSWORD_MODE_MAX,
} Keyboard_password_mode;

#endif //!defined(DEF_KEYBOARD_TYPES_H)
