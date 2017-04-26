#include "ScreenCapture.h"

mfxStatus Alloc_SysSurfaces(mfxFrameSurface1*& surf_pool, mfxFrameAllocRequest& request, mfxFrameAllocResponse& response)
{
	try
	{
		surf_pool = new mfxFrameSurface1[request.NumFrameSuggested];

		for (mfxU32 i = 0; i < (mfxU32)request.NumFrameSuggested; ++i)
		{
			memset(&surf_pool[i], 0, sizeof(surf_pool[i]));
			surf_pool[i].Info = request.Info;
			if (MFX_FOURCC_NV12 == surf_pool->Info.FourCC)
			{
				surf_pool[i].Data.Y = new mfxU8[surf_pool->Info.Width * request.Info.Height * 3 / 2];
				surf_pool[i].Data.UV = surf_pool[i].Data.Y + request.Info.Width * request.Info.Height;
				surf_pool[i].Data.Pitch = request.Info.Width;
			}
			else if (MFX_FOURCC_RGB4 == request.Info.FourCC || DXGI_FORMAT_AYUV == request.Info.FourCC)
			{
				surf_pool[i].Data.B = new mfxU8[request.Info.Width * request.Info.Height * 4];
				surf_pool[i].Data.G = surf_pool[i].Data.B + 1;
				surf_pool[i].Data.R = surf_pool[i].Data.B + 2;
				surf_pool[i].Data.A = surf_pool[i].Data.B + 3;
				surf_pool[i].Data.Pitch = 4 * request.Info.Width;
			}
		}
		memset(&response, 0, sizeof(response));
		response.NumFrameActual = request.NumFrameSuggested;

		return MFX_ERR_NONE;
	}
	catch (...)
	{
		return MFX_ERR_MEMORY_ALLOC;
	}
}
mfxStatus Free_SysSurfaces(mfxFrameSurface1*& surf_pool, mfxFrameAllocResponse& response)
{
	try
	{
		if (surf_pool)
		{
			for (mfxU32 i = 0; i < (mfxU32)response.NumFrameActual; ++i)
			{
				if (surf_pool[i].Data.Y)
				{
					delete[] surf_pool[i].Data.Y;
					surf_pool[i].Data.Y = 0;
				}
			}
			delete[] surf_pool;
			surf_pool = 0;
		}

		return MFX_ERR_NONE;
	}
	catch (...)
	{
		return MFX_ERR_MEMORY_ALLOC;
	}
}

captureScreen::captureScreen()
{
	memset(&m_param, 0, sizeof(m_param));
	memset(&m_mfxAllocator, 0, sizeof(m_mfxAllocator));
	memset(&m_depar, 0, sizeof(m_depar));
	memset(&m_enpar, 0, sizeof(m_enpar));
	memset(&m_decRequest, 0, sizeof(m_decRequest));
	memset(&m_encRequest, 0, sizeof(m_encRequest));
	memset(&m_bitstream, 0, sizeof(m_bitstream));
	memset(&m_enCodectrl, 0, sizeof(m_enCodectrl));

	m_mfxSes = NULL;
	m_version.Major = MFX_VERSION_MAJOR;
	m_version.Minor = MFX_VERSION_MINOR;
	m_mfxStat = MFX_ERR_NONE;
	m_device_hdl = NULL;
	m_decSurfPool = NULL;
	m_decsyncpoint = 0;
	m_encsyncpoint = 0;
	m_pFrame = NULL;
	m_pOutFrame = NULL;

	//m_param.width = 1920;
	//m_param.height = 1080;
	m_param.width = GetSystemMetrics(SM_CXSCREEN);
	m_param.height = GetSystemMetrics(SM_CYSCREEN);
	m_param.impl =  MFX_IMPL_VIA_D3D9;
	m_param.enc_codec = MFX_CODEC_AVC;
	m_param.fourcc = MFX_FOURCC_NV12;
	m_param.system_memory = false;
	m_param.async_depth = 1;
	m_param.chroma_format = MFX_CHROMAFORMAT_YUV420;

}

