import base64

def encode_file(input_file, output_file):
    with open(input_file, "rb") as file:
        encoded = base64.b64encode(file.read())
    with open(output_file, "wb") as file:
        file.write(encoded)


def decode_file(input_file, output_file):
    with open(input_file, "rb") as file:
        decoded = base64.b64decode(file.read())
    with open(output_file, "wb") as file:
        file.write(decoded)

encode_file("hello.txt", "hello_base64.txt") 
decode_file("hello_base64.txt", "hello_decoded.txt") 
