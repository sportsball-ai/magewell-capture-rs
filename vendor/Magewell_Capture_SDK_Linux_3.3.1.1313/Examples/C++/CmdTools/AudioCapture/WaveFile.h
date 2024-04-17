/************************************************************************************************/
// MAGEWELL PROPRIETARY INFORMATION

// The following license only applies to head files and library within Magewell’s SDK 
// and not to Magewell’s SDK as a whole. 

// Copyrights © Nanjing Magewell Electronics Co., Ltd. (“Magewell”) All rights reserved.

// Magewell grands to any person who obtains the copy of Magewell’s head files and library 
// the rights,including without limitation, to use, modify, publish, sublicense, distribute
// the Software on the conditions that all the following terms are met:
// - The above copyright notice shall be retained in any circumstances.
// -The following disclaimer shall be included in the software and documentation and/or 
// other materials provided for the purpose of publish, distribution or sublicense.

// THE SOFTWARE IS PROVIDED BY MAGEWELL “AS IS” AND ANY EXPRESS, INCLUDING BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL MAGEWELL BE LIABLE 

// FOR ANY CLAIM, DIRECT OR INDIRECT DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT,
// TORT OR OTHERWISE, ARISING IN ANY WAY OF USING THE SOFTWARE.

// CONTACT INFORMATION:
// SDK@magewell.net
// http://www.magewell.com/
//
/************************************************************************************************/

#pragma once

#include "LibMWCapture/MWLinux.h"
#include "LibMWCapture/WinTypes.h"
#include "stdio.h"

#define FOURCC(ch0, ch1, ch2, ch3)											\
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |							\
    ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))

class CWaveFile
{
public:
    CWaveFile(void) {
        m_pFile = NULL;
    }
    virtual ~CWaveFile(void) {
        if (m_pFile != NULL) {
            Exit();
        }
    }

public:
    BOOL Init(const char * pszFilePath, int nSamplesPerSec, int nChannels = 2, WORD wBitsPerSample = 16)
    {
        m_wfHeader.dwRiffFlag = FOURCC('R','I','F','F');;
        m_wfHeader.dwFileSize = sizeof(WAVE_FILE_HEADER) - 12;

        m_wfHeader.dwWaveFlag = FOURCC('W','A','V','E');;
        m_wfHeader.dwFmtFlag = FOURCC('f','m','t',' ');
        m_wfHeader.dwFmtSize = sizeof(PCMWAVEFORMAT);

        m_wfHeader.pcmFormat.wBitsPerSample = wBitsPerSample;
        m_wfHeader.pcmFormat.wFormatTag = 0x0001;
        m_wfHeader.pcmFormat.wChannels = nChannels;
        m_wfHeader.pcmFormat.dwSamplesPerSec = nSamplesPerSec;
        m_wfHeader.pcmFormat.wBlockAlign = m_wfHeader.pcmFormat.wBitsPerSample * nChannels / 8;
        m_wfHeader.pcmFormat.dwAvgBytesPerSec =  m_wfHeader.pcmFormat.wBlockAlign * nSamplesPerSec;

        m_wfHeader.dwDataFlag = FOURCC('d','a','t','a');
        m_wfHeader.dwDataSize = 0;

        m_pFile=fopen(pszFilePath,"wb");
        if (NULL == m_pFile)
            return FALSE;

        if (fwrite(&m_wfHeader, sizeof(WAVE_FILE_HEADER), 1, m_pFile) != 1) {
            fclose(m_pFile);
            return FALSE;
        }
        return TRUE;
    }
    void Exit()
    {
        if (m_pFile) {
            m_wfHeader.dwFileSize = m_wfHeader.dwDataSize + sizeof(WAVE_FILE_HEADER) - 12;

            fseek(m_pFile, 0, SEEK_SET);
            fwrite(&m_wfHeader, sizeof(m_wfHeader), 1, m_pFile);

            fclose(m_pFile);
            m_pFile = NULL;
        }
    }
    BOOL Write(const BYTE * pData, int nSize)
    {
        if (fwrite(pData, nSize,1,  m_pFile) != 1)
            return FALSE;

        m_wfHeader.dwDataSize += nSize;
        return TRUE;
    }
    BOOL IsOpen() {
        return (m_pFile != NULL);
    }

protected:
    typedef struct {
        WORD 	wFormatTag; //0x0001
        WORD 	wChannels;
        DWORD	dwSamplesPerSec;
        DWORD	dwAvgBytesPerSec;
        WORD	wBlockAlign;
        WORD	wBitsPerSample;
    }PCMWAVEFORMAT;

    typedef struct tagWAVE_FILE_HEADER {
        DWORD dwRiffFlag; // 'RIFF'
        DWORD dwFileSize;
        DWORD dwWaveFlag; // 'WAVE'
        DWORD dwFmtFlag;  // 'fmt'
        DWORD dwFmtSize;
        PCMWAVEFORMAT pcmFormat;
        DWORD dwDataFlag; // 'data'
        DWORD dwDataSize;
    } WAVE_FILE_HEADER, *PWAVE_FILE_HEADER;
    FILE * m_pFile;
    WAVE_FILE_HEADER m_wfHeader;
};
