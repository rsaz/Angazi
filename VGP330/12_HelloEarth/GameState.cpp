#include "GameState.h"
#include "ImGui/Inc/imgui.h"

using namespace Angazi;
using namespace Angazi::Graphics;
using namespace Angazi::Input;
using namespace Angazi::Math;


void GameState::Initialize()
{
	GraphicsSystem::Get()->SetClearColor(Colors::Black);

	mCamera.SetPosition({ 0.0f, 0.0f,-3.0f });
	mCamera.SetDirection({ 0.0f,0.0f,1.0f });

	mMesh = MeshBuilder::CreateSphere(1.0f,256,256);
	mMeshBuffer.Initialize(mMesh);

	mTransformBuffer.Initialize();
	mLightBuffer.Initialize();
	mMaterialBuffer.Initialize();
	mSettingsBuffer.Initialize();

	mDirectionalLight.direction = Normalize({ 1.0f, -1.0f,1.0f });
	mDirectionalLight.ambient = { 0.3f,0.3f,0.3f ,0.3f };
	mDirectionalLight.diffuse = { 0.7f,0.7f,0.7f ,0.7f };
	mDirectionalLight.specular = { 0.5f,0.5f,0.5f ,0.5f };

	mMaterial.ambient = { 0.3f,0.3f,0.3f ,0.3f };
	mMaterial.diffuse = { 0.7f,0.7f,0.7f ,0.7f };
	mMaterial.specular = { 0.5f,0.5f,0.5f ,0.5f };
	mMaterial.power = 80.0f;

	mSettings.normalMapWeight = 1.0f;
	mSettings.specularMapWeight = 1.0f;

	mPhongShadingVertexShader.Initialize("../../Assets/Shaders/Standard.fx", Vertex::Format);
	mPhongShadingPixelShader.Initialize("../../Assets/Shaders/Standard.fx");

	mCloudShadingVertexShader.Initialize("../../Assets/Shaders/Standard2.fx", Vertex::Format);
	mCloudShadingPixelShader.Initialize("../../Assets/Shaders/Standard2.fx");

	mSampler.Initialize(Sampler::Filter::Anisotropic, Sampler::AddressMode::Clamp);
	//mTexture.Initialize("../../Assets/Images/earth.jpg");
	mTexture.Initialize("../../Assets/Images/8k_earth.jpg");
	mSpecularTexture.Initialize("../../Assets/Images/earth_spec.jpg");
	mDisplacementTexture.Initialize("../../Assets/Images/earth_bump.jpg");
	mNormalMap.Initialize("../../Assets/Images/earth_normal.jpg");
	mNightMap.Initialize("../../Assets/Images/earth_lights.jpg");
	//mNightMap.Initialize("../../Assets/Images/8k_earth_nightmap.jpg");
	mClouds.Initialize("../../Assets/Images/earth_clouds.jpg");

	/*ID3D11BlendState* d3dBlendState;
	D3D11_BLEND_DESC omDesc;
	ZeroMemory(&omDesc,

		sizeof(D3D11_BLEND_DESC));
	omDesc.RenderTarget[0].BlendEnable =

		true;
	omDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	omDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	omDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	omDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	omDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	omDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	omDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	GraphicsSystem::Get()->GetDevice()->CreateBlendState(&omDesc, &d3dBlendState);
	GraphicsSystem::Get()->GetContext()->OMSetBlendState(d3dBlendState, 0, 0xffffffff);*/
}

void GameState::Terminate()
{
	mClouds.Terminate();
	mNightMap.Terminate();
	mNormalMap.Terminate();
	mDisplacementTexture.Terminate();
	mTexture.Terminate();
	mSpecularTexture.Terminate();
	mSampler.Terminate();
	mCloudShadingPixelShader.Terminate();
	mCloudShadingVertexShader.Terminate();
	mPhongShadingPixelShader.Terminate();
	mPhongShadingVertexShader.Terminate();
	mSettingsBuffer.Terminate();
	mMaterialBuffer.Terminate();
	mLightBuffer.Terminate();
	mTransformBuffer.Terminate();
	mMeshBuffer.Terminate();
}

void GameState::Update(float deltaTime)
{
	const float kMoveSpeed = 5.0f;
	const float kTurnSpeed = 1.0f;

	auto inputSystem = InputSystem::Get();
	if (inputSystem->IsKeyDown(KeyCode::W))
		mCamera.Walk(kMoveSpeed*deltaTime);
	if (inputSystem->IsKeyDown(KeyCode::S))
		mCamera.Walk(-kMoveSpeed * deltaTime);
	if (inputSystem->IsMouseDown(MouseButton::RBUTTON))
	{
		mCamera.Yaw(inputSystem->GetMouseMoveX() *kTurnSpeed*deltaTime);
		mCamera.Pitch(inputSystem->GetMouseMoveY() *kTurnSpeed*deltaTime);
	}

	if (inputSystem->IsKeyDown(KeyCode::A))
		mCamera.Strafe(-kMoveSpeed * deltaTime);
	//mRotation += deltaTime;
	if (inputSystem->IsKeyDown(KeyCode::D))
		mCamera.Strafe(kMoveSpeed*deltaTime);
	//mRotation -= deltaTime;

}

