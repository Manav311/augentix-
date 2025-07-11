#ifndef STAMEN_DECRYPT_H_
#define STAMEN_DECRYPT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STAMEN_DECRYPT_H_ */
