#include <Arduino.h>
#include <SPI.h>
#include <Seeed_FS.h>
#include <SD/Seeed_SD.h>
#include "mbedtls/config.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/error.h"

/* 
 * This example demonstrates RSA encryption/decryption using the mbedtls library
 * on the Wio Terminal. It uses pre-generated OpenSSL keys stored on the SD card.
 * 
 * Steps to generate keys using OpenSSL (on your computer, not on the Wio):
 * 1. Generate private key: openssl genrsa -out private.key 2048
 * 2. Extract public key in DER format: 
 *    openssl rsa -in private.key -pubout -outform DER -out public.der
 * 3. For private key, convert to DER format:
 *    openssl pkcs8 -topk8 -inform PEM -outform DER -nocrypt -in private.key -out private.der
 * 4. Copy these files to the root directory of the SD card for the Wio Terminal
 */

// Buffer sizes
#define RSA_KEY_SIZE 2048
#define MAX_MESSAGE_LENGTH 100
#define MAX_ENCRYPTED_LENGTH 256  // For RSA-2048

// File paths for keys
const char *PRIVATE_KEY_FILE = "/private.der";  // Use DER format for private key
const char *PUBLIC_KEY_FILE = "/public.der";    // Use DER format for public key

// Sample message to encrypt
const char *message = "Hello, Wio Terminal with RSA encryption!";

// Function declarations
bool loadKeyFromSD(const char *filename, char *buffer, size_t buffer_size);
void displayRsaStatus(int ret_val);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Wait for serial connection 
  while (!Serial) {
    delay(100);
  }
  delay(2000);  // Wait for serial connection
  
  Serial.println("\nWio Terminal RSA Encryption Demo Starting...");
  
  // Initialize SD card
  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card initialized successfully.");
  
  // Buffers for keys and messages
  char public_key_buf[2048] = {0};
  char private_key_buf[2048] = {0};
  unsigned char encrypted[MAX_ENCRYPTED_LENGTH] = {0};
  unsigned char decrypted[MAX_MESSAGE_LENGTH] = {0};
  
  // mbedtls contexts
  mbedtls_pk_context pk_ctx_pub;
  mbedtls_pk_context pk_ctx_priv;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  const char *pers = "rsa_encrypt";
  size_t encrypted_length = 0;
  size_t decrypted_length = 0;
  int ret = 0;
  
  // Load keys from SD card
  if (!loadKeyFromSD(PUBLIC_KEY_FILE, public_key_buf, sizeof(public_key_buf))) {
    Serial.println("Failed to load public key!");
    while (1);
  }
  
  if (!loadKeyFromSD(PRIVATE_KEY_FILE, private_key_buf, sizeof(private_key_buf))) {
    Serial.println("Failed to load private key!");
    while (1);
  }
  
  Serial.println("Keys loaded successfully!");
  
  // Initialize mbedtls contexts
  mbedtls_pk_init(&pk_ctx_pub);
  mbedtls_pk_init(&pk_ctx_priv);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  
  // Seed the random number generator
  ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                              (const unsigned char *)pers, strlen(pers));
  if (ret != 0) {
    Serial.print("Failed to seed random number generator: ");
    displayRsaStatus(ret);
    while (1);
  }
  
  // Get file sizes for DER format keys
  File pubFile = SD.open(PUBLIC_KEY_FILE, FILE_READ);
  size_t pubKeySize = pubFile.size();
  pubFile.close();
  
  File privFile = SD.open(PRIVATE_KEY_FILE, FILE_READ);
  size_t privKeySize = privFile.size();
  privFile.close();
  
  Serial.print("Public key size: ");
  Serial.println(pubKeySize);
  Serial.print("Private key size: ");
  Serial.println(privKeySize);
  
  // Parse public key (using exact byte length for DER)
  ret = mbedtls_pk_parse_public_key(&pk_ctx_pub, (const unsigned char *)public_key_buf, 
                                    pubKeySize);
  if (ret != 0) {
    Serial.print("Failed to parse public key: ");
    displayRsaStatus(ret);
    while (1);
  }
  
  // Parse private key (using exact byte length for DER)
  ret = mbedtls_pk_parse_key(&pk_ctx_priv, (const unsigned char *)private_key_buf,
                             privKeySize, NULL, 0);
  if (ret != 0) {
    Serial.print("Failed to parse private key: ");
    displayRsaStatus(ret);
    while (1);
  }
  
  Serial.println("Keys parsed successfully!");
  
  // Display original message
  Serial.print("Original message: ");
  Serial.println(message);
  
  // Encryption
  Serial.println("Encrypting...");
  ret = mbedtls_pk_encrypt(&pk_ctx_pub, (const unsigned char *)message, strlen(message),
                          encrypted, &encrypted_length, MAX_ENCRYPTED_LENGTH,
                          mbedtls_ctr_drbg_random, &ctr_drbg);
  if (ret != 0) {
    Serial.print("Encryption failed: ");
    displayRsaStatus(ret);
    while (1);
  }
  
  Serial.print("Encrypted message (hex): ");
  for (size_t i = 0; i < encrypted_length; i++) {
    char hex[3];
    sprintf(hex, "%02X", encrypted[i]);
    Serial.print(hex);
  }
  Serial.println();
  
  // Decryption
  Serial.println("Decrypting...");
  ret = mbedtls_pk_decrypt(&pk_ctx_priv, encrypted, encrypted_length,
                          decrypted, &decrypted_length, MAX_MESSAGE_LENGTH,
                          mbedtls_ctr_drbg_random, &ctr_drbg);
  if (ret != 0) {
    Serial.print("Decryption failed: ");
    displayRsaStatus(ret);
    while (1);
  }
  
  // Ensure null termination
  decrypted[decrypted_length] = '\0';
  
  // Display decrypted message
  Serial.print("Decrypted message: ");
  Serial.println((char *)decrypted);
  
  // Free resources
  mbedtls_pk_free(&pk_ctx_pub);
  mbedtls_pk_free(&pk_ctx_priv);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);
  
  Serial.println("Demo completed!");
}

void loop() {
  // Nothing to do here
  delay(1000);
}

// Function to load key data from SD card
bool loadKeyFromSD(const char *filename, char *buffer, size_t buffer_size) {
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    Serial.print("Failed to open file: ");
    Serial.println(filename);
    return false;
  }
  
  // For binary DER files, we don't need null termination but we do need the exact byte count
  size_t bytes_read = file.readBytes(buffer, buffer_size);
  Serial.print("Loaded ");
  Serial.print(bytes_read);
  Serial.print(" bytes from ");
  Serial.println(filename);
  
  file.close();
  return bytes_read > 0;
}

// Function to display mbedtls error codes
void displayRsaStatus(int ret_val) {
  char error_buf[100];
  mbedtls_strerror(ret_val, error_buf, sizeof(error_buf));
  Serial.println(error_buf);
}