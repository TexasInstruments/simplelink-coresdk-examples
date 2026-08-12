## Example Summary

This example demonstrates using the PSA Crypto API to import keys with various
lifetimes and use them to perform an AEAD encryption with AES-CCM.
And, the example performs 5 key derivation operations on the same test
vector per key lifetime.

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

* Run the example.

* `CONFIG_GPIO_LED_0` turns ON to indicate PSA Crypto init is complete

* The target will print execution details to the UART.

* `CONFIG_GPIO_LED_1` turns ON if encryption passes for all key lifetimes

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


    Starting key derivation...

    Key persistence/location: [Volatile / Local Storage]
    Capacity is set to = 80

    A key has been newly derived, KeyID = 10
    Read KeyID: 10
    Remaining Capacity is = 64
    Calling psa_cipher_encrypt()
    Ciphertext: 0x51EE2B6FD66FF55EF0B952B1EE3468D5
    ciphertext should be different every time

    A key has been newly derived, KeyID = 11
    Read KeyID: 11
    Remaining Capacity is = 48
    Calling psa_cipher_encrypt()
    Ciphertext: 0x81B6738F94B5A2A733F309D24CA59009
    ciphertext should be different every time

    A key has been newly derived, KeyID = 12
    Read KeyID: 12
    Remaining Capacity is = 32
    Calling psa_cipher_encrypt()
    Ciphertext: 0x4FB862593225D61E5D6C6F89A0D67357
    ciphertext should be different every time

    A key has been newly derived, KeyID = 13
    Read KeyID: 13
    Remaining Capacity is = 16
    Calling psa_cipher_encrypt()
    Ciphertext: 0xE8B67C02B82FF1D2DAC0158653B6855B
    ciphertext should be different every time

    A key has been newly derived, KeyID = 14
    Read KeyID: 14
    Remaining Capacity is = 0
    Calling psa_cipher_encrypt()
    Ciphertext: 0xF897E4C0C1487B163B63DF125CD9E06F
    ciphertext should be different every time

    Key persistence/location: [Default / Local Storage]
    Capacity is set to = 80

    A key has been newly derived, KeyID = 20
    Read KeyID: 20
    Remaining Capacity is = 64
    Calling psa_cipher_encrypt()
    Ciphertext: 0x51EE2B6FD66FF55EF0B952B1EE3468D5
    ciphertext should be different every time

    A key has been newly derived, KeyID = 21
    Read KeyID: 21
    Remaining Capacity is = 48
    Calling psa_cipher_encrypt()
    Ciphertext: 0x81B6738F94B5A2A733F309D24CA59009
    ciphertext should be different every time

    A key has been newly derived, KeyID = 22
    Read KeyID: 22
    Remaining Capacity is = 32
    Calling psa_cipher_encrypt()
    Ciphertext: 0x4FB862593225D61E5D6C6F89A0D67357
    ciphertext should be different every time

    A key has been newly derived, KeyID = 23
    Read KeyID: 23
    Remaining Capacity is = 16
    Calling psa_cipher_encrypt()
    Ciphertext: 0xE8B67C02B82FF1D2DAC0158653B6855B
    ciphertext should be different every time

    A key has been newly derived, KeyID = 24
    Read KeyID: 24
    Remaining Capacity is = 0
    Calling psa_cipher_encrypt()
    Ciphertext: 0xF897E4C0C1487B163B63DF125CD9E06F
    ciphertext should be different every time

    Key persistence/location: [Volatile / HSM Asset Store]
    Capacity is set to = 80

    A key has been newly derived, KeyID = 30
    Read KeyID: 30
    Remaining Capacity is = 64
    Calling psa_cipher_encrypt()
    Ciphertext: 0x51EE2B6FD66FF55EF0B952B1EE3468D5
    ciphertext should be different every time

    A key has been newly derived, KeyID = 31
    Read KeyID: 31
    Remaining Capacity is = 48
    Calling psa_cipher_encrypt()
    Ciphertext: 0x81B6738F94B5A2A733F309D24CA59009
    ciphertext should be different every time

    A key has been newly derived, KeyID = 32
    Read KeyID: 32
    Remaining Capacity is = 32
    Calling psa_cipher_encrypt()
    Ciphertext: 0x4FB862593225D61E5D6C6F89A0D67357
    ciphertext should be different every time

    A key has been newly derived, KeyID = 33
    Read KeyID: 33
    Remaining Capacity is = 16
    Calling psa_cipher_encrypt()
    Ciphertext: 0xE8B67C02B82FF1D2DAC0158653B6855B
    ciphertext should be different every time

    A key has been newly derived, KeyID = 34
    Read KeyID: 34
    Remaining Capacity is = 0
    Calling psa_cipher_encrypt()
    Ciphertext: 0xF897E4C0C1487B163B63DF125CD9E06F
    ciphertext should be different every time

    Key persistence/location: [Default / HSM Asset Store]
    Capacity is set to = 80

    A key has been newly derived, KeyID = 40
    Read KeyID: 40
    Remaining Capacity is = 64
    Calling psa_cipher_encrypt()
    Ciphertext: 0x51EE2B6FD66FF55EF0B952B1EE3468D5
    ciphertext should be different every time

    A key has been newly derived, KeyID = 41
    Read KeyID: 41
    Remaining Capacity is = 48
    Calling psa_cipher_encrypt()
    Ciphertext: 0x81B6738F94B5A2A733F309D24CA59009
    ciphertext should be different every time

    A key has been newly derived, KeyID = 42
    Read KeyID: 42
    Remaining Capacity is = 32
    Calling psa_cipher_encrypt()
    Ciphertext: 0x4FB862593225D61E5D6C6F89A0D67357
    ciphertext should be different every time

    A key has been newly derived, KeyID = 43
    Read KeyID: 43
    Remaining Capacity is = 16
    Calling psa_cipher_encrypt()
    Ciphertext: 0xE8B67C02B82FF1D2DAC0158653B6855B
    ciphertext should be different every time

    A key has been newly derived, KeyID = 44
    Read KeyID: 44
    Remaining Capacity is = 0
    Calling psa_cipher_encrypt()
    Ciphertext: 0xF897E4C0C1487B163B63DF125CD9E06F
    ciphertext should be different every time

    Key persistence/location: [HSM Asset Store / HSM Asset Store]
    Capacity is set to = 80

    A key has been newly derived, KeyID = 50
    Read KeyID: 50
    Remaining Capacity is = 64
    Calling psa_cipher_encrypt()
    Ciphertext: 0x51EE2B6FD66FF55EF0B952B1EE3468D5
    ciphertext should be different every time

    A key has been newly derived, KeyID = 51
    Read KeyID: 51
    Remaining Capacity is = 48
    Calling psa_cipher_encrypt()
    Ciphertext: 0x81B6738F94B5A2A733F309D24CA59009
    ciphertext should be different every time

    A key has been newly derived, KeyID = 52
    Read KeyID: 52
    Remaining Capacity is = 32
    Calling psa_cipher_encrypt()
    Ciphertext: 0x4FB862593225D61E5D6C6F89A0D67357
    ciphertext should be different every time

    A key has been newly derived, KeyID = 53
    Read KeyID: 53
    Remaining Capacity is = 16
    Calling psa_cipher_encrypt()
    Ciphertext: 0xE8B67C02B82FF1D2DAC0158653B6855B
    ciphertext should be different every time

    A key has been newly derived, KeyID = 54
    Read KeyID: 54
    Remaining Capacity is = 0
    Calling psa_cipher_encrypt()
    Ciphertext: 0xF897E4C0C1487B163B63DF125CD9E06F
    ciphertext should be different every time

    DONE!
```

## Application Design Details

This application uses two threads, `mainThread` and `encryptThread`, which
performs the following actions:

1. Opens the Display driver for UART output.

2. Initializes PSA Crypto API.

3. Imports a key with a valid lifetime.

4. Performs an AEAD encryption.

5. Validates the ciphertext output matches the expected ciphertext.

6. Destroys the key.

7. Repeats steps 3-6 for every valid key lifetime.

8. Import a symmetric key with a valid lifetime.

9. Perform a key derivation operation.

10. Perform a cipher encrypt operation with the derived key.

11. Repeats steps 9-10 5 times (Reaching full capacity of 80 Bytes).

12. Repeats steps 8-11 for every valid key lifetime.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
