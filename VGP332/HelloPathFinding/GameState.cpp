#include "TileMap.h"
#include "GameState.h"

using namespace Angazi;
using namespace Angazi::Graphics;
using namespace Angazi::Input;
using namespace Angazi::Math;

void GameState::Initialize()
{
	GraphicsSystem::Get()->SetClearColor(Colors::LightGray);

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
}