#include <iostream>
#include <raylib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
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
void updateEnemies(float dt);
void InitEnemies();
void DrawEnemies();
void CheckBulletEnemyCollision();
void DrawHealth();
void CheckPlayerEnemyCollision();
void DrawScore();
void ShowGameOverScreen();
void updateSkyEnemies(float dt);
void DrawSkyEnemies();
void DrawSkyEnemyBullets();
void UpdateAfterCollisions();

const int maxBullets = 10;

struct bullet
{
	Rectangle rect;
	float x = 0, y = 0, rad = 5.0f;
	bool isFired = false;
	bool ismovingUP = false;
	bool isMovingLeft = false;
	void (*bulletAction)(bullet&) = nullptr;

};
bullet Bullet[maxBullets];

struct enemy
{
	Rectangle rect;
	bool isActive;
	float speed;
	int direction;
	float fireRate;
	float fireTimer;

};
struct enemyBullet
{
	Rectangle rect;
	bool isAcitve;
	float speed;
};

const int maxSkyEnemyBullets = 15;
enemyBullet skyEnemyBullets[maxSkyEnemyBullets];

const int maxEnemies = 5;
enemy enemies[maxEnemies];

const int maxSkyEnemies = 3;
enemy skyEnemies[maxSkyEnemies];

float enemySpawnTimer = 0.0f;
float enemySpawnInterval = 0.0f;

float skyEnemySpawnTimer = 0.0f;
float skyEnemySpawnInterval = 0.0f;


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
const float startX = 300.0f; // X position
const float groundY = 470.0f; // Y position
const float startY = groundY; // for jumping to land on ground

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
Texture2D enemy1, enemy2;
Texture2D skyEnemy;
Texture2D missile;

Texture2D bulletTexture, bulletLeft;
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

int health = 5;
int score = 0;
Texture2D healthTexture;
Sound dyingA, heli;

Texture2D end1, end2;

bool playerBlinking = false;
float playerBlinkTimer = 0.0f;
const float playerBlinkDuration = 0.1f;

bool enemyBlinking[maxEnemies] = { false };
float enemyBlinkTimer[maxEnemies] = { 0.0f };
const float enemyBlinkDuration = 0.1f;

bool skyEnemyBlinking[maxSkyEnemies] = { false };
float skyEnemyBlinkTimer[maxSkyEnemies] = { 0.0f };
const float skyEnemyBlinkDuration = 0.1f;

bool enemyDying[maxEnemies] = { false };
float enemyDyingTimer[maxEnemies] = { 0.0f };
const float enemyDyingDuration = 0.1f;

bool skyEnemyDying[maxSkyEnemies] = { false };
float skyEnemyDyingTimer[maxSkyEnemies] = { 0.0f };
const float skyEnemyDyingDuration = 0.1f;

int main() {
	InitWindow(screenWidth, screenHeight, "Metal Slug!");
	InitAudioDevice();
	LoadBulletTexture();
	SetTargetFPS(90);
	InitPlatforms();
	InitEnemies();

	bg = LoadTexture("perfectBG1.png");
	P.leftTexture = LoadTexture("left face.png");
	P.rightTexture = LoadTexture("main_char.png");
	enemy1 = LoadTexture("enemy1.png");
	enemy2 = LoadTexture("enemy2.png");
	skyEnemy = LoadTexture("skyEnemy.png");
	missile = LoadTexture("missile.png");
	healthTexture = LoadTexture("health.png");
	Music bgAudio = LoadMusicStream("bgAudio1.mp3");
	Music bgAudio1 = LoadMusicStream("bgAudio.ogg");
	dyingA = LoadSound("dyingMain.mp3");
	heli = LoadSound("helicrash.mp3");
	Sound bullets = LoadSound("bullets.mp3");
	end1 = LoadTexture("end1.png");
	end2 = LoadTexture("end2.png");


	PlayMusicStream(bgAudio);

	ShowLoadingScreen(bgAudio1);

	while (!WindowShouldClose()) {

		UpdateMusicStream(bgAudio);
		float dt = GetFrameTime();
		updateEnemies(dt);
		updateSkyEnemies(dt);

		if (health > 0) {
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
			CheckBulletEnemyCollision();
			CheckPlayerEnemyCollision();
			UpdateAfterCollisions(); //sky enemy all collisions

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
			DrawEnemies(); // Ensure this is called
			DrawSkyEnemies();
			DrawSkyEnemyBullets();
			DrawHealth();
			DrawScore();
			EndDrawing();
		}
		else {
			ShowGameOverScreen();
		}
	}

	// Unload textures
	UnloadTexture(bg);
	UnloadTexture(P.leftTexture);
	UnloadTexture(P.rightTexture);
	UnloadTexture(bulletTexture);
	UnloadTexture(bulletTexture);
	UnloadTexture(enemy1);
	UnloadTexture(enemy2);
	UnloadTexture(end1);
	UnloadTexture(end2);

	CloseWindow();
	return 0;
}

