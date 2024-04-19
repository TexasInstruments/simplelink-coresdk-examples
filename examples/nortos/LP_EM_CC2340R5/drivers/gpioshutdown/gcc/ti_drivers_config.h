/*
 *  ======== ti_drivers_config.h ========
 *  Configured TI-Drivers module declarations
 *
 *  The macros defines herein are intended for use by applications which
 *  directly include this header. These macros should NOT be hard coded or
 *  copied into library source code.
 *
 *  Symbols declared as const are intended for use with libraries.
 *  Library source code must extern the correct symbol--which is resolved
 *  when the application is linked.
 *
 *  DO NOT EDIT - This file is generated for the LP_EM_CC2340R5
 *  by the SysConfig tool.
 */
#ifndef ti_drivers_config_h
#define ti_drivers_config_h

#define CONFIG_SYSCONFIG_PREVIEW

#define CONFIG_LP_EM_CC2340R5
#ifndef DeviceFamily_CC23X0R5
#define DeviceFamily_CC23X0R5
#endif

#include <ti/devices/DeviceFamily.h>

#include <stdint.h>

/* support C++ sources */
#ifdef __cplusplus
extern "C" {
#endif


/*
 *  ======== GPIO ========
 */
/* Owned by CONFIG_LED_0 as  */
extern const uint_least8_t CONFIG_LED_0_GPIO_CONST;
#define CONFIG_LED_0_GPIO 14

/* Owned by CONFIG_LED_1 as  */
extern const uint_least8_t CONFIG_LED_1_GPIO_CONST;
#define CONFIG_LED_1_GPIO 15

/* Owned by CONFIG_BUTTON_SHUTDOWN as  */
extern const uint_least8_t CONFIG_GPIO_BUTTON_SHUTDOWN_INPUT_CONST;
#define CONFIG_GPIO_BUTTON_SHUTDOWN_INPUT 10

extern const uint_least8_t CONFIG_GPIO_WAKEUP_CONST;
#define CONFIG_GPIO_WAKEUP 9

/* The range of pins available on this device */
extern const uint_least8_t GPIO_pinLowerBound;
extern const uint_least8_t GPIO_pinUpperBound;

/* LEDs are active high */
#define CONFIG_GPIO_LED_ON  (1)
#define CONFIG_GPIO_LED_OFF (0)

#define CONFIG_LED_ON  (CONFIG_GPIO_LED_ON)
#define CONFIG_LED_OFF (CONFIG_GPIO_LED_OFF)


/*
 *  ======== Button ========
 */

extern const uint_least8_t                  CONFIG_BUTTON_SHUTDOWN_CONST;
#define CONFIG_BUTTON_SHUTDOWN              0
#define CONFIG_TI_DRIVERS_BUTTON_COUNT      1


/*
 *  ======== LED ========
 */

extern const uint_least8_t              CONFIG_LED_0_CONST;
#define CONFIG_LED_0                    0
extern const uint_least8_t              CONFIG_LED_1_CONST;
#define CONFIG_LED_1                    1
#define CONFIG_TI_DRIVERS_LED_COUNT     2


/*
 *  ======== Board_init ========
 *  Perform all required TI-Drivers initialization
 *
 *  This function should be called once at a point before any use of
 *  TI-Drivers.
 */
extern void Board_init(void);

/*
 *  ======== Board_initGeneral ========
 *  (deprecated)
 *
 *  Board_initGeneral() is defined purely for backward compatibility.
 *
 *  All new code should use Board_init() to do any required TI-Drivers
 *  initialization _and_ use <Driver>_init() for only where specific drivers
 *  are explicitly referenced by the application.  <Driver>_init() functions
 *  are idempotent.
 */
#define Board_initGeneral Board_init

#ifdef __cplusplus
}
#endif

#endif /* include guard */
