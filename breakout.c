/*******************************************************************************************
*
*   raylib - classic game: arkanoid
*
*   Sample game developed by Marc Palau and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

/* Modified by Ville Seppänen, August 2024
 * Expanded the game a bunch.
 */

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define PLAYER_MAX_LIFE     5
#define LINES_OF_BRICKS     18
#define BRICKS_PER_LINE     32

// structs
typedef struct Player {
	Texture2D	texture;
    Vector2		position;
    Vector2		size;
	Color		color;
    int 	    life;
    int			speed;
} Player;

typedef struct Ball {
	Texture2D	texture;
    Vector2		position;
    Vector2		speed;
	Color   	color;
    int     	radius;
    int     	revealradius;
    bool    	active;
} Ball;

typedef struct Brick {
    Vector2 	position;
	Texture2D	texture;
	Color   	color;
    int 	    hitpoints;
    bool    	active;
    bool    	collidable;
    bool    	visible;
    
} Brick;

typedef struct { // remember to update textures count below!
    Texture2D *texture;
} Terrain;

#define TEXTURE_COUNT 5
Texture2D textures[TEXTURE_COUNT];

// globals
static const int screenWidth = 1920;
static const int screenHeight = 1080;

static bool gameOver = false;
static bool pause = false;

static Player player = { 0 };
static Ball ball = { 0 };
static Brick brick[LINES_OF_BRICKS][BRICKS_PER_LINE] = { 0 };
static Vector2 brickSize = { 0 };

// prototypes
static void Inits(void);         
static void UpdateGame(void);    
static void DrawGame(void);   
static void UpdateDrawFrame(void);

// static void InitTextures(void);
// static void Unload(void);

int main(void)
{
    InitWindow(screenWidth, screenHeight, "DungeonBreak");

	// music stuff, move
	InitAudioDevice();              
    Music music = LoadMusicStream("keepitsimple.mp3");
    PlayMusicStream(music);

    LoadTextures();
	Inits();

    SetTargetFPS(120);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
		// music, move
		UpdateMusicStream(music);
		
		// draw
        UpdateDrawFrame();
    }
	
	// music stuff, move
	UnloadMusicStream(music);
    CloseAudioDevice();

    CloseWindow();        // Close window and OpenGL context
	
	// Unload();        // unload/free stuff
	// UnloadTexture(brwall); // move this 
    return 0;
}

void LoadTextures()
{
	textures[0] = LoadTexture("brground.png");
	textures[1] = LoadTexture("brstonepath.png");
	textures[2] = LoadTexture("brdoor.png");
	textures[3] = LoadTexture("brfloor2.png");
	textures[4] = LoadTexture("brwall.png");
}
	
void Unload(void)
{
	
}	

