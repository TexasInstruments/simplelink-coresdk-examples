## Example Summary

This example demonstrates using PSA Crypto APIs to export the attestation public
key and generate a nonce to be used as a challenge. The PSA Attestation APIs are
used to retrieve an attestation token with the challenge. The token is compliant
to https://datatracker.ietf.org/doc/html/draft-tschofenig-rats-psa-token-07.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (/*.syscfg) present in the
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

## Example Configuration and Build
Follow the [How to Setup Secure Boot](#how-to-setup-secure-boot) section below to configure and build the
secure boot composite hex image.

## Example Usage

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
 Homepage"), etc.) to the appropriate COM port.
    * The COM port can be determined via Device Manager in Windows or via `ls
     /dev/tty*` in Linux.

The connection will have the following settings:

```text
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

* `CONFIG_GPIO_LED_0` turns ON to indicate driver initialization is complete.

* The target will print execution details to the UART.

* `CONFIG_GPIO_LED_1` turns ON if attestation token generation is successful.

#### Steps to run the example with CCS:
 1. Within the CCS Workspace, expand the example project's folder. Right-click
  on the `Debug` folder, select *Reveal in File Explorer*. Copy the directory path.
 2. Within the CCS Workspace, expand the *targetConfigs* folder under the project.
  Right-click on the `<DEVICE>.ccxml` file and select *Start Project-less Debug*
 3. Select *Run* -> *Load* -> *Load Program*. Click *Browse* and select
  `tfm_attestation_XXXX_composite_sb.hex` from the project's `Debug` folder
  using path from Step 1.
 4. If the *Debug Configuration* selected in SysConfig allows debugging, select
  *Run* -> *Continue* to run the program. Otherwise, press the reset button on
  the debugger to initiate secure boot and run the program.

#### Steps to run the example with IAR:
 1. Right-click on the project and select *"Open Containing Folder..."*. Copy the
  directory path.
 2. Select *Project* -> *Download* -> *Download File...*. Ensure the *"All Files
  (\*.\*)"* filter is selected. Select `tfm_attestation_XXXX_composite_sb.hex`
  which was generated in the project's `Debug/Exe` folder using the path from
  Step 1.
 3. Press the reset button on the debugger to initiate secure boot and
  run the program.

### Sample UART output

```text
    PSA Attestation with Secure/Non-Secure isolation example:

    Exporting attestation public key...
    Public IAK: 0455C9098954F517E7D21E7F917C18138D506F38456150AA61ED8C1F5B7C73F23FFD0E399DD
    8A9A974B5AEC0C5EE2A70DCAF502A40F57B147F085AFF7A08524EFA

    Generating nonce for challenge...
    Nonce: 4600DD482C538C81F1832F00AA362C5B33AB9BF7740D9764B61599DDBE12BEC37E94639E5EBF2036
    C4841D8539F2AD963FEC13095DDC8073BA9075DB4586E253

    Getting token size...
    Token size: 308 bytes

    Retrieving attestation token...
    Token: D28443A10126A058E9A83A000124FF58404600DD482C538C81F1832F00AA362C5B33AB9BF7740D97
    64B61599DDBE12BEC37E94639E5EBF2036C4841D8539F2AD963FEC13095DDC8073BA9075DB4586E2533A000
    124FB58209DE52218C9597873535359AD55488E4CB741B7A9965381F6600CABBC953AD1213A000125005821
    013D378EB37DE78FDB4BD64442DCA50A17E8FE10C24010AEE4C9F8FBBDD4AEE88E3A000124FA5820961D00A
    000000000306C9FA720000BE8C44AEE903C28000044BA3634004B12003A000124F8203A000124F91930003A
    000124FE013A000124F7715053415F494F545F50524F46494C455F315840E4B555F39E058C16F5A359B5F63
    234B262487934CC6087621DC98781C131E70EBEA04975831961458442447288DFEA8D4AAD8EA44473FE9E64
    6316FAF35C9248
```

To validate the token, follow the steps in [Attestation Token Verification](#attestation-token-verification).

## Application Design Details

* The main application uses a single thread, `mainThread` which performs the
  following actions:

1. Opens the Display driver for UART output.
2. Initializes PSA Crypto API.
3. Exports the attestation public key and prints to UART.
4. Generates a nonce to be used for the challenge and prints to UART.
5. Reads the attestation token size and prints to UART.
6. Verifies the attestation token size is less than buffer size.
7. Retrieves the attestation token and prints to UART.

Security and TrustZone:

* Refer to the `TI Trusted Firmware-M User Guide` in `docs/tfm` for
general information on using security features.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
  information.


## How to Setup Secure Boot

### Key Generation

Run `imgtool` (found in `<SDK>/tools/common/mcuboot`) to generate an asymmetric
key pair which will be used to sign both the application and TF-M images. Note
that only RSA-3K-PKCS, ECDSA P256, and ECDSA P521 are supported.

For example, to generate a key pair for RSA-3K-PKCS, run the following command:

`imgtool keygen --key app-rsa-3k-pkcs-priv.pem --type rsa-3072`

Copy the `app-rsa-3k-pkcs-priv.pem` to the root directory of the target project.
In the CCS IDE, you can access the project's root directory by right-clicking on
the project, selecting *Reveal in File Explorer*, and opening the example's
folder. In the IAR IDE, right-click the project and select *Open Containing Folder...*.

For test purposes only, the key generation step can be skipped and instead use
the provided test keys as mentioned in [Test Keys](#test-keys) section.

### SCFG Key Ring Initialization

Once the key pair has been generated, run `sbtool` (found in
`<SDK>/tools/common/sbtool`) to generate the scfg key ring configuration:

`sbtool init --key-update-key --app-key app-rsa-3k-pkcs-priv.pem`

From directory `output`, copy file `sb_key_ring_init.h` to the root directory of
the target project.

For test purposes only, this step can be skipped and instead use the provided
`sb_key_ring_init.h` as mentioned in [Test Keys](#test-keys) section.

### Test Keys

Test Keys along with the corresponding `sb_key_ring_init.h` are stored under
`<SDK>/tools/common/sbtool/keys`. Note that there are separate directories for
each of the algorithms supported by Secure Boot, so pick either the APP or SSB
key (depending on what type of target is being built) and `sb_key_ring_init.h`
from the directory that matches the algorithm that has been configured in
SysConfig.

### Configuration via SysConfig

Open the `attestation.syscfg` file in the IDE and follow the configuration steps
in the SysConfig GUI below:

#### Secure Boot Configuration

The secure boot configuration is found under `TI DEVICES` -> `Device
Configuration` -> `Security Configuration`.

* Set the **Authentication Method** to *Signature* or *Hash Lock*.
* Set the **Authentication Algorithm** to match the one used to generate the key pair.
* Select the **Update Mode** (*Overwrite*, *XIP Revert Enabled*, or *XIP Revert Disabled*).
* Set **Boot Seed** to 0x0. Other values are not valid for the default TF-M image.
* Set the **Image Type** to `APP 1` which corresponds to the Primary Non-secure slot.
* Set **Update Mode** to `Overwrite`.
* Enter the desired **Security Counter** to be associated with this target image.
* Set the **Header Size** to 0x100. Other values are not valid for the default TF-M image.
* Enter the desired **Version** for the target image.
* Select the generated key pair \*.pem file for the **Private Key**.
* Configure the Primary slots start address and length. The following values can
  be used for the default out-of-box images:
  * **Primary Secure** (APP0, used for TF-M image):
    * Start:  0xD000
    * Length: 0x2B000
  * **Primary Non-Secure** (APP1, used for Non-Secure application image):
    * Start:  0x38000
    * Length: 0xB0000

#### Debug Configuration

The debug configuration is found under `TI DEVICES` -> `Device Configuration` ->
`Debug Configuration`.

* Set **Debug Authorization Configuration** to one of the following
  * Non-Invasive Only
  * Require Debug Authentication
  * Debug Not Allowed

Those debug restrictions prevent secure memory from being read or modified by
any unauthorized entities to ensure attestation is trustworthy. This is
*required* for production to allow the correct security lifecycle to be reported.

### Building

Build the application from the IDE project's menu using `Rebuild Project` for
CCS or `Rebuild All` for IAR. The example's post-build steps will process the
executable file, sign the application and TF-M binaries, and combine them into a
single composite image in hex format to support secure boot. The composite hex
file is what must be programmed to the device.

### Debugging

Symbols must be manually loaded for the debugger to resolve symbols.

* Select *Run* -> *Load* -> *Add Symbols* and select the
  application `tfm_attestation_XXXX.out` file.
* To load the symbols for the TF-M, repeat the process for
  `<SDK>/tfm_s/build/cc27xx/production_full/Release/bin/tfm_s.axf`

Set your desired breakpoints and use the IDE to reset the device to run the
program.

### Additional Info

Reference the TRM and the secure boot example README in
`<SDK>/examples/nortos/LP_EM_CC2745R10_Q1/secureboot` for additional details
about secure boot options and usage.

## Attestation Token Verification

The *Initial Attestation Verifier* included with TF-M tools and utilities can be
used to verify the token signature and extract the token claims in human readable
format.

* Clone the repo: https://git.trustedfirmware.org/plugins/gitiles/TF-M/tf-m-tools.git
* Read the *Initial Attestation Verifier* section of the docs to setup the tool:
  * https://trustedfirmware-m.readthedocs.io/projects/tf-m-tools/en/latest/
* Use the python scripts included with this example to convert the UART hex to
  the proper format for the Initial Attestation Verifier tool.
  * Copy the hex key from UART output into the `hex2pem.py` script and run it to
    convert the key to PEM format.
  * Copy the hex token from UART output into the `hex2cbor.py` script and run it
    to convert the token to a CBOR binary.
* Run `check_iat` script from TF-M tools:

```sh
check_iat -k public_key.pem -K -p -t PSA-IoT-Profile1-token attest_token.cbor
```

Sample output:
```
Signature OK
Token format OK
Token:
{
    "CHALLENGE": "b'4600DD482C538C81F1832F00AA362C5B33AB9BF7740D9764B61599DDBE12BEC37E94639E5EBF2036C4841D8539F2AD963FEC13095DDC8073BA9075DB4586E253'",
    "BOOT_SEED": "b'9DE52218C9597873535359AD55488E4CB741B7A9965381F6600CABBC953AD121'",
    "INSTANCE_ID": "b'013D378EB37DE78FDB4BD64442DCA50A17E8FE10C24010AEE4C9F8FBBDD4AEE88E'",
    "IMPLEMENTATION_ID": "b'961D00A000000000306C9FA720000BE8C44AEE903C28000044BA3634004B1200'",
    "CLIENT_ID": -1,
    "SECURITY_LIFECYCLE": "sl_secured_3000",
    "NO_MEASUREMENTS": 1,
    "PROFILE_ID": "PSA_IOT_PROFILE_1"
}
```

`CHALLENGE` should match the nonce which was provided when requesting the token.
`BOOT_SEED` is a random number which will change upon every boot.
`INSTANCE_ID` is a SHA-2 hash of the attestation public key.
`IMPLEMENTATION_ID` is a device-unique value comprised of FCFG data.

See the TF-M Attestation section of the SDK documentation for additional info.
