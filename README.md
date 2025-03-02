# Wio Terminal Public Key Encryption Example

This repository demonstrates public key encryption on the Seeed Wio Terminal.

## Overview

This project implements public key encryption functionality on the Seeed Wio Terminal, utilizing mbedTLS for cryptographic operations. It's built with PlatformIO and uses various Seeed Arduino libraries to handle storage, encryption, and communication.

## Hardware Requirements

- [Seeed Wio Terminal](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/)
- microSD card with FAT32 format (for storing keys)

## Dependencies

The project relies on several Seeed Arduino libraries:

- [Seeed_Arduino_mbedtls](https://github.com/Seeed-Studio/Seeed_Arduino_mbedtls) - Cryptography implementation
- [Seeed_Arduino_rpcUnified](https://github.com/Seeed-Studio/Seeed_Arduino_rpcUnified) - RPC functionality
- [Seeed_Arduino_FS](https://github.com/Seeed-Studio/Seeed_Arduino_FS) - File system operations
- [Seeed_Arduino_SFUD](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD) - SPI Flash Universal Driver

## Key Generation and Storage
Before running the project, you need to generate RSA keys using OpenSSL:

1. Generate an RSA private key:
```bash
openssl genrsa -out private.key 2048
```
2. Convert private key to DER format (DER is a binary format that is easier to work with mbedTLS):
```bash
openssl pkcs8 -topk8 -inform PEM -outform DER -nocrypt -in private.key -out private.der
```
3. Extract public key in DER format: 
```bash
openssl rsa -in private.key -pubout -outform DER -out public.der
```
4. Place the `private.der` and `public.der` files at the root of your SD card

## Building and Running

1. Install [PlatformIO](https://platformio.org/install)
    - vscode will prompt you to install PlatformIO when you open the project
2. Clone this repository
3. Open in VS Code with PlatformIO extension
4. Build and upload to your Wio Terminal:

```bash
pio run -t upload
```

5. Monitor serial output:

```bash
pio run -t monitor
```

## License

This project is licensed under the MIT License.
