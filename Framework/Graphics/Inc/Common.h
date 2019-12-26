#pragma once

// Engine headers
#include <Core/Inc/Core.h>
#include <Math/Inc/EngineMath.h>

// DirectX headers
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// DirectX libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// OpenGL headers
#include<GL/gl.h>
#include<GL/glu.h>

// OpenGL libraries
#pragma comment(lib, "opengl32.lib")  
#pragma comment(lib, "glu32.lib") 

template <class T>
void SafeRelease(T*& ptr) //*& otherwise copy of the pointer is passed
{
	if (ptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}