mfxI32 captureScreen::Init()
{
	if (m_mfxSes)
	{
		std::cout << "Has been initialized" << std::endl;
		return -1;
	}
	mfxIMPL	impl = MFX_IMPL_HARDWARE_ANY | m_param.impl;
	m_mfxStat = MFXInit(impl, &m_version, &m_mfxSes);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "MFXInit Error");
	if (m_param.impl == MFX_IMPL_VIA_D3D9)
	{
		m_hdl_type = MFX_HANDLE_D3D9_DEVICE_MANAGER;
		m_mfxStat = CreateDx9Device(m_mfxSes, &m_device_hdl, 0);
	}
	else if (m_param.impl == MFX_IMPL_VIA_D3D11)
	{
		m_hdl_type = MFX_HANDLE_D3D11_DEVICE;
		m_mfxStat = CreateDx11Device(m_mfxSes, &m_device_hdl, 0);
	}
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Create device Error");

	m_mfxStat = MFXVideoCORE_SetHandle(m_mfxSes, m_hdl_type, m_device_hdl);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "SetHandle Error");

	m_mfxAllocator.pthis = &m_mfxAllocator;
	if (!m_param.system_memory)
	{
		if (MFX_IMPL_VIA_D3D9 == m_param.impl)
		{
			m_mfxAllocator.Alloc = dx9_simple_alloc;
			m_mfxAllocator.Free = dx9_simple_free;
			m_mfxAllocator.Lock = dx9_simple_lock;
			m_mfxAllocator.Unlock = dx9_simple_unlock;
			m_mfxAllocator.GetHDL = dx9_simple_gethdl;
		}
		else
		{
			m_mfxAllocator.Alloc = dx11_simple_alloc;
			m_mfxAllocator.Free = dx11_simple_free;
			m_mfxAllocator.Lock = dx11_simple_lock;
			m_mfxAllocator.Unlock = dx11_simple_unlock;
			m_mfxAllocator.GetHDL = dx11_simple_gethdl;
		}
		m_mfxStat = MFXVideoCORE_SetFrameAllocator(m_mfxSes, &m_mfxAllocator);
		CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "SetFrameAllocator Error");
	}

	m_mfxStat = MFXVideoUSER_Load(m_mfxSes, &MFX_PLUGINID_CAPTURE_HW, 1);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Load screen capture plug-in Error");

	////////////////设置解码参数
	m_depar.AsyncDepth = m_param.async_depth;
	if (!m_param.system_memory)
	{
		m_depar.IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
	}
	else
	{
		m_depar.IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
	}
	m_depar.mfx.FrameInfo.Width = MSDKALIGN16(m_param.width);
	m_depar.mfx.FrameInfo.Height = MSDKALIGN16(m_param.height);
	m_depar.mfx.FrameInfo.CropW = m_param.width;
	m_depar.mfx.FrameInfo.CropH = m_param.height;
	m_depar.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	m_depar.mfx.FrameInfo.FrameRateExtN = 30;
	m_depar.mfx.FrameInfo.FrameRateExtD = 1;
	m_depar.mfx.FrameInfo.FourCC = m_param.fourcc;
	m_depar.mfx.FrameInfo.ChromaFormat = m_param.chroma_format;
	m_depar.mfx.CodecId = MFX_CODEC_CAPTURE;
	////////////////////////设置编码参数
	m_enpar.AsyncDepth = m_param.async_depth;
	if (!m_param.system_memory)
	{
		m_enpar.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;
	}
	else
	{
		m_enpar.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
	}
	m_enpar.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;

	if (MFX_PICSTRUCT_PROGRESSIVE == m_enpar.mfx.FrameInfo.PicStruct)
	{
		m_enpar.mfx.FrameInfo.Width = MSDKALIGN16(m_param.width);
		m_enpar.mfx.FrameInfo.Height = MSDKALIGN16(m_param.height);
	}
	else
	{
		m_enpar.mfx.FrameInfo.Width = MSDKALIGN32(m_param.width);
		m_enpar.mfx.FrameInfo.Height = MSDKALIGN32(m_param.height);
	}
	m_enpar.mfx.FrameInfo.CropW = m_param.width;
	m_enpar.mfx.FrameInfo.CropH = m_param.height;
	m_enpar.mfx.FrameInfo.FrameRateExtN = 30;
	m_enpar.mfx.FrameInfo.FrameRateExtD = 1;

	if (MFX_CODEC_JPEG == m_param.enc_codec && MFX_FOURCC_RGB4 == m_param.fourcc)
	{
		//JPEG encoder can encode from NV12, but for this tutorial, let's encode in JPEG from RGB4 without VPP
		m_enpar.mfx.FrameInfo.FourCC = MFX_FOURCC_RGB4;
		m_enpar.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
	}
	else {
		m_enpar.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
		m_enpar.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
	}
	m_enpar.mfx.CodecId = m_param.enc_codec;
	m_enpar.mfx.NumRefFrame = 1;
	m_enpar.mfx.NumThread = 2;
	if (MFX_CODEC_AVC == m_enpar.mfx.CodecId)
	{
		m_enpar.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
		m_enpar.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
		m_enpar.mfx.TargetKbps = 5000;
		m_enpar.mfx.GopRefDist = 1;
	}
	else
	{
		m_enpar.mfx.Quality = 75;
	}
	//////////////////查询解码参数
	m_mfxStat = MFXVideoDECODE_Query(m_mfxSes, &m_depar, &m_depar);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Query Decoder Error");
	////////////////初始化解码器
	m_mfxStat = MFXVideoDECODE_Init(m_mfxSes, &m_depar);
	CHECKSTATUS(m_mfxStat,MFX_ERR_NONE, "Init DECODE Error");

	///////////////查询编码参数
	m_mfxStat = MFXVideoENCODE_Query(m_mfxSes, &m_enpar, &m_enpar);
	if (m_mfxStat == MFX_WRN_PARTIAL_ACCELERATION)
		m_mfxStat = MFX_ERR_NONE;
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Query Encoder Error");
	//////////////初始化解码器
	m_mfxStat = MFXVideoENCODE_Init(m_mfxSes, &m_enpar);
	if (m_mfxStat == MFX_WRN_PARTIAL_ACCELERATION)
		m_mfxStat = MFX_ERR_NONE;
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Init Encoder Error");
	////////////////////////////////////////////////////
	m_mfxStat = MFXVideoDECODE_QueryIOSurf(m_mfxSes, &m_depar, &m_decRequest);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "QueryIOSurf decode Error");
	m_decRequest.NumFrameMin = m_decRequest.NumFrameSuggested = m_decRequest.NumFrameSuggested * 200;

	m_mfxStat = MFXVideoENCODE_QueryIOSurf(m_mfxSes, &m_enpar, &m_encRequest);
	if (m_mfxStat == MFX_WRN_PARTIAL_ACCELERATION)
		m_mfxStat = MFX_ERR_NONE;
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "QueryIOSurf encode Error");

	m_sumRequest.Info = m_decRequest.Info;
	m_sumRequest.Type = m_decRequest.Type | m_encRequest.Type;
	m_sumRequest.NumFrameMin = m_decRequest.NumFrameMin + m_encRequest.NumFrameMin;
	m_sumRequest.NumFrameSuggested = m_decRequest.NumFrameSuggested + m_encRequest.NumFrameSuggested;

	if ((m_sumRequest.Type & MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET) && ((m_sumRequest.Type & 0xf0) != MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET))
		m_sumRequest.Type = (mfxU16)(m_sumRequest.Type & (~MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET));

	////////////////分配帧内存
	if (!m_param.system_memory)
	{
		m_mfxStat = m_mfxAllocator.Alloc(m_mfxAllocator.pthis, &m_sumRequest, &m_responce);
		CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Alloc frames Error");
		m_decSurfPool = new mfxFrameSurface1[m_responce.NumFrameActual];
		for (mfxU32 i = 0; i < (mfxU32)m_responce.NumFrameActual; i++)
		{
			memset(&m_decSurfPool[i], 0, sizeof(m_decSurfPool[i]));
			m_decSurfPool[i].Info = m_sumRequest.Info;
			m_decSurfPool[i].Data.MemId = m_responce.mids[i];
		}
	}
	else
	{
		m_mfxStat = Alloc_SysSurfaces(m_decSurfPool, m_decRequest, m_responce);
		CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Alloc frames Error");
	}
