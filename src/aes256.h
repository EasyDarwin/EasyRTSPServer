#ifndef uint8_t
#define uint8_t unsigned char
#endif
#ifdef __cplusplus
extern "C" 
{ 
#endif

//=======================================================================
//=======================================================================
//==========AESº”Ω‚√‹====================================================
//=======================================================================
//=======================================================================

  typedef struct {
    uint8_t key[32]; 
    uint8_t enckey[32]; 
    uint8_t deckey[32];
  } aes256_context; 

  void aes256_init(aes256_context *, uint8_t * );
  void aes256_done(aes256_context *);
  void aes256_encrypt_ecb(aes256_context *, uint8_t * );
  void aes256_decrypt_ecb(aes256_context *, uint8_t * );

#ifdef __cplusplus
}
#endif