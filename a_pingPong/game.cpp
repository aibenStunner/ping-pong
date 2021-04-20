#include <algorithm>
#include <iostream>
#include <sstream>


#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "text_renderer.h"

// Game-related State data
SpriteRenderer     *Renderer;
GameObject         *Player1;
GameObject         *Player2;
BallObject         *Ball;
ParticleGenerator  *Particles;
TextRenderer       *Text;
TextRenderer       *Text_;

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_MENU), Keys(), Width(width), Height(height), isPlayer1(true)
{

}

Game::~Game()
{
	delete Renderer;
	delete Player1;
	delete Player2;
	delete Ball;
	delete Particles;
	delete Text;
	delete Text_;
}

void Game::Init()
{
	// load shaders
	ResourceManager::LoadShader("shaders/sprite/sprite.vs", "shaders/sprite/sprite.fs", nullptr, "sprite");
	ResourceManager::LoadShader("shaders/particle/particle.vs", "shaders/particle/particle.fs", nullptr, "particle");
	// configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
		static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
	// load textures
	ResourceManager::LoadTexture("textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("textures/ball.png", true, "ball");
	ResourceManager::LoadTexture("textures/paddle1.png", true, "paddle1");
	ResourceManager::LoadTexture("textures/paddle2.png", true, "paddle2");
	ResourceManager::LoadTexture("textures/particle.png", true, "particle");
	// set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	Particles = new ParticleGenerator(
		ResourceManager::GetShader("particle"),
		ResourceManager::GetTexture("particle"),
		500
	);
	Text = new TextRenderer(this->Width, this->Height);
	Text_ = new TextRenderer(this->Width, this->Height);
	Text->Load("fonts/OCRAEXT.TTF", 20);
	Text_->Load("fonts/ALLSTAR.TTF", 85);
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

	glm::vec2 ball1Pos = player1Pos + glm::vec2(-BALL_RADIUS * 2.0f, PLAYER_SIZE.y / 2.0f - BALL_RADIUS);
	glm::vec2 ball2Pos = player2Pos + glm::vec2(BALL_RADIUS * 2.0f, PLAYER_SIZE.y / 2.0f - BALL_RADIUS);
	// load levels
	GameLevel one; one.Load(ball1Pos, ball2Pos, BALL_RADIUS, 4.0f * INITIAL_BALL_VELOCITY);
	GameLevel two; two.Load(ball1Pos, ball2Pos, BALL_RADIUS, 6.5f * INITIAL_BALL_VELOCITY);
	GameLevel three; three.Load(ball1Pos, ball2Pos, BALL_RADIUS, 9.0f * INITIAL_BALL_VELOCITY);
	GameLevel four; four.Load(ball1Pos, ball2Pos, BALL_RADIUS, 12.0f * INITIAL_BALL_VELOCITY);

	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;

	// set default selected player as player 1
	Ball = this->Levels[this->Level].getBall1();

}

void Game::Update(float dt)
{
	// update objects
	Ball->Move(dt, this->Height, this->isPlayer1);
	// check for collisions
	this->DoCollisions();
	// update particles
	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
	// check loss condition - player1
	if (Ball->Position.x >= this->Width) // did ball reach right edge?
	{
		Player2->Score++;
		this->ResetPlayer1Game();
		this->ResetPlayer2();
	}
	// check loss condition - player2
	if (Ball->Position.x <= 0.0f) // did ball reach right edge?
	{
		Player1->Score++;
		this->ResetPlayer2Game();
		this->ResetPlayer1();
	}

	// check win condition
	if (this->State == GAME_ACTIVE)
	{
		if (Player1->Score == 11)
		{
			Player1Win = true;
			this->State = GAME_WIN;
		}
			
		if (Player2->Score == 11)
		{
			Player1Win = false;
			this->State = GAME_WIN;
		}
	}
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
		// serve
		if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE])
		{
			Ball->Stuck = false;
			this->KeysProcessed[GLFW_KEY_SPACE] = true;
		}
	}

	if (this->State == GAME_MENU)
	{
		// start game
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
		}
		// select player to serve
		if (this->Keys[GLFW_KEY_RIGHT] && !this->KeysProcessed[GLFW_KEY_RIGHT])
		{
			Ball = this->Levels[this->Level].getBall1();
			this->isPlayer1 = true;
		}
		if (this->Keys[GLFW_KEY_LEFT] && !this->KeysProcessed[GLFW_KEY_LEFT])
		{
			Ball = this->Levels[this->Level].getBall2();
			this->isPlayer1 = false;
		}
		// select level difficulty
		if (this->Keys[GLFW_KEY_D] && !this->KeysProcessed[GLFW_KEY_D])
		{
			this->Level = (this->Level + 1) % 4;
			this->KeysProcessed[GLFW_KEY_D] = true;

			// set default selected player as player 1
			Ball = this->Levels[this->Level].getBall1();
			this->isPlayer1 = true;
		}
		if (this->Keys[GLFW_KEY_A] && !this->KeysProcessed[GLFW_KEY_A])
		{
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = 3;
			this->KeysProcessed[GLFW_KEY_A] = true;

			// set default selected player as player 1
			Ball = this->Levels[this->Level].getBall1();
		}
	}
	
	if (this->State == GAME_WIN)
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
			this->State = GAME_MENU;
			this->ResetGame();
		}
	}
}

