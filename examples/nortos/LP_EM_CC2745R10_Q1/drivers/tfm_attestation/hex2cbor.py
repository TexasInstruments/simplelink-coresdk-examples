# This script converts a hex string to a binary file in CBOR format.
import binascii


def hex_to_cbor_file(hex_string, output_file):
    # Remove any whitespace from the hex string
    hex_string = hex_string.replace(" ", "")

    # Convert hex to binary
    binary_data = binascii.unhexlify(hex_string)

    # Write binary data to file
    with open(output_file, "wb") as f:
        f.write(binary_data)


# Example usage

token_hex_data = "D28443A10126A058E9A83A000124FF58404600DD482C538C81F1832F00AA362C5B33AB9BF7740D9764B61599DDBE12BEC37E94639E5EBF2036C4841D8539F2AD963FEC13095DDC8073BA9075DB"
token_hex_data += "4586E2533A000124FB58209DE52218C9597873535359AD55488E4CB741B7A9965381F6600CABBC953AD1213A000125005821013D378EB37DE78FDB4BD64442DCA50A17E8FE10C24010AEE4C9F8"
token_hex_data += "FBBDD4AEE88E3A000124FA5820961D00A000000000306C9FA720000BE8C44AEE903C28000044BA3634004B12003A000124F8203A000124F91930003A000124FE013A000124F7715053415F494F"
token_hex_data += "545F50524F46494C455F315840E4B555F39E058C16F5A359B5F63234B262487934CC6087621DC98781C131E70EBEA04975831961458442447288DFEA8D4AAD8EA44473FE9E646316FAF35C9248"

output_file = "attest_token.cbor"

hex_to_cbor_file(token_hex_data, output_file)
print(f"CBOR file '{output_file}' has been created.")
