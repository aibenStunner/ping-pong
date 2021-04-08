#ifndef GAME_H
#define GAME_H
#include <vector>
#include <tuple>

#include <glm/glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Represents the current state of the game
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

// Represents the four possible (collision) directions
enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(105.0f, 25.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 13.0f;

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
	// game state
	GameState               State;
	bool					isPlayer1;
	bool                    KeysProcessed[1024];
	bool                    Keys[1024];
	unsigned int            Width, Height;
	// constructor/destructor
	Game(unsigned int width, unsigned int height);
	~Game();
	// initialize game state (load all shaders/textures/levels)
	void Init();
	// game loop
	void ProcessInput(float dt);
	void Update(float dt);
	void Render();
	
	// reset
	void ResetPlayer();
};

#endif