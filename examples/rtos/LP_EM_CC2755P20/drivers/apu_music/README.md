## Example Summary

This example uses the Algorithm Processing Unit (APU) to implement the
[MUSIC][music-algo] algorithm, used for [Bluetooth Channel Sounding][bt-cs].
This example uses test measurements for a distance of 16m.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_APU_0` - Singleton APU instance used by the application.

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

```text
<SDK_INSTALL_DIR>/source/ti/boards/<BOARD>
```

## Example Usage

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
 Homepage"), etc.) to the appropriate COM port.
    * The COM port can be determined via Device Manager in Windows or via
      `ls /dev/tty*` in Linux.

The connection will have the following settings:

```text
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

* Before running the MUSIC algorithm, `Hello MUSIC!` is displayed.

* After running the algorithm, the run time and estimated distance is displayed.

## Application Design Details

* This example shows how to use many of the APU's most important functions,
  which are the main steps of the MUSIC algorithm.

* Note that aside from the initial buffer, all operations happen within the APU
  memory. This is far more efficient, as it avoid copying data in and out for
  each operation.

* The input data is a Phase Corrected Term (PCT) vector. See
  [this page][bt-cs] for more information about measurements.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.

[music-algo]: https://en.wikipedia.org/wiki/MUSIC_(algorithm) "MUSIC algorithm (Wikipedia)"
[bt-cs]: https://www.bluetooth.com/blog/bluetooth-channel-sounding-a-step-towards-10-cm-ranging-accuracy-for-secure-access-digital-key-and-proximity-services/ "Bluetooth Channel Sounding"