void ShowGameOverScreen() {
	static float frameTime = 0.0f;
	static int currentFrame = 0;
	Texture2D frames[] = { end1, end2 };
	int numFrames = 2;

	frameTime += GetFrameTime();
	if (frameTime >= 0.5f) {
		frameTime = 0.0f;
		currentFrame = (currentFrame + 1) % numFrames;
	}

	BeginDrawing();
	ClearBackground(BLACK);
	DrawTexture(frames[currentFrame], 0, 0, WHITE);
	int scorePosX = 456;
	int scorePosY = 256;
	DrawText(TextFormat("%d", score), scorePosX, scorePosY, 22, BLACK);
	EndDrawing();

	if (IsKeyPressed(KEY_R)) {
		health = 5;
		score = 0;
		P.player.x = startX;
		P.player.y = startY;
		scrollX = 0.0f;
		for (int i = 0; i < maxEnemies; i++) {
			enemies[i].isActive = false;
		}
		for (int i = 0; i < maxBullets; i++) {
			Bullet[i].isFired = false;
		}
	}
	else if (IsKeyPressed(KEY_Q)) {
		CloseWindow();
	}
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

void DrawHealth() {
	healthTexture.width = 15;
	healthTexture.height = 15;
	for (int i = 0; i < health; i++) {
		DrawTexture(healthTexture, 10 + i * (healthTexture.width + 5), 10, WHITE);
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
	DrawTextureEx(bg, { scrollX, 0 }, 0.0f, 1.0f, WHITE);
	DrawTextureEx(bg, { scrollX + bgWidth, 0 }, 0.0f, 1.0f, WHITE);
}

void InitEnemies() {
	//ground enemies
	for (int i = 0; i < maxEnemies; i++) {
		enemies[i].rect = { 0, groundY + 20, 80, 80 };
		enemies[i].isActive = false;
		enemies[i].speed = 140.0f;
		enemies[i].direction = GetRandomValue(0, 1); // Random direction (0 or 1)
	}
	//sky enemies
	for (int i = 0; i < maxSkyEnemies; i++)
	{
		skyEnemies[i].rect = { (float)GetRandomValue(screenWidth - 200, screenWidth - 50), 100, 80,80 };
		skyEnemies[i].isActive = false;
		skyEnemies[i].fireRate = 1.0f;
		skyEnemies[i].fireTimer = 0.0f;
	}
	for (int i = 0; i < maxSkyEnemyBullets; i++) {
		skyEnemyBullets[i].isAcitve = false;
		skyEnemyBullets[i].speed = 250.0f;
	}
}

void updateEnemies(float dt) {
    enemySpawnTimer += dt;

    if (enemySpawnTimer >= enemySpawnInterval) {
        for (int i = 0; i < maxEnemies; i++) {
            if (!enemies[i].isActive && !enemyDying[i]) {
                enemies[i].isActive = true;
                enemies[i].direction = GetRandomValue(0, 1);

                if (enemies[i].direction == 0) {
                    enemies[i].rect.x = -50;
                }
                else {
                    enemies[i].rect.x = screenWidth;
                }

                enemies[i].rect.y = groundY + 20;
                break;
            }
        }

        enemySpawnInterval = GetRandomValue(1, 3);
        enemySpawnTimer = 0.0f;
    }

    for (int i = 0; i < maxEnemies; i++) {
        if (enemyDying[i]) {
            enemyDyingTimer[i] -= dt;
            if (enemyDyingTimer[i] <= 0.0f) {
                enemies[i].isActive = false;
                enemyDying[i] = false;
                enemyBlinking[i] = false;
            }
        }
    }

    for (int i = 0; i < maxEnemies; i++) {
        if (enemies[i].isActive && !enemyDying[i]) {
            if (enemies[i].direction == 0) {
                enemies[i].rect.x += enemies[i].speed * dt;
                if (enemies[i].rect.x > screenWidth) {
                    enemies[i].isActive = false;
                }
            }
            else {
                enemies[i].rect.x -= enemies[i].speed * dt;
                if (enemies[i].rect.x < -enemies[i].rect.width) {
                    enemies[i].isActive = false;
                }
            }
        }
    }
}
void updateSkyEnemies(float dt) {
    skyEnemySpawnTimer += dt;

    if (skyEnemySpawnTimer >= skyEnemySpawnInterval) {
        for (int i = 0; i < maxSkyEnemies; i++) {
            if (!skyEnemies[i].isActive && !skyEnemyDying[i]) {
                skyEnemies[i].isActive = true;
                skyEnemies[i].rect.x = GetRandomValue(screenWidth - 200, screenWidth - 50);
                skyEnemies[i].rect.y = 100;
                skyEnemies[i].fireTimer = 0.0f;
                break;
            }
        }

        skyEnemySpawnInterval = GetRandomValue(3, 5);
        skyEnemySpawnTimer = 0.0f;
    }

    for (int i = 0; i < maxSkyEnemies; i++) {
        if (skyEnemyDying[i]) {
            skyEnemyDyingTimer[i] -= dt;
            if (skyEnemyDyingTimer[i] <= 0.0f) {
                skyEnemies[i].isActive = false;
                skyEnemyDying[i] = false;
                skyEnemyBlinking[i] = false;
            }
        }
    }

    for (int i = 0; i < maxSkyEnemies; i++) {
        if (skyEnemies[i].isActive && !skyEnemyDying[i]) {
            skyEnemies[i].fireTimer += dt;
            if (skyEnemies[i].fireTimer >= skyEnemies[i].fireRate) {
                for (int j = 0; j < maxSkyEnemyBullets; j++) {
                    if (!skyEnemyBullets[j].isAcitve) {
                        skyEnemyBullets[j].isAcitve = true;
                        skyEnemyBullets[j].rect = { skyEnemies[i].rect.x + 10, skyEnemies[i].rect.y + 20, 40, 40 };
                        skyEnemies[i].fireTimer = 0.0f;
                        break;
                    }
                }
            }
            
            skyEnemies[i].rect.x -= 100.0f * dt;
            if (skyEnemies[i].rect.x < -skyEnemies[i].rect.width) {
                skyEnemies[i].isActive = false;
            }
        }
    }

    for (int i = 0; i < maxSkyEnemyBullets; i++) {
        if (skyEnemyBullets[i].isAcitve) {
            skyEnemyBullets[i].rect.y += skyEnemyBullets[i].speed * dt;

            if (skyEnemyBullets[i].rect.y > screenHeight) {
                skyEnemyBullets[i].isAcitve = false;
            }
        }
    }
}
void DrawSkyEnemyBullets() {
	for (int i = 0; i < maxSkyEnemyBullets; i++) {
		if (skyEnemyBullets[i].isAcitve) {
			Rectangle sourceRec = { 0, 0, (float)missile.width, (float)missile.height };
            Rectangle destRec = { skyEnemyBullets[i].rect.x, skyEnemyBullets[i].rect.y, skyEnemyBullets[i].rect.width, skyEnemyBullets[i].rect.height };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(missile, sourceRec, destRec, origin, 0.0f, WHITE);
		}
	}
}

void DrawEnemies() {
    for (int i = 0; i < maxEnemies; i++) {
        if (enemies[i].isActive || enemyDying[i]) {
            Texture2D currentTexture = (enemies[i].direction == 0) ? enemy1 : enemy2;
            Rectangle sourceRec = { 0, 0, (float)currentTexture.width, (float)currentTexture.height };
            Rectangle destRec = { enemies[i].rect.x, enemies[i].rect.y, enemies[i].rect.width, enemies[i].rect.height };
            Vector2 origin = { 0, 0 };

            if (enemies[i].direction == 0) {
                destRec.y = enemies[i].rect.y - 25;
            } else {
                destRec.y = enemies[i].rect.y - 13;
            }

            Color color = (enemyDying[i] || enemyBlinking[i]) ? RED : WHITE;
            DrawTexturePro(currentTexture, sourceRec, destRec, origin, 0.0f, color);
        }
    }
}

void DrawSkyEnemies() {
    for (int i = 0; i < maxSkyEnemies; i++) {
        if (skyEnemies[i].isActive || skyEnemyDying[i]) {
            Rectangle sourceRec = { 0, 0, (float)skyEnemy.width, (float)skyEnemy.height };
            Rectangle destRec = { skyEnemies[i].rect.x, skyEnemies[i].rect.y, skyEnemies[i].rect.width, skyEnemies[i].rect.height };
            Vector2 origin = { 0, 0 };
            Color color = (skyEnemyDying[i] || skyEnemyBlinking[i]) ? RED : WHITE;
            DrawTexturePro(skyEnemy, sourceRec, destRec, origin, 0.0f, color);
        }
    }
}

void DrawBullets() {
    frameCounter++;
    if (frameCounter >= frameSpeed) {
        frameCounter = 0;
        currentFrame = (currentFrame + 1) % bulletFrames;
    }

    for (int i = 0; i < maxBullets; i++) {
        if (Bullet[i].isFired) {
            Rectangle sourceRec = { (float)(currentFrame * bulletFrameWidth), 0, (float)bulletFrameWidth, (float)bulletTexture.height };
            Texture2D currentTexture = Bullet[i].isMovingLeft ? bulletLeft : bulletTexture;

            Rectangle destRec = { Bullet[i].x, Bullet[i].y, Bullet[i].rad * 5, Bullet[i].rad * 5 };
            Vector2 origin = { Bullet[i].rad, Bullet[i].rad };

            float rotation = 0.0f;
            if (Bullet[i].ismovingUP) {
                rotation = Bullet[i].isMovingLeft ? 90.0f : -90.0f; // Same rotation for both directions when moving up
            }

            DrawTexturePro(currentTexture, sourceRec, destRec, origin, rotation, WHITE);
        }
    }
}

void UpdateAndDrawPlayer() {
    P.hitbox.x = P.player.x;
    P.hitbox.y = P.player.y;
    P.hitbox.width = P.player.width;
    P.hitbox.height = P.player.height;

    if (playerBlinking) {
        playerBlinkTimer -= GetFrameTime();
        if (playerBlinkTimer <= 0.0f) {
            playerBlinking = false;
        }
    }

    Texture2D currentTexture = P.isFacingRight ? P.rightTexture : P.leftTexture;
    Color color = playerBlinking ? RED : WHITE;
    DrawTexturePro(
        currentTexture,
        { 0, 0, (float)currentTexture.width, (float)currentTexture.height },
        { P.player.x, P.player.y, P.player.width, P.player.height },
        { 0, 0 }, 0, color
    );
}

void MoveBullets(struct bullet& b)
{
    if (b.ismovingUP)
    {
        b.y -= 300.0f * GetFrameTime();
        if (b.isMovingLeft) {
            b.x -= 50.0f * GetFrameTime(); 
        } 
		else {
            b.x += 50.0f * GetFrameTime(); 
        }
        
        if (b.y < 0)
        {
            b.isFired = false;
        }
    }
    else if (b.isMovingLeft)
    {
        b.x -= 300.0f * GetFrameTime();
        if (b.x < 0)
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
            if (P.isFacingRight) {
                Bullet[i].x = P.player.x + P.player.width;
            } else {
                Bullet[i].x = P.player.x;
            }
            
            Bullet[i].y = P.player.y + P.player.height / 2 - 5;
            Bullet[i].rad = 5.0f;
            Bullet[i].isFired = true;
            Bullet[i].ismovingUP = movingUp;
            Bullet[i].isMovingLeft = !P.isFacingRight;
            Bullet[i].bulletAction = MoveBullets;
            break;
        }
    }
}
void LoadBulletTexture() {
	bulletTexture = LoadTexture("bullets.png");
	bulletLeft = LoadTexture("bulletsLeft.png");
	bulletFrameWidth = bulletTexture.width / bulletFrames;
}

// platform function
void InitPlatforms() {
	bgWidth = bg.width;

	platforms[0] = { {50, 493, 165, 10}, 440, true };
	platforms[1] = { {750, 372, 175, 10}, 175, true };
	platforms[2] = { {0, 435, 110, 10}, 0, true };
	platforms[3] = { {1200, 493, 110, 10}, 1179, true };
}

void WaitTime(float seconds) {
	int ms = seconds * 1000;
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif 
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

	Texture2D loadingFrames[] = { load25, load50, load75, load100 };
	for (int i = 0; i < 4; i++) {
		float startTime = GetTime();
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
		if ((int)(GetTime() * 2) % 2 == 0) {
			DrawTexture(enter1, centerX - enter1.width / 2, centerY - enter1.height / 2, WHITE);
		}
		else {
			DrawTexture(enter2, centerX - enter2.width / 2, centerY - enter2.height / 2, WHITE);
		}
		EndDrawing();
	}

	float alpha = 255.0f; // Start fully opaque
	float fadeSpeed = 150.0f; // Adjust speed for a smoother fade

	while (alpha > 0 && !WindowShouldClose()) {
		UpdateMusicStream(music);

		alpha -= fadeSpeed * GetFrameTime(); // Decrease gradually

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, alpha / 255.0f)); // Fading overlay, faida koi nhi

		EndDrawing();
	}

	UnloadTexture(load25);
	UnloadTexture(load50);
	UnloadTexture(load75);
	UnloadTexture(load100);
	UnloadTexture(enter1);
	UnloadTexture(enter2);
}


