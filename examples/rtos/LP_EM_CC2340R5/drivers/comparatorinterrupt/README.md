## Example Summary

This example uses the Comparator driver to detect a voltage threshold and toggle an LED.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Note for this example the positive terminal is used as a reference.
And the input voltage is on the negative terminal. The negative terminal is
determined by the comparator module ID. Additionally, the System Configuration file (\*.syscfg)
present in the project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0`    - Indicates that the driver was initialized within `mainThread()`
* `CONFIG_GPIO_BUTTON_0` - Configure `LaunchPad Button BTN-1` in `mainThread()` to
  serve as input trigger to the comparator.
* `CONFIG_COMPARATOR_0`  - Defines the comparator peripheral, `Positive
                        Terminal`, `Negative Terminal`, and `Output Pin`.

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>

## Example Usage

* Connect a jumper between `LaunchPad Button BTN-1` and `Comparator Positive Input (A1)`.

* Run the example

* Press `BTN-1` (Left Button on the launch pad) to trigger a voltage level
change. The example has been set up to toggle LED0, the red LED on the
launchpad, upon detection of a falling edge.

## Application Design Details

* This example shows how to initialize the Comparator driver, set up
comparator interrupts, and read the current comparator output.

* The comparator's internal voltage reference is set to be half the value of
VDDs. In this example, the user-supplied input voltage is connected to the
positive input of the comparator, and the internal voltage reference is
connected to the negative comparator terminal.

* The comparator generates an interrupt when a rising or falling edge, which
also crosses the internal voltage reference threshold, occurs on the
comparator's `Positive Input Pin`.

* The comparator interrupt reads the current output level of the comparator's
`Positive Input Pin` and toggles the `CONFIG_GPIO_LED0` upon detection of a
falling edge followed by a low voltage level measured by the comparator.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
