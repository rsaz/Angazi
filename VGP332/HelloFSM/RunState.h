#pragma once
#include "Enemy.h"
#include <Angazi/Inc/Angazi.h>

class RunState : public Angazi::AI::State<Enemy>
{
	void Enter(Enemy& agent) override;
	void Update(Enemy& agent, float deltaTime) override;
	void Exit(Enemy& agent) override;
};