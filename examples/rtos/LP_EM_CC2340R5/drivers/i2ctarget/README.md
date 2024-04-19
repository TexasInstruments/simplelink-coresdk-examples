# Example Summary

Sample application using the I2CTarget driver to be an I2C target device.
The example implements a simple command interface (see [cmdinterface.h]).

This example is intended for use together with the `i2ccontroller` example.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0` - Indicator LED
* `CONFIG_GPIO_LED_1` - Indicator LED toggling for incoming commands
* `CONFIG_I2C_0` - I2C bus used to communicate with the TMP sensor.

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

Before running the example the following pins must be connected between the
I2C controller and I2C target devices, in addition to __`GND`__.

  |  I2C Controller LaunchPad   |    I2C Target LaunchPad     |
  |:---------------------------:|:---------------------------:|
  | __`CONFIG_GPIO_I2C_0_SCL`__ | __`CONFIG_GPIO_I2C_0_SCL`__ |
  | __`CONFIG_GPIO_I2C_0_SDA`__ | __`CONFIG_GPIO_I2C_0_SDA`__ |

## Example Usage

Run the example.

1. __`CONFIG_GPIO_LED_0`__ turns ON to indicate the I2CTarget driver initialization is complete.

2. __`CONFIG_GPIO_LED_1`__ toggles when commands are received over the `CONFIG_I2C_0` I2C bus.

3. The example implements 4 commands (see [cmdinterface.h])
    * `CMD_CTL_SET_STATUS`: The example receives 1 byte and stores it in `rxBuffer[0]`
    * `CMD_CTL_GET_STATUS`: The example returns the value stored in `rxBuffer[0]`
    * `CMD_CTL_WRITE_BLOCK`: The example receives N bytes and stores it in `rxBuffer`
    * `CMD_CTL_READ_BLOCK`: The example fills `txBuffer` with 'Hello, I am the target!' and returns the N bytes
      requested by the I2C controller.

## Application Design Details

This application uses one task:

`'mainThread'` - performs the following actions:

1. Opens and initializes an I2CTarget driver object.

2. Starts I2CTarget driver to send/receive data over the defined command interface

3. The task wakes up every 1 second.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.

[cmdinterface.h]: ./cmdinterface.h