void Game::Render()
{
	if (this->State == GAME_ACTIVE || this->State == GAME_MENU)
	{
		// draw background
		Renderer->DrawSprite(ResourceManager::GetTexture("background"),
			glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
		);

		// draw particles	
		Particles->Draw();

		// draw ball
		Ball->Draw(*Renderer);

		// draw player one
		Player1->Draw(*Renderer);

		// draw player two
		Player2->Draw(*Renderer);

		// render text
		std::stringstream ss; ss << LEVEL_DIFFICULTY[Level];
		Text->RenderText("Difficulty: " + ss.str(), 45.0f, 20.0f, 0.93f);
	}

	if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
	{
		std::stringstream ss2; ss2 << Player2->Score;
		(Player2->Score > 9) ? Text_->RenderText(ss2.str(), 345.0f, 30.0f, 0.85) : Text_->RenderText(ss2.str(), 380.0f, 30.0f, 0.85);

		std::stringstream ss1; ss1 << Player1->Score;
		Text_->RenderText(ss1.str(), 480.0f, 30.0f, 0.85);
	}

	if (this->State == GAME_MENU)
	{
		Text_->RenderText("PING PONG", 250.0f, Height / 2 - 20.0f, 1.0f);

		Text->RenderText("Press ENTER to start or ESC to quit", 467.0f, Height / 2 + 80.0f, 0.80f);
		Text->RenderText("Press A or D to select Difficulty", 120.0f, Height / 2 + 80.0f, 0.80f);

		Text->RenderText("Press LEFT ARROW to make Player 2 serve", 50.0f, 500.0f, 0.80f);
		Text->RenderText("Use W or S to move", 50.0f, 520.0f, 0.80f);

		Text->RenderText("Press RIGHT ARROW to make Player 1 serve", 467.0f, 500.0f, 0.80f);
		Text->RenderText("Use UP ARROW or DOWN ARROW to move", 467.0f, 520.0f, 0.80f);
	}

	if (this->State == GAME_WIN)
	{
		(Player1Win) ? Text->RenderText(
			"Player 1 WON!!!", 280.0, Height / 2 - 50.0f, 2.0f, glm::vec3(0.0, 1.0, 0.0)
		) : Text->RenderText(
			"Player 2 WON!!!", 280.0, Height / 2 - 50.0f, 2.0f, glm::vec3(0.0, 1.0, 0.0)
		);
		Text->RenderText(
			"Press ENTER to replay or ESC to quit", 245.0, Height / 2 + 20, 1.0f, glm::vec3(1.0, 1.0, 0.0)
		);
	}
}

void Game::ResetPlayer1Game()
{
	Player1->Position = glm::vec2(
		this->Width - PLAYER_SIZE.x - 5.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);
	Ball->Reset(Player1->Position + glm::vec2(-BALL_RADIUS * 2.0f, PLAYER_SIZE.y / 2.0f - BALL_RADIUS), this->Levels[this->Level].getBall1()->Velocity);
	this->isPlayer1 = true;
}

void Game::ResetPlayer2Game()
{
	Player2->Position = glm::vec2(
		0.0f + 5.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);
	Ball->Reset(Player2->Position + glm::vec2(BALL_RADIUS * 2.0f, PLAYER_SIZE.y / 2.0f - BALL_RADIUS), this->Levels[this->Level].getBall2()->Velocity);
	this->isPlayer1 = false;
}

void Game::ResetPlayer1()
{
	Player1->Position = glm::vec2(
		this->Width - PLAYER_SIZE.x - 5.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);
}

void Game::ResetPlayer2()
{
	Player2->Position = glm::vec2(
		0.0f + 5.0f,
		this->Height / 2.0f - PLAYER_SIZE.y / 2.0f
	);

}

void Game::ResetGame()
{
	this->ResetPlayer1Game();
	this->ResetPlayer2();
	Player1->Score = 0;
	Player2->Score = 0;
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
		float strength = 2.0f;
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
		float strength = 2.0f;
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