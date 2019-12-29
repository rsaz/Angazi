#include "Precompiled.h"
#include "MeshBufferGL.h"
#include "Graphics/Inc/VertexTypes.h"

using namespace Angazi;
using namespace Angazi::Graphics;

namespace
{
	struct VertexElementDesc
	{
		unsigned int type;
		unsigned int count;
		unsigned char normalized;

		static unsigned int GetSizeOfType(unsigned int type)
		{
			switch (type)
			{
			case GL_FLOAT:			return 4;
			case GL_UNSIGNED_INT:	return 4;
			case GL_UNSIGNED_BYTE:	return 1;
			}
			return(0);
		}
	};

	auto GetVertexLayout(uint32_t vertexFormat)
	{
		std::vector<VertexElementDesc> vertexLayout; // rvo - copy elision

		if (vertexFormat & VE_Position)
			vertexLayout.push_back({GL_FLOAT,3,GL_FALSE });

		//if (vertexFormat & VE_Normal)

		//if (vertexFormat & VE_Tangent)

		if (vertexFormat & VE_Color)
			vertexLayout.push_back({ GL_FLOAT,4,GL_FALSE });

		//if (vertexFormat & VE_TexCoord)


		return vertexLayout;
	}
}


void Angazi::Graphics::MeshBufferGL::Terminate()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &mVertexArray);
}

void Angazi::Graphics::MeshBufferGL::Draw()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBindVertexArray(mVertexArray);

	glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, nullptr);
}

void Angazi::Graphics::MeshBufferGL::InitializeInternal(const void * vertices, int vertexSize, int vertexCount, const uint32_t * indices, int indexCount, uint32_t vertexFormat)
{
	mVertexSize = vertexSize;
	mIndexCount = indexCount;
	
	glGenVertexArrays(1, &mVertexArray);
	glGenBuffers(1, &mVertexBuffer);
	glBindVertexArray(mVertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, vertices, GL_STATIC_DRAW);

	auto vertexLayout = GetVertexLayout(vertexFormat);
	unsigned int offset = 0;
	for (int i = 0; i < vertexLayout.size(); i++)
	{
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, vertexLayout[i].count, vertexLayout[i].type , vertexLayout[i].normalized ,
			vertexSize, (const void*) offset );
		offset += vertexLayout[i].count * VertexElementDesc::GetSizeOfType(vertexLayout[i].type);
	}

	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32_t), indices, GL_STATIC_DRAW);

}
