#include "voAAC.h"
#include "cmnMemory.h"
#include "VoAacEnc.h"

static VO_AUDIO_CODECAPI g_voaac_api;
static VO_MEM_OPERATOR g_voaac_mem = { 0 };
static VO_CODEC_INIT_USERDATA g_user_data;


mw_voaac_enc_t mw_voaac_enc_create(unsigned int channels, unsigned int sample_rate, unsigned int bits_per_sample, unsigned int bit_rate)
{
    VO_HANDLE vo_handle = NULL;//32000, 1, 44100, 16
    AACENC_PARAM parms = {0};
    if (bits_per_sample != 16)
    {
        printf("voaac bits_per_sample[%u] != 16\n",bits_per_sample);
        return NULL;
    }

    voGetAACEncAPI(&g_voaac_api);
    g_voaac_mem.Alloc = cmnMemAlloc;
    g_voaac_mem.Copy = cmnMemCopy;
    g_voaac_mem.Free = cmnMemFree;
    g_voaac_mem.Set = cmnMemSet;
    g_voaac_mem.Check = cmnMemCheck;
    g_user_data.memflag = VO_IMF_USERMEMOPERATOR;
    g_user_data.memData = &g_voaac_mem;
    if(g_voaac_api.Init(&(vo_handle), VO_AUDIO_CodingAAC, &g_user_data) != VO_ERR_NONE){
        printf("voaac handle alloc err\n");
        return NULL;
    }

    parms.sampleRate = sample_rate;
    parms.bitRate = bit_rate;
    parms.nChannels = channels;
    parms.adtsUsed = 1; 

    if (g_voaac_api.SetParam(vo_handle, VO_PID_AAC_ENCPARAM, &parms) != VO_ERR_NONE)
    {
        printf("voaac SetParam error bit_rate[%u], channels[%u], sample_rate[%u]\n", bit_rate, channels, sample_rate);
        g_voaac_api.Uninit(vo_handle);
        return NULL;
    }
    printf("voaac task[%u] create\n", vo_handle);
    return (mw_voaac_enc_t)vo_handle;
}
int mw_voaac_enc_frame(mw_voaac_enc_t handle, unsigned char *p_pcm, unsigned int pcm_len, unsigned char *p_aac, unsigned int aac_len, unsigned int *p_aac_frame_len)
{
    int i = 0;
    int ret = 0;
    unsigned int out_size = 0;
    VO_CODECBUFFER input = { 0 }, output = { 0 };
    VO_AUDIO_OUTPUTINFO output_info = { 0 };
    VO_HANDLE vo_handle = (VO_HANDLE )handle;
    input.Buffer = (VO_PBYTE)p_pcm;
    input.Length = pcm_len;
    g_voaac_api.SetInputData(vo_handle, &input);

    out_size = 0;
    *p_aac_frame_len = 0;
    while(1){
        output.Buffer = (VO_PBYTE)p_aac + out_size;
        output.Length = aac_len - out_size;
        ret = g_voaac_api.GetOutputData(vo_handle, &output, &output_info);
        if(ret == VO_ERR_INPUT_BUFFER_SMALL){
            return (int)(out_size);
        }
        if(ret == VO_ERR_OUTPUT_BUFFER_SMALL){
            printf("voaac task[%p] out buf is small or put too much data to enc pcm_len[%u], aac_len[%u]\n", vo_handle, pcm_len, aac_len);
            return (int)(out_size);
        }
        if(ret != VO_ERR_NONE){
             printf("voaac task[%p] enc error", vo_handle);
            return -1;
        }
        out_size += output.Length;
        p_aac_frame_len[i++] = output.Length;
    }
    return (int)(out_size);
}

int mw_voaac_enc_destory(mw_voaac_enc_t handle)
{
    VO_HANDLE vo_handle = (VO_HANDLE )handle;
    g_voaac_api.Uninit(vo_handle);
    printf("voaac task[%p] free", handle);
    return 0;
}


