#include <iostream>
#include <raylib.h>
using namespace std;

// Function pointer for movement
void (*playerAction)(void) = nullptr;

//void (* Shoot) (txture2D ....)
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
    LoadBulletTexture();
    SetTargetFPS(90);

    
    bg = LoadTexture("perfectBG1.png");
    P.leftTexture = LoadTexture("left face.png");
    P.rightTexture = LoadTexture("main_char.png");

    while (!WindowShouldClose()) {

        
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_LEFT) && P.player.x > 0) 
        { // can not go back
            playerAction = moveLeft;
            P.isFacingRight = false;
        } 
        if (IsKeyDown(KEY_RIGHT)) {
            playerAction = moveRight;
            P.isFacingRight = true;
        } 
        if (IsKeyPressed(KEY_SPACE)) {
            playerAction = Jump;
        } 
        if (!(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_SPACE)))
        {
            playerAction = nullptr;
        }
        if (playerAction) {
            playerAction();
        }
        applyGravity(dt);

        if (IsKeyPressed(KEY_F))
        {
            FiredBullet(false); 
        }
        if (IsKeyPressed(KEY_R))
        {
            FiredBullet(true);
        }
        for (int i = 0; i < maxBullets; i++) {
            if (Bullet[i].isFired && Bullet[i].bulletAction) {
                Bullet[i].bulletAction(Bullet[i]); // Calling function pointer to move bullet
            }
        }
        // Camera effect
        if (P.player.x > screenWidth / 2) {
            scrollX -= P.speed * dt; // Move background
            P.player.x = screenWidth / 2; // Keep player fixed in center
        }

        if (scrollX <= -bg.width) {
            scrollX = 0; // Looping background 
        }


        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawScrollingBackground(bg, scrollX);
        DrawBullets();
        UpdateAndDrawPlayer();
        EndDrawing();
    }

    // Unload textures
    UnloadTexture(bg);
    UnloadTexture(P.leftTexture);
    UnloadTexture(P.rightTexture);
    UnloadTexture(bulletTexture);

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
        if (P.player.x < screenWidth / 3) {
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