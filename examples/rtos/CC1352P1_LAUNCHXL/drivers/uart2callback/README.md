## Example Summary

Example that uses the UART2 driver in callback mode to echo back to the console.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0` - Indicates the UART2 driver was initialized within `mainThread()`
* `CONFIG_UART2_0` - Used to echo characters from host serial session

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>


## Example Usage

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
Homepage"), etc.) to the appropriate COM port.
    * The COM port can be determined via Device Manager in Windows or via
`ls /dev/tty*` in Linux.

The connection should have the following settings
```
    Baud-rate:  115200
    Data bits:       8
    Stop bits:       1
    Parity:       None
    Flow Control: None
```

* Run the example. `CONFIG_GPIO_LED_0` turns ON to indicate driver
initialization is complete.

* The target echoes back any character that is typed in the serial session.

* If the serial session is started before the target completes initialization,
the following is displayed:
`Echoing characters:`

## Application Design Details

* This example shows how to initialize the UART2 driver in callback read
mode and blocking write mode and echo characters back to a console.

* A single thread, `mainThread`, reads a character from `CONFIG_UART2_0` and writes it
back.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