void GameState::Render()
{
	auto context = GraphicsSystem::Get()->GetContext();

	auto matTrans = Matrix4::Translation({-1.0f,0.0f,0.0f});
	auto matRot = Matrix4::RotationX(mRotation.x) * Matrix4::RotationY(mRotation.y) * Matrix4::RotationZ(mRotation.z);
	auto matWorld = matRot * matTrans;
	auto matView = mCamera.GetViewMatrix();
	auto matProj = mCamera.GetPerspectiveMatrix();

	mSampler.BindVS();
	mSampler.BindPS();

	mTexture.BindPS(0);
	mSpecularTexture.BindPS(1);
	mDisplacementTexture.BindVS(2);
	mNormalMap.BindPS(3);
	mNightMap.BindPS(4);

	TransformData transformData;
	mTransformBuffer.BindVS(0);

	mLightBuffer.Update(&mDirectionalLight);
	mLightBuffer.BindVS(1);
	mLightBuffer.BindPS(1);

	mMaterialBuffer.Update(&mMaterial);
	mMaterialBuffer.BindVS(2);
	mMaterialBuffer.BindPS(2);

	mSettingsBuffer.Update(&mSettings);
	mSettingsBuffer.BindVS(3);
	mSettingsBuffer.BindPS(3);
	
	transformData.world = Transpose(matWorld);
	transformData.wvp = Transpose(matWorld * matView *matProj);
	transformData.viewPosition = mCamera.GetPosition();
	mTransformBuffer.Update(&transformData);

	mPhongShadingVertexShader.Bind();
	mPhongShadingPixelShader.Bind();

	mMeshBuffer.Draw();

	matWorld = Matrix4::Scaling(1.2f) *  matRot * matTrans;

	mCloudShadingVertexShader.Bind();
	mCloudShadingPixelShader.Bind();

	mSettings.normalMapWeight = 0.0f;
	mSettingsBuffer.Update(&mSettings);
	mSettingsBuffer.BindVS(3);
	mSettingsBuffer.BindPS(3);

	transformData.world = Transpose(matWorld);
	transformData.wvp = Transpose(matWorld * matView *matProj);
	transformData.viewPosition = mCamera.GetPosition();
	mTransformBuffer.Update(&transformData);

	mClouds.BindPS(0);
	mMeshBuffer.Draw();

}

void GameState::DebugUI()
{
	ImGui::Begin("Setting",nullptr,ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::CollapsingHeader("Light",ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool directionChanged = false;
		directionChanged |= ImGui::DragFloat("Direction X##Light", &mDirectionalLight.direction.x, 0.01f, -2.0f, 2.0f);
		directionChanged |= ImGui::DragFloat("Direction Y##Light", &mDirectionalLight.direction.y, 0.01f, -2.0f, 2.0f);
		directionChanged |= ImGui::DragFloat("Direction Z##Light", &mDirectionalLight.direction.z, 0.01f, -2.0f, 2.0f);
		if (directionChanged)
		{
			mDirectionalLight.direction = Normalize(mDirectionalLight.direction);
		}
		ImGui::ColorEdit4("Ambient##Light", &mDirectionalLight.ambient.x);
		ImGui::ColorEdit4("Diffuse##Light", &mDirectionalLight.diffuse.x);
		ImGui::ColorEdit4("Specular##Light", &mDirectionalLight.specular.x);
	}
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::ColorEdit4("Ambient##Material", &mMaterial.ambient.x);
		ImGui::ColorEdit4("Diffuse##Material", &mMaterial.diffuse.x);
		ImGui::ColorEdit4("Specular##Material", &mMaterial.specular.x);
		ImGui::DragFloat("Power##Material", &mMaterial.power, 1.0f, 1.0f, 100.0f);
	}
	if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static bool normal = true;
		static bool specular = true;
		ImGui::SliderFloat("Displacement", &mSettings.bumpMapWeight, 0.0f, 1.0f);
		if(ImGui::Checkbox("Normal Map", &normal))
		{
			mSettings.normalMapWeight = normal ? 1.0f : 0.0f;
		}
		if (ImGui::Checkbox("Specular Map", &specular))
		{
			mSettings.specularMapWeight = specular? 1.0f : 0.0f;
		}

	}
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Rotation##Transform", &mRotation.x, 0.01f);
	}
	ImGui::End();
}