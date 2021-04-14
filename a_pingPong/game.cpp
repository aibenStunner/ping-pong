#include <iostream>
#include <algorithm>

#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "ball_object.h"

// Game-related State data
SpriteRenderer     *Renderer;
GameObject         *Player1;
GameObject         *Player2;
BallObject         *Ball1;
BallObject         *Ball2;
BallObject         *Ball;

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height), isPlayer1(true)
{

}

Game::~Game()
{
	delete Renderer;
	delete Player1;
	delete Player2;
	delete Ball;
}

void Game::Init()
{
	// load shaders
	ResourceManager::LoadShader("shaders/sprite/sprite.vs", "shaders/sprite/sprite.fs", nullptr, "sprite");
	// configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
		static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	// set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	// load textures
	ResourceManager::LoadTexture("textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("textures/ball.png", true, "ball");
	ResourceManager::LoadTexture("textures/paddle1.png", true, "paddle1");
	ResourceManager::LoadTexture("textures/paddle2.png", true, "paddle2");
	// configure game objects
	glm::vec2 player1Pos = glm::vec2(
		this->Width - PLAYER_SIZE.x - 5.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);
	Player1 = new GameObject(player1Pos, PLAYER_SIZE, ResourceManager::GetTexture("paddle1"));
	glm::vec2 player2Pos = glm::vec2(
		0.0f + 5.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);
	Player2 = new GameObject(player2Pos, PLAYER_SIZE, ResourceManager::GetTexture("paddle2"));

	glm::vec2 ballPos = player1Pos + glm::vec2(-BALL_RADIUS * 2.0f, PLAYER_SIZE.y / 2.0f - BALL_RADIUS);
	Ball1 = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
		ResourceManager::GetTexture("ball"));

	ballPos = player2Pos + glm::vec2(BALL_RADIUS * 2.0f, PLAYER_SIZE.y / 2.0f - BALL_RADIUS);
	Ball2 = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
		ResourceManager::GetTexture("ball"));

	// set default selected player as player 1
	Ball = Ball1;

}

void Game::Update(float dt)
{
	// update objects
	Ball->Move(dt, this->Height, this->isPlayer1);
	// check for collisions
	this->DoCollisions();
}

void Game::ProcessInput(float dt)
{
	if (this->State == GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;
		// move player1board
		if (this->Keys[GLFW_KEY_UP])
		{
			if (Player1->Position.y >= 0.0f)
			{
				Player1->Position.y -= velocity;
				if (Ball->Stuck && this->isPlayer1)
					Ball->Position.y -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_DOWN])
		{
			if (Player1->Position.y <= this->Height - PLAYER_SIZE.y)
			{
				Player1->Position.y += velocity;
				if (Ball->Stuck && this->isPlayer1)
					Ball->Position.y += velocity;
			}
		}
		// move player2board
		if (this->Keys[GLFW_KEY_W])
		{
			if (Player2->Position.y >= 0.0f)
			{
				Player2->Position.y -= velocity;
				if (Ball->Stuck && !this->isPlayer1)
					Ball->Position.y -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_S])
		{
			if (Player2->Position.y <= this->Height - PLAYER_SIZE.y)
			{
				Player2->Position.y += velocity;
				if (Ball->Stuck && !this->isPlayer1)
					Ball->Position.y += velocity;
			}
		}

		if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE])
		{
			Ball->Stuck = false;
			this->KeysProcessed[GLFW_KEY_SPACE] = true;
		}

		if (this->Keys[GLFW_KEY_RIGHT] && !this->KeysProcessed[GLFW_KEY_RIGHT]) 
		{ 
			Ball = Ball1; 
			this->isPlayer1 = true;
		}
		if (this->Keys[GLFW_KEY_LEFT] && !this->KeysProcessed[GLFW_KEY_LEFT])
		{
			Ball = Ball2;
			this->isPlayer1 = false;
		}
	}
}

void Game::Render()
{
	// draw background
	Renderer->DrawSprite(ResourceManager::GetTexture("background"),
		glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
	);

	// draw ball
	Ball->Draw(*Renderer);

	// draw player one
	Player1->Draw(*Renderer);
	
	// draw player two
	Player2->Draw(*Renderer);
}

void Game::ResetPlayer()
{

}

// collision detection
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
	// check collisions for player one paddle
	Collision result = CheckCollision(*Ball, *Player1);
	if (!Ball->Stuck && std::get<0>(result))
	{
		// check where it hit the board, and change velocity based on where it hit the board
		float centerBoard = Player1->Position.y + Player1->Size.y / 2.0f;
		float distance = (Ball->Position.y + Ball->Radius) - centerBoard;
		float percentage = distance / (Player1->Size.y / 2.0f);
		// then move accordingly
		float strength = 1.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.y = INITIAL_BALL_VELOCITY.y * percentage * strength;
		
		Ball->Velocity.x = -1.0f * Ball->Velocity.x;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
	}

	// check collisions for player two paddle
	result = CheckCollision(*Ball, *Player2);
	if (!Ball->Stuck && std::get<0>(result))
	{
		// check where it hit the board, and change velocity based on where it hit the board
		float centerBoard = Player2->Position.y + Player2->Size.y / 2.0f;
		float distance = (Ball->Position.y + Ball->Radius) - centerBoard;
		float percentage = distance / (Player2->Size.y / 2.0f);
		// then move accordingly
		float strength = 1.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.y = INITIAL_BALL_VELOCITY.y * percentage * strength;

		Ball->Velocity.x = -1.0f * Ball->Velocity.x;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
	}
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
	// get center point circle first 
	glm::vec2 center(one.Position + one.Radius);
	// calculate AABB info (center, half-extents)
	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// get difference vector between both centers
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
	glm::vec2 closest = aabb_center + clamped;
	// now retrieve vector between center circle and closest point AABB and check if length < radius
	difference = closest - center;

	if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
		return std::make_tuple(true, VectorDirection(difference), difference);
	else
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}