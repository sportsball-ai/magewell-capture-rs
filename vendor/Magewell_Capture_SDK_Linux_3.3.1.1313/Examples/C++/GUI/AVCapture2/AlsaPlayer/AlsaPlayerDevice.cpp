#include "AlsaPlayerDevice.h"

ALSAPlayerDevice::ALSAPlayerDevice()
{
    m_handle=NULL;
    m_stream=SND_PCM_STREAM_PLAYBACK;
}

ALSAPlayerDevice::~ALSAPlayerDevice()
{

}

bool ALSAPlayerDevice::create_device(int card, int device)
{
    char dev_path[100];
    sprintf(dev_path,"hw:%d,%d",card,device);
    iCard=card;
    iDevice=device;
    int err=snd_pcm_open(&m_handle,"default",m_stream,0);
#ifdef MYDEBUG
    qDebug("err=%d",err);
#endif
    return (err==0);
}

bool ALSAPlayerDevice::create_device(char *pName)
{
    int err=snd_pcm_open(&m_handle, pName, m_stream,0);
#ifdef MYDEBUG
    qDebug("err=%d",err);
#endif
    return (err==0);
}

void ALSAPlayerDevice::destory_device()
{
    if(m_handle!=NULL)
    {
        snd_pcm_drain(m_handle);
        snd_pcm_close(m_handle);
        snd_config_update_free_global();
        m_handle=NULL;
    }

}

bool ALSAPlayerDevice::SetFormat(int nSamplesPerSecond, int nChannels, int nSamplesPerFrames, int uBufferMs)
{
    m_nSamplesPerSecond=nSamplesPerSecond;
    m_nChannels=nChannels;
    m_nSamplesPerFrames=nSamplesPerFrames;

    snd_pcm_hw_params_t *params=NULL;
    snd_pcm_hw_params_alloca(&params);
    int nNearDir=0;

    bool bRet=false;
    do{
        if(params==NULL)
            break;

        if(snd_pcm_hw_params_any(m_handle,params)<0)
            break;
        if(snd_pcm_hw_params_set_access(m_handle,params,SND_PCM_ACCESS_RW_INTERLEAVED)<0)
            break;

        if(snd_pcm_hw_params_set_format(m_handle,params,SND_PCM_FORMAT_S16_LE)<0)
            break;

        if(snd_pcm_hw_params_set_channels(m_handle,params,m_nChannels)<0)
            break;

        if(snd_pcm_hw_params_set_rate_near(m_handle,params,&m_nSamplesPerSecond,&nNearDir)<0)
            break;

        if(snd_pcm_hw_params_set_period_size_near(m_handle,params,&m_nSamplesPerFrames,&nNearDir)<0)
            break;

        if(uBufferMs==0)
        {
            snd_pcm_uframes_t uBuffers=m_nSamplesPerFrames*4;
            if(snd_pcm_hw_params_set_buffer_size_near(m_handle,params,&uBuffers)<0)
                break;
        }
        else
        {
            unsigned int uBuffers=uBufferMs*1000;
            if(snd_pcm_hw_params_set_buffer_time_near(m_handle,params,&uBuffers,&nNearDir)<0)
                break;
        }


        if(snd_pcm_hw_params(m_handle,params)<0)
            break;

        if(snd_pcm_hw_params_get_period_size(params,&m_nSamplesPerFrames,&nNearDir)<0)
            break;

        if((int)m_nSamplesPerFrames!=nSamplesPerFrames)
            break;

        if(snd_pcm_prepare(m_handle)<0)
            break;

        bRet=true;
    }while(false);

    return bRet;
}

bool ALSAPlayerDevice::SetVolume(long nValue, long nMin, long nMax)
{
    if(m_handle==NULL)
        return false;
    snd_pcm_info_t *pInfo=NULL;
    snd_pcm_info_alloca(&pInfo);
    if(pInfo==NULL)
    {
        return false;
    }
    bool bRet=false;
    do{
        if(snd_pcm_info(m_handle,pInfo)<0)
            break;
        char cardName[100];
        sprintf(cardName,"hw:%d",iCard);
#ifdef MYDEBUG
        qDebug()<<"alsa play set format"<<endl;
        qDebug()<<cardName<<endl;
#endif

        bool bRet1=false;
        snd_mixer_t *pMixer=NULL;
        do{
            if(snd_mixer_open(&pMixer,0)<0)
                break;

            if(snd_mixer_attach(pMixer,cardName)<0)
                break;

            if(snd_mixer_selem_register(pMixer,NULL,NULL)<0)
                break;

            if(snd_mixer_load(pMixer)<0)
                break;

            for(snd_mixer_elem_t *pElement=snd_mixer_first_elem(pMixer);pElement;pElement=snd_mixer_elem_next(pElement))
            {
                if(snd_mixer_elem_get_type(pElement)!=SND_MIXER_ELEM_SIMPLE||!snd_mixer_selem_is_active(pElement))
                    break;
                if(!snd_mixer_selem_has_capture_volume(pElement))
                    break;

                long lMinOld;
                long lMaxOld;
                int nErr=snd_mixer_selem_get_capture_volume_range(pElement,&lMinOld,&lMaxOld);
                nErr|=snd_mixer_selem_set_capture_volume_all(pElement,lMinOld);
                nErr|=snd_mixer_selem_set_capture_volume_range(pElement,nMin,nMax);
                nErr|=snd_mixer_selem_set_capture_volume_all(pElement,nValue);
                if(nErr!=0)
                    break;
            }
            bRet1=true;
        }while(false);
        if(pMixer)
        {
            snd_mixer_close(pMixer);
        }
        if(!bRet1)
            break;
        bRet=true;
    }while(false);
    if(bRet)
    {
#ifdef MYDEBUG
        qDebug()<<"playset success"<<endl;
#endif
    }

    return bRet;
}

bool ALSAPlayerDevice::write_data(unsigned char *data,int cSamples)
{
    if(m_handle==NULL)
        return false;

    snd_pcm_sframes_t nSamples=snd_pcm_writei(m_handle,data,cSamples);

    if(nSamples==-EPIPE)
    {
        snd_pcm_prepare(m_handle);
        return write_data(data,cSamples);
    }
    else if(nSamples<0)
    {
        snd_pcm_recover(m_handle,nSamples,1);
    }
    else if(nSamples!=(snd_pcm_sframes_t)cSamples)
    {
        write_data(data+m_nChannels*2*nSamples,cSamples-nSamples);
    }
    else
    {
        return true;
    }
    return false;
}

