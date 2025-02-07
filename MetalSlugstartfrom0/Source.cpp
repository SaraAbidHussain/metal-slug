#include<iostream>
#include<raylib.h>
using namespace std;

//function pointer declaration
typedef void (*playerAction)();
playerAction currentAction = nullptr;

//functions prototypes
void moveLeft();
void moveRight();
void Jump();
void applyGravity(float dt);


//Player struct
struct Player
{
	Rectangle player; // position and size(x, y, width, height)
	float speed;
	float verticalVelocity;
	bool isJumping;
};

Player P = { {100, 400, 50, 50}, 200.0f ,0.0f,false}; //hard coded for now, adjust after screen setup

const float gravity = 600.0f; // Gravity force
const float jumpStrength = -300.0f; // Jump force

int main()
{
	InitWindow(800, 600, "Metal Slug!");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		if (IsKeyDown(KEY_LEFT))
			currentAction = moveLeft;

		else if (IsKeyDown(KEY_RIGHT))
			currentAction = moveRight;

		else if (IsKeyPressed(KEY_SPACE))
			currentAction = Jump;
		else
			currentAction = nullptr;

		if (currentAction) currentAction();
		applyGravity(dt);


		BeginDrawing();
		ClearBackground(RAYWHITE);

		// Draw Player
		DrawRectangleRec(P.player, RED);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
void moveLeft()
{
	P.player.x -= P.speed * GetFrameTime();
	//P.player.x--;

}
void moveRight()
{
	P.player.x += P.speed * GetFrameTime();
	//P.player.x++;
}
void Jump()
{
	if (!P.isJumping)
	{
		P.isJumping = true;
		P.verticalVelocity = jumpStrength;
	}
}
void applyGravity(float dt)
{
	if (P.isJumping)
	{
		P.verticalVelocity += gravity * dt;
		P.player.y += P.verticalVelocity * dt;
	}
	// Collision with ground 
	if (P.player.y >= 400) {
		P.player.y = 400;
		P.isJumping = false;
		P.verticalVelocity = 0;
	}
}
void Shoot()
{
	//add code later
}