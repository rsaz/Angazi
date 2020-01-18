#pragma once
#include <Angazi/Inc/Angazi.h>

class GameState : public Angazi::AppState
{
public:
	void Initialize() override;
	void Terminate() override;

	void Update(float deltaTime) override;
	void Render() override;

	void DebugUI() override;

private:
	Angazi::Graphics::Camera mCamera;

	Angazi::Graphics::MeshPN mMeshSphere;
	Angazi::Graphics::MeshBuffer mMeshBufferSphere;

	Angazi::Graphics::ConstantBuffer mConstantBuffer;

	Angazi::Graphics::VertexShader mVertexShader;
	Angazi::Graphics::PixelShader mPixelShader;

	Angazi::Math::Vector3 mRotation = 0.0f;
};
