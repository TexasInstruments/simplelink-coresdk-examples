## Example Summary

This example is intended to show how an application can use the S2R RAM (Samples
to RAM RAM) for extra scratch memory, and highlight some of the limitations of
using S2R RAM.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_Display_UART` - Used to display UART output.

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

* Example output is generated through use of Display driver APIs. Refer to the
Display driver documentation found in the  SimpleLink MCU SDK User's Guide.

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

Run the example.

* The example will ensure that the S2R RAM is clocked.
* The example will then perform the following sequence twice: write to S2R
    RAM, perform a dummy operation, and then verify that the S2R RAM contents
    remains unchanged. The two dummy operations are:
    * Sleep
    * Write to UART

The following is the expected example output.

```text
Starting the s2rram example
Writing 0xAA to all bytes in S2R RAM buffer.
Verifying S2R RAM buffer after sleeping...
Verification was successful.
Writing 0x55 to all bytes in S2R RAM buffer.
This is a dummy operation that puts the device into Idle but not STANDBY.
It could be any operation that is known not to put the device in STANDBY.
For example a SPI transaction, crypto operation, etc.
Verifying S2R RAM buffer after performing dummy operation...
Verification was successful.
```

## Application Design Details

The S2R RAM is intended for the S2R LRFD module, which for example is used for
Channel Sounding. If however the S2R module is not used, the application can use
the S2R RAM as additional scratch memory.
There are some important limitations described in the list below:

* The S2R RAM must be clocked, and since this is part of the LRFD module, the
  LRFD module must be clocked as well.
    * This will result in higher current consumption.
    * Since the S2R RAM module is not clocked at boot, you cannot place
      initialized data in S2R RAM, since the auto-initialization sequence at
      startup will fail.
* The S2R RAM is not retained through STANDBY
    * If the application relies on the S2R RAM being retained, it must not put
      the device in STANDBY for the period that the S2R RAM content must remain
      coherent.
    * Whenever the device enters STANDBY, the application can no longer rely on
      any value in S2R RAM written to it before entering STANDBY. Once exiting
      STANDBY, new values can be written and read to and from S2R RAM again.

The TI provided linker files has memory regions defined for the S2R RAM, and the
`.s2rram` section is placed in this memory region. To place a symbol in S2R RAM
the application must place that symbol in the `.s2rram` section using compiler
attributes. As mentioned above, no initialized data can be placed in S2R RAM, so
only uninitialized data may be placed in the `.s2rram` section. Additionally
data placed in `.s2rram` will not be auto zero-initialized.

The above mentioned limitations and requirements are handled in the example
application. The key aspects are described in the list below.

* The application declares an uninitialized array `s2rramBuffer` and places it in
the `.s2rram` section.
* To be able to read and write to the buffer, the S2R RAM must be clocked. This is
  achieved by setting a power dependency on the S2R RAM using this statement:
  `Power_setDependency(PowerLPF3_PERIPH_LFRD_S2RRAM);`
* When performing the `sleep(1)` dummy operation, we must ensure that the device
  is not put into STANDBY, otherwise the verification might fail because the S2R
  RAM is not retained through STANDBY. To achieve this we set a power constraint
  to disallow STANDBY using this statement:
  `Power_setConstraint(PowerLPF3_DISALLOW_STANDBY);`
* When performing the dummy operation to write to UART, we do not need set a
  power constraint because we know that that particular API call will never
  result in the device entering STANDBY. The power constraint is therefore
  released with this statement: `Power_releaseConstraint(PowerLPF3_DISALLOW_STANDBY);`
* At the end of the `mainThread` function, we release the dependency on S2R RAM,
  since it is no longer needed. This is done with the following statement:
  `Power_releaseDependency(PowerLPF3_PERIPH_LFRD_S2RRAM);`

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
