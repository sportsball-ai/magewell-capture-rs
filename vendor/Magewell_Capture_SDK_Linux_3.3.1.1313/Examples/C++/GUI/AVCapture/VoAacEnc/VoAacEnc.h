#ifndef VOAAC_ENC_H
#define VOAAC_ENC_H
#include <stdio.h>
typedef  struct st_voaac_enc_handle { int unused; }*mw_voaac_enc_t;
#ifdef __cplusplus
extern "C" {
#endif
mw_voaac_enc_t mw_voaac_enc_create(unsigned int channels, unsigned int sample_rate, unsigned int bits_per_sample, unsigned int bit_rate);
int mw_voaac_enc_frame(mw_voaac_enc_t handle, unsigned char *p_pcm, unsigned int pcm_len, unsigned char *p_aac, unsigned int aac_len, unsigned int *p_aac_frame_len);
int mw_voaac_enc_destory(mw_voaac_enc_t handle);
#ifdef __cplusplus
}
#endif
#endif
