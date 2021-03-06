#include "Precompiled.h"
#ifdef ENABLE_DIRECTX11

#include "D3DUtil.h"

#include "GraphicsSystemDX11.h"

ID3D11Device * Angazi::Graphics::GetDevice()
{
	return GraphicsSystem::Get()->mD3DDevice;
}

ID3D11DeviceContext * Angazi::Graphics::GetContext()
{
	return  GraphicsSystem::Get()->mImmediateContext;
}

#endif