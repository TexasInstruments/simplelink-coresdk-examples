## Example Summary

This example shows how to use the AESGCM driver. The target expects the user
to enter `1` or `2` and press `Enter`. The predefined plaintext is first encrypted
and displayed to the user. Then, the encrypted message is decrypted to get
back the original plaintext. After decryption, plaintext is displayed back
to the user. `1` is to select 24 byte plaintext and `2` is for 128 byte
plaintext. The user can interact with the target until the user presses `q`.
Note that AES-128 (AES) engine will be used on CC23XX devices and
Hardware Security Module (HSM) will be used on CC27XX devices. CC27XX has AES
engine as well, which can be used as an alternative.

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

The connection should have the following settings

```text
    Baud-rate:  115200
    Data bits:       8
    Stop bits:       1
    Parity:       None
    Flow Control: None
```

The console application also needs to be setup to send the carriage return (CR)
character when `Enter` is pressed. This is because it is used as the termination
character in this demo. If the demo is not working properly, check the default
configuration of the console application in question and make sure that it is
sending the carriage return character as expected.

* Run the example.

* Enter `1` or `2` and press `Enter`.

* Enter `q` to exit the application.

* The target will encrypt predefined plaintext and displays ciphertext. Then, it will
  decrypt the ciphertext to get the original plaintext back. Initial plaintext and
  decrypted ciphertext should match.

## Application Design Details

This application waits for an input from the user (`1` or `2`) in a thread, `mainThread`.
It performs an encryption on a predefined message using `AESGCM_oneStepEncrypt`
and displays the encrypted message to the console. Then, it performs a decryption
using `AESGCM_oneStepDecrypt` on the encrypted message. The decrypted message is also
displayed to the console. Both the predefined plaintext message and the
decrypted ciphertext message should be the same.
