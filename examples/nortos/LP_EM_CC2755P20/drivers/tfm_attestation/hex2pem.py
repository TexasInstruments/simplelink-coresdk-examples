# This script converts a SECP256R1 public key in uncompressed hex format to a PEM file.
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
import binascii


def uncompressed_to_pem(uncompressed_key_hex, output_file):
    # Remove '04' prefix if present
    if uncompressed_key_hex.startswith("04"):
        uncompressed_key_hex = uncompressed_key_hex[2:]

    # Convert hex string to bytes
    uncompressed_key_bytes = binascii.unhexlify(uncompressed_key_hex)

    # Split the key into x and y coordinates
    x = int.from_bytes(uncompressed_key_bytes[:32], "big")
    y = int.from_bytes(uncompressed_key_bytes[32:], "big")

    # Create the public key object
    public_key = ec.EllipticCurvePublicNumbers(x, y, ec.SECP256R1()).public_key()

    # Serialize the public key to PEM format
    pem = public_key.public_bytes(
        encoding=serialization.Encoding.PEM, format=serialization.PublicFormat.SubjectPublicKeyInfo
    )

    # Write the PEM-encoded key to a file
    with open(output_file, "wb") as f:
        f.write(pem)

    print(f"Public key successfully converted and saved to {output_file}")


# Example usage

# Uncompressed public key (65 bytes: 1 byte prefix + 32 bytes x + 32 bytes y)
uncompressed_key = "0455C9098954F517E7D21E7F917C18138D506F38456150AA61ED8C1F5B7C73F23FFD0E399DD8A9A974B5AEC0C5EE2A70DCAF502A40F57B147F085AFF7A08524EFA"

output_file = "public_key.pem"
uncompressed_to_pem(uncompressed_key, output_file)
print(f"PEM file '{output_file}' has been created.")
