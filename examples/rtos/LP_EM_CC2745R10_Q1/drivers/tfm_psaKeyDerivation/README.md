## Example Summary

This TF-M example demonstrates using the PSA Crypto API to import base keys with
various lifetimes and derives new keys from the base key to perform an AES-ECB
encryption and decryption.

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

* `CONFIG_GPIO_LED_1` turns ON if encryption/decryption with derived keys passes
  for all base key lifetimes.

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
   Select the NS application `tfm_psaKeyDerivation_XXXX.out`.
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
    Starting the PSA Crypto Key Derivation example.

    Provisioning Hardware Unique Key (HUK)...

    Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0

    Base Key persistence/location: [Volatile / Local Storage]
    Base Key ID: 0x7ffffffe

    Starting key derivation...

    Initial capacity is 0x1fffffff bytes
    Setting capacity to 96 bytes

    Derived Key ID: 1
    Remaining capacity: 64 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x7253D96CDFF54B0D3CE9A4A17FB660ED
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 2
    Remaining capacity: 32 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x56E94821317B7D545D2A913A057A1E63
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 3
    Remaining capacity: 0 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0xAD2B4D3D3159D120F67F0C1C7DA8A175
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Base Key persistence/location: [Default / Local Storage]
    Base Key ID: 0x2

    Starting key derivation...

    Initial capacity is 0x1fffffff bytes
    Setting capacity to 96 bytes

    Derived Key ID: 11
    Remaining capacity: 64 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x7253D96CDFF54B0D3CE9A4A17FB660ED
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 12
    Remaining capacity: 32 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x56E94821317B7D545D2A913A057A1E63
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 13
    Remaining capacity: 0 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0xAD2B4D3D3159D120F67F0C1C7DA8A175
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Base Key persistence/location: [Volatile / HSM Asset Store]
    Base Key ID: 0x7ffffffe

    Starting key derivation...

    Initial capacity is 0x1fffffff bytes
    Setting capacity to 96 bytes

    Derived Key ID: 21
    Remaining capacity: 64 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x7253D96CDFF54B0D3CE9A4A17FB660ED
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 22
    Remaining capacity: 32 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x56E94821317B7D545D2A913A057A1E63
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 23
    Remaining capacity: 0 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0xAD2B4D3D3159D120F67F0C1C7DA8A175
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Base Key persistence/location: [Default / HSM Asset Store]
    Base Key ID: 0x4

    Starting key derivation...

    Initial capacity is 0x1fffffff bytes
    Setting capacity to 96 bytes

    Derived Key ID: 31
    Remaining capacity: 64 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x7253D96CDFF54B0D3CE9A4A17FB660ED
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 32
    Remaining capacity: 32 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x56E94821317B7D545D2A913A057A1E63
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 33
    Remaining capacity: 0 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0xAD2B4D3D3159D120F67F0C1C7DA8A175
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Base Key persistence/location: [HSM Asset Store / HSM Asset Store]
    Base Key ID: 0x5

    Starting key derivation...

    Initial capacity is 0x1fffffff bytes
    Setting capacity to 96 bytes

    Derived Key ID: 41
    Remaining capacity: 64 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x7253D96CDFF54B0D3CE9A4A17FB660ED
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 42
    Remaining capacity: 32 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0x56E94821317B7D545D2A913A057A1E63
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    Derived Key ID: 43
    Remaining capacity: 0 bytes
    Calling psa_cipher_encrypt()
    Ciphertext: 0xAD2B4D3D3159D120F67F0C1C7DA8A175
    Calling psa_cipher_decrypt()
    Decrypted Plaintext: 0xA265480CA88D5F536DB0DC6ABC40FAF0
    PASSED!

    DONE!
```

## Application Design Details

This application uses two threads, `mainThread` and `derivationThread`, which
performs the following actions:

1. Opens the Display driver for UART output.

2. Initializes PSA Crypto API.

3. Imports a base key with a valid lifetime.

4. Sets up a key derivation operation and sets capacity.

5. Performs a key derivation operation.

6. Performs a cipher encrypt operation to generate ciphertext with the derived key.

7. Performs a cipher decrypt operation on the ciphertext with the derived key.

8. Verifies the decrypted plaintext matches the original plaintext.

9. Destroys the derive key.

10. Repeats steps 4-9 three times to exhaust capacity.

11. Destroys the base key.

12. Repeats steps 3-11 for every valid key lifetime.

Security and TrustZone:

* Refer to the `TI Trusted Firmware-M User Guide` in `docs/tfm for
general information on using security features.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
