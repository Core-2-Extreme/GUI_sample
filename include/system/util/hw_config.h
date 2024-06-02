#if !defined(HW_CONFIG_H)
#define HW_CONFIG_H

extern "C"
{
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t Hw_config_wakeup_bit;
#define HW_CONFIG_WAKEUP_BIT_NONE				(Hw_config_wakeup_bit)(0 << 0)	//No wake up event.
#define HW_CONFIG_WAKEUP_BIT_PRESS_HOME_BUTTON	(Hw_config_wakeup_bit)(1 << 2)	//Wake up if home button is pressed.
#define HW_CONFIG_WAKEUP_BIT_OPEN_SHELL			(Hw_config_wakeup_bit)(1 << 5)	//Wake up if shell is opened.

/**
 * @brief Set screen brightness.
 * @param top_screen (in) When true, top screen's brightness will be set.
 * @param bottom_screen (in) When true, bottom screen's brightness will be set.
 * @param brightness (in) brightness level 0~200.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_hw_config_set_screen_brightness(bool top_screen, bool bottom_screen, uint8_t brightness);

/**
 * @brief Set wifi state.
 * @param wifi_state (in) When true, wifi will be turned on.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_hw_config_set_wifi_state(bool wifi_state);

/**
 * @brief Set screen state(ON or OFF).
 * @param top_screen (in) When true, top screen's state will be set.
 * @param bottom_screen (in) When true, bottom screen's state will be set.
 * @param state (in) When true, screen will be turned on.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_hw_config_set_screen_state(bool top_screen, bool bottom_screen, bool state);

/**
 * @brief Sleep the system.
 * If sleep is not allowed this function will fail with DEF_ERR_OTHER.
 * @param wakeup_events (in) Wakeup events.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_hw_config_sleep_system(Hw_config_wakeup_bit wakeup_events);
}
#endif //!defined(HW_CONFIG_H)
