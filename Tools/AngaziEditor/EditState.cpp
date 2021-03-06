#include "EditState.h"
#include "ImGui/Inc/imgui.h"

using namespace Angazi;
using namespace Angazi::Graphics;
using namespace Angazi::Physics;
using namespace Angazi::Input;
using namespace Angazi::Math;


void EditState::Initialize()
{
	GraphicsSystem::Get()->SetClearColor(Colors::Black);

	//mWorld.AddService<TerrainService>();

	mCameraService = mWorld.AddService<CameraService>();
	mCameraService->AddCamera("Camera 1");
	mCameraService->AddCamera("Camera 2");

	mEnvironmentService = mWorld.AddService<EnvironmentService>();
	mShaderService = mWorld.AddService<ShaderService>();
	mLightService = mWorld.AddService<LightService>();
	mShaderService->AddShader<PbrEffect>();
	mShaderService->AddShader<StandardEffect>();
	mShaderService->AddShader<HdrEffect>();
	mShaderService->AddShader<ShadowEffect>();

	mEnvironmentService->AddEnvironment("Helipad HDR");
	mEnvironmentService->AddEnvironment("Shiodome HDR");
	mWorld.Initialize(100);

	auto skybox = mEnvironmentService->FindEnvironment("Helipad HDR");
	skybox->CreateSkybox("../../Assets/Images/HdrMaps/Helipad_GoldenHour/LA_Downtown_Helipad_GoldenHour_3k.hdr");
	skybox = mEnvironmentService->FindEnvironment("Shiodome HDR");
	skybox->CreateSkybox("../../Assets/Images/HdrMaps/Shiodome_Stairs/10-Shiodome_Stairs_3k.hdr");

	auto& camera = mCameraService->GetActiveCamera();
	camera.SetNearPlane(0.1f);
	camera.SetFarPlane(100.0f);
	camera.SetPosition({ 0.0f, 2.0f, -5.0f });
	camera.SetDirection({ 0.0f,0.0f, 1.0f });

	auto& light = mLightService->GetActiveLight();
	light.direction = Math::Normalize({ 1.0f, -0.7f,1.0f });
	//light.ambient = { 0.892f, 0.835f, 0.835f, 0.000f };
	light.ambient = { 0.292f, 0.235f, 0.235f, 0.000f };
	light.diffuse = { 0.7f };
	light.specular = { 0.0f };

	mWorld.LoadScene("../../Assets/Scenes/Scene1.json");

	mRenderTarget.Initialize(GraphicsSystem::Get()->GetBackBufferWidth(),
		GraphicsSystem::Get()->GetBackBufferHeight(), RenderTarget::Format::RGBA_U8);
}

void EditState::Terminate()
{
	mRenderTarget.Terminate();
	mWorld.Terminate();
}

void EditState::Update(float deltaTime)
{
	auto inputSystem = InputSystem::Get();

	if (inputSystem->IsMouseDown(MouseButton::RBUTTON) && mIsSceneHovered)
		mIsUsingCameraControl = true;
	if (!inputSystem->IsMouseDown(MouseButton::RBUTTON))
		mIsUsingCameraControl = false;

	if (mIsUsingCameraControl)
	{
		const float kMoveSpeed = inputSystem->IsKeyDown(KeyCode::LSHIFT) ? 100.0f : 10.0f;
		const float kTurnSpeed = 10.0f * Constants::DegToRad;

		auto& camera = mCameraService->GetActiveCamera();
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		if (inputSystem->IsKeyDown(KeyCode::W))
			camera.Walk(kMoveSpeed * deltaTime);
		if (inputSystem->IsKeyDown(KeyCode::S))
			camera.Walk(-kMoveSpeed * deltaTime);
		if (inputSystem->IsKeyDown(KeyCode::A))
			camera.Strafe(-kMoveSpeed * deltaTime);
		if (inputSystem->IsKeyDown(KeyCode::D))
			camera.Strafe(kMoveSpeed * deltaTime);
		camera.Yaw(inputSystem->GetMouseMoveX() * kTurnSpeed * deltaTime);
		camera.Pitch(inputSystem->GetMouseMoveY() * kTurnSpeed * deltaTime);
	}
	else
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

	mWorld.Update(deltaTime);
}

void EditState::Render()
{
	mWorld.RenderShadowMap();

	auto hdrEffect = mShaderService->GetShader<HdrEffect>();
	if (hdrEffect)
	{
		hdrEffect->BeginRender();
		RenderScene();
		hdrEffect->EndRender();

		mRenderTarget.BeginRender();
		hdrEffect->RenderHdrQuad();
		mRenderTarget.EndRender();
	}
	else
	{
		mRenderTarget.BeginRender();
		RenderScene();
		mRenderTarget.EndRender();
	}

}

void EditState::DebugUI()
{
	mWorld.DebugUI();
	mEditor.Show();
	ShowSceneView();

	ImGui::Begin("Shadow");
	const auto& shadowShader = mWorld.GetService<ShaderService>()->GetShader<ShadowEffect>();
	ImGui::Image(
		shadowShader->GetRenderTarget()->GetShaderResourceView(),
		{ 150.0f,150.0f },
		{ 0.0f,0.0f },
		{ 1.0f,1.0f },
		{ 1.0f,1.0f ,1.0f,1.0f },
		{ 1.0f,1.0f ,1.0f,1.0f }
	);
	ImGui::End();
}

void EditState::RenderScene()
{
	mWorld.Render();

	//SimpleDraw::AddGroundPlane(30);
	auto& camera = mCameraService->GetActiveCamera();

	SimpleDraw::Render(camera);
}

void EditState::ShowSceneView()
{
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.0f,0.0f,0.0f,1.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f,0.0f,0.0f,1.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Scene");

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	float width = vMax.x - vMin.x;
	float height = vMax.y - vMin.y;
	mCameraService->GetActiveCamera().SetAspectRatio(width / height);

	ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(135, 206, 239, 255));
	ImGui::Image(mRenderTarget.GetShaderResourceView(), { width, height }, { 0.0f,0.0f }, { 1.0f,1.0f });
	mIsSceneHovered = ImGui::IsWindowHovered();
	ImGui::End();
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
}
