#include <iostream>
#include <raylib.h>
using namespace std;

// Function pointer for movement
typedef void (*playerAction)();
playerAction currentAction = nullptr;


void moveLeft();
void moveRight();
void Jump();
void applyGravity(float dt);
void DrawScrollingBackground(Texture2D bg, float scrollX);
void UpdateAndDrawPlayer();



struct bullet{
    float x, y, rad;
    bool isFired;
}; 


struct Player {
    Rectangle player;
    Rectangle hitbox;
    float speed;
    float verticalVelocity;
    bool isJumping;
    bool isFacingRight; // for direction
    Texture2D leftTexture;
    Texture2D rightTexture;
};


const int screenWidth = 800;
const int screenHeight = 600;
const float gravity = 800.0f;
const float jumpStrength = -500.0f;

// player dimensions
const float playerWidth = 50.0f;  
const float playerHeight = 50.0f; 
const float startX = 300.0f;      // X position
const float groundY = 490.0f;  // Y position
const float startY = groundY;  // for jumping to land on ground

// Player initialization
Player P = { 
    {startX, startY, playerWidth, playerHeight}, 
    {startX, startY, playerWidth, playerHeight},
    200.0f, 
    0.0f, 
    false, 
    true, 
    {0}, 
    {0}  
};

float scrollX = 0.0f;  
Texture2D bg;          



int main() {
    InitWindow(screenWidth, screenHeight, "Metal Slug!");
    SetTargetFPS(90);

    
    bg = LoadTexture("perfectBG1.png");
    P.leftTexture = LoadTexture("left face.png");
    P.rightTexture = LoadTexture("main_char.png");

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_LEFT) && P.player.x > 0) { // can not go back
            currentAction = moveLeft;
            P.isFacingRight = false;
        } 
        else if (IsKeyDown(KEY_RIGHT)) {
            currentAction = moveRight;
            P.isFacingRight = true;
        } 
        else if (IsKeyPressed(KEY_SPACE)) {
            currentAction = Jump;
        } 
        else {
            currentAction = nullptr;
        }

        if (currentAction) {
            currentAction();
        }
        applyGravity(dt);

        // Camera effect
        if (P.player.x > screenWidth / 2) {
            scrollX -= P.speed * dt; // Move background
            P.player.x = screenWidth / 2; // Keep player fixed in center
        }

        // Prevent white space at the end of the background
        if (scrollX <= -bg.width) {
            scrollX = 0; // Loop background properly
        }


        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawScrollingBackground(bg, scrollX);

        UpdateAndDrawPlayer();

        EndDrawing();
    }

    // Unload textures
    UnloadTexture(bg);
    UnloadTexture(P.leftTexture);
    UnloadTexture(P.rightTexture);

    CloseWindow();
    return 0;
}

void moveLeft() {
    if (P.player.x > 0) { // Prevent going beyond the left boundary
        P.player.x -= P.speed * GetFrameTime();
    }
}

void moveRight() {
    if (P.player.x < screenWidth) { // can't go beyond right boundary
        if (P.player.x < screenWidth / 2) {
            P.player.x += P.speed * GetFrameTime();
        } 
        else {
            scrollX -= P.speed * GetFrameTime();
        }
    }
}

void Jump() {
    if (!P.isJumping) {
        P.isJumping = true;
        P.verticalVelocity = jumpStrength;
    }
}

void applyGravity(float dt) {
    if (P.isJumping) {
        P.verticalVelocity += gravity * dt;
        P.player.y += P.verticalVelocity * dt;
    }

    if (P.player.y >= groundY) {
        P.player.y = groundY;
        P.isJumping = false;
        P.verticalVelocity = 0;
    }
}

void Shoot() {
    // Future feature
}


void DrawScrollingBackground(Texture2D bg, float scrollX) {

    float bgWidth = bg.width;
    
   
    DrawTextureEx(bg, {scrollX, 0}, 0.0f, 1.0f, WHITE);
    DrawTextureEx(bg, {scrollX + bgWidth, 0}, 0.0f, 1.0f, WHITE);
}

void UpdateAndDrawPlayer() {
    
    P.hitbox.x = P.player.x;
    P.hitbox.y = P.player.y;
    P.hitbox.width = P.player.width;
    P.hitbox.height = P.player.height;

    Texture2D currentTexture = P.isFacingRight ? P.rightTexture : P.leftTexture;
    DrawTexturePro(
        currentTexture, 
        {0, 0, (float)currentTexture.width, (float)currentTexture.height}, 
        {P.player.x, P.player.y, P.player.width, P.player.height}, 
        {0, 0}, 0, WHITE
    );
}