void CheckBulletEnemyCollision() {
    for (int i = 0; i < maxBullets; i++) {
        if (Bullet[i].isFired) {
            for (int j = 0; j < maxEnemies; j++) {
                if (enemies[j].isActive && !enemyDying[j]) {
                    if (CheckCollisionCircleRec({ Bullet[i].x + Bullet[i].rad, Bullet[i].y + Bullet[i].rad }, Bullet[i].rad, enemies[j].rect)) {
                        Bullet[i].isFired = false;
                        enemyDying[j] = true;
                        enemyDyingTimer[j] = enemyDyingDuration;
                        enemyBlinking[j] = true;
                        enemyBlinkTimer[j] = enemyBlinkDuration;
                        PlaySound(dyingA);
                        score++;
                        break;
                    }
                }
            }
        }
    }
}


void CheckPlayerEnemyCollision() {
    for (int i = 0; i < maxEnemies; i++) {
        if (enemies[i].isActive && CheckCollisionRecs(P.hitbox, enemies[i].rect)) {
            health--;
            enemies[i].isActive = false;
            playerBlinking = true;
            playerBlinkTimer = playerBlinkDuration;
            break;
        }
    }
}

void DrawScore() {
	DrawText(TextFormat("Score : %d", score), screenWidth - 140, 10, 20, WHITE);
}


