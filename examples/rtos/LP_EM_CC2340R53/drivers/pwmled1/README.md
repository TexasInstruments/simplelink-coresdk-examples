## Example Summary

Sample application to control an on-board LED with the PWM driver.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_PWM_0` - PWM instance used to control brightness of LED

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>

## Example Usage

* Run the example.

* The LED connected to `CONFIG_PWM_0` will fade-in and fade-out when running
the application.

## Application Design Details

This application uses one thread, `mainThread`, which performs the following
actions:

1. Opens and initializes PWM driver objects.

2. Uses the PWM driver to change the intensity of the LED.

3. The thread sleeps for 50 milliseconds before changing LED intensity again.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
