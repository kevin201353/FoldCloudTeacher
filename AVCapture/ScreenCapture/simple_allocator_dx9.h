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
#include <atlbase.h>

#include <initguid.h>
#pragma warning(disable : 4201) // Disable annoying DX warning
#include <d3d9.h>
#include <dxva2api.h>
//#include "C:\Program Files (x86)\Windows Kits\8.1\Include\um\dxva2api.h"

#include "mfxdefs.h"
#include "mfxstructures.h"
#include "mfxvideo.h"
#include "mfxplugin.h"

#define MAX_FILENAME_LEN 1024

#define  MSDK_SAFE_RELEASE(X)            {if (X) { X->Release(); X = NULL; }}

// =================================================================
// DirectX functionality required to manage D3D surfaces
//

// Create DirectX 9 device context
// - Required when using D3D surfaces.
// - D3D Device created and handed to Intel Media SDK
// - Intel graphics device adapter id will be determined automatically (does not have to be primary),
//   but with the following caveats:
//     - Device must be active. Normally means a monitor has to be physically attached to device
//     - Device must be enabled in BIOS. Required for the case when used together with a discrete graphics card
//     - For switchable graphics solutions (mobile) make sure that Intel device is the active device
mfxStatus CreateDx9Device(mfxSession session, mfxHDL* deviceHandle, HWND hWnd, bool bCreateSharedHandles = false);
void CleanupDx9Device();
IDirect3DDevice9Ex* GetDx9Device();

mfxStatus dx9_simple_alloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
mfxStatus dx9_simple_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus dx9_simple_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus dx9_simple_gethdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
mfxStatus dx9_simple_free(mfxHDL pthis, mfxFrameAllocResponse *response);
