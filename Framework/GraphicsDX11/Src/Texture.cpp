#include "Precompiled.h"
#include "Texture.h"

#ifdef ENABLE_DIRECTX11

#include "D3DUtil.h"
#include <DirectXTK/Inc/WICTextureLoader.h>
#include <DirectXTex/DirectXTex/DirectXTexP.h>
#include "TextureUtil.h"

using namespace Angazi::Graphics;

Texture::~Texture()
{
	ASSERT(mShaderResourceView == nullptr, "[Texture] Terminate() must be called to clean up!");
}

void Texture::Initialize(const std::filesystem::path& fileName, bool gammaCorrection)
{
	HRESULT hr;
	if (gammaCorrection)
		hr = DirectX::CreateWICTextureFromFileEx(GetDevice(), GetContext(), fileName.c_str()
			, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0
			, D3D11_RESOURCE_MISC_GENERATE_MIPS, DirectX::WIC_LOADER_FORCE_SRGB, nullptr, &mShaderResourceView);
	else
		hr = DirectX::CreateWICTextureFromFile(GetDevice(), GetContext(), fileName.c_str(), nullptr, &mShaderResourceView);
	ASSERT(SUCCEEDED(hr), "Failed to load texture %ls.", fileName.c_str());

	// Get Width and Height of the texture
	ID3D11Resource* resource = nullptr;
	mShaderResourceView->GetResource(&resource);

	ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(resource);
	D3D11_TEXTURE2D_DESC desc{};
	texture->GetDesc(&desc);

	mWidth = desc.Width;
	mHeight = desc.Height;
}

void Texture::Initialize(const std::vector<std::filesystem::path>& cubeSides, bool gammaCorrection)
{
	HRESULT hr;
	std::array<ID3D11Resource*, 6> resourceArray;
	std::array<ID3D11ShaderResourceView*, 6> srvArray;
	int i = 0;
	for (auto side : cubeSides)
	{
		if (gammaCorrection)
			hr = DirectX::CreateWICTextureFromFileEx(GetDevice(), GetContext(), side.c_str()
				, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0
				, D3D11_RESOURCE_MISC_GENERATE_MIPS, DirectX::WIC_LOADER_FORCE_SRGB, nullptr, &srvArray[i]);
		else
			hr = DirectX::CreateWICTextureFromFile(GetDevice(), GetContext(), side.c_str(), nullptr, &srvArray[i]);

		ASSERT(SUCCEEDED(hr), "[Texture] Failed to load texture %ls.", side.c_str());
		srvArray[i]->GetResource(&resourceArray[i]);
		i++;
	}

	D3D11_TEXTURE2D_DESC texElementDesc;
	static_cast<ID3D11Texture2D*>(resourceArray[0])->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = 6;
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	ID3D11Texture2D* texArray = 0;
	hr = GetDevice()->CreateTexture2D(&texArrayDesc, 0, &texArray);
	ASSERT(SUCCEEDED(hr), "[Texture] Failed to create 2D texture");

	// Copy individual texture elements into texture array.
	D3D11_BOX sourceRegion;
	// Copy Mip Map
	for (UINT x = 0; x < 6; x++)
	{
		for (UINT mipLevel = 0; mipLevel < texArrayDesc.MipLevels; mipLevel++)
		{
			sourceRegion.left = 0;
			sourceRegion.right = (texArrayDesc.Width >> mipLevel);
			sourceRegion.top = 0;
			sourceRegion.bottom = (texArrayDesc.Height >> mipLevel);
			sourceRegion.front = 0;
			sourceRegion.back = 1;

			//test for overflow
			if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
				break;

			GetContext()->CopySubresourceRegion(texArray, D3D11CalcSubresource(mipLevel, x,
				texArrayDesc.MipLevels), 0, 0, 0, resourceArray[x], mipLevel, &sourceRegion);
		}
		SafeRelease(resourceArray[x]);
		SafeRelease(srvArray[x]);
	}

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = texArrayDesc.MipLevels;

	hr = GetDevice()->CreateShaderResourceView(texArray, &viewDesc, &mShaderResourceView);
	ASSERT(SUCCEEDED(hr), "[Texture] Failed to create cube texture");
}

void Texture::InitializeHdrCube(const std::filesystem::path & filePath, const std::filesystem::path & shaderFilePath, uint32_t cubeLength)
{
	DirectX::ScratchImage image;
	ID3D11ShaderResourceView* hdrShaderResourceView;

	HRESULT hr = DirectX::LoadFromHDRFile(filePath.c_str(), nullptr, image);
	ASSERT(SUCCEEDED(hr), "[Texture] Failed to load texture");
	hr = CreateShaderResourceView(GetDevice(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), &hdrShaderResourceView);
	ASSERT(SUCCEEDED(hr), "[Texture] Failed to create shader resource view");

	mShaderResourceView = TextureUtil::CreateCubeMapFromTexture(hdrShaderResourceView, shaderFilePath, cubeLength);

	SafeRelease(hdrShaderResourceView);

	// Get Width and Height of the texture
	ID3D11Resource* resource = nullptr;
	mShaderResourceView->GetResource(&resource);

	ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(resource);
	D3D11_TEXTURE2D_DESC desc{};
	texture->GetDesc(&desc);
	SafeRelease(resource);

	mWidth = desc.Width;
	mHeight = desc.Height;
}

void Texture::InitializeCubeMap(Texture & texture, const std::filesystem::path & shaderFilePath, uint32_t cubeLength, CubeMapType type)
{
	switch (type)
	{
	case Angazi::Graphics::Texture::IrradianceMap:
		mShaderResourceView = TextureUtil::CreateCubeMapFromTexture(texture.GetShaderResourceView(), shaderFilePath, cubeLength);
		break;
	case Angazi::Graphics::Texture::PreFiltered:
		mShaderResourceView = TextureUtil::CreatePreFilteredCubeMap(texture.GetShaderResourceView(), shaderFilePath, cubeLength);
		break;
	default:
		break;
	}

	// Get Width and Height of the texture
	ID3D11Resource* resource = nullptr;
	mShaderResourceView->GetResource(&resource);

	ID3D11Texture2D* tex = static_cast<ID3D11Texture2D*>(resource);
	D3D11_TEXTURE2D_DESC desc{};
	tex->GetDesc(&desc);
	SafeRelease(resource);

	mWidth = desc.Width;
	mHeight = desc.Height;
}

void Texture::Terminate()
{
	SafeRelease(mShaderResourceView);
}

void Texture::BindVS(uint32_t slot) const
{
	GetContext()->VSSetShaderResources(slot, 1, &mShaderResourceView);
}

void Texture::BindPS(uint32_t slot) const
{
	GetContext()->PSSetShaderResources(slot, 1, &mShaderResourceView);
}

void Texture::UnbindVS(uint32_t slot) const
{
	static ID3D11ShaderResourceView* dummy = nullptr;
	GetContext()->VSSetShaderResources(slot, 1, &dummy);
}

void Texture::UnbindPS(uint32_t slot) const
{
	static ID3D11ShaderResourceView* dummy = nullptr;
	GetContext()->PSSetShaderResources(slot, 1, &dummy);
}

#endif