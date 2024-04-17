#ifndef ALSAPLAYERDEVICE_H
#define ALSAPLAYERDEVICE_H
#include "alsa/asoundlib.h"


class ALSAPlayerDevice
{
public:
    ALSAPlayerDevice();
    ~ALSAPlayerDevice();

public:

protected:
    int iCard;
    int iDevice;
    snd_pcm_t *m_handle;
    snd_pcm_stream_t m_stream;

    unsigned int m_nSamplesPerSecond;
    unsigned int m_nChannels;

    snd_pcm_uframes_t m_nSamplesPerFrames;

public:
    bool create_device(int card,int device);
	bool create_device(char *pName);
    void destory_device();
    bool SetFormat(int nSamplesPerSecond,int nChannels,int nSamplesPerFrames,int uBufferMs=0);
    bool SetVolume(long nValue,long nMin=0,long nMax=100);
    bool write_data(unsigned char *data, int cSamples);
};

#endif // ALSAPLAYERDEVICE_H
