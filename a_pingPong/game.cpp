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
		this->Width - 67.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);
	Player1 = new GameObject(player1Pos, PLAYER_SIZE, ResourceManager::GetTexture("paddle1"), 90.0f);
	glm::vec2 player2Pos = glm::vec2(
		-35.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);
	Player2 = new GameObject(player2Pos, PLAYER_SIZE, ResourceManager::GetTexture("paddle2"), 90.0f);

	glm::vec2 ballPos = player1Pos + glm::vec2(BALL_RADIUS, 0.0f);
	Ball1 = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
		ResourceManager::GetTexture("ball"));

	ballPos = player2Pos + glm::vec2(5 * BALL_RADIUS, 0.0f);
	Ball2 = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
		ResourceManager::GetTexture("ball"));

	// set default selected player as player 1
	Ball = Ball1;

}

void Game::Update(float dt)
{
	Ball->Move(dt, this->Height, this->isPlayer1);
}

void Game::ProcessInput(float dt)
{
	if (this->State == GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;
		// move player1board
		if (this->Keys[GLFW_KEY_UP])
		{
			if (Player1->Position.y >= 43.0f)
			{
				Player1->Position.y -= velocity;
				if (Ball->Stuck && this->isPlayer1)
					Ball->Position.y -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_DOWN])
		{
			if (Player1->Position.y <= this->Height - 14.0f - PLAYER_SIZE.x / 2.0f)
			{
				Player1->Position.y += velocity;
				if (Ball->Stuck && this->isPlayer1)
					Ball->Position.y += velocity;
			}
		}
		// move player2board
		if (this->Keys[GLFW_KEY_W])
		{
			if (Player2->Position.y >= 43.0f)
			{
				Player2->Position.y -= velocity;
				if (Ball->Stuck && !this->isPlayer1)
					Ball->Position.y -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_S])
		{
			if (Player2->Position.y <= this->Height - 14.0f - PLAYER_SIZE.x / 2.0f)
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