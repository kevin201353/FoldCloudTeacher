#ifndef _SCREENCAPTURE_H_
#define _SCREENCAPTURE_H_
#pragma once
#include <map>
#include "simple_allocator_dx9.h"
#include "simple_allocator_dx11.h"
#include "mfxjpeg.h"


//#define max(a,b)  ((a) > (b) ? (a) : (b) )

#define CHECKSTATUS(STS,EXPECTED,MESSAGE)						\
if(STS != EXPECTED)												\
{																\
	std::cout << MESSAGE <<" [" << STS << "]" << std::endl;	\
	return STS;													\
}										

#define MSDKALIGN32(value)                      (((value + 31) >> 5) << 5) // round up to a multiple of 32
#define MSDKALIGN16(value)                      (((value + 15) >> 4) << 4) // round up to a multiple of 16
typedef struct InputParams
{
	mfxIMPL		impl;
	mfxU32		width;
	mfxU32		height;
	mfxU32		fourcc;			//MFX_FOURCC_NV12
	mfxU32		enc_codec;		//MFX_CODE_AVC
	mfxU32		async_depth;
	mfxU32		chroma_format;	//MFX_CHROMAFORMAT_YUV420
	bool		system_memory;
}param;
class captureScreen
{
private:
	param			m_param;
	mfxSession		m_mfxSes;
	mfxStatus		m_mfxStat;
	mfxVersion		m_version;
	mfxHDL			m_device_hdl;
	mfxHandleType	m_hdl_type;
	mfxFrameAllocator	m_mfxAllocator;
	mfxVideoParam	m_depar;
	mfxVideoParam	m_enpar;
	mfxFrameAllocRequest	m_decRequest;
	mfxFrameAllocRequest	m_encRequest;
	mfxFrameAllocRequest	m_sumRequest;
	mfxFrameAllocResponse	m_responce;
	mfxFrameSurface1		*m_decSurfPool;
	mfxFrameSurface1		*m_pFrame;
	mfxFrameSurface1		*m_pOutFrame;
	mfxBitstream			m_bitstream;
	mfxSyncPoint			m_decsyncpoint;
	mfxSyncPoint			m_encsyncpoint;
	mfxEncodeCtrl			m_enCodectrl;
public:
	captureScreen();
	mfxI32 Init();
	mfxI32 GetH264FrameData(void *data, mfxU32 size,bool Is_I_frame = false);
	mfxI32 Destory();
};
#endif // !_SCREENCAPTURE_H_