/////////////////////////设置分配视屏流的大小
	m_mfxStat = MFXVideoENCODE_GetVideoParam(m_mfxSes,&m_enpar);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "GetVideoParam Encoder Error");
	if (!m_enpar.mfx.BufferSizeInKB)
	{
		m_enpar.mfx.BufferSizeInKB = 10240;
	}
	m_bitstream.MaxLength = m_enpar.mfx.BufferSizeInKB * 1000 * max(1, m_enpar.mfx.BRCParamMultiplier)*m_param.async_depth;
	m_bitstream.Data = new mfxU8[m_bitstream.MaxLength];

	std::cout << "Init OK" << std::endl;
	return MFX_ERR_NONE;
}

mfxI32 captureScreen::GetH264FrameData(void *data,mfxU32 size,bool Is_I_frame)
{
	int iLen = 0;
	m_pFrame = NULL;
	m_pOutFrame = NULL;
	m_decsyncpoint = 0;
	m_encsyncpoint = 0;
	while (1)
	{
		for (mfxU32 j = 0; j < 1000; j++)
		{
			for (mfxU32 i = 0; i < m_responce.NumFrameActual; i++)
			{
				if (!m_decSurfPool[i].Data.Locked)
				{
					m_pFrame = &m_decSurfPool[i];
					break;
				}
			}
			if (m_pFrame)
				break;
			Sleep(1);
		}
		if (!m_pFrame)
		{
			std::cout << "No Frame Data Get" << std::endl;
			return 0;
		}
		m_mfxStat = MFXVideoDECODE_DecodeFrameAsync(m_mfxSes, 0, m_pFrame, &m_pOutFrame, &m_decsyncpoint);
		//printf("decode video fram async  m_mfxStart :%d.\n", m_mfxStat);
		if (MFX_ERR_MORE_DATA == m_mfxStat)
			continue;
		if (MFX_ERR_MORE_SURFACE == m_mfxStat || m_mfxStat > 0
			)
			continue;
		CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "DecodeFrameAsync Error");

		m_mfxStat = MFXVideoCORE_SyncOperation(m_mfxSes, m_decsyncpoint, MFX_INFINITE);
		CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Decode SyncOperation Error");
		m_decsyncpoint = 0;
		if (Is_I_frame)
			m_enCodectrl.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR;
		else
			m_enCodectrl.FrameType = 0;
		m_mfxStat = MFXVideoENCODE_EncodeFrameAsync(m_mfxSes, &m_enCodectrl, m_pOutFrame, &m_bitstream, &m_encsyncpoint);
		//printf("encode video fram async  m_mfxStart :%d.\n", m_mfxStat);
		if (MFX_ERR_MORE_DATA == m_mfxStat)
		{
			//printf("11111.\n");
			continue;
		}
		if (MFX_ERR_MORE_SURFACE == m_mfxStat || m_mfxStat > 0)
		{
			//printf("2222.\n");
			continue;
		}
		CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "EncodeFrameAsync Error");

		m_mfxStat = MFXVideoCORE_SyncOperation(m_mfxSes, m_encsyncpoint, MFX_INFINITE);
		printf("core sync Operation video fram async  m_mfxStart :%d.\n", m_mfxStat);
		CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Decode SyncOperation Error");
		m_encsyncpoint = 0;

		memset(data, 0, size);
		if (size >= m_bitstream.DataLength)
		{
			iLen = m_bitstream.DataLength;
			memcpy(data, m_bitstream.Data + m_bitstream.DataOffset, m_bitstream.DataLength);
			m_bitstream.DataOffset = 0;
			m_bitstream.DataLength = 0;
			//printf("return capture vido len:%d.\n", iLen);
			return iLen;
		}
		else
		{
			m_bitstream.DataOffset = 0;
			m_bitstream.DataLength = 0;
			std::cout << "Run Out of Memory" << std::endl;
			return 0;
		}
	}
	return 0;
}

