#include "GameState.h"
#include "ImGui/Inc/imgui.h"

using namespace Angazi;
using namespace Angazi::Graphics;
using namespace Angazi::Input;
using namespace Angazi::Math;

void GameState::Initialize()
{
	GraphicsSystem::Get()->SetClearColor(Colors::Black);

	mCamera.SetNearPlane(0.1f);
	mCamera.SetFarPlane(100.0f);
	mCamera.SetPosition({ 0.057f, 2.20f,3.0f });
	mCamera.SetDirection({ -0.01f,-0.44f, -0.894f });

	mCameraOffset = mCamera.GetPosition() - mModelPosition;

	mDirectionalLight.direction = Normalize({ 0.327f,-0.382f, 0.864f });
	mDirectionalLight.ambient = { 0.8f,0.8f,0.8f ,1.0f };
	mDirectionalLight.diffuse = { 0.75f,0.75f,0.75f ,1.0f };
	mDirectionalLight.specular = { 0.5f,0.5f,0.5f ,1.0f };

	mMaterial.ambient = { 0.8f,0.8f,0.8f ,1.0f };
	mMaterial.diffuse = { 0.8f,0.8f,0.8f ,1.0f };
	mMaterial.specular = { 0.5f,0.5f,0.5f ,1.0f };
	mMaterial.power = 80.0f;

	// Post Processing
	mPostProcessingEffect.Initialize("../../Assets/Shaders/PostProcessing.fx", "VS", "PSNoProcessing");

	// Model
	model.Initialize("../../Assets/Models/Swat/Swat.model");
	animator.Initialize(model);
	animator.SetClipLooping(0, true); // left
	animator.SetClipLooping(1, true); // right
	animator.SetClipLooping(2, true); // idle
	animator.SetClipLooping(3, true); // backwards
	animator.SetClipLooping(4 ,true); // forwards
	animator.PlayAnimation(2);

	animator.AddBlendAnimation({ -1, 0 }, 0);
	animator.AddBlendAnimation({ +1, 0 }, 1);
	//animator.AddBlendAnimation({  0, 0 }, 2);
	animator.AddBlendAnimation({  0,-1 }, 3);
	animator.AddBlendAnimation({  0, 1 }, 4);

	// Effects
	mModelStandardEffect.Initialize("../../Assets/Shaders/Standard.fx");
	mModelStandardEffect.UseShadow(true);
	mModelStandardEffect.SetNormalMapWeight(1.0f);
	mModelStandardEffect.SetSkinnedMesh(1.0f);

	mGroundStandardEffect.Initialize("../../Assets/Shaders/Standard.fx");
	mGroundStandardEffect.SetDiffuseTexture("../../Assets/Images/Floor/Stone_Tiles_004_diffuse.jpg");
	mGroundStandardEffect.SetNormalTexture("../../Assets/Images/Floor/Stone_Tiles_004_normal.jpg");
	mGroundStandardEffect.SetAOTexture("../../Assets/Images/Floor/Stone_Tiles_004_ao.jpg");
	mGroundStandardEffect.SetDisplacementTexture("../../Assets/Images/Floor/Stone_Tiles_004_height.png");
	mGroundStandardEffect.UseShadow(true);
	mGroundStandardEffect.SetBumpMapWeight(6.0f);

	mGroundMesh = MeshBuilder::CreatePlane(100.0f, 50, 50);
	mGroundMeshBuffer.Initialize(mGroundMesh);

	mShadowEffect.Initialize("../../Assets/Shaders/DepthMap.fx");

	mSkybox.CreateSkybox();
	mHdrEffect.Initialize();
}

void GameState::Terminate()
{
	mHdrEffect.Terminate();
	mSkybox.Terminate();

	// Effects
	mShadowEffect.Terminate();
	mGroundStandardEffect.Terminate();
	mModelStandardEffect.Terminate();

	// Model
	animator.Terminate();
	model.Terminate();

	// Post Processing
	mPostProcessingEffect.Terminate();
}

