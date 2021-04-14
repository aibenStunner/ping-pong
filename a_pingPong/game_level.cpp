#include "game_level.h"
#include "resource_manager.h"

void GameLevel::Load(glm::vec2 ball1Pos, glm::vec2 ball2Pos, float ball_radius, glm::vec2 initial_ball_velocity)
{
	Ball1 = new BallObject(ball1Pos, ball_radius, initial_ball_velocity, ResourceManager::GetTexture("ball"));
	Ball2 = new BallObject(ball2Pos, ball_radius, initial_ball_velocity, ResourceManager::GetTexture("ball"));
}

BallObject *GameLevel::getBall1()
{
	return Ball1;
}

BallObject *GameLevel::getBall2()
{
	return Ball2;
}
