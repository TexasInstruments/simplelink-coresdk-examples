## Example Summary

This TF-M example demonstrates using the PSA Crypto API to import keys with
various lifetimes and use them to perform an AEAD encryption with AES-CCM
(Counter with CBC-MAC).

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_Display_0` - Displays execution details on the UART.
* `CONFIG_GPIO_LED_0` - Indicates driver initialization completed successfully.
* `CONFIG_GPIO_LED_1` - Indicates all operations completed successfully.

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

* `CONFIG_GPIO_LED_0` turns ON to indicate driver initialization is complete.

* The target will print execution details to the UART.

* `CONFIG_GPIO_LED_1` turns ON if encryption passes for all key lifetimes.

#### Steps to run the example with CCS:
  1. Open the project properties, select __Debug__. Select "Cortex_M33_0" for the
   *Core* and "Debugger Options" for the *Category*. Under "Auto Run
   Options" disable "On a program load or restart".
  2. Launch the CCS debug session.
  3. Execution should be suspended at `_c_int00()` or `ResetISR()`.
  4. Right-click the processor in the debugger *THREADS* window and select *Properties*.
     Select *Flash Settings* under the *Category* menu. Enable "Do not erase before program load"
     option. This ensures the application image is not erased when programming the Secure image.
  5. Select *Run* -> *Load* -> *Load Program*.  Select `tfm_s.axf` located in
   `<SDK_INSTALL_DIR>/tfm_s/build/<DEVICE_FAMILY>/production_full/Release/bin/` to
   program the Secure image.  This will overwrite the NS application symbols.
  6. To restore NS application symbols: Select *Run* -> *Load* -> *Add Symbols*.
   Select the NS application `tfm_psaAeadEncrypt_XXXX.out`.
  7. Execution should be suspended at `Reset_Handler()` if the above steps were
   done correctly.
  8. Set breakpoints if desired and resume execution to run the example.
  9. Disable "Do not erase before program load" option before exiting the debug
   session or reloading the application. A programming error will occur otherwise.

#### Steps to run the example with IAR:
  1. This example can be built with IAR but multiple images cannot be loaded for execution
   at this time.

### Sample UART output

```text
    Starting the PSA Crypto AEAD Encrypt example.

    Provisioning Hardware Unique Key (HUK)...

    Nonce: 0x5A8AA485C316E9
    AAD: 0x3796CF51B8726652A4204733B8FBB047CF00FB91A9837E22EC22B1A268F88E2C
    Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0D05BE7A966977768
    Key: 0xF9FDCA4AC64FE7F014DE0F43039C7571

    Key persistence/location: [Volatile / Local Storage]
    Key ID: 0x7ffffffe
    Calling psa_aead_encrypt()
    Ciphertext: 0x6BE31860CA271EF448DE8F8D8B39346DAF4B81D7E92D65B338F125FA
    PASSED!

    Key persistence/location: [Default / Local Storage]
    Key ID: 0x2
    Calling psa_aead_encrypt()
    Ciphertext: 0x6BE31860CA271EF448DE8F8D8B39346DAF4B81D7E92D65B338F125FA
    PASSED!

    Key persistence/location: [Volatile / HSM Asset Store]
    Key ID: 0x7ffffffe
    Calling psa_aead_encrypt()
    Ciphertext: 0x6BE31860CA271EF448DE8F8D8B39346DAF4B81D7E92D65B338F125FA
    PASSED!

    Key persistence/location: [Default / HSM Asset Store]
    Key ID: 0x4
    Calling psa_aead_encrypt()
    Ciphertext: 0x6BE31860CA271EF448DE8F8D8B39346DAF4B81D7E92D65B338F125FA
    PASSED!

    Key persistence/location: [HSM Asset Store / HSM Asset Store]
    Key ID: 0x5
    Calling psa_aead_encrypt()
    Ciphertext: 0x6BE31860CA271EF448DE8F8D8B39346DAF4B81D7E92D65B338F125FA
    PASSED!

    DONE!
```

## Application Design Details

Security and TrustZone:

* Refer to the `TI Trusted Firmware-M User Guide` in `docs/tfm for
general information on using security features.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