void GameState::Update(float deltaTime)
{
	auto inputSystem = InputSystem::Get();

	const float kMoveSpeed = inputSystem->IsKeyDown(KeyCode::LSHIFT) ? 100.0f : 10.0f;
	const float kTurnSpeed = 10.0f * Constants::DegToRad;

	if ((inputSystem->IsKeyDown(KeyCode::W) || inputSystem->IsKeyDown(KeyCode::S)))
	{
		if (inputSystem->IsKeyDown(KeyCode::W))
		{
			//animator.BlendTo(4, 0.3f);
			mInputAxis.y += deltaTime;
		}
		if (inputSystem->IsKeyDown(KeyCode::S))
		{
			//animator.BlendTo(3, 0.3f);
			mInputAxis.y -= deltaTime;
		}
	}
	else
	{
		//mInputAxis.y = 0.0f;
		if (fabs(mInputAxis.y) < 0.001f)
			mInputAxis.y = 0.0f;
		else if (mInputAxis.y > 0.0f)
			mInputAxis.y -= deltaTime;
		else if(mInputAxis.y < 0.0f)
			mInputAxis.y += deltaTime;
		
	}
	if ((inputSystem->IsKeyDown(KeyCode::A) || inputSystem->IsKeyDown(KeyCode::D)))
	{
		if (inputSystem->IsKeyDown(KeyCode::A))
		{
			//animator.BlendTo(0, 0.3f);
			mInputAxis.x -= deltaTime;
		}
		if (inputSystem->IsKeyDown(KeyCode::D))
		{
			//animator.BlendTo(1, 0.3f);
			mInputAxis.x += deltaTime;
		}
	}
	else
	{
		//mInputAxis.x = 0.0f;
		if (fabs(mInputAxis.x) < 0.001f)
			mInputAxis.x = 0.0f;
		else if (mInputAxis.x > 0.0f)
			mInputAxis.x -= deltaTime;
		else if (mInputAxis.x < 0.0f)
			mInputAxis.x += deltaTime;

		
	}

	//if(!(inputSystem->IsKeyDown(KeyCode::A) || inputSystem->IsKeyDown(KeyCode::D) 
	//	|| inputSystem->IsKeyDown(KeyCode::W) || inputSystem->IsKeyDown(KeyCode::S)))
	if (mInputAxis.x == 0.0f && mInputAxis.y == 0.0f)
		animator.BlendTo(2, 0.5f);
	else
		animator.SetBlendVelocity(mInputAxis);
	//animator.SetBlendVelocity({0.0,-1.0f});

	if (inputSystem->IsMouseDown(MouseButton::RBUTTON))
	{
		mCamera.Yaw(inputSystem->GetMouseMoveX() *kTurnSpeed*deltaTime);
		mCamera.Pitch(inputSystem->GetMouseMoveY() *kTurnSpeed*deltaTime);
	}

	mInputAxis.x = Clamp(mInputAxis.x, -1.0f, 1.0f);
	mInputAxis.y = Clamp(mInputAxis.y, -1.0f, 1.0f);

	Vector2 movement = mInputAxis * deltaTime *mMovementSpeed;
	mModelPosition.x -= movement.x;
	mModelPosition.z -= movement.y;

	mCamera.SetPosition(mModelPosition + mCameraOffset);
	mShadowEffect.SetLightDirection(mDirectionalLight.direction, mCamera);
	//if (mShowSkeleton)
	//	animator.PlaySkeletalAnimation(4);
	//else
	//	animator.PlayAnimation(2);

	//if (mInputAxis.x > 0.0f && mInputAxis.y > 0.0f)
	//else if (mInputAxis.x > 0.0f && mInputAxis.x > 0.0f)
	//else if (mInputAxis.x > 0.0f && mInputAxis.x > 0.0f)
	//else if (mInputAxis.x > 0.0f && mInputAxis.x > 0.0f)

	//if (inputSystem->IsKeyPressed(KeyCode::SPACE))
	//	animator.BlendTo(4,2.0f);
	//if(inputSystem->IsKeyPressed(KeyCode::B))
	//	animator.BlendTo(2, 2.0f);

	animator.Update(deltaTime);
}

void GameState::Render()
{
	mShadowEffect.Begin();
	DrawDepthMap();
	mShadowEffect.End();

	//mPostProcessingEffect.BeginRender();
	mHdrEffect.BeginRender();
	DrawScene();
	mHdrEffect.EndRender();
	//mPostProcessingEffect.EndRender();

	//mPostProcessingEffect.PostProcess();
	mHdrEffect.RenderHdrQuad();
}

