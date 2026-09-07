## Example Summary

This example demonstrates the usage of the ECDSA driver.

## Example Usage

This examples shows how to use the ECDSA and SHA2 drivers in both sign and
verify operations and single step hash mode.

* The example initializes the ECDSA and SHA2 drivers, and turns on a LED.

* The example runs ECDSA operations in callback mode and the user of this
example can change this behavior by modifying the following line of code at
the top of the `ecdsaSignVerify.c` file.

```sh
ECDSA_ReturnBehavior ecdsaReturnBehavior = ECDSA_RETURN_BEHAVIOR_CALLBACK;
```

* The SHA2 operations are conducted using the default parameters (blocking
mode).

* For SHA2 operations, they are only conducted on SECP256 and BRP256 test
vectors and the digest that is part of the test vector is then ignored.

* For other curves, the example relies on pre-hashed digest present in the test
vectors.

* If opening the SHA2 or ECDSA driver fails, or the ECDSA signing or verification
step fails, the application will toggle the LED in an infinite loop. If the
LED is turned on and not blinking, the example completed successfully.

## Application Design Details

This example is intended for CC27XX devices only.

CC27XX devices leverage a standalone HSM hardware accelerator and its
corresponding software architecture for its cryptography operations.

This example project runs through an array of test vectors each designed to test
against a specific Elliptic Curve (Please refer to "source/ti/drivers/ECDSA.h"
for a list of all supported Elliptic Curves) and performs a hash operation for
two of those test vectors (SECP256 and BRP256).

The example performs ECDSA sign and verify operations on a digest
provided the private/public key and signature (depending on the operation).
