#include "Precompiled.h"
#ifdef ENABLE_DIRECTX11

#include "MeshBuffer.h"

#include "Graphics/Inc/Mesh.h"
#include "D3DUtil.h"

using namespace Angazi;
using namespace Angazi::Graphics;

void MeshBuffer::InitializeInternal(const void * vertices, int vertexSize, int vertexCount, const  uint32_t* indices, int indexCount, bool dynamic)
{
	mVertexSize = vertexSize;
	mVertexCount = vertexCount;
	mIndexCount = indexCount;

	//Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = vertexCount * vertexSize;
	bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	//Initialization
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices;

	//hresult is an error code as a long
	auto device = GetDevice();
	HRESULT	hr = device->CreateBuffer(&bufferDesc, vertices ? &initData : nullptr, &mVertexBuffer);
	ASSERT(SUCCEEDED(hr), "Failed to create vertex buffer.");

	if (indices != nullptr && indexCount > 0)
	{
		//Create index Buffer
		bufferDesc.ByteWidth = indexCount * sizeof(uint32_t);
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		initData.pSysMem = indices;

		hr = device->CreateBuffer(&bufferDesc, &initData, &mIndexBuffer);
		ASSERT(SUCCEEDED(hr), "Failed to create index buffer.");
	}
	//auto context = GetContext();
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

MeshBuffer::~MeshBuffer()
{
	ASSERT(mVertexBuffer == nullptr, "[MeshBuffer] Terminate() must be called to clean up!");
	ASSERT(mIndexBuffer == nullptr, "[MeshBuffer] Terminate() must be called to clean up!");
}

void MeshBuffer::Terminate()
{
	SafeRelease(mIndexBuffer);
	SafeRelease(mVertexBuffer);
}

void MeshBuffer::SetTopology(Topology topology)
{
	if (topology == Topology::Lines)
		mTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	else if (topology == Topology::Triangles)
		mTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	else if (topology == Topology::TrianglesStrip)
		mTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
}

void MeshBuffer::Update(const void * vertexData, uint32_t numVertices)
{
	mVertexCount = numVertices;

	auto context = GetContext();

	D3D11_MAPPED_SUBRESOURCE resource;
	context->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, vertexData, numVertices*mVertexSize);
	context->Unmap(mVertexBuffer, 0);
}

void MeshBuffer::Draw() const
{
	auto context = GetContext();

	context->IASetPrimitiveTopology(mTopology);

	UINT stride = mVertexSize;
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

	if (mIndexBuffer)
	{
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed((UINT)mIndexCount, 0, 0);
	}
	else
	{
		context->Draw(mVertexCount, 0);
	}

}

#endif