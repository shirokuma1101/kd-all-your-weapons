#pragma once

/**************************************************
* 
* Precompile Header File and forced include file
* 
**************************************************/



/**************************************************
* Windows
**************************************************/

// Socket (Must be included before windows.h)
// disable MIN MAX macro
#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
// Windows
#include <Windows.h>
// ComPtr
#include <wrl/client.h>



/**************************************************
* DirectX
**************************************************/

#include <d3d11_4.h>
#include <dxgi1_6.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
//#pragma comment(lib,"dxguid.lib")

// DirectX Tex
#include "DirectXTex.h"
#pragma comment(lib, "DirectXTex.lib")

// DirectX Tool Kit
#include "Audio.h"
#include "SimpleMath.h"
#include "SpriteFont.h"
#pragma comment(lib, "DirectXTKAudioWin8.lib")
#pragma comment(lib, "DirectXTK.lib")



/**************************************************
* fmt
**************************************************/

#define FMT_HEADER_ONLY
#include "fmt/format.h"



/**************************************************
* strconv
**************************************************/

#include "strconv.h"



/**************************************************
* Library
**************************************************/

#include "Utility/Assert.h"
#include "Utility/Macro.h"
#include "Utility/Memory.h"
#include "Utility/StdC++.h"
#include "Utility/Templates.h"

#include "System/DirectX11/DirectX11System.h"
#include "System/DirectX11/DirectX11BufferSystem.h"
#include "System/DirectX11/DirectX11TextureSystem.h"



/**************************************************
* Framework
**************************************************/

namespace Math = DirectX::SimpleMath;

// マテリアル
#include "Framework/Direct3D/KdMaterial.h"
// メッシュ
#include "Framework/Direct3D/KdMesh.h"
// モデル
#include "Framework/Direct3D/KdModel.h"
// 基底ポリゴン
#include "Framework/Direct3D/KdPolygon.h"
// 板ポリゴン
#include "Framework/Direct3D/KdSquarePolygon.h"
// アニメーション
#include "Framework/Math/KdAnimation.h"

// シェーダー
#include "System/Shader/ShaderManager.h"


/**************************************************
* Application
**************************************************/

#include "Application/Application.h"


