//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2015 Intel Corporation. All Rights Reserved.
//

#pragma once

#include <memory>
#include <iostream>
#include <assert.h>

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <atlbase.h>

#include <initguid.h>
#pragma warning(disable : 4201) // Disable annoying DX warning

#include "mfxdefs.h"
#include "mfxstructures.h"
#include "mfxvideo.h"
#include "mfxplugin.h"

#define MAX_FILENAME_LEN 1024

#define  MSDK_SAFE_RELEASE(X)            {if (X) { X->Release(); X = NULL; }}

#define WILL_READ  0x1000
#define WILL_WRITE 0x2000

mfxStatus CreateDx11Device(mfxSession session, mfxHDL* deviceHandle, HWND hWnd);
void CleanupDx11Device();

mfxStatus dx11_simple_alloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
mfxStatus dx11_simple_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus dx11_simple_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus dx11_simple_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
mfxStatus dx11_simple_free(mfxHDL pthis, mfxFrameAllocResponse *response);