void UpdateAfterCollisions() {
    // if sky enemy bullets hit player
    for (int i = 0; i < maxSkyEnemyBullets; i++) {
        if (skyEnemyBullets[i].isAcitve) {
            if (CheckCollisionRecs(P.hitbox, skyEnemyBullets[i].rect)) {
                health--;
                skyEnemyBullets[i].isAcitve = false;
                playerBlinking = true;
                playerBlinkTimer = playerBlinkDuration;
            }
        }
    }

    // if player bullet hits sky enemies
    for (int i = 0; i < maxBullets; i++) {
        if (Bullet[i].isFired) {
            for (int j = 0; j < maxSkyEnemies; j++) {
                if (skyEnemies[j].isActive && !skyEnemyDying[j]) {
                    Vector2 bulletOrigin = { Bullet[i].x + Bullet[i].rad, Bullet[i].y + Bullet[i].rad };
                    if (CheckCollisionCircleRec(bulletOrigin, Bullet[i].rad, skyEnemies[j].rect)) {
						PlaySound(heli);
                        Bullet[i].isFired = false;
                        skyEnemyDying[j] = true;
                        skyEnemyDyingTimer[j] = skyEnemyDyingDuration;
                        skyEnemyBlinking[j] = true;
                        skyEnemyBlinkTimer[j] = skyEnemyBlinkDuration;
                        score++;
                    }
                }
            }
        }
    }

    // if player bullet hits sky enemy's bullet
    for (int i = 0; i < maxBullets; i++) {
        if (Bullet[i].isFired) {
            for (int j = 0; j < maxSkyEnemyBullets; j++) {
                if (skyEnemyBullets[j].isAcitve) {
                    Vector2 bulletOrigin = { Bullet[i].x + Bullet[i].rad, Bullet[i].y + Bullet[i].rad };
                    if (CheckCollisionCircleRec(bulletOrigin, Bullet[i].rad, skyEnemyBullets[j].rect)) {
                        Bullet[i].isFired = false;
                        skyEnemyBullets[j].isAcitve = false;
                    }
                }
            }
        }
    }
}