void GameState::DebugUI()
{
	ImGui::Begin("Setting", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("FPS: %.2f", Angazi::Core::TimeUtil::GetFramesPerSecond());
	/*ImGui::Image(
		mShadowEffect.GetRenderTarget()->GetShaderResourceView(),
		{ 150.0f,150.0f },
		{ 0.0f,0.0f },
		{ 1.0f,1.0f },
		{ 1.0f,1.0f ,1.0f,1.0f },
		{ 1.0f,1.0f ,1.0f,1.0f }
	);*/
	if (ImGui::CollapsingHeader("Light"))
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
	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::ColorEdit4("Ambient##Material", &mMaterial.ambient.x);
		ImGui::ColorEdit4("Diffuse##Material", &mMaterial.diffuse.x);
		ImGui::ColorEdit4("Specular##Material", &mMaterial.specular.x);
		ImGui::DragFloat("Power##Material", &mMaterial.power, 1.0f, 1.0f, 100.0f);
	}
	if (ImGui::CollapsingHeader("Settings"))
	{
		static bool normal = true;
		static bool specular = true;
		static bool aoMap = true;
		//static bool useShadow = mSettings.useShadow == 1;
		if (ImGui::Checkbox("Normal Map", &normal))
		{
			float normalMapWeight = normal ? 1.0f : 0.0f;
		}
		if (ImGui::Checkbox("Specular Map", &specular))
		{
			float specularMapWeight = specular ? 1.0f : 0.0f;
		}
		if (ImGui::Checkbox("Ambient occlusion", &aoMap))
		{
			float aoMapWeight = aoMap ? 1.0f : 0.0f;
		}
	}
	if (ImGui::SliderFloat("Animation Speed", &animationSpeed, 0.0f, 3.0f))
	{
		animator.SetAnimationSpeed(animationSpeed);
	}
	ImGui::SliderFloat("Movement Speed", &mMovementSpeed, 0.0f, 3.0f);
	ImGui::Checkbox("Show Skeleton", &mShowSkeleton);
	ImGui::DragFloat("x", &mInputAxis.x, 0.1f, -1.0f, 1.0f);
	ImGui::DragFloat("y", &mInputAxis.y, 0.1f, -1.0f, 1.0f);


	ImGui::End();
}

void GameState::DrawScene()
{
	auto lightVP = mShadowEffect.GetVPMatrix();
	RenderTarget* target = mShadowEffect.GetRenderTarget();

	auto matView = mCamera.GetViewMatrix();
	auto matProj = mCamera.GetPerspectiveMatrix();

	// Model
	auto matWorld = Matrix4::Translation(mModelPosition);
	mModelStandardEffect.Begin();
	mModelStandardEffect.SetMaterial(mMaterial);
	mModelStandardEffect.SetDirectionalLight(mDirectionalLight);
	mModelStandardEffect.SetTransformData(matWorld, matView, matProj, mCamera.GetPosition());
	mModelStandardEffect.SetDepthTexture(target);
	mModelStandardEffect.UpdateSettings();
	mModelStandardEffect.SetBoneTransforms(animator.GetBoneMatrices());

	if (mShowSkeleton)
		DrawSkeleton(model.skeleton, animator.GetBoneMatrices());
	else
		model.Draw(&mModelStandardEffect);
	mModelStandardEffect.End();
	SimpleDraw::Render(mCamera, matWorld);

	// Ground
	matWorld = Matrix4::Translation({ 0.0f,-2.5,0.0f });;
	mGroundStandardEffect.Begin();
	mGroundStandardEffect.SetMaterial(mMaterial);
	mGroundStandardEffect.SetDirectionalLight(mDirectionalLight);
	mGroundStandardEffect.SetTransformData(matWorld, matView, matProj, mCamera.GetPosition());
	mGroundStandardEffect.SetDepthTexture(target);
	auto wvpLight = Transpose(matWorld * lightVP);
	mGroundStandardEffect.UpdateShadowBuffer(wvpLight);
	mGroundStandardEffect.UpdateSettings();

	mGroundMeshBuffer.Draw();
	mGroundStandardEffect.End();

	SimpleDraw::AddGroundPlane(100);
	SimpleDraw::Render(mCamera);

	mSkybox.Draw(mCamera);
}

void GameState::DrawDepthMap()
{
	auto matWorld = Matrix4::Translation(mModelPosition);
	mShadowEffect.SetWorldMatrix(matWorld);

	// Model
	mShadowEffect.SetWorldMatrix(matWorld);
	mShadowEffect.SetBoneTransforms(animator.GetBoneMatrices());
	mShadowEffect.SetSkinnedMesh(true);
	mShadowEffect.UpdateSettings();
	if (!mShowSkeleton)
		model.Draw(&mShadowEffect);
}