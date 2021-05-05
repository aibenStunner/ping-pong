#ifndef GAME_H
#define GAME_H
#include <vector>
#include <tuple>

#include <glm/glm.hpp>

#include "game_level.h"

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

// Represents the four levels of difficulty
const std::vector<std::string>  LEVEL_DIFFICULTY {
	"Amateur",
	"Pro",
	"Expert",
	"World Class"
};

// Defines a Collision typedef that represents collision data
typedef std::tuple<bool, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

// Initial size of the player paddle
//const glm::vec2 PLAYER_SIZE(105.0f, 25.0f);
const glm::vec2 PLAYER_SIZE(25.0f, 105.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -100.0f);
// Radius of the ball object
const float BALL_RADIUS = 13.0f;
// win score
const int WIN_SCORE = 11;

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
	// game state
	GameState               State;
	bool					isPlayer1;
	bool					Player1Win;
	bool                    KeysProcessed[1024];
	bool                    Keys[1024];
	std::vector<GameLevel>  Levels;
	unsigned int            Level;
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
	void DoCollisions();
	// reset
	void ResetPlayer1Game();
	void ResetPlayer2Game();
	void ResetPlayer1();
	void ResetPlayer2();
	void ResetGame();
};

#endif