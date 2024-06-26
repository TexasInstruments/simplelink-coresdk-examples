## Example Summary

Sample application that reads the temperature from a TMP sensor.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0` - Indicator LED
* `CONFIG_I2C_TMP` - I2C bus used to communicate with the TMP sensor.
* `CONFIG_GPIO_TMP_EN` - Optional TMP power pin

## BoosterPacks, Board Resources & Jumper Settings

This example requires a
[__BP-BASSENSORSMKII BoosterPack__][bp-bassensorsmkii].

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

* Example output is generated through use of Display driver APIs. Refer to the
Display driver documentation found in the SimpleLink MCU SDK User's Guide.

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
 Homepage"), etc.) to the appropriate COM port.
  * The COM port can be determined via Device Manager in Windows or via `ls /dev/tty*` in Linux.

The connection will have the following settings:

```text
Baud-rate:     115200
Data bits:          8
Stop bits:          1
Parity:          None
Flow Control:    None
```

Run the example.

1. `CONFIG_GPIO_LED_0` turns ON to indicate the I2C driver initialization is complete.

    ```text
    I2C Initialized!
    ```

2. If required, the TMP sensor is enabled using a GPIO output.

3. The I2C bus is queried for all supported TMP sensors. The output varies depending on
which TMP sensor(s) is found.

    ```text
    I2C target address 0x49 not acknowledged!
    Detected TMP116 sensor with target address 0x48
    ```

4. The last known TMP sensor will be used to read temperature samples. Samples
are converted to celsius and displayed via UART.

    ```text
    Sample 0: 24 C
    Sample 1: 24 C
        .
        .
        .
    Sample 18: 24 C
    Sample 19: 24 C
    ```

5. After a number of samples, the I2C driver instance is closed.

    ```text
    I2C closed!
    ```

* If the I2C communication fails, an error message describing
the failure is displayed.

    ```text
    I2C target address 0x49 not acknowledged!
    ```

## Application Design Details

This application uses one task:

`'mainThread'` - performs the following actions:

1. Opens and initializes an I2C driver object.

2. Determines which TMP sensor is available. Sensors are described in the
`sensors` data structure at the top of the source.

3. Uses the I2C driver in blocking mode to get data from the TMP sensor.

4. Extracts the temperature (in Celsius) and prints the value via the UART.

5. The task sleeps for 1 second.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.

[bp-bassensorsmkii]: http://www.ti.com/tool/bp-bassensorsmkii