void Inits(void)
{
    brickSize = (Vector2){ screenWidth/BRICKS_PER_LINE, screenHeight/18 };

    // Initialize player
	player.texture = LoadTexture("brdude.png");
    player.position = (Vector2){ screenWidth/2, screenHeight*17/18 };
    player.size = (Vector2){ screenWidth/10, 20 };
    player.life = PLAYER_MAX_LIFE;
    player.speed = 5;
	player.color = WHITE;

    // Initialize ball
	ball.texture = LoadTexture("brball.png");
    ball.position = (Vector2){ screenWidth/2, screenHeight*7/8 - 30 };
    ball.speed = (Vector2){ 0, 0 };
    ball.radius = 16;
	ball.color = WHITE;
    ball.revealradius = ball.radius * 12;
    ball.active = false;

    // Initialize bricks
    int initialDownPosition = 50;

    int map[18][32] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,0,0,0,0,0},
        {0,3,3,3,3,3,3,3,0,0,0,0,3,3,3,3,3,0,0,0,3,2,2,2,2,2,3,0,0,0,0,0},
        {0,3,2,2,2,2,2,1,4,4,0,0,3,2,2,2,3,0,4,4,1,2,2,2,2,2,3,0,0,0,0,0},
        {0,3,2,2,2,2,2,3,0,4,4,4,1,2,2,2,1,4,4,0,3,2,2,2,2,2,3,0,0,0,0,0},
        {0,3,2,2,2,2,2,3,0,4,0,0,3,1,3,3,3,0,0,0,3,3,3,3,1,3,3,0,0,0,0,0},
        {0,3,2,2,2,2,2,1,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0},
        {0,3,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0},
        {0,3,2,2,2,2,2,3,0,0,0,0,0,3,3,1,3,3,0,3,3,3,3,1,3,3,3,3,3,3,0,0},
        {0,3,3,3,3,3,3,3,0,0,0,0,0,3,2,2,2,3,0,3,2,2,2,2,2,2,2,2,2,3,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,3,0,3,2,2,2,2,2,2,2,2,2,3,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,3,0,3,2,2,2,2,2,2,2,2,2,3,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,1,3,3,0,3,3,3,3,3,3,3,3,1,3,3,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0}, // tiles below will not collide
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,4,4,4,4,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0}
    };
    
    // int map[18][32] = {
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		// {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // tiles below will not collide
		// {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		// {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		// {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        // {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    // };

	for (int i = 0; i < LINES_OF_BRICKS; i++)
	{
		for (int j = 0; j < BRICKS_PER_LINE; j++)
		{
            brick[i][j].position = (Vector2){ j*brickSize.x + brickSize.x/2, i*brickSize.y + initialDownPosition };
			switch (map[i][j]) {
				case 0: // grass
					brick[i][j].active = true;
					brick[i][j].collidable = false;
					brick[i][j].color = WHITE;
					brick[i][j].texture = textures[0];
					break;
				case 4: // stone path
					brick[i][j].active = true;
					brick[i][j].collidable = false;
					brick[i][j].color = WHITE;
					brick[i][j].texture = textures[1];
					break;
				case 1: // door
					brick[i][j].hitpoints = 1;
					brick[i][j].active = true;
					brick[i][j].collidable = true;
					brick[i][j].visible = false;
					brick[i][j].color = WHITE;
					brick[i][j].texture = textures[2];
					break;
				case 2: // floor
					brick[i][j].hitpoints = 2;
					brick[i][j].active = true;
					brick[i][j].collidable = false;
					brick[i][j].visible = false;
					brick[i][j].color = WHITE;
					brick[i][j].texture = textures[3];
					break;
				case 3: // brick wall
					brick[i][j].hitpoints = 3;
					brick[i][j].active = true;
					brick[i][j].collidable = true;
					brick[i][j].visible = false;
					brick[i][j].color = WHITE;
					brick[i][j].texture = textures[4];
					break;
              default:
            }
        }
    }
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // Player movement logic
            if (IsKeyDown(KEY_LEFT)) player.position.x -= player.speed;
            if ((player.position.x - player.size.x/2) <= 0) player.position.x = player.size.x/2;
            if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed;
            if ((player.position.x + player.size.x/2) >= screenWidth) player.position.x = screenWidth - player.size.x/2;

            // Ball launching logic
            if (!ball.active)
            {
                if (IsKeyPressed(KEY_SPACE))
                {
                    ball.active = true;
                    ball.speed = (Vector2){ GetRandomValue(-2, 2), -5 };
                }
            }

            // Ball movement logic
            if (ball.active)
            {
                ball.position.x += ball.speed.x;
                ball.position.y += ball.speed.y;
            }
            else
            {
                ball.position = (Vector2){ player.position.x, screenHeight*15/16 - 30 };
            }

            // Collision logic: ball vs walls
            if (((ball.position.x + ball.radius) >= screenWidth) || ((ball.position.x - ball.radius) <= 0)) ball.speed.x *= -1;
            if ((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;
            if ((ball.position.y + ball.radius) >= screenHeight)
            {
                ball.speed = (Vector2){ 0, 0 };
                ball.active = false;
                player.life--;
            }

            // Collision logic: ball vs player
            if (CheckCollisionCircleRec(ball.position, ball.radius,
                (Rectangle){ player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y}))
            {
                if (ball.speed.y > 0)
                {
                    ball.speed.y *= -1;
                    ball.speed.x = (ball.position.x - player.position.x)/(player.size.x/2)*5;
                }
            }

            // Collision logic: ball vs bricks
            for (int i = 0; i < LINES_OF_BRICKS; i++)
            {
                for (int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    if (brick[i][j].active && brick[i][j].collidable)
                    {
                        // Hit below
                        if (((ball.position.y - ball.radius) <= (brick[i][j].position.y - brickSize.y/8 )) &&
                            ((ball.position.y - ball.radius) > (brick[i][j].position.y - brickSize.y/8 + ball.speed.y)) &&
                            ((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) && (ball.speed.y < 0))
                        {
                            brick[i][j].hitpoints--;
                            if (brick[i][j].hitpoints == 0)
                            {
                                brick[i][j].collidable = false;
                                brick[i][j].color = GRAY;
                            }
                            ball.speed.y *= -1;
                        }
                        // Hit above
                        else if (((ball.position.y + ball.radius) >= (brick[i][j].position.y - brickSize.y)) &&
                                ((ball.position.y + ball.radius) < (brick[i][j].position.y - brickSize.y + ball.speed.y)) &&
                                ((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) && (ball.speed.y > 0))
                        {
                            brick[i][j].hitpoints--;
                            if (brick[i][j].hitpoints == 0)
                            {
                                brick[i][j].collidable = false;
                                brick[i][j].color = GRAY;
                            }
                            ball.speed.y *= -1;
                        }
                        // Hit left
                        else if (((ball.position.x + ball.radius) >= (brick[i][j].position.x - brickSize.x/2)) &&
                                ((ball.position.x + ball.radius) < (brick[i][j].position.x - brickSize.x/2 + ball.speed.x)) &&
                                ((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) && (ball.speed.x > 0))
                        {
                            brick[i][j].hitpoints--;
                            if (brick[i][j].hitpoints == 0)
                            {
                                brick[i][j].collidable = false;
                                brick[i][j].color = GRAY;
                            }
                            ball.speed.x *= -1;
                        }
                        // Hit right
                        else if (((ball.position.x - ball.radius) <= (brick[i][j].position.x + brickSize.x/2)) &&
                                ((ball.position.x - ball.radius) > (brick[i][j].position.x + brickSize.x/2 + ball.speed.x)) &&
                                ((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) && (ball.speed.x < 0))
                        {
                            brick[i][j].hitpoints--;
                            {
                                brick[i][j].collidable = false;
                                brick[i][j].color = GRAY;
                            }
                            brick[i][j].active = false;
                            ball.speed.x *= -1;
                        }
                    }
                }
            }
            
             // Make bricks visible when ball is near
            for (int i = 0; i < LINES_OF_BRICKS; i++)
            {
                for (int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    if (brick[i][j].active)
                    {
						if (((fabs(ball.position.y - brick[i][j].position.y + brickSize.y)) <= ball.revealradius)
                            && ((fabs(ball.position.x - brick[i][j].position.x + brickSize.x)) <= ball.revealradius))
                            brick[i][j].visible = true;
                    }
                }
            }
            
            // Game over logic
            if (player.life <= 0) gameOver = true;
            else
            {
                gameOver = true;

                for (int i = 0; i < LINES_OF_BRICKS; i++)
                {
                    for (int j = 0; j < BRICKS_PER_LINE; j++)
                    {
                        if (brick[i][j].active) gameOver = false;
                    }
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            Inits();
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(BLACK);

        if (!gameOver)
        {
	
            // Draw bricks
            for (int i = 0; i < LINES_OF_BRICKS; i++)
            {
                for (int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    if (brick[i][j].visible)
                    {
                        //DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, brick[i][j].color);
						
						DrawTexture(brick[i][j].texture, brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y*7/8, brick[i][j].color);
                    }
                }
            }
            
            // Draw ball
			DrawTexture(ball.texture, ball.position.x - ball.radius/2, ball.position.y - ball.radius/2, ball.color);
            //DrawCircleV(ball.position, ball.radius, ball.color);
			
			// Draw player
			DrawTexture(player.texture, player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.color);
            // DrawRectangle(player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y, player.color);

			if (!ball.active)
			{
				// dark text (shadow)
				DrawText("RogueOut!", screenWidth/2 - MeasureText("RogueOut!", 50)/2, screenHeight*3.05/16 - 50, 50, BLACK);
				DrawText("ARROW KEYS left/right to move, SPACE to launch spell ball!", screenWidth/2 - MeasureText("ARROW KEYS left/right to move, SPACE to launch spell ball!", 50)/2, screenHeight*5.05/16 - 50, 50, BLACK);
				// light text
				DrawText("RogueOut!", screenWidth/2 - MeasureText("RogueOut!", 50)/2, screenHeight*3/16 - 50, 50, WHITE);
				DrawText("ARROW KEYS left/right to move, SPACE to launch spell ball!", screenWidth/2 - MeasureText("ARROW KEYS left/right to move, SPACE to launch spell ball!", 50)/2, screenHeight*5/16 - 50, 50, GREEN);
			}
			
            // Draw player lives
            for (int i = 0; i < player.life; i++) DrawRectangle(20 + 40*i, screenHeight - 30, 35, 10, LIGHTGRAY);
			
            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();
}



void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}