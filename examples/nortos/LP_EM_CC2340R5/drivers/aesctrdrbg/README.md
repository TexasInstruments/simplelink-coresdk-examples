## Example Summary

This example demonstrates the usage of the AES CTR-DRBG driver against
the NIST SP800-90A test vectors.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

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

The connection should have the following settings:

```text
    Baud-rate:  115200
    Data bits:       8
    Stop bits:       1
    Parity:       None
    Flow Control: None
```

* After build and flashing the example on a CC2340R5 board, the application is going to
  run through all of the test vectors and print test result (Success / Fail) as well as
  logs on the display.

  The following can be observed on the LaunchPad:

  1. A solid RED light `CONFIG_GPIO_LED_0` indicate that the tests are currently running.

  2. A blinking RED ligh `CONFIG_GPIO_LED_0` indicate that a test vector has failed and therfore,
     the example failed.

  3. A solid GREEN ligh `CONFIG_GPIO_LED_1` indicates that all test vectors have run successfully.

### Sample UART Output

```text

AESCTRDRBG Driver encrypt demo.

This example will run NIST SP800-90A test vectors.


COUNT: 0
EntropyInput           = 34CBC2B217F3D907FA2AD6A0D7A813B0FDA1E17FBEED94B0E0A0ABFBEC947146
PersonalizationString  = E8FA4C5DE825791E68180F2BA107E829C48299CB01BE939CD0BE76DA120A91F2
EntropyInputReseed     = 8326F8E9CFBD02EB076BBB9819D96A02386F80BF913C8E4A80361D82CAFAD52E
ExpectedReturnedBits   = 52F5E718BF48D99E498775C00378E545799BB2059AEF0B74BE573D8283F02B5293917913BC8F26FC23760A1C86C3F5C844857419868EAFEB17C9248227D026B8
ReturnedBits           = 52F5E718BF48D99E498775C00378E545799BB2059AEF0B74BE573D8283F02B5293917913BC8F26FC23760A1C86C3F5C844857419868EAFEB17C9248227D026B8

COUNT: 1
EntropyInput           = BA811BF491AC4597D79D0F4473208011C5D48575A156D969F071CD5AE5AA4558
PersonalizationString  = 0909E7809F076ED3747625CD2B80615875407A133E77D677FDF8D9D378DE4FD9
EntropyInputReseed     = F556C3AFEA212FF060ED01B7F7F5DBB73F960EA6A3A93F248AE4D2DF2BF49948
ExpectedReturnedBits   = 96EEE34E4CFC905BE64CF1DC64C6E07F1CEB3BDB745F42332568873B80B11F1A1AC6D0D576AFEFCDD7C70CE6A882EE940463323B51C1633998A809003B947210
ReturnedBits           = 96EEE34E4CFC905BE64CF1DC64C6E07F1CEB3BDB745F42332568873B80B11F1A1AC6D0D576AFEFCDD7C70CE6A882EE940463323B51C1633998A809003B947210

.
.
.
.
.
.

COUNT: 118
EntropyInput           = 64591D27DBFB756FA882399D93E24604BBBAA5320238C36D8B66554CB342FC23
PersonalizationString  =
EntropyInputReseed     = E28B909ECBBF0C5FAF80BD2CAEA14F55DCA08F97206C0E0A44C54725544DD4AC
ExpectedReturnedBits   = 0F5B8F9E65AF247EC6E64A99E7C0A19D9A757B88895DB9CF015E6177EC2464DEC78E42F19A0FE52F90795B3FDD331223348D4328C1840507881DB7A19F168ADF
ReturnedBits           = 0F5B8F9E65AF247EC6E64A99E7C0A19D9A757B88895DB9CF015E6177EC2464DEC78E42F19A0FE52F90795B3FDD331223348D4328C1840507881DB7A19F168ADF

COUNT: 119
EntropyInput           = E3D8FCB8C049E442D2BD07104C46F0602A1F60F87BDC02DBECDCFCF4006B5B0A
PersonalizationString  =
EntropyInputReseed     = E25327867FF27456EFF9F4AE4375C7A85788B400DCAE03AE8C892472C8A05221
ExpectedReturnedBits   = 754063C679269931FDAB8F90DEAA967969F20B1805D93FE5B1928512CD2FE98984974B0BB1D7494D81F53E073F1A3A9378EA27307A154DC8A1FB1D3E17998F85
ReturnedBits           = 754063C679269931FDAB8F90DEAA967969F20B1805D93FE5B1928512CD2FE98984974B0BB1D7494D81F53E073F1A3A9378EA27307A154DC8A1FB1D3E17998F85

AESCTRDRBG demo Success!

```

## Application Design Details

This application runs the NIST SP800-90A DRBGVS test vectors against the AESCTRDRBG driver
for CC2340R5 devices. The test sequence follows the recommended NIST SP800-90A Deterministic
Random Bit Generator Validation System (DRBGVS). It is testing against CTR_DRBG mechanism with
the following limitations:

  1. Prediction resistance is not enabled.

  2. Derivation Function (df) is not supported.

The following sequence of operations take place:

  1. Instantiate the DRBG driver using the `AESCTRDRBG_open()` API with the properly populated `AESCTRDRBG_Params`.

  2. Reseed the DRBG driver using the `AESCTRDRBG_reseed()` API.

  3. Call to `AESCTRDRBG_getRandomBytes()` API.

  4. Call `AESCTRDRBG_getRandomBytes()` API a second time.

  5. Compare output DRBG data against expected (found in the NIST test vectors).

  6. Uninstantiate the DRBG driver using the `AESCTRDRBG_close()` API.
