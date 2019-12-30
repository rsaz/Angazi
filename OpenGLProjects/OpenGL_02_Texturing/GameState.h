#pragma once
#include <Angazi/Inc/Angazi.h>

class GameState : public Angazi::AppState
{
public:
	void Initialize() override;
	void Terminate() override;

	void Update(float deltaTime) override;
	void Render() override;

private:
	Angazi::GraphicsGL::Shader mShader;
	Angazi::GraphicsGL::TextureGL mTexture;

	Angazi::GraphicsGL::MeshBufferGL mMeshBuffer;
	Angazi::GraphicsGL::MeshPX mMeshPC;

};