mfxI32 captureScreen::Destory()
{
	m_mfxStat = MFXVideoDECODE_Close(m_mfxSes);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Decode close failed");
	m_mfxStat = MFXVideoENCODE_Close(m_mfxSes);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Encode close failed");
	if (m_bitstream.Data)
	{
		delete[] m_bitstream.Data;
		m_bitstream.Data = NULL;
	}
	if (!m_param.system_memory)
	{
		m_mfxStat = m_mfxAllocator.Free(m_mfxAllocator.pthis, &m_responce);
	}
	else
	{
		m_mfxStat = Free_SysSurfaces(m_decSurfPool, m_responce);
	}
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Free surface failed");

	if (m_decSurfPool)
	{
		delete m_decSurfPool;
		m_decSurfPool = NULL;
	}

	m_mfxStat = MFXVideoUSER_UnLoad(m_mfxSes, &MFX_PLUGINID_CAPTURE_HW);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Free Plug-in failed");

	m_mfxStat = MFXClose(m_mfxSes);
	CHECKSTATUS(m_mfxStat, MFX_ERR_NONE, "Close Session Error");

	if (m_param.impl == MFX_IMPL_VIA_D3D11)
	{
		CleanupDx11Device();
	}
	else if (m_param.impl == MFX_IMPL_VIA_D3D9)
	{
		CleanupDx9Device();
	}
	return 0;
}