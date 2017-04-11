#include "audio.h"
#include "../include/global.h"
#include "MyRingBuffer.h"

#define REFTIMES_PER_SEC        10000000
#define REFTIMES_PER_MILLISEC   10000

#define EXIT_ON_ERROR(hres)  \
    if (FAILED(hres)) { goto Exit; }

#define SAFE_RELEASE(punk)  \
    if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator    = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator       = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient              = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient       = __uuidof(IAudioCaptureClient);


FILE *fp = NULL;
extern char g_szDataBuf_A[MAX_DATA_BUF_A];
extern MyRingBuffer   g_myRingBuffer_Au;
extern struct cyclic_buf cb;
extern myCriticalSection  g_criticalSection;
extern MyRingBuffer2  g_myRingRuffer2;
extern struct cyclic_buf  g_cbAudio;

MyAudioSink::MyAudioSink()
{
	CoInitialize(NULL);
}

MyAudioSink::~MyAudioSink()
{
	CoUninitialize();
}

int MyAudioSink::SetFormat(WAVEFORMATEX *pwfx)
{
	FILE *fp = NULL;
	fopen_s(&fp, "format.txt", "w");
    char str[128];
    sprintf_s(str, "wFormatTag \t\tis %x\nnChannels \t\tis %x\nnSamplesPerSec \tis %ld\nnAvgBytesPerSec is %ld\nwBitsPerSample \tis %d",
            pwfx->wFormatTag, pwfx->nChannels, pwfx->nSamplesPerSec, pwfx->nAvgBytesPerSec, pwfx->wBitsPerSample);
    fwrite(str, strlen(str), 1, fp);
    fclose(fp);
    return 0;
}

int MyAudioSink::CopyData(char *pData, UINT32 numFramesAvailable, BOOL *pbDone)
{
    if(pData != NULL)
        fwrite(pData, numFramesAvailable, 1, fp);
    return 0;
}

// pwfx->nSamplesPerSec = 44100;
BOOL MyAudioSink::AdjustFormatTo16Bits(WAVEFORMATEX *pwfx)
{
    BOOL bRet(FALSE);
    if(pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        pwfx->wFormatTag = WAVE_FORMAT_PCM;
        pwfx->wBitsPerSample = 16;
        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
        bRet = TRUE;
    }
    else if(pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
        if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
        {
            pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            pEx->Samples.wValidBitsPerSample = 16;
            pwfx->wBitsPerSample = 16;
            pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
            pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
            bRet = TRUE;
        }
    }
    return bRet;
}

HRESULT MyAudioSink::RecordAudioStream()
{
    HRESULT         hr;
    REFERENCE_TIME  hnsActualDuration;
    UINT32          bufferFrameCount;
    UINT32          numFramesAvailable;
    BYTE *          pData;
    DWORD           flags;
    REFERENCE_TIME  hnsDefaultDevicePeriod(0);

    IMMDeviceEnumerator *   pEnumerator             = NULL;
    IMMDevice *             pDevice                 = NULL;
    IAudioClient *          pAudioClient            = NULL;
    IAudioCaptureClient *   pCaptureClient          = NULL;
    WAVEFORMATEX *          pwfx                    = NULL;
    UINT32                  packetLength            = 0;
    BOOL                    bDone                   = FALSE;
    HANDLE                  hTimerWakeUp            = NULL;


    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, NULL);

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

    AdjustFormatTo16Bits(pwfx);

    hTimerWakeUp = CreateWaitableTimer(NULL, FALSE, NULL);

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pwfx, NULL);
    EXIT_ON_ERROR(hr)

    // Get the size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    LARGE_INTEGER liFirstFire;
    liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2; // negative means relative time
    LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000);

    SetWaitableTimer(hTimerWakeUp, &liFirstFire, lTimeBetweenFires, NULL, NULL, FALSE);

    // Notify the audio sink which format to use.
    hr = SetFormat(pwfx);
    //EXIT_ON_ERROR(hr)

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start recording.
    EXIT_ON_ERROR(hr)

    HANDLE waitArray[1] = {hTimerWakeUp};

    // Each loop fills about half of the shared buffer.
	DWORD  dwStart = 0;
	DWORD  dwEnd = 0;
	//int ncount = 0;
	//long total_len = 0;
    while (bDone == FALSE)
    {
        WaitForMultipleObjects(sizeof(waitArray) / sizeof(waitArray[0]), waitArray, FALSE, INFINITE);

        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)

        while (packetLength != 0)
        {
            // Get the available data in the shared buffer.
			dwStart = GetTickCount();
			DWORD tmpT = dwEnd - dwStart;
			printf("apture audio timeT :%d.\n", tmpT);
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
			EXIT_ON_ERROR(hr)

				// Copy the available capture data to the audio sink.
				//hr = pMySink->CopyData((char *)pData, numFramesAvailable * pwfx->nBlockAlign, &bDone);
				//EXIT_ON_ERROR(hr)
			int len = numFramesAvailable * pwfx->nBlockAlign;
			if (len >= MAX_DATA_BUF_A)
				len = MAX_DATA_BUF_A;
			Audio_Package  aupk;
			aupk.len = len;
			memcpy(aupk.pdata, (char *)pData, len);
			cyclic_buf_produce_data(&g_cbAudio, &aupk, sizeof(Audio_Package));
			dump_cyclic_buf(&g_cbAudio, "after produce");
			//g_myRingBuffer_Au.write((char *)pData, len);
			//g_criticalSection.enter();
			/*
			cyclic_buf_produce_data(&cb, g_szDataBuf_A, len);
			cyclic_buf_produce(&cb);
			g_criticalSection.leave();
			*/
           /*
			if (ncount >= 65535)
			{
				ncount = 0;
				total_len = 0;
			}
			if (ncount <= 2)
			{
				memcpy(g_myRingRuffer2.g_szDataBuf_A + total_len, (char *)pData, len);
				g_myRingBuffer_Au.write((char *)pData, len)
			}
			else
			{
				memset(g_myRingRuffer2.g_szDataBuf_A, 0, MAX_DATA_BUF_A);
				memcpy(g_myRingRuffer2.g_szDataBuf_A , (char *)pData, len);
				g_myRingRuffer2.m_len = len;
			}
			*/
			/*
			ReportMsg msg;
			msg.action = USER_MSG_AUDIO_START;
			msg.len = len;
			call_msg_back(msg_respose, msg);
			*/
			/*
			ncount++;
			total_len = ncount*len;
			g_myRingRuffer2.m_count = ncount;
			*/

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr)
            hr = pCaptureClient->GetNextPacketSize(&packetLength);
			EXIT_ON_ERROR(hr);
			dwEnd = GetTickCount();
        }
    }
    hr = pAudioClient->Stop();  // Stop recording.
    EXIT_ON_ERROR(hr)

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pCaptureClient)

    return hr;
}

/*
int main()
{
    fp = fopen("foobar","wb");
    MyAudioSink test;

    CoInitialize(NULL);
    RecordAudioStream(&test);
    CoUninitialize();

    fclose(fp);
    return 0;
}*/
