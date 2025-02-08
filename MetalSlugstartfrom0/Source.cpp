#include <iostream>
#include <raylib.h>
#include <unistd.h>
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
void FiredBullet(bool movingUp);
void DrawBullets();
void LoadBulletTexture();
void InitPlatforms();
void ShowLoadingScreen(Music& music);

const int maxBullets = 10;

struct bullet
{
    float x = 0, y = 0, rad = 5.0f;
    bool isFired = false;
    bool ismovingUP = false;
    void (*bulletAction)(bullet&) = nullptr;

}; 
bullet Bullet[maxBullets];


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
const float playerWidth = 70.0f;  
const float playerHeight = 70.0f; 
const float startX = 300.0f;      // X position
const float groundY = 470.0f;  // Y position
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

Texture2D bulletTexture;
const int bulletFrames = 7; // Number of frames
int bulletFrameWidth;
int currentFrame = 0;
int frameCounter = 0;
const int frameSpeed = 2; // Adjust speed of animation


// standing platforms
struct Platform {
    Rectangle rect;
    float originalX;  
    bool isActive;
};

const int maxPlatforms = 4;
Platform platforms[maxPlatforms];
float bgWidth;



int main() {
    InitWindow(screenWidth, screenHeight, "Metal Slug!");
    InitAudioDevice();
    LoadBulletTexture();
    SetTargetFPS(90);
    InitPlatforms();

    bg = LoadTexture("perfectBG1.png");
    P.leftTexture = LoadTexture("left face.png");
    P.rightTexture = LoadTexture("main_char.png");
    Music bgAudio = LoadMusicStream("bgAudio1.mp3"); 
    Music bgAudio1 = LoadMusicStream("bgAudio.ogg"); 
    Sound bullets = LoadSound("bullets.mp3");
    PlayMusicStream(bgAudio);

    ShowLoadingScreen(bgAudio1);

    while (!WindowShouldClose()) {

        UpdateMusicStream(bgAudio);
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
            PlaySound(bullets);
            FiredBullet(false); 
        }
        if (IsKeyPressed(KEY_R))
        {
            PlaySound(bullets);
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

        for (int i = 0; i < maxPlatforms; i++) {
            platforms[i].rect.x = platforms[i].originalX + scrollX;
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
        P.player.y += P.verticalVelocity * GetFrameTime();  
    }
}

void applyGravity(float dt) {
    bool onPlatform = false;

    for (int i = 0; i < maxPlatforms; i++) {
        // for platforms
        if (platforms[i].isActive &&
            P.player.y + P.player.height >= platforms[i].rect.y &&
            P.player.y + P.player.height <= platforms[i].rect.y + 10 &&
            P.player.x + P.player.width > platforms[i].rect.x &&
            P.player.x < platforms[i].rect.x + platforms[i].rect.width) {
            
            P.player.y = platforms[i].rect.y - P.player.height;
            
            if (P.verticalVelocity > 0) {
                P.isJumping = false;
            }

            P.verticalVelocity = 0;
            onPlatform = true;
            break;
        }
    }

    if (!onPlatform) {
        P.verticalVelocity += gravity * dt;
        P.player.y += P.verticalVelocity * dt;

        if (P.player.y >= groundY) {
            P.player.y = groundY;
            P.isJumping = false;
            P.verticalVelocity = 0;
        }
    }
}


void DrawScrollingBackground(Texture2D bg, float scrollX) {
    float bgWidth = bg.width;
    DrawTextureEx(bg, {scrollX, 0}, 0.0f, 1.0f, WHITE);
    DrawTextureEx(bg, {scrollX + bgWidth, 0}, 0.0f, 1.0f, WHITE);
}

void DrawBullets() {
    frameCounter++;
    if (frameCounter >= frameSpeed) {
        frameCounter = 0;
        currentFrame = (currentFrame + 1) % bulletFrames;
    }

    for (int i = 0; i < maxBullets; i++) {
        if (Bullet[i].isFired) {
            Rectangle sourceRec = {static_cast<float>(currentFrame * bulletFrameWidth), 0, (float)bulletFrameWidth, (float)bulletTexture.height};
            Rectangle destRec = {Bullet[i].x, Bullet[i].y, Bullet[i].rad * 5, Bullet[i].rad * 5};
            Vector2 origin = {Bullet[i].rad, Bullet[i].rad};

            float rotation = Bullet[i].ismovingUP ? -90.0f : 0.0f; // Rotating if moving up
            DrawTexturePro(bulletTexture, sourceRec, destRec, origin, rotation, WHITE);
        }
    }
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

void MoveBullets(struct bullet& b)
{
    if (b.ismovingUP)
    {
        b.y -= 300.0f * GetFrameTime();
        if (b.y < 0)
        {
            b.isFired = false;
        }
    }
    else
    {
        b.x += 300.0f * GetFrameTime();
        if (b.x > screenWidth)
        {
            b.isFired = false;
        }
    }
}

void FiredBullet(bool movingUp)
{
    for (int i = 0; i < maxBullets; i++)
    {
        if (!Bullet[i].isFired)
        {
            Bullet[i].x = P.player.x + (P.isFacingRight ? P.player.width : 0);
            Bullet[i].y = P.player.y + P.player.height / 2 - 5;
            Bullet[i].rad = 5.0f;
            Bullet[i].isFired = true;
            Bullet[i].ismovingUP = movingUp;
            Bullet[i].bulletAction = MoveBullets; //function pointer
            break;  
        }
    }
}

void LoadBulletTexture() {
    bulletTexture = LoadTexture("bullets.png");
    bulletFrameWidth = bulletTexture.width / bulletFrames;
}

// platform function
void InitPlatforms() {
    bgWidth = bg.width; 

    platforms[0] = {{50, 493, 165, 10}, 440, true};
    platforms[1] = {{750, 372, 175, 10}, 175, true};
    platforms[2] = {{0, 435, 110, 10}, 0, true};
    platforms[3] = {{1200, 493, 110, 10}, 1179, true};
}

void WaitTime(float seconds) {
    int ms = seconds * 1000;  
    usleep(ms * 1000);        
}

void ShowLoadingScreen(Music& music) {
    SetTargetFPS(60);
    PlayMusicStream(music);

    Texture2D load25 = LoadTexture("load25.png");
    Texture2D load50 = LoadTexture("load50.png");
    Texture2D load75 = LoadTexture("load75.png");
    Texture2D load100 = LoadTexture("load100.png");
    Texture2D enter1 = LoadTexture("enter1.png");
    Texture2D enter2 = LoadTexture("enter2.png");

    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    Texture2D loadingFrames[] = {load25, load50, load75, load100};
    for (int i = 0; i < 4; i++) {
        float startTime = GetTime();
        // loop will run for 0.5 seconds for each frame
        while (GetTime() - startTime < 0.5f) {
            UpdateMusicStream(music);
            BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(loadingFrames[i], centerX - loadingFrames[i].width / 2, centerY - loadingFrames[i].height / 2, WHITE);
            EndDrawing();
        }
    }

    while (!IsKeyPressed(KEY_ENTER) && !WindowShouldClose()) {
        UpdateMusicStream(music);
        BeginDrawing();
        ClearBackground(BLACK);
        // even and odd for enter buttons
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawTexture(enter1, centerX - enter1.width / 2, centerY - enter1.height / 2, WHITE);
        }
         else {
            DrawTexture(enter2, centerX - enter2.width / 2, centerY - enter2.height / 2, WHITE);
        }
        EndDrawing();
    }
    
    UnloadTexture(load25);
    UnloadTexture(load50);
    UnloadTexture(load75);
    UnloadTexture(load100);
    UnloadTexture(enter1);
    UnloadTexture(enter2);
}