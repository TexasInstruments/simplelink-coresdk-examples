# Example Summary

Example that uses the SYSTIM module to capture timestamps on UART and GPIO events.
The example demonstrates how HW events on the device can be captured and how the
timestamps for the events can be reported by use of UART.
The device operates as a target activated by a specific message that must
be input by a user in a terminal connected to a UART session.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0` - Indicates the UART2 driver was initialized within `mainThread()`
* `CONFIG_GPIO_LED_1` - Indicates execution of the SYSTIM channel 1 capture event ISR
* `CONFIG_UART2_0` - Used to receive/send messages from/to the console of a serial session
* `CONFIG_GPIO_BUTTON_0` - Forces timestamp to be captured and displayed when pressed


## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

```text
<SDK_INSTALL_DIR>/source/ti/boards/<BOARD>
\```


## Example Usage

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
Homepage"), etc.) to the appropriate COM port.
  * The COM port can be determined via Device Manager in Windows or via
`ls /dev/tty*` in Linux.

The connection should have the following settings
```text
    Baud-rate:  115200
    Data bits:       8
    Stop bits:       1
    Parity:       None
    Flow Control: None
```

* Run the example. `CONFIG_GPIO_LED_0` turns ON to indicate driver
initialization is complete.

* The target transfers the following message to the serial session:
`Start listening for UART RX start bit!`

* The target will wait for the user to type the following message in the serial session:
`RX_ID_0`

* The target will respond with the following message containing the timestamp (0xNNNNNNNN)
for the detection of the UART RX start bit of the message typed by the user:
`Captured UART RX start bit at timestamp: 0xNNNNNNNN`
The timestamp is the number of microseconds from target reset.

* The target will then output the following message containing the timestamp (0xNNNNNNNN)
for the detection of the UART TX start bit of the previous message transferred by the target:
`Captured UART TX start bit at timestamp: 0xNNNNNNNN`
The timestamp is the number of microseconds from target reset.

* The target transfers the following message to the serial session:
`Start listening for GPIO event!`

* When BUTTON_0 is pressed the target will output the following message containing the
timestamp (0xNNNNNNNN) for the detection of BUTTON_0 activation:
`Captured GPIO input event at timestamp: 0xNNNNNNNN`
The timestamp is the number of microseconds from target reset.
The example will wait forever for BUTTON_0 to be pressed transferring the described
message each time.


## Application Design Details

* This example shows how to use the SYSTIM channel 1 capture functionality to register
HW events like start of UART RX/TX operations and edges on GPIO pin.
The HW events are registered with timestamps holding the number of micro seconds from
target reset.
The SYSTIM channel 1 can only operate with a 1 usec resolution.
Note that the example does not allow the device to enter the Standby power mode as
the SYSTIM module is not running during Standby.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
