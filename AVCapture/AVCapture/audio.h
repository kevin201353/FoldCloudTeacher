#pragma once
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

class MyAudioSink {
public:
	MyAudioSink();
	~MyAudioSink();
	int SetFormat(WAVEFORMATEX *pwfx);
	int CopyData(char *pData, UINT32 numFramesAvailable, BOOL *pbDone);
	BOOL AdjustFormatTo16Bits(WAVEFORMATEX *pwfx);
	HRESULT RecordAudioStream();
};