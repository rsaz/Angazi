#include "GameState.h".
#include "ImGui/Inc/imgui.h"

using namespace Angazi;
using namespace Angazi::Graphics;
using namespace Angazi::Input;
using namespace Angazi::Math;


void GameState::Initialize()
{
	GraphicsSystem::Get()->SetClearColor(Colors::DarkGray);
	tileMap.Load();
}

void GameState::Terminate()
{
	tileMap.Unload();
}

void GameState::Update(float deltaTime)
{
	tileMap.Update(deltaTime);
}

void GameState::Render()
{
	tileMap.Render();
	SimpleDraw::Render(mCamera);
}

void GameState::DebugUI()
{
	ImGui::ShowDemoWindow();
}