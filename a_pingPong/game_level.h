#ifndef GAMELEVEL_H
#define GAMELEVEL_H

#include "ball_object.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

///GameLevel based of initial velocity of ball
class GameLevel
{
public:
	// Balls for level
	BallObject         *Ball1;
	BallObject         *Ball2;

	// constructor
	GameLevel() { }
	// loads level from file
	void Load(glm::vec2 ball1Pos, glm::vec2 ball2Pos, float ball_radius, glm::vec2 initial_ball_velocity);

	// functions to return balls
	BallObject *getBall1();
	BallObject *getBall2();
};

#endif