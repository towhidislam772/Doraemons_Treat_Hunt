#define GL_SILENCE_DEPRECATION
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//------------ For using Sound -----------
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#endif
#define TOTAL_OBSTACLES 4
#define TOTAL_FIRES 3
#define TOTAL_STONES 3

//Character Movement
float charX = -9.7f;
float charY = -7.5f;
bool moving = false;
bool jumping = false;
int facing = 1;
float legAngle = 0.0f;
int legDir = 1;
float jumpSpeed = 0.1f;

float _moveSpeed = 0.2f;
float _legSwingSpeed = 2.0f;
float _jumpSpeed = 0.2f;
float _legResetSpeed = 2.0f;
float _maxLegAngle = 3.0f;

//Scene Movement
float worldX = 0.0f;
int sceneCount = 1;
int totalScenes = 4;
float sceneWidth = 23.0f;

// Doracakes
float doracakeX[10];
float doracakeY[10];
bool doracakeCollected[10] = { false };
int totalDoracakes = 10;
int doracakesCollected = 0;

// Hearts
float heartX[3];
float heartY[3];
bool heartLost[3] = { false, false, false };
int totalHearts = 3;
int lives = 3;
int totalLevels = 3;
int currentLevel = 0;

// Game end condition
bool showEndScreen = false;
bool gameWon = false;
bool showHome = true;

// Pause
bool gamePaused = false;


// Invincibility frames  ( 2 s )
bool  isInvincible = false;
float invincTimer = 0.0f;
const float INVINC_DURATION = 45.0f;   // Cooldown time for obstacles damage (continius)

// Hit-flash effect
bool  isFlashing = false;
float hitFlashTimer = 0.0f;
const float HIT_FLASH_DURATION = 30.0f;

// Scores
int score = 0;   // current run
int levelScore = 0;   // current level
int hiScore = 0;   // best score for the current level

const int MAX_GAME_LEVELS = 5;
int hiScoreByLevel[MAX_GAME_LEVELS + 1] = {0};

// Clouds , Level 1
const int   TOTAL_CLOUDS   = 5;
float cloudX[TOTAL_CLOUDS] = {-8.0f, -1.0f,  5.0f, 11.0f, 17.0f};
float cloudY[TOTAL_CLOUDS] = { 6.5f,  8.2f,  7.0f,  5.8f,  8.5f};
float cloudSpd             = 0.012f;

// Go to Next-level button
float nextLevelX = -1.7f, nextLevelY = -2.5f;


// ---------- LEVEL 4 & 5
bool level4Started = false;
bool level5Started = false;


// Falling Rocks Level 4
const int ROCK_COUNT = 3;
float rockPosX    [ROCK_COUNT];
float rockPosY    [ROCK_COUNT];
float rockVelocity[ROCK_COUNT];
bool  rockIsHit   [ROCK_COUNT];

// Twinkling Stars Level 5 sky
const int STAR_COUNT = 80;
float starPosX [STAR_COUNT];
float starPosY [STAR_COUNT];
float starPhase[STAR_COUNT];   // blink
float starTime5 = 0.0f;        // frame time

// Lightning Level 5 visual
bool  lightningFlash  = false;
float lightningClock  = 0.0f;
const float LIGHTNING_PERIOD = 220.0f;
float lightningBoltX  = 0.0f;  // random X each strike


// Ghost enemy (Level 5)
float ghostPosX  = -20.0f;
float ghostPosY  =  -6.5f;
bool  ghostIsHit = false;


// Button positions
float buttonWidth = 3.5f;
float buttonHeight = 0.7f;
float mainMenuX = -1.7f, mainMenuY = -0.5f;
float restartX = -1.7f, restartY = -1.5f;

bool level3Started = false;

//--------------Adding Sound---------------------

char SND_BGM[260]       = "sounds\\bgm.mp3";      // default background music for loops
const char* SND_COLLECT  = "sounds/collect.wav";   // cake collected
const char* SND_HIT      = "sounds/hit.wav";       // obstacle hit
const char* SND_LEVELUP  = "sounds/levelup.wav";   // level completed
const char* SND_GAMEOVER = "sounds/gameover.wav";  // game over
const char* SND_GAMEWIN  = "sounds/gamewin.wav";   // final win
const char* MUSIC_CONFIG_FILE = "music_config.txt";

bool bgmStarted = false;

void loadMusicPreference() {
    FILE* fp = fopen(MUSIC_CONFIG_FILE, "r");
    if (fp) {
        if (fgets(SND_BGM, sizeof(SND_BGM), fp)) {     // force null termination

            SND_BGM[sizeof(SND_BGM) - 1] = '\0';


            SND_BGM[strcspn(SND_BGM, "\r\n")] = '\0';     // remove newline characters


            if (strlen(SND_BGM) == 0) {                    // default music if file empty
                strcpy(SND_BGM, "sounds\\bgm.mp3");
            }
        } else {                                           // fgets failed, use default

            strcpy(SND_BGM, "sounds\\bgm.mp3");
        }
        fclose(fp);
    }
}
#ifdef _WIN32

//  Music error helper: if music fails
void showMciError(MCIERROR error, const char* step)
{
    if (error != 0)
    {
        char errorText[256];
        char message[700];

        mciGetErrorString(error, errorText, 256);
        sprintf(message,
                "%s failed!\n\nMusic path used:\n%s\n\nError:\n%s\n\nFix: Keep bgm.mp3 inside a folder named sounds beside your .exe, or edit music_config.txt.",
                step, SND_BGM, errorText);

        MessageBox(NULL, message, "Music Error", MB_OK | MB_ICONERROR);
    }
}

//  Background Music loops
void playBGM()
{
    MCIERROR error;
    char cmd[512];

    mciSendString("stop bgm",  NULL, 0, NULL);
    mciSendString("close bgm", NULL, 0, NULL);

    sprintf(cmd, "open \"%s\" type mpegvideo alias bgm", SND_BGM);
    error = mciSendString(cmd, NULL, 0, NULL);
    if (error != 0)
    {
        showMciError(error, "Open music");
        return;
    }

    // Play on infinite loop
    error = mciSendString("play bgm repeat", NULL, 0, NULL);
    if (error != 0)
    {
        showMciError(error, "Play music");
        return;
    }

    bgmStarted = true;
}

void stopBGM() {
    mciSendString("stop bgm",  NULL, 0, NULL);
    mciSendString("close bgm", NULL, 0, NULL);
    bgmStarted = false;
}

void playSound(const char* file) {
    mciSendString("close sfx", NULL, 0, NULL);
    char cmd[512];
    sprintf(cmd, "open \"%s\" alias sfx", file);
    mciSendString(cmd, NULL, 0, NULL);
    mciSendString("play sfx",  NULL, 0, NULL);
}

void playSoundForce(const char* file) {
    mciSendString("close sfx2", NULL, 0, NULL);
    char cmd[512];
    sprintf(cmd, "open \"%s\" alias sfx2", file);
    mciSendString(cmd, NULL, 0, NULL);
    mciSendString("play sfx2",  NULL, 0, NULL);
}

#else
// Placeholder stubs for non-Windows (Linux / Mac)
void playBGM()  { bgmStarted = true; }
void stopBGM()  { bgmStarted = false; }
void playSound(const char*) {}
void playSoundForce(const char*) {}
#endif


// PER-LEVEL HIGHSCORE FILE SAVE / LOAD  (plain text file)

const char* HISCORE_FILE = "highscore.txt";

void refreshCurrentHiScore() {
    if (currentLevel >= 1 && currentLevel <= MAX_GAME_LEVELS)
        hiScore = hiScoreByLevel[currentLevel];
    else
        hiScore = 0;
}

void saveHiScore() {
    FILE* fp = fopen(HISCORE_FILE, "w");
    if (fp) {
        for (int lvl = 1; lvl <= MAX_GAME_LEVELS; lvl++) {
            fprintf(fp, "Level %d %d\n", lvl, hiScoreByLevel[lvl]);
        }
        fclose(fp);
    }
}

void loadHiScore() {
    for (int lvl = 1; lvl <= MAX_GAME_LEVELS; lvl++)
        hiScoreByLevel[lvl] = 0;

    FILE* fp = fopen(HISCORE_FILE, "r");
    if (fp) {
        char word[32];
        int lvl, value;

        while (fscanf(fp, "%31s", word) == 1) {
            if (strcmp(word, "Level") == 0) {
                if (fscanf(fp, "%d %d", &lvl, &value) == 2) {
                    if (lvl >= 1 && lvl <= MAX_GAME_LEVELS)
                        hiScoreByLevel[lvl] = value;
                }
            } else {
                value = atoi(word);
                hiScoreByLevel[1] = value;
                break;
            }
        }

        fclose(fp);
    }

    refreshCurrentHiScore();
}

void updateHiScoreForCurrentLevel() {
    if (currentLevel >= 1 && currentLevel <= MAX_GAME_LEVELS) {
        if (levelScore > hiScoreByLevel[currentLevel]) {
            hiScoreByLevel[currentLevel] = levelScore;
            hiScore = hiScoreByLevel[currentLevel];
            saveHiScore();
        } else {
            refreshCurrentHiScore();
        }
    }
}

void resetLevelScoreOnly() {
    levelScore = 0;
    refreshCurrentHiScore();
}

void resetFullScoreForNewGame() {
    score = 0;
    levelScore = 0;
    refreshCurrentHiScore();
}

// COLLISION CONSTANTS
const float CHAR_HB_W = 0.80f;    // character hitbox width
const float CHAR_HB_H = 1.00f;    // character hitbox heigh

// Per-type obstacle hitbox sizes
void getObstacleHitbox(int otype, int lvl, float& w, float& h) {
    if (lvl == 2 || lvl == 4) {
        w = 0.9f;  h = 0.9f;
    } else {
        switch(otype) {
            case 0: w = 0.9f; h = 0.9f; break;
            case 1: w = 0.8f; h = 0.7f; break;
            case 2: w = 0.8f; h = 0.8f; break;
            default: w = 0.9f; h = 0.9f; break;
        }
    }
}


// functions use (x, y) as the visual centre,
bool centerAABB(float ax, float ay, float aw, float ah,
                float bx, float by, float bw, float bh)
{
    return fabs(ax - bx) <= (aw * 0.5f + bw * 0.5f) &&
           fabs(ay - by) <= (ah * 0.5f + bh * 0.5f);
}

void finishGameOverIfNeeded()
{
    if (lives <= 0 && !showEndScreen && !gamePaused) {
        showEndScreen = true;
        gameWon = false;
        moving = false;
        jumping = false;
        isInvincible  = false;
        invincTimer   = 0.0f;
        isFlashing    = false;
        hitFlashTimer = 0.0f;
        playSoundForce(SND_GAMEOVER);
        updateHiScoreForCurrentLevel();
        saveHiScore();
    }
}

void damagePlayer()
{
    // Only the short cooldown controls repeated damage.
    if (!isInvincible && !showHome && !showEndScreen && !gamePaused && lives > 0) {
        lives--;
        isInvincible  = true;
        invincTimer   = 0.0f;
        isFlashing    = true;
        hitFlashTimer = HIT_FLASH_DURATION;
        playSound(SND_HIT);
        finishGameOverIfNeeded();
    }
}

// ---- OBSTACLE RESPAWN -----
int obstacleRespawnTimer[TOTAL_OBSTACLES] = {0};
const int RESPAWN_INTERVAL = 400;                  // frames before respawn

bool level2Started = false;
bool level1Started = false;

// Position
float obstaclePosX[TOTAL_OBSTACLES];
float obstaclePosY[TOTAL_OBSTACLES];

// Active & level
bool obstacleEnabled[TOTAL_OBSTACLES];
int obstacleLevel[TOTAL_OBSTACLES];
bool obstacleHit[TOTAL_OBSTACLES] = {false};

// Movement
float obstacleMoveSpeedX[TOTAL_OBSTACLES];
int obstacleMoveDirX[TOTAL_OBSTACLES];  // 1 = right, -1 = left
float obstacleLeftBound[TOTAL_OBSTACLES];
float obstacleRightBound[TOTAL_OBSTACLES];

float obstacleMoveSpeedY[TOTAL_OBSTACLES];
int obstacleMoveDirY[TOTAL_OBSTACLES];  // 1 = up, -1 = down
float obstacleTopBound[TOTAL_OBSTACLES];
float obstacleBottomBound[TOTAL_OBSTACLES];
int obstacleType[TOTAL_OBSTACLES];

// Size for collision
float obstacleWidth  = 2.0f;
float obstacleHeight = 2.0f;

bool fireHit[TOTAL_FIRES];
float fireX[TOTAL_FIRES];
float fireY[TOTAL_FIRES];

bool  fireOn[TOTAL_FIRES];
float fireTimer[TOTAL_FIRES];

const float FIRE_ON_TIME  = 60.0f;   // frames (1 sec)
const float FIRE_OFF_TIME = 60.0f;

// SignBoard (per obstacle) rotation
float signAngle[TOTAL_OBSTACLES];
float signRotateSpeed[TOTAL_OBSTACLES];

float stoneX[TOTAL_STONES];
float stoneY[TOTAL_STONES];

// drawStone original size 2-7 range
const float STONE_SCALE = 0.18f;
const float STONE_W = 1.0f;   // width in world units
const float STONE_H = 1.0f;   // height in world





void respawnObstacle(int i) {
    // Pick a new random world X away from the player
    float newX;
    int tries = 0;
    do {
        int si = rand() % totalScenes;
        float localX = -3.0f + (rand() / (float)RAND_MAX) * 12.0f;
        newX = si * sceneWidth + localX;
        tries++;
    } while (fabs(newX + worldX - charX - 4.75f) < 6.0f && tries < 50);

    obstaclePosX[i] = newX;
    obstaclePosY[i] = -6.0f + (rand() / (float)RAND_MAX) * 1.5f;
    obstacleHit[i]  = false;
    obstacleType[i] = rand() % 3;

    // Random movement after respawn
    obstacleMoveSpeedX[i] = (rand() % 2 == 0) ? (0.025f + (rand() / (float)RAND_MAX) * 0.035f) : 0.0f;
    obstacleMoveDirX[i]   = (rand() % 2 == 0) ? 1 : -1;
    obstacleLeftBound[i]  = newX - 5.0f;
    obstacleRightBound[i] = newX + 5.0f;

    obstacleMoveSpeedY[i] = (rand() % 2 == 0) ? (0.015f + (rand() / (float)RAND_MAX) * 0.030f) : 0.0f;
    obstacleMoveDirY[i]   = (rand() % 2 == 0) ? 1 : -1;
    obstacleTopBound[i]   = -3.8f;
    obstacleBottomBound[i]= -7.2f;

    signRotateSpeed[i] = 2.0f + (rand() / (float)RAND_MAX) * 4.0f;
    obstacleRespawnTimer[i] = 0;
    obstacleLevel[i] = currentLevel;
    obstacleEnabled[i] = true;
}

// AUTO-ADVANCE LEVEL

void advanceToNextLevel() {
    playSoundForce(SND_LEVELUP);
    saveHiScore();
    int nextLvl = currentLevel + 1;

    doracakesCollected = 0;
    for (int i = 0; i < 10; i++) doracakeCollected[i] = false;
    for (int i = 0; i < 3; i++)  heartLost[i] = false;
    for (int i = 0; i < TOTAL_OBSTACLES; i++) obstacleHit[i] = false;
    for (int i = 0; i < TOTAL_FIRES; i++) fireHit[i] = false;
    lives = totalHearts;

    charX = -9.5f;  charY = -7.5f;
    worldX = 0.0f;  sceneCount = 0;
    isInvincible = false;  isFlashing = false;
    invincTimer = 0.0f;
    hitFlashTimer = 0.0f;
    moving = false; jumping = false;

    currentLevel = nextLvl;
    resetLevelScoreOnly();
    level1Started = false;
    level2Started = false;
    level3Started = false;
    level4Started = false;
    level5Started = false;
}

void completeLevelAndReturnHome()
{
    // Saves current level score, then return to level selection.
    updateHiScoreForCurrentLevel();
    saveHiScore();
    playSoundForce(SND_LEVELUP);

    doracakesCollected = 0;
    for (int i = 0; i < 10; i++) doracakeCollected[i] = false;
    for (int i = 0; i < 3; i++)  heartLost[i] = false;
    for (int i = 0; i < TOTAL_OBSTACLES; i++) obstacleHit[i] = false;
    for (int i = 0; i < TOTAL_FIRES; i++) fireHit[i] = false;
    for (int i = 0; i < ROCK_COUNT; i++) rockIsHit[i] = false;
    ghostIsHit = false;

    lives = totalHearts;
    charX = -9.5f;
    charY = -7.5f;
    worldX = 0.0f;
    sceneCount = 0;

    moving = false;
    jumping = false;
    isInvincible = false;
    invincTimer = 0.0f;
    isFlashing = false;
    hitFlashTimer = 0.0f;
    gamePaused = false;

// Reset level-start for fresh start

    level1Started = false;
    level2Started = false;
    level3Started = false;
    level4Started = false;
    level5Started = false;

// Reset level score only, keep total score and save high scores.
    levelScore = 0;
    refreshCurrentHiScore();

    showEndScreen = false;
    gameWon = false;
    showHome = true;
}

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void drawButton(float x, float y, float w, float h, const char* label)
{
    glColor3f(0.2f, 0.4f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y - h);
        glVertex2f(x, y - h);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y - h);
        glVertex2f(x, y - h);
    glEnd();

    glLineWidth(1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    int len = strlen(label);
    float charWidth = 0.10f;
    float textW = len * charWidth;
    float textX = x + (w / 2.0f) - textW;     // center the text
    float textY = y - (h / 1.5f);
    drawText(textX, textY, label);
}


// keyPress() can calls update()
void update(int value);


// ------ NEW FEATURE FUNCTIONS


// Pause overlay ---------------

void drawPauseScreen() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dark overlay
    glColor4f(0.0f, 0.0f, 0.1f, 0.65f);
    glBegin(GL_QUADS);
        glVertex2f(-9.5f, -10.0f); glVertex2f(9.5f, -10.0f);
        glVertex2f(9.5f,  10.0f);  glVertex2f(-9.5f, 10.0f);
    glEnd();

    // Pause box
    glColor4f(0.1f, 0.1f, 0.3f, 0.95f);
    glBegin(GL_QUADS);
        glVertex2f(-5.0f, -4.5f); glVertex2f(5.0f, -4.5f);
        glVertex2f(5.0f,   4.0f); glVertex2f(-5.0f, 4.0f);
    glEnd();

    // Box border
    glColor3f(0.4f, 0.7f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-5.0f, -4.5f); glVertex2f(5.0f, -4.5f);
        glVertex2f(5.0f,   4.0f); glVertex2f(-5.0f, 4.0f);
    glEnd();
    glLineWidth(1.0f);

    // Title
    glColor3f(1.0f, 0.9f, 0.2f);
    drawText(-1.8f, 3.0f, "~ PAUSED ~");

    // Score
    char buf[64];
    sprintf(buf, "Score: %d", score);
    glColor3f(0.7f, 1.0f, 0.7f);
    drawText(-1.2f, 2.0f, buf);

    // -- RESUME button ---
    glColor3f(0.1f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(-4.0f,  1.2f); glVertex2f(4.0f,  1.2f);
        glVertex2f(4.0f,   0.3f); glVertex2f(-4.0f, 0.3f);
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-4.0f,  1.2f); glVertex2f(4.0f,  1.2f);
        glVertex2f(4.0f,   0.3f); glVertex2f(-4.0f, 0.3f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-1.8f, 0.55f, "Resume  (P)");

    // -- RESTART button --
    glColor3f(0.2f, 0.4f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(-4.0f, -0.0f); glVertex2f(4.0f, -0.0f);
        glVertex2f(4.0f,  -0.9f); glVertex2f(-4.0f,-0.9f);
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-4.0f, -0.0f); glVertex2f(4.0f, -0.0f);
        glVertex2f(4.0f,  -0.9f); glVertex2f(-4.0f,-0.9f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-1.2f, -0.6f, "Restart");

    // -- MAIN MENU button --
    glColor3f(0.2f, 0.4f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(-4.0f, -1.2f); glVertex2f(4.0f, -1.2f);
        glVertex2f(4.0f,  -2.1f); glVertex2f(-4.0f,-2.1f);
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-4.0f, -1.2f); glVertex2f(4.0f, -1.2f);
        glVertex2f(4.0f,  -2.1f); glVertex2f(-4.0f,-2.1f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-1.8f, -1.75f, "Main Menu");

    // Hint text
    glColor3f(0.6f, 0.75f, 1.0f);
    drawText(-3.5f, -3.5f, "Click a button or press P to resume");

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}
// ---------------- cloud ---------
void drawCloud(float cx, float cy, float sc) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float rx[] = {1.4f, 1.0f, 0.85f};
    float ry[] = {0.55f, 0.75f, 0.50f};
    float ox[] = {0.0f, -1.0f * sc, 1.0f * sc};
    float oy[] = {0.0f,  0.2f * sc, 0.15f * sc};
    for (int c = 0; c < 3; c++) {
        glBegin(GL_POLYGON);
        glColor4f(1.0f, 1.0f, 1.0f, 0.82f);
        for (int i = 0; i < 60; i++) {
            float a = i * 2.0f * 3.14159f / 60;
            glVertex2f(cx + ox[c] + rx[c] * sc * cosf(a),
                       cy + oy[c] + ry[c] * sc * sinf(a));
        }
        glEnd();
    }
    glDisable(GL_BLEND);
}

void updateClouds() {
    for (int i = 0; i < TOTAL_CLOUDS; i++) {
        cloudX[i] -= cloudSpd;
        if (cloudX[i] < -12.0f)
            cloudX[i] = 20.0f;
    }
}

// ------------- Ground strip ----------
void drawGroundStrip(float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(-9.5f, -7.7f);
        glVertex2f( 9.5f, -7.7f);
        glVertex2f( 9.5f, -8.3f);
        glVertex2f(-9.5f, -8.3f);
    glEnd();
    glColor3f(r * 0.6f, g * 0.6f, b * 0.6f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-9.5f, -7.7f);
        glVertex2f( 9.5f, -7.7f);
    glEnd();
    glLineWidth(1.0f);
}

// ----- Hit-flash overlay ----------

void drawHitFlash() {
    if (!isFlashing) return;
    float alpha = (hitFlashTimer / HIT_FLASH_DURATION) * 0.55f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.1f, 0.1f, alpha);
    glBegin(GL_QUADS);
        glVertex2f(-9.5f, -10.0f); glVertex2f(9.5f, -10.0f);
        glVertex2f(9.5f,  10.0f);  glVertex2f(-9.5f, 10.0f);
    glEnd();
    glDisable(GL_BLEND);
}

// ------- Keyboard handler (Pause = P) ---------------

void keyPress(unsigned char key, int x, int y) {
    if (key == 'p' || key == 'P') {
        if (showHome || showEndScreen) return;
        gamePaused = !gamePaused;
        glutPostRedisplay();
        if (!gamePaused)
            glutTimerFunc(16, update, 0);
        return;
    }
    if (showHome || showEndScreen) return;
}


//Doracake

void drawEllipse(float cx, float cy, float rx, float ry, float r, float g, float b)
{
    int segments = 100;
    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    for(int i = 0; i < segments; i++)
    {
        float theta = 2.0f * 3.1416f * i / segments;
        glVertex2f(cx + rx*cos(theta), cy + ry*sin(theta));
    }
    glEnd();

    glLineWidth(1);
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < segments; i++)
    {
        float theta = 2.0f * 3.1416f * i / segments;
        glVertex2f(cx + rx*cos(theta), cy + ry*sin(theta));
    }
    glEnd();
}
void Doracake()
{
    float d1_cx = (-1.50f + 1.16f)/2;
    float d1_cy = (-0.72f + 0.89f)/2;
    float d1_rx = (1.53f - (-1.50f))/2;
    float d1_ry = (0.19f - (-0.72f))/2;
    drawEllipse(d1_cx, d1_cy, d1_rx, d1_ry, 0.55f, 0.3f, 0.2f);

    float c2_cx = (-1.68f + 1.32f)/2;
    float c2_cy = (-0.46f + 0.85f)/2;
    float c2_rx = (1.52f - (-1.68f))/2;
    float c2_ry = (0.43f - (-0.46f))/2;
    drawEllipse(c2_cx, c2_cy, c2_rx, c2_ry, 0.96f, 0.70f, 0.45f);

    float c_cx = (-1.88f + 1.59f)/2;
    float c_cy = (-0.14f + 0.76f)/2;
    float c_rx = (1.59f - (-1.88f))/2;
    float c_ry = (0.76f - (-0.14f))/2;
    drawEllipse(c_cx, c_cy, c_rx, c_ry, 0.90f, 0.64f, 0.38f);

    float d_cx = (-1.69f + 1.30f)/2;
    float d_cy = (0.28f + 0.6f)/2;
    float d_rx = (1.30f - (-1.69f))/2;
    float d_ry = (1.20f - 0.28f)/2;
    drawEllipse(d_cx, d_cy, d_rx, d_ry, 0.55f, 0.3f, 0.2f);
    glEnd();

    glFlush();
}

void homeScreen()
{

    // Title
    glColor3f(0.1f, 0.1f, 0.1f);
    drawText(-2.5f, 6.0f, "DORAEMON'S TREAT HUNT");

    glPushMatrix();
        glTranslatef(-6.0f, 3.0f, 0.0f);
        glScalef(0.8f, 1.5f, 0.5f);
        Doracake();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-2.5f, -4.0f, 0.0f);
         glScalef(0.8f, 1.5f, 0.5f);
        Doracake();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(3.0f, 3.5f, 0.0f);
         glScalef(0.8f, 1.5f, 0.5f);
        Doracake();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(6.0f, -4.2f, 0.0f);
         glScalef(0.8f, 1.5f, 0.5f);
        Doracake();
    glPopMatrix();


    // Buttons
    drawButton(-2.0f, 3.0f, 4.0f, 1.2f, "LEVEL 1");
    drawButton(-2.0f, 1.0f, 4.0f, 1.2f, "LEVEL 2");
    drawButton(-2.0f, -1.0f, 4.0f, 1.2f, "LEVEL 3");
    drawButton(-2.0f, -3.0f, 4.0f, 1.2f, "LEVEL 4");
    drawButton(-2.0f, -5.0f, 4.0f, 1.2f, "LEVEL 5");

    glFlush();

    glutSwapBuffers();

}


void drawEndScreen() {
    if (!showEndScreen) return;

    // Dark semi-transparent background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(-6.0f, -5.0f); glVertex2f(6.0f, -5.0f);
        glVertex2f(6.0f,  4.5f);  glVertex2f(-6.0f, 4.5f);
    glEnd();

    // Border
    glColor4f(0.4f, 0.7f, 1.0f, 0.9f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-6.0f, -5.0f); glVertex2f(6.0f, -5.0f);
        glVertex2f(6.0f,  4.5f);  glVertex2f(-6.0f, 4.5f);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    // Message
    if (gameWon && currentLevel >= 5) {
        glColor3f(1.0f, 0.85f, 0.1f);
        drawText(-3.2f, 3.5f, "ALL LEVELS CLEARED!");
    } else if (gameWon) {
        glColor3f(0.3f, 1.0f, 0.4f);
        drawText(-1.5f, 3.5f, "YOU WIN!");
    } else {
        glColor3f(1.0f, 0.3f, 0.3f);
        drawText(-1.8f, 3.5f, "GAME OVER");
    }

    // Score display
    char scoreBuf[64];
    sprintf(scoreBuf, "Score: %d", score);
    glColor3f(1.0f, 0.9f, 0.3f);
    drawText(-1.5f, 2.5f, scoreBuf);

    char hiBuf[64];
    sprintf(hiBuf, "Level Best: %d", hiScore);
    glColor3f(0.6f, 1.0f, 0.6f);
    drawText(-1.5f, 1.6f, hiBuf);

    // Next Level button if won ,levels 1-4
    if (gameWon && currentLevel < 5) {
        glColor3f(0.1f, 0.6f, 0.2f);
        glBegin(GL_QUADS);
            glVertex2f(-4.5f, 0.8f);
            glVertex2f(4.5f,  0.8f);
            glVertex2f(4.5f,  -0.1f);
            glVertex2f(-4.5f, -0.1f);
        glEnd();
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(-1.5f, 0.2f, "Next Level");

        // update button bounds for click detection
        nextLevelX = -4.5f;
        nextLevelY = 0.8f;
        buttonWidth  = 9.0f;
        buttonHeight = 0.9f;
    }

    // Main Menu button
    glColor3f(0.2f, 0.4f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(-4.5f, -0.4f);
        glVertex2f(4.5f,  -0.4f);
        glVertex2f(4.5f,  -1.3f);
        glVertex2f(-4.5f, -1.3f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-1.5f, -0.9f, "Main Menu");

    // Restart button
    glColor3f(0.2f, 0.4f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(-4.5f, -1.6f);
        glVertex2f(4.5f,  -1.6f);
        glVertex2f(4.5f,  -2.5f);
        glVertex2f(-4.5f, -2.5f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-1.0f, -2.1f, "Restart");

    glLineWidth(1.0f);
}

void drawSky1()
   {
        glDisable(GL_LIGHTING);

        glBegin(GL_QUADS);


        glColor3f(0.0f, 0.4f, 0.8f);
        glVertex2f(-7.0, 7.0);
        glVertex2f(7.0, 7.0);


        glColor3f(0.6f, 0.8f, 1.0f);
         glVertex2f(7.0f, 0.0f);
        glVertex2f(-7.0f, 0.0f);


        glColor3f(0.4f, 0.7f, 0.992f);
        glVertex2f(-7.0f, -7.0f);
        glVertex2f(7.0f, -7.0f);

        glEnd();

        glEnable(GL_LIGHTING);

   }
void drawSky2()
{   //glDisable(GL_LIGHTING);

    // TOP QUAD
    glBegin(GL_QUADS);
    glColor3f(0.36f, 0.41f, 0.53f); // cool blue-gray
    glVertex2f(0.0f, 20.0f);
    glVertex2f(18.0f, 20.0f);
    glColor3f(0.47f, 0.45f, 0.55f); // upper-mid
    glVertex2f(18.0f, 18.0f);
    glVertex2f(0.0f, 18.0f);
   // glVertex2f(18.0f, 17.0f);
   // glVertex2f(0.0f, 17.0f);
    glEnd();


    // MIDDLE QUAD
    glBegin(GL_QUADS);
    glColor3f(0.47f, 0.45f, 0.55f); // upper-mid
   // glVertex2f(0.0f, 17.0f);
   // glVertex2f(18.0f, 17.0f);
    glVertex2f(0.0f, 18.0f);
    glVertex2f(18.0f, 18.0f);
    glColor3f(0.67f, 0.55f, 0.50f); // lower-mid
    glVertex2f(18.0f, 13.5f);
    glVertex2f(0.0f, 13.5f);
    glEnd();


    // BOTTOM QUAD
    glBegin(GL_QUADS);
    glColor3f(0.67f, 0.55f, 0.50f); // lower-mid
    glVertex2f(0.0f, 13.5f);
    glVertex2f(18.0f, 13.5f);
    glColor3f(0.93f, 0.58f, 0.38f); // wbody sunset
    glVertex2f(18.0f, 10.0f);
    glVertex2f(0.0f, 10.0f);
    glEnd();
    glFlush();
    //glEnable(GL_LIGHTING);
}
void drawSky3()
{
    // TOP QUAD
    glBegin(GL_QUADS);
    glColor3f(0.02f, 0.02f, 0.10f); // dark blue
    glVertex2f(0.0f, 20.0f);
    glVertex2f(18.0f, 20.0f);
    glColor3f(0.05f, 0.05f, 0.2f); // upper-mid
    glVertex2f(18.0f, 18.0f);
    glVertex2f(0.0f, 18.0f);
   // glVertex2f(18.0f, 17.0f);
   // glVertex2f(0.0f, 17.0f);
    glEnd();

    // MIDDLE QUAD
    glBegin(GL_QUADS);
    glColor3f(0.05f, 0.05f, 0.2f); // dark navy
   // glVertex2f(0.0f, 17.0f);
   // glVertex2f(18.0f, 17.0f);
    glVertex2f(0.0f, 18.0f);
    glVertex2f(18.0f, 18.0f);
    glColor3f(0.1f, 0.08f, 0.3f); // lower-mid
    glVertex2f(18.0f, 13.5f);
    glVertex2f(0.0f, 13.5f);
    glEnd();

    // BOTTOM QUAD
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.08f, 0.3f); // bluish purple
    glVertex2f(0.0f, 13.5f);
    glVertex2f(18.0f, 13.5f);
    glColor3f(0.2f, 0.12f, 0.4f); // faint purple
    glVertex2f(18.0f, 10.0f);
    glVertex2f(0.0f, 10.0f);
    glEnd();
    glFlush();
}


void Doraemon()
{
    //NOSE
    glLineWidth(3);
    glBegin(GL_POLYGON);

    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 0.0, 0.0);
        float pi = 3.1416f;
        float A = (i * 2 * pi) / 300.0f;
        float r = 0.33f;

        float x = 7.53 + r * cos(A);
        float y = 8.39 + r * sin(A);

        glVertex2f(x, y);
    }
    glEnd();

    glLineWidth(3);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);

    for(int i = 0; i < 300; i++)
    {
        float pi = 3.1416f;
        float A = (i * 2 * pi) / 300.0f;
        float r = 0.33f;

        float x = 7.53 + r * cos(A);
        float y = 8.39 + r * sin(A);

        glVertex2f(x, y);
    }


    glEnd();
    //HEAD
    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0); // white fill
        float pi = 3.1416;
        float A = (i * pi) / 300; // half circle
        float r = 2.1;
        float x = 5.46 + r * cos(A);
        float y = 7.42 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();



    // Black border on curved edge
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(int i = 20; i <= 90; i++)   // 300 steps (20–90) mapped
    {
        float pi = 3.1416f;
        float A = (i * pi) / 300.0f;

        float r = 2.1f;
        float x = 5.46f + r * cos(A);
        float y = 7.42f + r * sin(A);

        glVertex2f(x, y);
    }
    glEnd();

    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(0.6,0.8,0.9);
        // white fill
        float pi = 3.1416;
        float A = (i *2* pi) / 300;    //  half circle
        float r = 0.86;
        float x = 4.88 + r * cos(A);
        float y = 6.3 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();

    glEnd();



    glBegin(GL_POLYGON);

    float angleOfRotationNew = 60.0f;
    float rotationAngleNew = angleOfRotationNew * (3.1416f / 180.0f);

    float txNew = 0.2f;
    float tyNew = -0.03f;

    float cxNew = 5.24f;
    float cyNew = 7.45f;

    float rNew = 2.1f;

    for (int i = 0; i < 300; i++) {

        glColor3f(0.6,0.8,0.9);

        float pi = 3.1416f;
        float theta = (i * pi) / 300.0f;

        float x = rNew * cos(theta);
        float y = rNew * sin(theta);

        // Rotate
        float x_rot = x * cos(rotationAngleNew) - y * sin(rotationAngleNew);
        float y_rot = x * sin(rotationAngleNew) + y * cos(rotationAngleNew);

        // Translate + center
        float x_final = x_rot + cxNew + txNew;
        float y_final = y_rot + cyNew + tyNew;

        glVertex2f(x_final, y_final);
    }

    glEnd();

    // Black border
    glLineWidth(3);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);

    for (int i = 0; i < 300; i++) {

        float pi = 3.1416f;
        float theta = (i * pi) / 300.0f;

        float x = rNew * cos(theta);
        float y = rNew * sin(theta);

        float x_rot = x * cos(rotationAngleNew) - y * sin(rotationAngleNew);
        float y_rot = x * sin(rotationAngleNew) + y * cos(rotationAngleNew);

        float x_final = x_rot + cxNew + txNew;
        float y_final = y_rot + cyNew + tyNew;

        glVertex2f(x_final, y_final);
    }

    glEnd();

    //HAND(ROUND)
    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i=0;i<300;i++)
    {
        glColor3f(1.0,1.0,1.0);
        float pi=3.1416;
        float A=(i*2*pi)/300;
        float r=0.43;
        float x = 3.2+r * cos(A);
        float y =5.71+ r * sin(A);
        glVertex2f(x,y );
    }
    glEnd();

    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<500;i++)
    {
        glColor3f(0.0,0.0,0.0);
        float pi=3.1416;
        float A=(i*2*pi)/500;
        float r=0.43;
        float x = 3.2+r * cos(A);
        float y =5.71+ r * sin(A);
        glVertex2f(x,y );
    }
    glEnd();

    //hand(blue)
    glLineWidth(4.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(5.12f, 5.3f);
    glVertex2f(4.01f, 5.78f);
    glVertex2f(3.78f, 5.82f);
    glVertex2f(3.61f, 5.81f);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex2f(3.43f, 5.39f);
    glVertex2f(3.55f, 5.28f);
    glVertex2f(3.65f, 5.19f);
    glVertex2f(3.8f,  5.08f);
    glVertex2f(3.93f, 4.99f);
    glVertex2f(4.09f, 4.89f);
    glVertex2f(4.26f, 4.78f);
    glVertex2f(4.43f, 4.67f);
    glEnd();

    glColor3f(0.6f, 0.8f, 0.9f);
    glBegin(GL_POLYGON);
    glVertex2f(5.12f,5.3f);
    glVertex2f(4.01f,5.78f);
    glVertex2f(3.78f,5.82f);
    glVertex2f(3.61f,5.81f);
    glVertex2f(3.43f,5.39f);
    glVertex2f(3.55f,5.28f);
    glVertex2f(3.65f,5.19f);
    glVertex2f(3.8f,5.08f);
    glVertex2f(3.93f,4.99f);
    glVertex2f(4.09f,4.89f);
    glVertex2f(4.26f,4.78f);
    glVertex2f(4.43f,4.67f);
    glEnd();


    //HEADWHITE

    glBegin(GL_POLYGON);

    float angleOfRotation = 73.5f;
    float rotationAngle = angleOfRotation * (3.1416 / 180.0f);

    float tx = -0.134f;
    float ty = -0.272f;

    float cx = 6.7f;
    float cy = 6.99f;

    for (int i = 0; i < 300; i++) {

        glColor3f(1.0f, 1.0f, 1.0f);

        float pi = 3.1416f;
        float A = (i * pi) / 300.0f;
        float r = 1.95f;

        float x = r * cos(A);
        float y = r * sin(A);

        float x_rot = x * cos(rotationAngle) - y * sin(rotationAngle);
        float y_rot = x * sin(rotationAngle) + y * cos(rotationAngle);

        float x_final = x_rot + cx + tx;
        float y_final = y_rot + cy + ty;

        glVertex2f(x_final, y_final);
    }

    glEnd();

    glLineWidth(3);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);

    for (int i = 0; i < 300; i++) {

        float pi = 3.1416f;
        float A = (i * pi) / 300.0f;
        float r = 1.95f;

        float x = r * cos(A);
        float y = r * sin(A);

        float x_rot = x * cos(rotationAngle) - y * sin(rotationAngle);
        float y_rot = x * sin(rotationAngle) + y * cos(rotationAngle);

        float x_final = x_rot + cx + tx;
        float y_final = y_rot + cy + ty;

        glVertex2f(x_final, y_final);
    }

    glEnd();



    //lower lips line

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(6.99f, 5.75f);
    glVertex2f(6.98f,5.83f);
    glVertex2f(6.98f,5.83f);
    glVertex2f(7.08f,5.95f);
    glVertex2f(7.08f,5.95f);
    glVertex2f(7.18,6.05f);
    glVertex2f(7.18,6.05f);
    glVertex2f(7.28f,6.16f);
    glVertex2f(7.28f,6.16f);
    glVertex2f(7.30f,6.18f);
    glVertex2f(7.30f,6.18f);
    glVertex2f(7.31f,6.23f);

    glEnd();

    //lower lips empty space
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(7.28f, 6.16f);
    glVertex2f(7.03f,6.11f);
    glVertex2f(6.93f,6.12f);
    glVertex2f(6.21f,5.67f);
    // glVertex2f(6.97f,5.75f);
    //  glVertex2f(6.90f,5.82f);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(6.21f,5.67f);
    glVertex2f(6.99f,5.75f);
    glVertex2f(6.92f,5.82f);
    glVertex2f(7.30f,6.18f);

    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(7.24f, 6.27f);
    glVertex2f(7.07f,6.28f);
    glVertex2f(6.93f,6.12f);
    glVertex2f(7.03f,6.11f);
    glVertex2f(7.28f,6.16f);

    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(6.21f, 5.67f);
    glVertex2f(6.8f, 6.2f);
    glVertex2f(6.16f,6.94f);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(6.8f, 6.2f);
    glVertex2f(6.21f, 5.67f);
    glVertex2f(7.19f,6.28f);
    glEnd();

    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0); // white fill
        float pi = 3.1416;
        float A = (i * pi) / 300; // half circle
        float r = 0.09;
        float x = 7.2 + r * cos(A);
        float y = 6.2 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();
    glLineWidth(3);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < 300; i++)
    {
        float pi = 3.1416;
        float A = (i * pi) / 300; // only half circle
        float r = 0.09;
        float x = 7.2 + r * cos(A);
        float y = 6.2 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();

    //lower lips line

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(6.99f, 5.75f);
    glVertex2f(6.98f,5.83f);
    glVertex2f(6.98f,5.83f);
    glVertex2f(7.08f,5.95f);
    glVertex2f(7.08f,5.95f);
    glVertex2f(7.18,6.05f);
    glVertex2f(7.18,6.05f);
    glVertex2f(7.28f,6.16f);
    glVertex2f(7.28f,6.16f);
    glVertex2f(7.30f,6.18f);
    glVertex2f(7.30f,6.18f);
    glVertex2f(7.31f,6.23f);

    glEnd();

    //nicher thot er khali jyga
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(7.28f, 6.16f);
    glVertex2f(7.03f,6.11f);
    glVertex2f(6.93f,6.12f);
    glVertex2f(6.21f,5.67f);

    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(6.21f,5.67f);
    glVertex2f(6.99f,5.75f);
    glVertex2f(6.92f,5.82f);
    glVertex2f(7.30f,6.18f);

    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(7.24f, 6.27f);
    glVertex2f(7.07f,6.28f);
    glVertex2f(6.93f,6.12f);
    glVertex2f(7.03f,6.11f);
    glVertex2f(7.28f,6.16f);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(6.21f, 5.67f);
    glVertex2f(6.8f, 6.2f);
    glVertex2f(6.16f,6.94f);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(6.8f, 6.2f);
    glVertex2f(6.21f, 5.67f);
    glVertex2f(7.19f,6.28f);
    glEnd();


    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0); // white fill
        float pi = 3.1416;
        float A = (i * pi) / 300; // half circle
        float r = 0.09;
        float x = 7.2 + r * cos(A);
        float y = 6.2 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();
    glLineWidth(3);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < 300; i++)
    {
        float pi = 3.1416;
        float A = (i * pi) / 300; // only half circle
        float r = 0.09;
        float x = 7.2 + r * cos(A);
        float y = 6.2 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();


    //SMILE RED HALF CIRCLE

    glBegin(GL_POLYGON);

    float angleOfRotationH1 = 160.0;
    float rotationAngleH1 = angleOfRotationH1 * (3.1416f / 180.0f);

    float txH1 = -0.225f;
    float tyH1 = -0.10f;


    float cxH1 = 6.78f;
    float cyH1 = 7.39f;

    float rH1 = 0.76f;

    for (int i = 0; i < 300; i++) {

        glColor3f(1.0f, 0.0f, 0.0f);

        float pi = 3.1416f;
        float AH1 = (i * pi) / 300.0f;

        float xH1 = rH1 * cos(AH1);
        float yH1 = rH1 * sin(AH1);

        // Rotate
        float xH1_rot = xH1 * cos(rotationAngleH1) - yH1 * sin(rotationAngleH1);
        float yH1_rot = xH1 * sin(rotationAngleH1) + yH1 * cos(rotationAngleH1);

        // Translate + center
        float xH1_final = xH1_rot + cxH1 + txH1;
        float yH1_final = yH1_rot + cyH1 + tyH1;

        glVertex2f(xH1_final, yH1_final);
    }

    glEnd();
    //HALF RED BORDER
    glLineWidth(3);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);

    for (int i = 0; i < 300; i++) {

        float pi = 3.1416f;
        float AH1 = (i * pi) / 300.0f;

        float xH1 = rH1 * cos(AH1);
        float yH1 = rH1 * sin(AH1);

        float xH1_rot = xH1 * cos(rotationAngleH1) - yH1 * sin(rotationAngleH1);
        float yH1_rot = xH1 * sin(rotationAngleH1) + yH1 * cos(rotationAngleH1);

        float xH1_final = xH1_rot + cxH1 + txH1;
        float yH1_final = yH1_rot + cyH1 + tyH1;

        glVertex2f(xH1_final, yH1_final);
    }
    glEnd();

    //MUKHER hasir edge border
    float x_start = rH1 * cos(0);
    float y_start = rH1 * sin(0);
    float x_end = rH1 * cos(3.1416f);
    float y_end = rH1 * sin(3.1416f);

    // Rotate
    float x_start_rot = x_start * cos(rotationAngleH1) - y_start * sin(rotationAngleH1);
    float y_start_rot = x_start * sin(rotationAngleH1) + y_start * cos(rotationAngleH1);

    float x_end_rot = x_end * cos(rotationAngleH1) - y_end * sin(rotationAngleH1);
    float y_end_rot = x_end * sin(rotationAngleH1) + y_end * cos(rotationAngleH1);

    x_start_rot += cxH1 + txH1;
    y_start_rot += cyH1 + tyH1;

    x_end_rot += cxH1 + txH1;
    y_end_rot += cyH1 + tyH1;

    glBegin(GL_LINES);
    glVertex2f(x_start_rot, y_start_rot);
    glVertex2f(x_end_rot, y_end_rot);
    glEnd();

    float txT = 0.02f;
    float tyT = -0.15f;

    float angle = 5.0f;
    glPushMatrix();
    glTranslatef(txT, tyT, 0.0f);

    glTranslatef(cx, cy, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    glTranslatef(-cx, -cy, 0.0f);

    glBegin(GL_POLYGON);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(7.33f, 7.3f);
    glVertex2f(6.73f, 7.42f);
    glVertex2f(7.13f, 6.89f);
    glEnd();

    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(7.33f, 7.3f);
    glVertex2f(6.73f, 7.42f);
    glVertex2f(7.13f, 6.89f);
    glEnd();

    glPopMatrix();

    //UPPER lips
    glLineWidth(3);
    glBegin(GL_POLYGON);

    float rotAngle2_deg = -75.0f;
    float rotAngle2_rad = rotAngle2_deg * (3.1416f / 180.0f);


    float transX2 = -0.05f;
    float transY2 = -.19f;


    float centerX2 = 7.61f;
    float centerY2 = 7.74f;

    for (int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0);

        float pi = 3.1416f;
        float A = (i * pi) / 300.0f;
        float r = 0.33f;


        float ox = r * cos(A);
        float oy = r * sin(A);


        float rx = ox * cos(rotAngle2_rad) - oy * sin(rotAngle2_rad);
        float ry = ox * sin(rotAngle2_rad) + oy * cos(rotAngle2_rad);


        float finalX = rx + centerX2 + transX2;
        float finalY = ry + centerY2 + transY2;

        glVertex2f(finalX, finalY);
    }

    glEnd();

    //THOTER BORDER
    glLineWidth(3);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);

    for (int i = 0; i < 300; i++)
    {
        float pi = 3.1416f;
        float A = (i * pi) / 300.0f;
        float r = 0.33f;

        float ox = r * cos(A);
        float oy = r * sin(A);

        float rx = ox * cos(rotAngle2_rad) - oy * sin(rotAngle2_rad);
        float ry = ox * sin(rotAngle2_rad) + oy * cos(rotAngle2_rad);

        float finalX = rx + centerX2 + transX2;
        float finalY = ry + centerY2 + transY2;

        glVertex2f(finalX, finalY);
    }
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);

    glVertex2f(6.66f, 7.55f);
    glVertex2f(6.73f, 7.27f);
    glVertex2f(7.07f, 7.25f);
    glVertex2f(7.3f, 7.22f);
    glVertex2f(7.7f, 7.26f);
    glVertex2f(7.75f, 7.72f);

    glEnd();

    //EYE
    float I_x = 6.22, I_y = 9.29;
    float J_x = 7.08, J_y = 8.41;   // Point J major axis
    float K_x = 6.6, K_y = 8.55;  // Point K minor axis

    // major (vertical) , minor (horizontal)
    float a = sqrt(pow(J_x - I_x, 2) + pow(J_y - I_y, 2)); // semi-major
    float b = sqrt(pow(K_x - I_x, 2) + pow(K_y - I_y, 2)); // semi-minor

    float scale = 0.5;
    a *= scale;
    b *= scale;

    // ellipse center at I
    float centerX = I_x;
    float centerY = I_y;

    // TRANSLATION
    float tx1 = 0.325;
    float ty1 = -0.55;

    centerX += tx1;
    centerY += ty1;

    float angleDeg = 52.0;
    float phi = angleDeg * 3.1416 / 180.0;

    glColor3f(1.0, 1.0, 1.0);   // White fill
    glBegin(GL_POLYGON);

    for(int i = 0; i < 360; i++)
    {
        float theta = i * 3.1416 / 180.0;

        float x0 = b * cos(theta);
        float y0 = a * sin(theta);

        float xr = x0 * cos(phi) - y0 * sin(phi);
        float yr = x0 * sin(phi) + y0 * cos(phi);

        glVertex2f(centerX + xr, centerY + yr);
    }

    glEnd();

    // BLACK BORDER

    glColor3f(0, 0, 0);
    glLineWidth(2.0);
    glBegin(GL_LINE_LOOP);

    for(int i = 0; i < 360; i++)
    {
        float theta = i * 3.1416 / 180.0;

        float x0 = b * cos(theta);
        float y0 = a * sin(theta);

        float xr = x0 * cos(phi) - y0 * sin(phi);
        float yr = x0 * sin(phi) + y0 * cos(phi);

        glVertex2f(centerX + xr, centerY + yr);
    }
    glEnd();

    //black part
    float O1_x = 6.83f, O1_y = 8.81f;  // ellipse center
    float P1_x = 7.09f, P1_y = 8.48f;  // point on major axis
    float Q1_x = 6.96f, Q1_y = 8.50f;  // point on minor axis

    float a2 = sqrt(pow(P1_x - O1_x, 2) + pow(P1_y - O1_y, 2));  // semi-major
    float b2 = sqrt(pow(Q1_x - O1_x, 2) + pow(Q1_y - O1_y, 2));  // semi-minor

    float scale2 = 0.5f;
    a2 *= scale2;
    b2 *= scale2;


    // Translation
    float tx2 = -.7f;
    float ty2 = 0.81f;
    centerX2 += tx2;
    centerY2 += ty2;

    float angleDeg2 = 30.0f;
    float phi2 = angleDeg2 * 3.1416f / 180.0f;

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);

    for(int i = 0; i < 360; i++)
    {
        float theta = i * 3.1416f / 180.0f;

        float x0 = b2 * cos(theta);
        float y0 = a2 * sin(theta);

        float xr = x0 * cos(phi2) - y0 * sin(phi2);
        float yr = x0 * sin(phi2) + y0 * cos(phi2);

        glVertex2f(centerX2 + xr, centerY2 + yr);
    }

    glEnd();

    //CHOKHER white part

    float R3_x = 6.97f, R3_y = 8.69f;  // ellipse center reference
    float S3_x = 7.07f, S3_y = 8.54f;  // point on major axis
    float T3_x = 7.02f, T3_y = 8.57f;  // point on minor axis

    float a3 = sqrt(pow(S3_x - R3_x, 2) + pow(S3_y - R3_y, 2));  // semi-major
    float b3 = sqrt(pow(T3_x - R3_x, 2) + pow(T3_y - R3_y, 2));  // semi-minor

    float scale3 = 0.5f;
    a3 *= scale3;
    b3 *= scale3;

    float centerX3 = R3_x;
    float centerY3 = R3_y;

    float tx3 = -0.0099f;
    float ty3 = -0.179f;
    centerX3 += tx3;
    centerY3 += ty3;

    float angleDeg3 = 25.0f;
    float phi3 = angleDeg3 * 3.1416f / 180.0f;

    glColor3f(1.0f, 1.0f, 1.0f);  // white fill
    glBegin(GL_POLYGON);

    for(int i = 0; i < 360; i++)
    {
        float theta = i * 3.1416f / 180.0f;

        float x0 = b3 * cos(theta);
        float y0 = a3 * sin(theta);

        // rotate
        float xr = x0 * cos(phi3) - y0 * sin(phi3);
        float yr = x0 * sin(phi3) + y0 * cos(phi3);

        // translate + center
        glVertex2f(centerX3 + xr, centerY3 + yr);
    }

    glEnd();

    //muuchstac

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(5.56f, 8.26f);
    glVertex2f(6.39f, 7.98f);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(6.3f, 7.82f);
    glVertex2f(5.16f, 7.8579f);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(6.27f, 7.63f);
    glVertex2f(5.22f, 7.37f);
    glEnd();


    //mouth
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(7.665f, 7.229f);
    glVertex2f(7.396f, 7.20f);
    glVertex2f(7.396f, 7.20f);
    glVertex2f(7.147f, 7.21f);
    glVertex2f(7.147f, 7.21f);
    glVertex2f(7.13f, 7.23f);
    glVertex2f(7.13, 7.23f);
    glVertex2f(7.11f, 7.25f);
    glVertex2f(7.11f, 7.25f);
    glVertex2f(6.9f, 7.27f);
    glVertex2f(6.9f, 7.27f);
    glVertex2f(6.77, 7.28f);
    glVertex2f(6.77, 7.28f);
    glVertex2f(6.65f, 7.29f);
    glEnd();

    // RED SMILE FILL
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);

    //  Curve A (first)
    float x0_A = 6.20,    y0_A = 6.82;
    float x1_A = 6.67,    y1_A = 6.41;
    float x2_A = 7.18,    y2_A = 6.33;
    float txA  = -0.1f,   tyA  = -0.18f;

    for (float t = 0; t <= 1.0f; t += 0.01f) {
        float x = (1 - t)*(1 - t)*x0_A + 2*(1 - t)*t*x1_A + t*t*x2_A;
        float y = (1 - t)*(1 - t)*y0_A + 2*(1 - t)*t*y1_A + t*t*y2_A;
        glVertex2f(x + txA, y + tyA);
    }

    //Curve B (second)
    float x0_B = 7.12978f, y0_B = 6.89011f;
    float x1_B = 7.1489f,  y1_B = 6.60329f;
    float x2_B = 7.07242f, y2_B = 6.28461f;
    float txB  = 0.05f,    tyB  = -0.09f;

    for (float t = 0; t <= 1.0f; t += 0.01f) {
        float x = (1 - t)*(1 - t)*x0_B + 2*(1 - t)*t*x1_B + t*t*x2_B;
        float y = (1 - t)*(1 - t)*y0_B + 2*(1 - t)*t*y1_B + t*t*y2_B;
        glVertex2f(x + txB, y + tyB);
    }

    glEnd();


    // BLACK BORDER
    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);

    // Curve A border
    for (float t = 0; t <= 1.0f; t += 0.01f) {
        float x = (1 - t)*(1 - t)*x0_A + 2*(1 - t)*t*x1_A + t*t*x2_A;
        float y = (1 - t)*(1 - t)*y0_A + 2*(1 - t)*t*y1_A + t*t*y2_A;
        glVertex2f(x + txA, y + tyA);
    }

    // Curve B border
    for (float t = 0; t <= 1.0f; t += 0.01f) {
        float x = (1 - t)*(1 - t)*x0_B + 2*(1 - t)*t*x1_B + t*t*x2_B;
        float y = (1 - t)*(1 - t)*y0_B + 2*(1 - t)*t*y1_B + t*t*y2_B;
        glVertex2f(x + txB, y + tyB);
    }

    glEnd();

    //pet er nil color fill

    // right leg color

    glColor3f(0.6f, 0.8f, 0.9f);
    glBegin(GL_POLYGON);


    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
            (1 - t)*(1 - t)*5.56f +
            2*(1 - t)*t*5.45f +
            t*t*5.60f;

        float y =
            (1 - t)*(1 - t)*3.42f +
            2*(1 - t)*t*3.02f +
            t*t*2.59f;

        glVertex2f(x, y);
    }

    for(float t = 1.0f; t >= 0.0f; t -= 0.02f)
    {
        float x =
            (1 - t)*(1 - t)*7.14f +
            2*(1 - t)*t*7.29f +
            t*t*6.75f;

        float y =
            (1 - t)*(1 - t)*5.09f +
            2*(1 - t)*t*3.93f +
            t*t*2.59f;

        glVertex2f(x, y);
    }

    glEnd();

    //pet er kichu jayga
    glColor3f(0.6,0.8,0.9);
    glBegin(GL_POLYGON);
    glVertex2f(4.43,4.77);
    glVertex2f(5.93,4.77);
    glVertex2f(5.93,3.27);
    glVertex2f(4.43,3.27);
    glEnd();

    // right leg color

    glColor3f(0.6f, 0.8f, 0.9f);
    glBegin(GL_POLYGON);


    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*5.56f +
        2*(1 - t)*t*5.45f +
        t*t*5.60f;

        float y =
        (1 - t)*(1 - t)*3.42f +
        2*(1 - t)*t*3.02f +
        t*t*2.59f;

        glVertex2f(x, y);
    }

    for(float t = 1.0f; t >= 0.0f; t -= 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*7.14f +
        2*(1 - t)*t*7.29f +
        t*t*6.75f;

        float y =
        (1 - t)*(1 - t)*5.09f +
        2*(1 - t)*t*3.93f +
        t*t*2.59f;

        glVertex2f(x, y);
    }

    glEnd();

    //pet er boro sada color fill
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POLYGON);

        // Curve 1
        for(float t = 0.0f; t <= 1.0f; t += 0.02f)
        {
            float x =
            (1 - t)*(1 - t)*5.73f +
            2*(1 - t)*t*5.37f +
            t*t*5.50f;

            float y =
            (1 - t)*(1 - t)*5.49f +
            2*(1 - t)*t*4.93f +
            t*t*4.89f;

            glVertex2f(x, y);
        }

        // Bottom straight line
        glVertex2f(5.50f, 4.16f);
        glVertex2f(6.75f, 4.93f);

        //Curve 2 (reverse direction)
        for(float t = 1.0f; t >= 0.0f; t -= 0.02f)
        {
            float x =
            (1 - t)*(1 - t)*7.14f +
            2*(1 - t)*t*7.29f +
            t*t*6.75f;

            float y =
            (1 - t)*(1 - t)*5.09f +
            2*(1 - t)*t*4.93f +
            t*t*4.89f;

            glVertex2f(x, y);
        }

        //Top straight line
        glVertex2f(7.14f, 5.09f);
        glVertex2f(5.73f, 5.49f);

        glEnd();

        glBegin(GL_POLYGON);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(7.1f, 4.6385f);
        glVertex2f(5.78f,4.81f)  ;
        glVertex2f(5.12f, 5.3f);
        glVertex2f(6.69f,5.48f)  ;
        glEnd();

        //bellly...
        glBegin(GL_POLYGON);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(7.1f, 4.6385f);
        glVertex2f(5.78f,4.81f)  ;
        glVertex2f(5.12f, 5.3f);
        glVertex2f(6.69f,5.48f)  ;
        glEnd();

        glBegin(GL_POLYGON);

        float angleOfRotationBelly = 118.5f;
        float rotationAngleBelly = angleOfRotationBelly * (3.1416f / 180.0f);

        float txBelly = 0.37f;
        float tyBelly = -0.03f;

        float cxBelly = 6.16f;
        float cyBelly = 4.29f;

        float rBelly = 1.05f;

        for (int i = 0; i < 300; i++) {

            glColor3f(1.0,1.0,1.0);

            float pi = 3.1416f;
            float theta = (i * pi) / 300.0f;

            float xBelly = rBelly * cos(theta);
            float yBelly = rBelly * sin(theta);

            // Rotate
            float x_rotBelly = xBelly * cos(rotationAngleBelly) - yBelly * sin(rotationAngleBelly);
            float y_rotBelly = xBelly * sin(rotationAngleBelly) + yBelly * cos(rotationAngleBelly);

            // Translate + center
            float x_finalBelly = x_rotBelly + cxBelly + txBelly;
            float y_finalBelly = y_rotBelly + cyBelly + tyBelly;

            glVertex2f(x_finalBelly, y_finalBelly);
        }

        glEnd();

        // Black border
        glLineWidth(3);
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_STRIP);

        for (int i = 110; i <= 300; i++) {

            float pi = 3.1416f;
            float theta = (i * pi) / 300.0f;

            float xBelly = rBelly * cos(theta);
            float yBelly = rBelly * sin(theta);

            float x_rotBelly = xBelly * cos(rotationAngleBelly) - yBelly * sin(rotationAngleBelly);
            float y_rotBelly = xBelly * sin(rotationAngleBelly) + yBelly * cos(rotationAngleBelly);

            float x_finalBelly = x_rotBelly + cxBelly+ txBelly;
            float y_finalBelly = y_rotBelly + cyBelly + tyBelly;

            glVertex2f(x_finalBelly, y_finalBelly);
        }

        glEnd();


        glLineWidth(3);
        glBegin(GL_POLYGON);
        for(int i = 0; i < 300; i++)
        {
            glColor3f(1.0, 1.0, 1.0); // white fill
            float pi = 3.1416;
            float A = (i * 2*pi) / 300; // half circle
            float r = 0.75;
            float x = 6.38 + r * cos(A);
            float y = 4.04 + r * sin(A);
            glVertex2f(x, y);
        }
        glEnd();
        glLineWidth(3);
        glBegin(GL_POLYGON);
        for(int i = 0; i < 300; i++)
        {
            glColor3f(1.0, 1.0, 1.0); // white fill
            float pi = 3.1416;
            float A = (i * 2*pi) / 300; // only half circle
            float r = 0.45;
            float x = 6.67 + r * cos(A);
            float y = 3.73 + r * sin(A);
            glVertex2f(x, y);
        }
        glEnd();



        glLineWidth(3.0f);
        glColor3f(0.0f, 0.0f, 0.0f);

        glBegin(GL_LINE_STRIP);

        for(float t = 0.0f; t <= 1.0f; t += 0.02f)
        {
            float x =
            (1 - t)*(1 - t)*5.73f +
            2*(1 - t)*t*5.37f +
            t*t*5.50f;

            float y =
            (1 - t)*(1 - t)*5.49f +
            2*(1 - t)*t*4.37f +
            t*t*4.16f;

            glVertex2f(x, y);
        }

        glEnd();



        glBegin(GL_POLYGON);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(7.14f,5.06f);
        glVertex2f(6.78f,5.08f)  ;
        glVertex2f(6.8f, 3.97f);
        glVertex2f(6.97f,3.97f)  ;
        glVertex2f(7.16f,4.06f);
        glEnd();

        //pocket

        glBegin(GL_POLYGON);

        float pi = 3.1416f;
        float rotationAnglePocket = 180.0f * pi / 180.0f;

        float cxPocket = 6.23f;
        float cyPocket = 4.24f;
        float rPocket  = 1.2;

        float txPocket = 0.8f;
        float tyPocket = 0.5f;

        glColor3f(1.0f, 1.0f, 1.0f);


        for (int i = 0; i <= 75; i++) {
            float theta = (i * pi) / 150.0f;

            float x = rPocket * cos(theta);
            float y = rPocket * sin(theta);

            float xr = x * cos(rotationAnglePocket) - y * sin(rotationAnglePocket);
            float yr = x * sin(rotationAnglePocket) + y * cos(rotationAnglePocket);

            glVertex2f(xr + cxPocket + txPocket,
                       yr + cyPocket + tyPocket);
        }

        // center point
        glVertex2f(cxPocket + txPocket, cyPocket + tyPocket);

        glEnd();

        glLineWidth(3);
        glColor3f(0,0,0);
        glBegin(GL_LINE_STRIP);

        for (int i = 0; i <= 75; i++) {
            float theta = (i * pi) / 150.0f;

            float x = rPocket * cos(theta);
            float y = rPocket * sin(theta);

            float xr = x * cos(rotationAnglePocket) - y * sin(rotationAnglePocket);
            float yr = x * sin(rotationAnglePocket) + y * cos(rotationAnglePocket);

            glVertex2f(xr + cxPocket + txPocket,
                       yr + cyPocket + tyPocket);
        }
        glEnd();

        glBegin(GL_LINES);

        // edge 1
        glVertex2f(cxPocket + txPocket, cyPocket + tyPocket);
        glVertex2f(cxPocket + txPocket + rPocket * cos(rotationAnglePocket),
                   cyPocket + tyPocket + rPocket * sin(rotationAnglePocket));

        glEnd();
        glLineWidth(3);
        glColor3f(0.0f,0.0f,0.0f);


        glBegin(GL_LINE_STRIP);
        glVertex2f(7.0f, 4.74f);
        glVertex2f(7.15f, 4.74f);


        glEnd();

        //pet er nil ektu baki seta
        glBegin(GL_POLYGON);
        glColor3f(0.6f, 0.8f, 0.9f);
        glVertex2f(4.43, 4.67f);
        glVertex2f(5.1f,5.43f)  ;
        glVertex2f(5.715f, 5.47f);
        glVertex2f(5.44f,4.59f)  ;
        glEnd();

    //belt er line
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(6.99f, 5.75f);
    glVertex2f(4.73325f,5.50063f)  ;
    glVertex2f(5.12f, 5.3f);
    glVertex2f(6.69f,5.48f)  ;
    glEnd();
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f,0.0f,0.0f);
    glVertex2f(6.99f, 5.75f);
    glVertex2f(4.73325f,5.50063f)  ;
    glVertex2f(5.12f, 5.3f);
    glVertex2f(6.69f,5.48f)  ;
    glEnd();

    //BELL
    float centerXB = 7.03f;
    float centerYB = 5.41f;
    float r = 0.35f;
   // float pi=3.1416;

    glColor3f(1.0f, 0.84f, 0.0f);   // golden color
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++) {
        float B = (i * 2 * pi) / 300.0f;
        float x = centerXB + r * cos(B);
        float y = centerYB + r * sin(B);
        glVertex2f(x, y);
    }
    glEnd();

    glLineWidth(3);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 300; i++) {
        float B = (i * 2 * pi) / 300.0f;
        float x = centerXB + r * cos(B);
        float y = centerYB + r * sin(B);
        glVertex2f(x, y);
    }
    glEnd();

    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(6.69f, 5.48f);
    glVertex2f(7.13f, 5.74f);
    glEnd();
    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(6.68f, 5.37f);    // x, y
    glVertex2f(7.22f, 5.69f);    // x, y
    glEnd();
    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(7.35f, 5.53f);
    glVertex2f(7.18f, 5.45f);
    glEnd();
    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(7.18f, 5.45f);
    glVertex2f(7.37f, 5.4f);


    //RIGHT LEG
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*7.14f +
        2*(1 - t)*t*7.29f +
        t*t*6.75f;

        float y =
        (1 - t)*(1 - t)*5.09f +
        2*(1 - t)*t*3.93f +
        t*t*2.59f;

        glVertex2f(x, y);
    }

    glEnd();

    //juta
    //colour

    glPushMatrix();
    glRotatef(legAngle, 0, 0, -1);
    glTranslatef(0.05f, 0.0f, 0.0f); // move to hip
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);

    //Right straight line
    glVertex2f(6.93f, 2.64f);
    glVertex2f(6.89f, 1.99f);

    //Lower Bezier
    for(float t = 1.0f; t >= 0.0f; t -= 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*6.89f +
        2*(1 - t)*t*5.78f +
        t*t*5.23f;

        float y =
        (1 - t)*(1 - t)*1.99f +
        2*(1 - t)*t*1.75f +
        t*t*2.23f;

        glVertex2f(x, y);
    }

    // Left straight line
    glVertex2f(5.23f, 2.23f);
    glVertex2f(5.54f, 2.5f);

    glEnd();
    //right circle
    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0); // white fill
        float pi = 3.1416;
        float A = (i *2* pi) / 300;
        float r = 0.34;
        float x = 6.84+ r * cos(A);
        float y = 2.35 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();

    // Black border only on the curve
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(int i = 230; i <= 400; i++)   // 300 steps (20–90) mapped
    {
        float pi = 3.1416f;
        float A = (i*2 * pi) / 300.0f;

        float r = 0.34f;
        float x = 6.84f + r * cos(A);
        float y = 2.35f + r * sin(A);

        glVertex2f(x, y);
    }
    glEnd();

    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*6.93 +
        2*(1 - t)*t*6.29f +
        t*t*5.54f;

        float y =
        (1 - t)*(1 - t)*2.69f +
        2*(1 - t)*t*2.51f +
        t*t*2.61f;

        glVertex2f(x, y);
    }
    glEnd();
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*6.89f +
        2*(1 - t)*t*5.78f +
        t*t*5.23f;

        float y =
        (1 - t)*(1 - t)*1.99f +
        2*(1 - t)*t*1.75f +
        t*t*2.23f;

        glVertex2f(x, y);
    }
    glEnd();
    //left circle
    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0); // white fill
        float pi = 3.1416;
        float A = (i *2* pi) / 300;
        float r = 0.27;
        float x = 5.5+ r * cos(A);
        float y = 2.35 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();

    // Black border only on the curved edge
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(int i =60; i <= 230; i++)   // 300 steps (20–90) mapped
    {
        float pi = 3.1416f;
        float A = (i*2 * pi) / 300.0f;

        float r = 0.27f;
        float x = 5.5f + r * cos(A);
        float y = 2.35f + r * sin(A);

        glVertex2f(x, y);
    }
    glEnd();
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*5.56f +
        2*(1 - t)*t*5.45f +
        t*t*5.60f;

        float y =
        (1 - t)*(1 - t)*3.42 +
        2*(1 - t)*t*3.02f +
        t*t*2.59f;

        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();

    //left leg


    glLineWidth(4.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);
    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t) * (1 - t) * 4.43f +
        2 * (1 - t) * t * 4.08f +
        t * t * 4.55f;

        float y =
        (1 - t) * (1 - t) * 4.77f +
        2 * (1 - t) * t * 3.13f +
        t * t * 2.09f;

        glVertex2f(x, y);
    }
    glEnd();


    //pa colour
    glColor3f(0.6f, 0.8f, 0.9f);   // WHITE fill
    glBegin(GL_POLYGON);

    // Curve 1
    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*4.43f +
        2*(1 - t)*t*4.08f +
        t*t*4.55f;

        float y =
        (1 - t)*(1 - t)*4.77f +
        2*(1 - t)*t*3.13f +
        t*t*2.09f;

        glVertex2f(x, y);
    }

    // Bottom straight line
    glVertex2f(4.55f, 2.09f);
    glVertex2f(6.00f, 2.36f);

    // Curve 2 (reverse direction)
    for(float t = 1.0f; t >= 0.0f; t -= 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*6.20f +
        2*(1 - t)*t*5.95f +
        t*t*6.00f;

        float y =
        (1 - t)*(1 - t)*3.03f +
        2*(1 - t)*t*2.51f +
        t*t*2.36f;

        glVertex2f(x, y);
    }

    // Top straight line
    glVertex2f(6.20f, 3.03f);
    glVertex2f(4.43f, 4.77f);

    glEnd();
    //juta
    //colour juta

    glPushMatrix();
    glTranslatef(-0.05f, 0.0f, 0.0f);
    glRotatef(legAngle, 0, 0, 1);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);

    // Right straight line
    glVertex2f(6.0f, 2.25);
    glVertex2f(6.2f, 1.73f);

    //  Lower curve
    for(float t=1.0f; t>=0.0f; t-=0.02f)
    {
        float x = (1-t)*(1-t)*4.3f + 2*(1-t)*t*5.26f + t*t*6.2f;

        float y = (1-t)*(1-t)*2.0f + 2*(1-t)*t*1.52f + t*t*1.73f;

        glVertex2f(x,y);
    }

    //Left straight line
    glVertex2f(4.3f, 1.95f);
    glVertex2f(4.49f, 2.56f);

    glEnd();


    //left side circle
    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0); // white fill
        float pi = 3.1416;
        float A = (i *2* pi) / 300;
        float r = 0.34;
        float x = 4.43 + r * cos(A);
        float y = 2.3 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();

    // Black border only on the curved edge
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(int i = 67; i <= 230; i++)   // 300 steps → 20–90 mapped
    {
        float pi = 3.1416f;
        float A = (i*2 * pi) / 300.0f;

        float r = 0.34f;
        float x = 4.43f + r * cos(A);
        float y = 2.3f + r * sin(A);

        glVertex2f(x, y);
    }
    glEnd();
    //curve
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*4.49f +
        2*(1 - t)*t*5.32f +
        t*t*6.0f;

        float y =
        (1 - t)*(1 - t)*2.65f +
        2*(1 - t)*t*2.2f +
        t*t*2.4f;

        glVertex2f(x, y);
    }

    glEnd();

    //right side circle
    glLineWidth(3);
    glBegin(GL_POLYGON);
    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 1.0, 1.0);
        float pi = 3.1416;
        float A = (i *2* pi) / 300;
        float r = 0.34;
        float x = 6.15 + r * cos(A);
        float y = 2.09 + r * sin(A);
        glVertex2f(x, y);
    }
    glEnd();

    // Black border only on the curved edge
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(int i = 225; i <= 410; i++)
    {
        float pi = 3.1416f;
        float A = (i*2 * pi) / 300.0f;

        float r = 0.34f;
        float x = 6.15f + r * cos(A);
        float y = 2.09f + r * sin(A);

        glVertex2f(x, y);
    }
    glEnd();
    //curve
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*4.3f +
        2*(1 - t)*t*5.26f +
        t*t*6.2f;

        float y =
        (1 - t)*(1 - t)*2.0f +
        2*(1 - t)*t*1.52f +
        t*t*1.73f;

        glVertex2f(x, y);
    }

    glEnd();

    //jutar uporer curve
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_STRIP);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x =
        (1 - t)*(1 - t)*6.2f +
        2*(1 - t)*t*5.95f +
        t*t*6.0f;

        float y =
        (1 - t)*(1 - t)*3.03f +
        2*(1 - t)*t*2.51f +
        t*t*2.36f;

        glVertex2f(x, y);
    }

    glEnd();

    //tail
    glLineWidth(4.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(4.25, 3.64f);
    glVertex2f(4.00f, 3.72f);
    glEnd();

    //dim
    glLineWidth(3);
    glBegin(GL_POLYGON);

    for(int i = 0; i < 300; i++)
    {
        glColor3f(1.0, 0.0, 0.0);
        float pi = 3.1416f;
        float A = (i * 2 * pi) / 300.0f;
        float r = 0.25f;

        float x = 3.80 + r * cos(A);
        float y = 3.75 + r * sin(A);

        glVertex2f(x, y);
    }

    glEnd();

    glLineWidth(3);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);

    for(int i = 0; i < 300; i++)
    {
        float pi = 3.1416f;
        float A = (i * 2 * pi) / 300.0f;
        float r = 0.25f;

        float x = 3.80 + r * cos(A);
        float y = 3.750 + r * sin(A);

        glVertex2f(x, y);
    }

    glEnd();
    glPopMatrix();
    glFlush();
}


//flower pot

void drawCircle(float cx, float cy, float cr,float r,float g,float b) {
    glBegin(GL_POLYGON);
    glColor3f(r,g,b);
    for (int i = 0; i < 360; i++) {

        float A = i * 2 * 3.1415926f / 360;
        float x = cr * cos(A) + cx;
        float y = cr * sin(A) + cy;
        glVertex2f(x, y);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 360; i++) {

        float A = i * 2 * 3.1415926f / 360;
        float x = cr * cos(A) + cx;
        float y = cr * sin(A) + cy;
        glVertex2f(x, y);
    }
    glEnd();
}


void drawFlower()
{ glColor3f(0.6,0.2,0);
    glBegin(GL_POLYGON);
    glVertex2f(11,1.11);
    glVertex2f(11,0.57);
    glVertex2f(11.01,0.5555);
    glVertex2f(11.03,0.54);
    glVertex2f(11.93,0.54);
    glVertex2f(11.95,0.5555);
    glVertex2f(11,0.57);
    glVertex2f(11.96,0.57);
    glVertex2f(11.96,1.11);
    glEnd();

    glColor3f(0.8,0.4,0.0);
    glBegin(GL_POLYGON);
    glVertex2f(11,1.11);
    glVertex2f(10.97,1.12);
    glVertex2f(10.97302,1.11894);
    glVertex2f(10.95455,1.15131);
    glVertex2f(10.96995,1.4031);
    glVertex2f(11,1.4);
    glVertex2f(11.97808,1.41);
    glVertex2f(12,1.4);
    glVertex2f(12.01195,1.35994);
    glVertex2f(12.01041,1.14412);
    glVertex2f(11.98886,1.11894);
    glVertex2f(11.95826,1.11117);
    glEnd();

    //pata

    float x0 = 10.76139f, y0 = 1.8923f;
    float x1 = 11.15541f, y1 = 1.82755f;
    float x2 = 11.45708f, y2 = 1.42828f;

    glColor3f(0.0f, 0.0f, 0.0f); // curve color
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);
    for(float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        float x = (1-t)*(1-t)*x0
        + 2*(1-t)*t*x1
        + t*t*x2;

        float y = (1-t)*(1-t)*y0
        + 2*(1-t)*t*y1
        + t*t*y2;

        glVertex2f(x, y);
    }
    glEnd();
    float x10 = 10.76139f, y10 = 1.8923f;
    float x11 = 10.87683f, y11 = 1.59375f;
    float x12 = 11.07692f,  y12 = 1.4067f;

    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);
    for(float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        float x = (1-t)*(1-t)*x10
                + 2*(1-t)*t*x11
                + t*t*x12;

        float y = (1-t)*(1-t)*y10
                + 2*(1-t)*t*y11
                + t*t*y12;

        glVertex2f(x, y);
    }
    glEnd();

//pata color

        glColor3f(0.0f, 0.45f, 0.0f);
        glBegin(GL_POLYGON);

        for(float t = 0.0f; t <= 1.0f; t += 0.02f)
        {
            float x = (1-t)*(1-t)*x0
                    + 2*(1-t)*t*x1
                    + t*t*x2;

            float y = (1-t)*(1-t)*y0
                    + 2*(1-t)*t*y1
                    + t*t*y2;

            glVertex2f(x, y);
        }

        for(float t = 1.0f; t >= 0.0f; t -= 0.02f)
        {
            float x = (1-t)*(1-t)*x10
                    + 2*(1-t)*t*x11
                    + t*t*x12;

            float y = (1-t)*(1-t)*y10
                    + 2*(1-t)*t*y11
                    + t*t*y12;

            glVertex2f(x, y);
        }

        glEnd();

    //pata right
    float x20 = 11.52173f, y20 = 1.41749f;
    float x21 = 11.8157f,  y21 = 1.82755f;
    float x22 = 12.24359f, y22 = 1.88151f;
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);
    for(float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        float x = (1-t)*(1-t)*x20
                + 2*(1-t)*t*x21
                + t*t*x22;

        float y = (1-t)*(1-t)*y20
                + 2*(1-t)*t*y21
                + t*t*y22;

        glVertex2f(x, y);
    }
    glEnd();
    float x30 = 11.93576f, y30 = 1.41749f;
    float x31 = 12.10814f,  y31 = 1.55058f;
    float x32 = 12.24359f, y32 = 1.88151f;
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);
    for(float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        float x = (1-t)*(1-t)*x30
                + 2*(1-t)*t*x31
                + t*t*x32;

        float y = (1-t)*(1-t)*y30
                + 2*(1-t)*t*y31
                + t*t*y32;

        glVertex2f(x, y);
    }
    glEnd();

    //pata color
    glColor3f(0.0f, 0.45f, 0.0f);
    glBegin(GL_POLYGON);

    for(float t = 0.0f; t <= 1.0f; t += 0.02f)
    {
        float x = (1-t)*(1-t)*x20
                + 2*(1-t)*t*x21
                + t*t*x22;

        float y = (1-t)*(1-t)*y20
                + 2*(1-t)*t*y21
                + t*t*y22;

        glVertex2f(x, y);
    }

    for(float t = 1.0f; t >= 0.0f; t -= 0.02f)
    {
        float x = (1-t)*(1-t)*x30
                + 2*(1-t)*t*x31
                + t*t*x32;

        float y = (1-t)*(1-t)*y30
                + 2*(1-t)*t*y31
                + t*t*y32;

        glVertex2f(x, y);
    }

    glEnd();

    glLineWidth(2.0);
    glBegin(GL_LINES);
    glColor3f(0.0,0.0,0.0);
    glVertex2f(10.91223,1.7664);
    glVertex2f(11.35242,1.41749);

    glVertex2f(11.66333,1.41389);
    glVertex2f(12.1,1.8);
    glEnd();
    //LOMBA
    glLineWidth(2.0);
    glBegin(GL_POLYGON);
    glColor3f(0.0f, 0.45f, 0.0f);
    glVertex2f(11.45708,1.42828);
    glVertex2f(11.46,1.81);

    glVertex2f(11.51,1.85);
    glVertex2f(11.52173,1.41749);
    glEnd();

    //full
    drawCircle(11.31,2.69,0.19771922640362516,0.8,0.4,1);
    drawCircle(11.15,2.44,0.17034343770221755,0.8,0.4,1);
    drawCircle(11.16,2.13,0.16801150074766016,0.8,0.4,1);
    drawCircle(11.39,1.96,0.1634418553389585,0.8,0.4,1);
    drawCircle(11.66,1.96,0.18765000067862558,0.8,0.4,1);
    drawCircle(11.86,2.17,0.17,0.8,0.4,1);
    drawCircle(11.86,2.49,0.18,0.8,0.4,1);
    drawCircle(11.61,2.72,0.19,0.8,0.4,1);

    drawCircle(11.49888,2.31652,0.28,1,0.84,0);
    glFlush();
}

void Heart()

{
    glColor3f(0,0,0);
    glLineWidth(4);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<100;i++)
    {
        float theta = 2.0f * 3.1416f * i / 100;
        glVertex2f(-0.18f + 0.2f*cos(theta), 0.2f + 0.2f*sin(theta));
    }
    glEnd();
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<100;i++)
    {
        float theta = 2.0f * 3.1416f * i / 100;
        glVertex2f(0.18f + 0.2f*cos(theta), 0.2f + 0.2f*sin(theta));
    }
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.38f, 0.15f);
    glVertex2f(0.0f, 0.3f);
    glVertex2f(0.38f, 0.15f);
    glVertex2f(0.0f, -0.4f);

    glEnd();

    glColor3f(1,0.3,0.4);

    glBegin(GL_POLYGON);

    for(int i=0;i<100;i++)

    {

        float theta = 2.0f * 3.1416f * i / 100;

        glVertex2f(-0.18f + 0.2f*cos(theta), 0.2f + 0.2f*sin(theta));

    }

    glEnd();

    glBegin(GL_POLYGON);

    for(int i=0;i<100;i++)

    {

        float theta = 2.0f * 3.1416f * i / 100;

        glVertex2f(0.18f + 0.2f*cos(theta), 0.2f + 0.2f*sin(theta));

    }

    glEnd();

    glBegin(GL_POLYGON);

    glVertex2f(-0.38f, 0.15f);

    glVertex2f(0.0f, 0.3f);

    glVertex2f(0.38f, 0.15f);

    glVertex2f(0.0f, -0.4f);

    glEnd();

    glFlush();

}
void electricPole()
{
    glPointSize(10.0f);

    //Borders
    glLineWidth(4);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_LOOP);
    float al = 0.03f;
    float bl = 0.03f;
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.1416f / 180.0f;
        float x =  al * cos(angle);
        float y = 0.6 + bl * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    float al1 = 0.03f;
    float bl1 = 0.03f;
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.1416f / 180.0f;
        float x =  al1 * cos(angle);
        float y = -0.6 + bl1 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(-0.175f, 0.5f);
    glVertex2f(0.175f, 0.5f);
    glVertex2f(0.135f, 0.45f);
    glVertex2f(-0.135f, 0.45f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(-0.105f, 0.4f);
    glVertex2f(0.105f, 0.4f);
    glVertex2f(0.065f, 0.35f);
    glVertex2f(-0.065f, 0.35f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.03f, 0.6f);
    glVertex2f(-0.03f, 0.6f);
    glVertex2f(-0.03f, -0.6f);
    glVertex2f(0.03f, -0.6f);
    glEnd();
    glFlush();


    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.7f, 0.7f);
    glVertex2f(0.03f, 0.6f);
    glVertex2f(-0.03f, 0.6f);
    glVertex2f(-0.03f, -0.6f);
    glVertex2f(0.03f, -0.6f);
    glEnd();
    glFlush();


    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_POLYGON);
    float a = 0.03f;
    float b = 0.03f;
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.1416f / 180.0f;
        float x =  a * cos(angle);
        float y = 0.6 + b * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glFlush();

    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_POLYGON);
    float a1 = 0.03f;
    float b1 = 0.03f;
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.1416f / 180.0f;
        float x =  a1 * cos(angle);
        float y = -0.6 + b1 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glFlush();

    glLineWidth(0.25);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(1.0f, 0.47f);
    glVertex2f(0.135f, 0.47f);
    glEnd();
    glFlush();

    //WIRES
    glLineWidth(2);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINES);
    glVertex2f(-1.0f, 0.47f);
    glVertex2f(-0.135f, 0.47f);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(1.0f, 0.37f);
    glVertex2f(0.065f, 0.37f);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-1.0f, 0.37f);
    glVertex2f(-0.065f, 0.37f);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(1.0f, 0.47f);
    glVertex2f(0.135f, 0.47f);
    glEnd();
    glFlush();

    glLineWidth(0.25);
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(-0.175f, 0.5f);
    glVertex2f(0.175f, 0.5f);
    glVertex2f(0.135f, 0.45f);
    glVertex2f(-0.135f, 0.45f);
    glEnd();
    //glFlush();

    glLineWidth(0.25);
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(-0.105f, 0.4f);
    glVertex2f(0.105f, 0.4f);
    glVertex2f(0.065f, 0.35f);
    glVertex2f(-0.065f, 0.35f);
    glEnd();
    //glFlush();

    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.7f, 0.7f);
    glVertex2f(-0.175f, 0.5f);
    glVertex2f(0.175f, 0.5f);
    glVertex2f(0.135f, 0.45f);
    glVertex2f(-0.135f, 0.45f);
    glEnd();
    glFlush();

    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.7f, 0.7f);
    glVertex2f(-0.105f, 0.4f);
    glVertex2f(0.105f, 0.4f);
    glVertex2f(0.065f, 0.35f);
    glVertex2f(-0.065f, 0.35f);
    glEnd();
    glFlush();
}

void drawHome()
{

    // RED  roof
    //SQUARE PART

    glColor3f(1.0f, 0.5f, 0.6f);
    glBegin(GL_POLYGON);
        glVertex2f(-3.1,3.07);
        glVertex2f(-3.1,3.20);
        glVertex2f(-0.05,4.55);
        glVertex2f(2.75,3.20);
        glVertex2f(2.75,3.07);
        glVertex2f(-0.05,4.5);
    glEnd();

    //SQUARE PART
    glColor3f(1.0f, 0.5f, 0.6f);
    glBegin(GL_POLYGON);

        glVertex2f(-0.20,4.60);
        glVertex2f(0.10,4.60);
        glVertex2f(0.10,4.3);
        glVertex2f(-0.20,4.3);
    glEnd();


// WHITE PART
   glColor3f(1.0f, 1.0f, 1.0f);
   glBegin(GL_POLYGON);
    glVertex2f(-2.58f, 3.11f);
    glVertex2f(-0.05f, 4.21f);
    glVertex2f(2.24f, 3.14f);
    glVertex2f(2.75f, 3.07f);
     glVertex2f(-0.05f, 4.4f);
    glVertex2f(-3.1f, 3.07f);
     glVertex2f(-2.56f, 2.89f);



glEnd();


    // 2nd floor body
    glColor3f(1.0f, 0.95f, 0.83f);
    glBegin(GL_POLYGON);
        glVertex2f(-2.58f, 0.54f);
         glVertex2f(-2.58f, 3.11f);
          glVertex2f(-0.05f, 4.21f);
          glVertex2f(2.24f, 3.11f);
           glVertex2f(2.24f, 1.19f);

    glEnd();
     glColor3f(1.0f, 0.75f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-2.58f, 0.54f);
         glVertex2f(-2.58f, 3.11f);
          glVertex2f(-0.05f, 4.21f);
          glVertex2f(2.24f, 3.11f);
           glVertex2f(2.24f, 1.19f);

    glEnd();


    // 1st window brown part
    glColor3f(0.76f, 0.60f, 0.42f);
    glBegin(GL_POLYGON);
        glVertex2f(-1.71,3.14);
        glVertex2f(-1.71,1.71);
        glVertex2f(-0.65,1.71);
        glVertex2f(-0.65,3.14);
    glEnd();

    //  window borders
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.5);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-1.71,3.14);
        glVertex2f(-1.71,1.71);
        glVertex2f(-0.65,1.71);
        glVertex2f(-0.65,3.14);
    glEnd();

    glBegin(GL_LINE_LOOP);
        glVertex2f(-1.60,2.99);
        glVertex2f(-1.6,1.86);
        glVertex2f(-0.76,1.86);
        glVertex2f(-0.76,2.99);
    glEnd();

    glLineWidth(1.0);

   //brown window lines
    glBegin(GL_LINES);
        glVertex2f(-1.6,2.70); glVertex2f(-0.87,2.70);
        glVertex2f(-1.6,2.85); glVertex2f(-0.79,2.85);
        glVertex2f(-1.6,2.00); glVertex2f(-0.8,2.00);
        glVertex2f(-1.6,2.23); glVertex2f(-1.27,2.23);
        glVertex2f(-1.6,2.57); glVertex2f(-0.98,2.57);
        glVertex2f(-1.6,2.46); glVertex2f(-0.77,2.46);
        glVertex2f(-1.6,2.33); glVertex2f(-0.8,2.33);
        glVertex2f(-1.6,2.12); glVertex2f(-0.9,2.12);
    glEnd();

    // big window blue part
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(-0.65,3.14);
        glVertex2f(1.40,3.14);
        glVertex2f(1.40,1.71);
        glVertex2f(-0.65,1.71);
    glEnd();

    glColor3f(0.5f, 0.8f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(-0.49,2.96);
        glVertex2f(1.22,2.96);
        glVertex2f(1.22,1.86);
        glVertex2f(-0.49,1.86);
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(0.32,2.96);
        glVertex2f(0.45,2.96);
        glVertex2f(0.45,1.86);
        glVertex2f(0.32,1.86);
    glEnd();

    glColor3f(0,0,0);
    glLineWidth(1.5);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.65,3.14);
        glVertex2f(1.40,3.14);
        glVertex2f(1.40,1.71);
        glVertex2f(-0.65,1.71);
    glEnd();

    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.49,2.96);
        glVertex2f(1.22,2.96);
        glVertex2f(1.22,1.86);
        glVertex2f(-0.49,1.86);
    glEnd();

    glBegin(GL_LINE_LOOP);
        glVertex2f(0.32,2.96);
        glVertex2f(0.45,2.96);
        glVertex2f(0.45,1.86);
        glVertex2f(0.32,1.86);
    glEnd();

    // white strip
    glColor3f(1,1,1);
    glBegin(GL_POLYGON);
        glVertex2f(-2,3.2);
        glVertex2f(-2,3.08);
        glVertex2f(1.59,3.08);
        glVertex2f(1.59,3.2);
    glEnd();

    // red top
    glColor3f(1.0f, 0.5f, 0.6f);
    glBegin(GL_POLYGON);
        glVertex2f(-2.0f, 3.2f);
        glVertex2f(-1.72f, 3.37f);
        glVertex2f(-0.4f, 3.37f);
        glVertex2f(1.27f, 3.37f);
        glVertex2f(1.59f, 3.2f);
    glEnd();

     // 1ST FLOOR  roof
     //red strip
     glColor3f(1.0f, 0.5f, 0.6f);
    glBegin(GL_POLYGON);
        glVertex2f(-3.20f, 0.5f);
        glVertex2f(-3.20f, 0.7f);
        glVertex2f(0.79f, 1.75f);
        glVertex2f(5.10f, 0.45f);
        glVertex2f(5.10f, 0.25f);
    glEnd();

//white strip
glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(-2.83f,0.4f);
        glVertex2f(0.79f,1.35f);
        glVertex2f(4.69,0.25);
        glVertex2f(5.10f,0.25f);
        glVertex2f(0.79f,1.55f);
        glVertex2f(-3.20f,0.5f);
          glVertex2f(-2.56f,0.0f);


    glEnd();


    // 1ST FLOOR body
    glColor3f(1.0f, 0.95f, 0.83f);
    glBegin(GL_POLYGON);
        glVertex2f(-2.83f,-2.72f);
        glVertex2f(-2.83f,0.4f);
        glVertex2f(0.79f,1.35f);
        glVertex2f(4.69f,0.25f);
        glVertex2f(4.69f,-2.72f);
    glEnd();

     glColor3f(1.0f, 0.75f, 0.0f);
 glLineWidth(1.0);
    glBegin(GL_LINE_LOOP);

        glVertex2f(-2.83f,-2.72f);
        glVertex2f(-2.83F,0.4f);
        glVertex2f(0.79f,1.35f);
        glVertex2f(4.69f,0.25f);
        glVertex2f(4.69f,-2.72f);
    glEnd();

// home road
 glColor3f(0.5f, 0.4f, 0.6f);

    glBegin(GL_POLYGON);
        glVertex2f(2.25f,-2.72f);
        glVertex2f(4.55f,-2.72f);
        glVertex2f(6.0f,-3.0f);
        glVertex2f(2.14f,-3.0f);

    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.5);
    glBegin(GL_LINE_LOOP);
        glVertex2f(2.25f,-2.72f);
        glVertex2f(4.55f,-2.72f);
        glVertex2f(6.0f,-3.0f);
        glVertex2f(2.14f,-3.0f);

    glEnd();

    //2ND WINDOW BROWN PART
     glColor3f(0.76f, 0.60f, 0.42f);
    glBegin(GL_POLYGON);
        glVertex2f(-2.45,0.0);
        glVertex2f(-1.1,0.0);
        glVertex2f(-1.1,-2);
        glVertex2f(-2.45,-2);
    glEnd();

    // BORDER
    glColor3f(0,0,0);
    glLineWidth(1.5);
     glBegin(GL_LINE_LOOP);
       glVertex2f(-2.45,0.0);
        glVertex2f(-1.1,0.0);
        glVertex2f(-1.1,-2);
        glVertex2f(-2.45,-2);
    glEnd();

   // INSIDE BORDER
    glColor3f(0,0,0);
    glLineWidth(1.5);
     glBegin(GL_LINE_LOOP);
       glVertex2f(-2.28, -0.20);
        glVertex2f(-1.25, -0.20);
        glVertex2f(-1.25, -1.91);
        glVertex2f(-2.28, -1.91);
    glEnd();
    // BROWN WINDOW LINES
       glLineWidth(1.0);

   //brown window lines
    glBegin(GL_LINES);
        glVertex2f(-2.28, -0.35); glVertex2f(-1.25,-0.35);
         glVertex2f(-2.28, -0.50);glVertex2f(-1.50, -0.5);
         glVertex2f(-2.28, -0.65);glVertex2f(-1.80, -0.65);
         glVertex2f(-2.28, -0.80);glVertex2f(-1.40, -0.80);
         glVertex2f(-2.28, -0.95); glVertex2f(-1.25,-0.95);
         glVertex2f(-2.28, -1.10); glVertex2f(-1.60,-1.10);
         glVertex2f(-2.28, -1.25); glVertex2f(-1.25,-1.25);
         glVertex2f(-2.28, -1.40); glVertex2f(-1.30,-1.40);
    glEnd();

     //BLUE WIMDOW PART
      glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_POLYGON);

        glVertex2f(-1.1,0.0);
        glVertex2f(1.35, 0.0);
         glVertex2f(1.35, -2);
        glVertex2f(-1.1,-2);

    glEnd();
    glColor3f(0,0,0);
    glLineWidth(1.5);
     glBegin(GL_LINE_LOOP);
        glVertex2f(-1.1,0.0);
        glVertex2f(1.35, 0.0);
         glVertex2f(1.35, -2);
        glVertex2f(-1.1,-2);

    glEnd();

  //WINDOW SKYBLUE PART
     glColor3f(0.5f, 0.8f, 1.0f);
     glBegin(GL_POLYGON);
       glVertex2f(-0.90, -0.20);
        glVertex2f(1.15, -0.20);
        glVertex2f(1.15,-1.91);
        glVertex2f(-0.90,-1.91);
    glEnd();

   glColor3f(0,0,0);
    glLineWidth(1.5);
     glBegin(GL_LINE_LOOP);
     glVertex2f(-0.90, -0.20);
        glVertex2f(1.15, -0.20);
        glVertex2f(1.15,-1.91);
        glVertex2f(-0.90,-1.91);
    glEnd();
     glColor3f(0.0f, 0.0f, 1.0f);
     glBegin(GL_POLYGON);
       glVertex2f(0.0, -0.20);
        glVertex2f(0.2, -0.20);
        glVertex2f(0.2,-1.91);
        glVertex2f(0.0,-1.91);
    glEnd();

glColor3f(0,0,0);
    glLineWidth(1.5);
     glBegin(GL_LINE_LOOP);
     glVertex2f(0.0, -0.20);
        glVertex2f(0.2, -0.20);
        glVertex2f(0.2,-1.91);
        glVertex2f(0.0,-1.91);
    glEnd();

    //white strip
     glColor3f(1.0f, 1.0f, 1.0f);
     glBegin(GL_POLYGON);
       glVertex2f(-2.68, 0.1);
        glVertex2f(1.35, 0.10);
        glVertex2f(1.35,-0.1);
        glVertex2f(-2.68,-0.1);
    glEnd();

//red top
     glColor3f(1.0f, 0.5f, 0.6f);
glBegin(GL_POLYGON);
    glVertex2f(-2.68,  0.1);
    glVertex2f( -2.0, 0.3);
    glVertex2f( 1.53,0.3);
    glVertex2f(1.35, 0.1);
glEnd();

//window another part
 glColor3f(1.0f, 1.0f, 1.0f);
glBegin(GL_POLYGON);
    glVertex2f(1.53, 0.3);
    glVertex2f( 1.35, 0.1);
    glVertex2f( 1.35,-0.1);
    glVertex2f(1.53, 0.1);
glEnd();



    // door
    glColor3f(0.5f, 0.2f, 0.03f);
    glBegin(GL_POLYGON);
        glVertex2f(2.25f,-2.72f);
        glVertex2f(4.35f,-2.72f);
        glVertex2f(4.35f,0.0f);
        glVertex2f(2.25f,0.0f);
    glEnd();
    glColor3f(0,0,0);
    glBegin(GL_LINE_STRIP);

        glVertex2f(4.35f,-2.72f);
        glVertex2f(4.35f,0.0f);
        glVertex2f(2.25f,0.0f);
         glVertex2f(2.25f,-2.72f);
    glEnd();

    // border
    glColor3f(0,0,0);
    glBegin(GL_LINE_STRIP);
        glVertex2f(2.45f, -2.72f);
        glVertex2f(2.45f, -0.2f);
        glVertex2f(4.15f, -0.2f);
        glVertex2f(4.15f, -2.72f);

    glEnd();


    glEnd();
 // DOOR SIDE SKYBLUE PART
    glColor3f(0.5f, 0.6f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(3.75f,0.0f);
        glVertex2f(4.15f,0.0f);
        glVertex2f(4.15f,-2.72f);
        glVertex2f(3.75f,-2.72f);
    glEnd();

    // INSIDE PART
     glColor3f(0.4f, 0.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(3.85,-0.7f);
        glVertex2f(4.05f,-0.7f);
        glVertex2f(4.05f,-1.4f);
        glVertex2f(3.85f,-1.4f);
    glEnd();
     glColor3f(0.4f, 0.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(3.85,-1.5f);
        glVertex2f(4.05f,-1.5f);
        glVertex2f(4.05f,-2.2f);
        glVertex2f(3.85f,-2.2f);
    glEnd();
     glBegin(GL_LINE_LOOP);
        glVertex2f(3.85,-2.3f);
        glVertex2f(4.05f,-2.3f);
        glVertex2f(4.05f,-2.9f);
        glVertex2f(3.85f,-2.9f);
    glEnd();

    //SIDE PART BORDER
 glColor3f(0,0,0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(3.75f,-2.72f);
        glVertex2f(3.75f,0.0f);
        glVertex2f(4.15f,0.0f);
        glVertex2f(4.15f,-2.72f);

    glEnd();
    // DOOR TOP SKYBLUE PART
    glColor3f(0.5f, 0.6f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(2.45f,0.0f);
        glVertex2f(4.15f,0.0f);
        glVertex2f(4.15f,-0.4f);
        glVertex2f(2.45f,-0.4f);
    glEnd();
    // TOP SKY BLUE INSIDE PART
    glColor3f(0.4f, 0.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(2.55f,-0.1f);
        glVertex2f(3.25f,-0.1f);
        glVertex2f(3.25f,-0.3f);
        glVertex2f(2.55f,-0.3f);
    glEnd();


     glColor3f(0.4f, 0.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(3.35f,-0.1f);
        glVertex2f(4.05f,-0.1f);
        glVertex2f(4.05f,-0.3f);
        glVertex2f(3.35f,-0.3f);
    glEnd();


    //border part
  glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(2.45f,-0.4f);
        glVertex2f(2.45f,0.0f);
        glVertex2f(4.15f,0.0f);
        glVertex2f(4.15f,-0.4f);

    glEnd();

   //border
    glBegin(GL_LINES);
    glVertex2f(2.45f,-0.4f);
    glVertex2f(3.75f,-0.4f);
      glEnd();

      //door white strip
     glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(2.05f,0.1f);
        glVertex2f(4.55f,0.1f);
        glVertex2f(4.55f,-0.1f);
        glVertex2f(2.05f,-0.1f);
    glEnd();
    // door REDtop part
         glColor3f(1.0f, 0.5f, 0.6f);
    glBegin(GL_POLYGON);
        glVertex2f(2.05f,0.1f);
        glVertex2f(2.45f,0.3f);
        glVertex2f(4.15f,0.3f);
        glVertex2f(4.55f,0.1f);

    glEnd();

    //DOOR MIDDLE BROWN PART
       glColor3f(0.5f, 0.2f, 0.03f);
    glBegin(GL_POLYGON);
        glVertex2f(2.45f,-0.4f);
        glVertex2f(4.15f,-0.4f);
        glVertex2f(4.15f,-0.6f);
        glVertex2f(2.45f,-0.6f);

    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
         glVertex2f(2.45f,-0.4f);
        glVertex2f(4.15f,-0.4f);
        glVertex2f(4.15f,-0.6f);
        glVertex2f(2.45f,-0.6f);

    glEnd();
    //DOOR SIDE BORDER
     glBegin(GL_LINES);
    glVertex2f(3.55f,-0.6f);
    glVertex2f(3.55f,-2.72f);
      glEnd();
    //DOOR INSIDE QUAD
    glColor3f(0.5f, 0.6f, 1.0f);
    glBegin(GL_POLYGON);
       glVertex2f(2.75f,-0.8f);
        glVertex2f(3.25f,-0.8f);
        glVertex2f(3.25f,-1.5f);
        glVertex2f(2.75f,-1.5f);


    glEnd();
     glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
         glVertex2f(2.75f,-0.8f);
        glVertex2f(3.25f,-0.8f);
        glVertex2f(3.25f,-1.5f);
        glVertex2f(2.75f,-1.5f);

    glEnd();
     glColor3f(0.4f, 0.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(2.85f,-0.9f);
        glVertex2f(3.15f,-0.9f);
        glVertex2f(3.15f,-1.4f);
        glVertex2f(2.85f,-1.4f);
    glEnd();
    //DOOR  LOCK
    glColor3f(0.5f, 0.6f, 1.0f);
    glBegin(GL_POLYGON);
       glVertex2f(3.33f,-1.6f);
        glVertex2f(3.45f,-1.6f);
        glVertex2f(3.45f,-1.9f);
        glVertex2f(3.33f,-1.9f);
glEnd();
glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
          glVertex2f(3.33f,-1.6f);
        glVertex2f(3.45f,-1.6f);
        glVertex2f(3.45f,-1.9f);
        glVertex2f(3.33f,-1.9f);
    glEnd();
//CIRCLE PART
glBegin(GL_POLYGON);
for( int i=0;i<200;i++)
    {
glColor3f(0.5f,0.6f,1.0f);
float pi= 3.1416;
float A =(i*2*pi)/200;
float r = 0.07;
float x= 3.39+r* cos(A);
float y= -1.72+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

glLineWidth(1.5);

glBegin(GL_LINE_LOOP);

for( int i=0;i<100;i++)
    {
glColor3f(0.0f,0.0f,0.0f);
float pi= 3.1416;
float A =(i*2*pi)/100;
float r = 0.07;
float x= 3.39+r* cos(A);
float y= -1.72+r* sin(A);
glVertex2f(x,y);
    }
glEnd();


// main gate part
//1ST LEFT PART
float startX = 2.14f;
float width  = 0.09f;
float gap    = 0.15f;

for(int i = 0; i < 3; i++)
{
    float x1 = startX + i * gap;     // left
    float x2 = x1 + width;           // right
    float y1 = -1.6f;                // top
    float y2 = -2.9f;                // bottom

    // Filled polygon (body)
    glColor3f(0.0f, 0.5f, 1.0f);

    glBegin(GL_POLYGON);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();

    // Border (outline)
    glLineWidth(2.0f);          // border thickness
    glColor3f(0.0f, 0.0f, 0.0f); // border color

    glBegin(GL_LINE_LOOP);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();
}



//LEFT PART
glColor3f(0.0f, 0.5f, 1.0f);
    glBegin(GL_POLYGON);

        glVertex2f(2.14f, -1.6f);
        glVertex2f(2.47f, -1.5f);
         glVertex2f(2.47f, -1.6f);
         glVertex2f(2.14f, -1.7f);

    glEnd();
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(2.14f, -1.6f);
        glVertex2f(2.47f, -1.5f);
         glVertex2f(2.47f, -1.6f);
         glVertex2f(2.14f, -1.7f);
glEnd();
glColor3f(0.0f, 0.5f, 1.0f);
    glBegin(GL_POLYGON);

        glVertex2f(2.14f, -2.9f);
        glVertex2f(2.47f, -2.8f);
         glVertex2f(2.47f, -2.9f);
         glVertex2f(2.14f, -3.0f);

    glEnd();
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(2.14f, -2.9f);
        glVertex2f(2.47f, -2.8f);
         glVertex2f(2.47f, -2.9f);
         glVertex2f(2.14f, -3.0f);
glEnd();

    glBegin(GL_POLYGON);
for( int i=0;i<200;i++)
    {
glColor3f(0.0f,0.5f,1.0f);
float pi= 3.1416;
float A =(i*2*pi)/200;
float r = 0.07;
float x= 2.47+r* cos(A);
float y= -1.55+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

glLineWidth(1.5);

glBegin(GL_LINE_LOOP);

for( int i=0;i<100;i++)
    {
glColor3f(0.0f,0.0f,0.0f);
float pi= 3.1416;
float A =(i*2*pi)/100;
float r = 0.07;
float x= 2.47+r* cos(A);
float y= -1.55+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

 glBegin(GL_POLYGON);
for( int i=0;i<200;i++)
    {
glColor3f(0.0f,0.5f,1.0f);
float pi= 3.1416;
float A =(i*2*pi)/200;
float r = 0.07;
float x= 2.47+r* cos(A);
float y= -2.85+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

glLineWidth(1.5);

glBegin(GL_LINE_LOOP);

for( int i=0;i<100;i++)
    {
glColor3f(0.0f,0.0f,0.0f);
float pi= 3.1416;
float A =(i*2*pi)/100;
float r = 0.07;
float x= 2.47+r* cos(A);
float y= -2.85+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

//2ND GATE PART
 startX = 4.46f;
 width  = 0.10f;
 gap    = 0.20f;

for(int i = 0; i < 6; i++)
{
    float x1 = startX - i * gap;     // left
    float x2 = x1 + width;           // right
    float y1 = -1.7f;                // top
    float y2 = -2.9f;                // bottom

    // Filled polygon (body)
    glColor3f(0.0f, 0.5f, 1.0f);

    glBegin(GL_POLYGON);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();

    // Border (outline)
    glLineWidth(2.0f);          // border thickness
    glColor3f(0.0f, 0.0f, 0.0f); // border color black

    glBegin(GL_LINE_LOOP);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();
}

//1ST PART
//BLUE PART
glColor3f(0.0f, 0.5f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(3.52f, -1.6f);
        glVertex2f(4.56f, -1.6f);
        glVertex2f(4.56f, -1.7f);
        glVertex2f(3.52f, -1.7f);
    glEnd();
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(3.52f, -1.6f);
        glVertex2f(4.56f, -1.6f);
        glVertex2f(4.56f, -1.7f);
        glVertex2f(3.52f, -1.7f);
    glEnd();

glBegin(GL_POLYGON);
for( int i=0;i<200;i++)
    {
glColor3f(0.0f,0.5f,1.0f);
float pi= 3.1416;
float A =(i*2*pi)/200;
float r = 0.07;
float x= 3.52+r* cos(A);
float y= -1.65+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

glLineWidth(1.5);

glBegin(GL_LINE_LOOP);

for( int i=0;i<100;i++)
    {
glColor3f(0.0f,0.0f,0.0f);
float pi= 3.1416;
float A =(i*2*pi)/100;
float r = 0.07;
float x= 3.52+r* cos(A);
float y= -1.65+r* sin(A);
glVertex2f(x,y);
    }
glEnd();


//2ND PART
//BLUE PART
glColor3f(0.0f, 0.5f, 1.0f);
    glBegin(GL_POLYGON);
        glVertex2f(3.52f, -2.9f);
        glVertex2f(4.56f, -2.9f);
        glVertex2f(4.56f, -3.0f);
        glVertex2f(3.52f, -3.0f);
    glEnd();
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(3.52f, -2.9f);
        glVertex2f(4.56f, -2.9f);
        glVertex2f(4.56f, -3.0f);
        glVertex2f(3.52f, -3.0f);
    glEnd();

glBegin(GL_POLYGON);
for( int i=0;i<200;i++)
    {
glColor3f(0.0f,0.5f,1.0f);
float pi= 3.1416;
float A =(i*2*pi)/200;
float r = 0.07;
float x= 3.52+r* cos(A);
float y= -2.95+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

glLineWidth(1.5);

glBegin(GL_LINE_LOOP);

for( int i=0;i<100;i++)
    {
glColor3f(0.0f,0.0f,0.0f);
float pi= 3.1416;
float A =(i*2*pi)/100;
float r = 0.07;
float x= 3.52+r* cos(A);
float y= -2.95+r* sin(A);
glVertex2f(x,y);
    }
glEnd();

    // first rectangle (wall)
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_POLYGON);
        glVertex2f(-6.34f, -3.0f);
        glVertex2f(2.14f, -3.0f);
        glVertex2f(2.14f, -1.44f);
        glVertex2f(-6.34f, -1.44f);
    glEnd();

    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-6.34f, -3.0f);
        glVertex2f(2.14f, -3.0f);
        glVertex2f(2.14f, -1.44f);
        glVertex2f(-6.34f, -1.44f);
    glEnd();


    float y1 = -3.0f, y2 = -1.44f;
    float start_x1 = -6.36f, x_spacing1 = 0.5f;
    int num_vlines1 = 18;

    glLineWidth(0.5);
    glBegin(GL_LINES);
        for(int i=0;i<num_vlines1;i++){
            float x = start_x1 + i * x_spacing1;
            glVertex2f(x, y1);
            glVertex2f(x, y2);
        }
    glEnd();

    float h_start_y1 = -1.44f;
    float h_start_x1 = -6.36f, h_end_x1 = 2.14f;
    float h_spacing1 = (-1.44f - (-3.0f)) / 6;
    int num_hlines1 = 7;

    glBegin(GL_LINES);
        for(int i=0;i<num_hlines1;i++){
            float y = h_start_y1 - i * h_spacing1;
            glVertex2f(h_start_x1, y);
            glVertex2f(h_end_x1, y);
        }
    glEnd();

    // second rectangle
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_POLYGON);
        glVertex2f(4.56f, -3.0f);
        glVertex2f(4.56f, -1.44f);
        glVertex2f(6.76f, -1.44f);
        glVertex2f(6.76f, -3.0f);
    glEnd();

    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(4.56f, -3.0f);
        glVertex2f(4.56f, -1.44f);
        glVertex2f(6.76f, -1.44f);
        glVertex2f(6.76f, -3.0f);
    glEnd();

    float start_x2 = 4.56f, x_spacing2 = 0.5f;
    int num_vlines2 = 5;

    glBegin(GL_LINES);
        for(int i=0;i<num_vlines2;i++){
            float x = start_x2 + i * x_spacing2;
            glVertex2f(x, y1);
            glVertex2f(x, y2);
        }
    glEnd();

    float h_start_x2 = 4.56f, h_end_x2 = 6.76f;
    float h_spacing2 = (-1.44f - (-3.0f)) / 6;
    int num_hlines2 = 7;

    glBegin(GL_LINES);
        for(int i=0;i<num_hlines2;i++){
            float y = -1.44f - i * h_spacing2;
            glVertex2f(h_start_x2, y);
            glVertex2f(h_end_x2, y);
        }
    glEnd();
    glFlush();
}

void drawEllipse1(float cx, float cy, float rx, float ry)
{
    int segments = 100;
    glColor3f(0.639f, 0.784f, 0.886f);
    //glColor3f(0.45f, 0.75f, 1.0f);

    glBegin(GL_POLYGON);
    for(int i = 0; i < segments; i++)
    {
        float theta = 2.0f * 3.1416f * i / segments;
        glVertex2f(cx + rx*cos(theta), cy + ry*sin(theta));
    }
    glEnd();

    glColor3f(0.353,0.549,0.631);
   // glColor3f(0.0f, 0.3f, 0.7f);

    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < segments; i++)
    {
        float theta = 2.0f * 3.1416f * i / segments;
        glVertex2f(cx + rx*cos(theta), cy + ry*sin(theta));
    }
    glEnd();
}

void waterPuddle(){
    float e_cx=(-0.66+2.35)/2;
    float e_cy=(0.6+1.58)/2;
    float e_rx=(2.35-(-0.66))/2;
    float e_ry=(1.58-0.6)/2;
    drawEllipse1(e_cx, e_cy, e_rx, e_ry);

    float f_cx=(-0.22+2.0)/2;
    float f_cy=(0.85+1.43)/2;
    float f_rx=(2-(-0.22))/2;
    float f_ry=(1.43-0.85)/2;
    drawEllipse1(f_cx, f_cy, f_rx, f_ry);

    float g_cx=(2.34+3.02)/2;
    float g_cy=(0.5+0.69)/2;
    float g_rx=(3.02-2.34)/2;
    float g_ry=(0.69-0.5)/2;
    drawEllipse1(g_cx, g_cy, g_rx, g_ry);

    float h_cx=(1.2+2.39)/2;
    float h_cy=(0.16+0.62)/2;
    float h_rx=(2.39-1.2)/2;
    float h_ry=(0.62-0.16)/2;
    drawEllipse1(h_cx, h_cy, h_rx, h_ry);

    glFlush();

}

void drawDoraemonShadow(float x, float y, float scale)
{
    glPushMatrix();
    glTranslatef(x, y - 0.05f, 0.0f);
    glScalef(scale, scale * 0.2f, 1.0f);

    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
    int segments = 50;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = cos(angle);
        float y = sin(angle);
        glVertex2f(x, y);
    }
    glEnd();

    glPopMatrix();
}

void drawLampPost()
{
    //glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
    glPointSize(10.0f);

    //glDisable(GL_LIGHTING);
    //glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(0.1f, 0.2f, 0.2f);
    glVertex2f(-0.1f, 0.6f);
    glVertex2f(-0.13f, 0.6f);
    glVertex2f(-0.13f, -0.6f);
    glVertex2f(-0.1f, -0.6f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(0.1f, 0.2f, 0.2f);
    glVertex2f(-0.13f, 0.6f);
    glVertex2f(-0.1f, 0.63f);
    glVertex2f(0.1f, 0.6f);
    glVertex2f(0.13f, 0.63f);
    glEnd();

    int layers = 20;

    for (int i = 0; i < layers; i++) {
        float t = (float)i / layers;

        float alpha = 0.15f - t * 0.12f;
        if (alpha < 0.01f) continue;

        float spread = t * 0.15f;

        glBegin(GL_TRIANGLES);
        glColor4f(0.7f, 0.7f, 0.7f, alpha);

        glVertex2f(0.103f, 0.632f);
        glVertex2f(-0.55f - spread, -1.5f);
        glVertex2f(0.55f + spread, -1.5f);

        glEnd();
    }


    float radius = 0.03f;
    int segments = 200;
    float cx = 0.1f;
    float cy = 0.62f;

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.9f, 0.9f, 0.9f);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = cx + radius * cos(angle);
        float y = cy + radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();




    glFlush();

}

void drawLampPostWithLighting()
{
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    drawLampPost();

    GLfloat lightPos[]     = { 0.0f, 0.6f, 1.5f, 1.0f };
    GLfloat lightAmbient[] = { 0.12f, 0.12f, 0.12f, 1.0f };
    GLfloat lightDiffuse[] = { 0.9f, 0.9f, 0.7f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glPopAttrib();
}

void Tree1()
{
    //glPointSize(5.0f);
    //glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.1, 0.2);
    glVertex2f(-0.1, 0.2);
    glVertex2f(-0.1, -0.9);
    glVertex2f(0.1, -0.9);
    glEnd();

    glColor3f(0.5f, 0.3f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(0.1, 0.2);
    glVertex2f(-0.1, 0.2);
    glVertex2f(-0.1, -0.9);
    glVertex2f(0.1, -0.9);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0.6,-0.45);
    glVertex2f(0.0,0.2);
    glVertex2f(-0.6,-0.45);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(0.5,-0.3);
    glVertex2f(0.0,0.4);
    glVertex2f(-0.5,-0.3);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(0.4,-0.1);
    glVertex2f(0.0,0.6);
    glVertex2f(-0.4,-0.1);

    glBegin(GL_LINE_LOOP);
    glVertex2f(0.3,0.1);
    glVertex2f(0.0,0.6);
    glVertex2f(-0.3,0.1);
    glEnd();

    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.6,-0.45);
    glVertex2f(0.0,0.2);
    glVertex2f(-0.6,-0.45);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(0.5,-0.3);
    glVertex2f(0.0,0.4);
    glVertex2f(-0.5,-0.3);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(0.4,-0.1);
    glVertex2f(0.0,0.6);
    glVertex2f(-0.4,-0.1);

    glBegin(GL_TRIANGLES);
    glVertex2f(0.3,0.1);
    glVertex2f(0.0,0.6);
    glVertex2f(-0.3,0.1);
    glEnd();

    glFlush();
}

void drawwall()
{
// first rectangle (wall)
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_POLYGON);
        glVertex2f(-6.34f, -3.0f);
        glVertex2f(2.14f, -3.0f);
        glVertex2f(2.14f, -1.44f);
        glVertex2f(-6.34f, -1.44f);
    glEnd();

    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-6.34f, -3.0f);
        glVertex2f(2.14f, -3.0f);
        glVertex2f(2.14f, -1.44f);
        glVertex2f(-6.34f, -1.44f);
    glEnd();


    float y1 = -3.0f, y2 = -1.44f;
    float start_x1 = -6.36f, x_spacing1 = 0.5f;
    int num_vlines1 = 18;

    glLineWidth(0.5);
    glBegin(GL_LINES);
        for(int i=0;i<num_vlines1;i++){
            float x = start_x1 + i * x_spacing1;
            glVertex2f(x, y1);
            glVertex2f(x, y2);
        }
    glEnd();

    float h_start_y1 = -1.44f;
    float h_start_x1 = -6.36f, h_end_x1 = 2.14f;
    float h_spacing1 = (-1.44f - (-3.0f)) / 6;
    int num_hlines1 = 7;

    glBegin(GL_LINES);
        for(int i=0;i<num_hlines1;i++){
            float y = h_start_y1 - i * h_spacing1;
            glVertex2f(h_start_x1, y);
            glVertex2f(h_end_x1, y);
        }
    glEnd();

    glFlush();
}
void drawBox(){
        glColor3f(0.7f, 0.4f, 0.2f);

//polygon1
            glBegin(GL_POLYGON);

                   glVertex2f(-3.32, -1.96);
                   glVertex2f(-3.32, 3.06);
                   glVertex2f(0.44, 2.13);
                   glVertex2f(0.44f, -2.99f);

             glEnd();

//polygon2
            glBegin(GL_POLYGON);
                   glVertex2f(0.44f, -2.99f);
                   glVertex2f(0.44, 2.13);
                   glVertex2f(4.0, 3.59);
                   glVertex2f(4.0f, -1.49f);

             glEnd();
//polygon3
            glBegin(GL_POLYGON);

                   glVertex2f(-3.32, 3.06);
                   glVertex2f(0.44, 2.13);
                   glVertex2f(4.0, 3.59);
                   glVertex2f(0.34f, 4.44f);

             glEnd();

//border
             glColor3f(0.0f, 0.0f, 0.0f);
             glLineWidth(2.0);
             glBegin(GL_LINE_LOOP);

                   glVertex2f(-3.32, -1.96);
                   glVertex2f(-3.32, 3.06);
                   glVertex2f(0.44, 2.13);
                   glVertex2f(0.44f, -2.99f);

             glEnd();

            glColor3f(0.0f, 0.0f, 0.0f);
             glBegin(GL_LINE_LOOP);
                   glVertex2f(0.44f, -2.99f);
                   glVertex2f(0.44, 2.13);
                   glVertex2f(4.0, 3.59);
                   glVertex2f(4.0f, -1.49f);

             glEnd();

             glBegin(GL_LINE_LOOP);
                     glVertex2f(-3.32, 3.06);
                   glVertex2f(0.44, 2.13);
                   glVertex2f(4.0, 3.59);
                   glVertex2f(0.34f, 4.44f);

             glEnd();

             glBegin(GL_LINES);
             glVertex2f(-3.32, 2.41);
             glVertex2f(0.44, 1.42);
             glEnd();

             glBegin(GL_LINES);
             glVertex2f(0.44, 1.42);
             glVertex2f(4.0, 2.95);
             glEnd();

             glBegin(GL_LINES);
             glVertex2f(0.44, -2.32);
             glVertex2f(4.0, -0.82);
             glEnd();

             glBegin(GL_LINES);
             glVertex2f(-3.32, -1.29);
             glVertex2f(0.44, -2.32);
             glEnd();
              glColor3f(0.7f, 0.5f, 0.3f);
              glBegin(GL_POLYGON);
                     glVertex2f(-2.65, 3.10);
                   glVertex2f(0.32, 4.23);
                   glVertex2f(3.37, 3.52);
                   glVertex2f(0.39, 2.38);

             glEnd();
             glColor3f(0.0f, 0.0f, 0.0f);
              glBegin(GL_LINE_LOOP);
                     glVertex2f(-2.65, 3.10);
                   glVertex2f(0.32, 4.23);
                   glVertex2f(3.37, 3.52);
                   glVertex2f(0.39, 2.38);

             glEnd();

            glBegin(GL_LINES);
             glVertex2f(-2.65, 3.10);
               glVertex2f(3.37, 3.52);
             glEnd();
             glBegin(GL_LINES);
             glVertex2f(0.32, 4.23);
              glVertex2f(0.39, 2.38);
             glEnd();
}

void wallScene()
{
    glPushMatrix();
        glTranslatef(4.0f, 3.0f, 0.0f);
        glScalef(6.0f, 6.9f, 1.0f);
        Tree1();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-3.0f, 3.5f, 0.0f);
        glScalef(6.5f, 7.9f, 1.0f);
        Tree1();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0f, 3.0f, 0.0f);
        glScalef(6.0f, 6.9f, 1.0f);
        Tree1();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-7.0f, 2.5f, 0.0f);
        glScalef(5.5f, 6.9f, 1.0f);
        Tree1();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(4.0f, 3.0f, 0.0f);
        glScalef(2.5f, 2.5f, 1.6f);
        drawwall();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-5.5f, -0.5f, 0.0f);
        glScalef(7.0f, 7.0f, 1.0f);
        electricPole();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(8.5f, -0.5f, 0.0f);
        glScalef(7.0f, 7.0f, 1.0f);
        electricPole();
    glPopMatrix();

     glPushMatrix();
        glTranslatef(2.0f, -5.f, 0.0f);
        glScalef(0.250f, 0.250f, 1.0f);
        //drawBox();
    glPopMatrix();

}

void houseScene()
{

        glPushMatrix();
        glScalef(1.5f,1.5f,1.0f);
        drawHome();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(8.5f, -0.5f, 0.0f);
        glScalef(7.0f, 7.0f, 1.0f);
        electricPole();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-5.5f, -0.5f, 0.0f);
        glScalef(7.0f, 7.0f, 1.0f);
        electricPole();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-8.0f, -2.650f, 0.0f);
        glScalef(.7f, .70f, 1.0f);
        drawFlower();
        glPopMatrix();



}

void placeStones()
{
    float minDistance = 5.0f;

    float margin = 2.0f;
    float minLocalX = -9.0f + margin;
    float maxLocalX =  9.0f - margin;

    for (int i = 0; i < TOTAL_STONES; i++) {
        bool valid = false;

        while (!valid) {
            int sceneIndex = rand() % totalScenes;
            float localX = minLocalX + (rand() / (float)RAND_MAX) * (maxLocalX - minLocalX);
            float candidateX = sceneIndex * sceneWidth + localX;

            valid = true;
            for (int j = 0; j < i; j++) {
                if (fabs(candidateX - stoneX[j]) < minDistance) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                stoneX[i] = candidateX;
                stoneY[i] = -7.5f;     // ground
            }
        }
    }
}

void drawStone()
{


// FRONT POLYGON[1]

    glColor3f(1.0f, 0.6f, 0.2f);
            glBegin(GL_POLYGON);
                glVertex2f(-1.11f, 2.25f);
                glVertex2f(-1.11f, 4.09f);
                glVertex2f(-0.63f, 4.64f);
                glVertex2f(-0.54f, 5.56f);
                glVertex2f(0.39f, 6.06f);
                glVertex2f(0.85f, 6.98f);
                glVertex2f(1.5f, 7.30f);
                glVertex2f(1.8f,5.27f);
                glVertex2f(2.40f, 5.03f);
                glVertex2f(2.40f, 2.77f);
    glEnd();

    // 1ST B0TTOM[2]

           glColor3f(0.8f, 0.4f, 0.2f);
            glBegin(GL_POLYGON);
                glVertex2f(-2.3f, 2.41f);
                 glVertex2f(-2.1f, 3.85f);
                glVertex2f(-1.11f, 4.09f);
                 glVertex2f(-1.11f, 2.25f);

             glEnd();



             // B0TTOM UP[3]
               glColor3f(1.0f, 0.6f, 0.2f);
            glBegin(GL_POLYGON);

                 glVertex2f(-2.1f, 3.85f);
                 glVertex2f(-1.41f, 4.32f);
                 glVertex2f(-0.63f, 4.64f);
                 glVertex2f(-1.11f, 4.09f);


             glEnd();
            glColor3f(0.7f, 0.4f, 0.2f);
            glBegin(GL_LINE_LOOP);

                 glVertex2f(-2.1f, 3.85f);
                 glVertex2f(-1.41f, 4.32f);
                 glVertex2f(-0.63f, 4.64f);
                 glVertex2f(-1.11f, 4.09f);


             glEnd();

     // B0TT0M UP UP[4]
             glColor3f(0.7f, 0.4f, 0.2f);
            glBegin(GL_POLYGON);

                 glVertex2f(-1.41f, 4.32f);
                 glVertex2f(-1.35f, 5.72f);
                  glVertex2f(-0.54f, 5.56f);
                  glVertex2f(-0.63f, 4.64f);

             glEnd();

// BOTTOM UP 3[5]
          glColor3f(1.0f, 0.6f, 0.2f);
            glBegin(GL_POLYGON);


                 glVertex2f(-1.35f, 5.72f);
                  glVertex2f(-0.49f, 6.17);
                   glVertex2f(0.39f, 6.06f);
                    glVertex2f(-0.54f, 5.56f);

             glEnd();
             glLineWidth(2.0);
             glColor3f(0.7f, 0.4f, 0.2f);
            glBegin(GL_LINE_STRIP);


                 glVertex2f(-1.35f, 5.72f);
                  glVertex2f(-0.49f, 6.17);
                   glVertex2f(0.39f, 6.06f);
                    glVertex2f(-0.54f, 5.56f);

             glEnd();
//POLYGON[6]
               glColor3f(0.8f, 0.5f, 0.2f);
            glBegin(GL_POLYGON);


                  glVertex2f(-0.49f, 6.17);
                  glVertex2f(0.14f, 7.15);
                  glVertex2f(0.85f, 6.98f);
                  glVertex2f(0.39f, 6.06f);


             glEnd();
             /* glColor3f(0.8f, 0.4f, 0.2f);
            glBegin(GL_LINE_LOOP);


                  glVertex2f(-0.49f, 6.17);
                  glVertex2f(0.14f, 7.15);
                  glVertex2f(0.85f, 6.98f);
                  glVertex2f(0.39f, 6.06f);


             glEnd();*/

    //POLY[7[7]
     glColor3f(1.0f, 0.6f, 0.2f);
            glBegin(GL_POLYGON);


                   glVertex2f(0.14f, 7.15);
                  glVertex2f(0.80f, 7.7);
                  glVertex2f(1.6f, 7.7f);
                  glVertex2f(1.5f, 7.30f);
                   glVertex2f(0.85f, 6.98f);

             glEnd();
             //
//POLY[8]
     glColor3f(0.7f, 0.4f, 0.2f);
            glBegin(GL_POLYGON);

                  glVertex2f(1.6f, 7.7f);
                  glVertex2f(2.0f, 5.6);
                  glVertex2f(1.8f, 5.27f);
                  glVertex2f(1.5f, 7.30f);

             glEnd();

    //POLY[9]
    glColor3f(1.0f, 0.6f, 0.2f);
            glBegin(GL_POLYGON);

                  glVertex2f(2.0f, 5.6);
                  glVertex2f(3.0f, 5.40f);
                  glVertex2f(2.40f, 5.03f);
                  glVertex2f(1.8f, 5.27f);


             glEnd();
     glColor3f(1.0f, 0.6f, 0.2f);
            glBegin(GL_LINE_LOOP);

                  glVertex2f(2.0f, 5.6);
                  glVertex2f(3.0f, 5.40f);
                  glVertex2f(2.40f, 5.03f);
                  glVertex2f(1.8f, 5.27f);


             glEnd();

    //POLY[10]
    glColor3f(0.7f, 0.4f, 0.2f);
            glBegin(GL_POLYGON);


                  glVertex2f(3.0f, 5.40f);
                   glVertex2f(3.30f, 3.80f);

                   glVertex2f(2.40f, 2.77f);
                   glVertex2f(2.40f, 5.03f);

             glEnd();
     glColor3f(1.0f, 0.6f, 0.2f);
            glBegin(GL_LINE_LOOP);


                  glVertex2f(3.0f, 5.40f);
                   glVertex2f(3.30f, 3.80f);

                   glVertex2f(2.40f, 2.77f);
                   glVertex2f(2.40f, 5.03f);

             glEnd();
             glLineWidth(3.0);
     glColor3f(0.7f, 0.4f, 0.0f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(-1.11f, 2.25f);
                glVertex2f(-1.11f, 4.09f);
                glVertex2f(-0.63f, 4.64f);
                glVertex2f(-0.54f, 5.56f);
                glVertex2f(0.39f, 6.06f);
                glVertex2f(0.85f, 6.98f);
                glVertex2f(1.5f, 7.30f);
                glVertex2f(1.8f,5.27f);
                glVertex2f(2.40f, 5.03f);
                glVertex2f(2.40f, 2.77f);
    glEnd();

             glLineWidth(2.0);
         glColor3f(0.9f, 0.5f, 0.2f);
            glBegin(GL_LINES);
           glVertex2f(1.8f,5.27f);
            glVertex2f(1.7f,4.20f);
             glEnd();

              glLineWidth(2.0);
         glColor3f(0.9f, 0.5f, 0.2f);
            glBegin(GL_LINES);
           glVertex2f(-0.63f, 4.64f);
            glVertex2f(-0.50f,3.90f);
             glEnd();

             glLineWidth(2.0);
         glColor3f(0.9f, 0.5f, 0.2f);
            glBegin(GL_LINES);
          glVertex2f(0.39f, 6.06f);
            glVertex2f(0.40f,4.0);
             glEnd();
               glBegin(GL_POLYGON);
            for( int i=0;i<200;i++)
                {
            glColor3f(0.9f,0.5f,0.0f);
            float pi= 3.1416;
            float A =(i*2*pi)/200;
            float r = 0.1;
            float x= 2.40+r* cos(A);
            float y= 3.60+r* sin(A);
            glVertex2f(x,y);
                }
            glEnd();
             glBegin(GL_POLYGON);
            for( int i=0;i<200;i++)
                {
            glColor3f(0.9f,0.5f,0.0f);
            float pi= 3.1416;
            float A =(i*2*pi)/200;
            float r = 0.1;
            float x= 1.40+r* cos(A);
            float y= 7.60+r* sin(A);
            glVertex2f(x,y);
                }
            glEnd();
             glBegin(GL_POLYGON);
            for( int i=0;i<200;i++)
                {
            glColor3f(0.9f,0.5f,0.0f);
            float pi= 3.1416;
            float A =(i*2*pi)/200;
            float r = 0.1;
            float x= 0.20+r* cos(A);
            float y= 4.60+r* sin(A);
            glVertex2f(x,y);
                }
            glEnd();
             glBegin(GL_POLYGON);
            for( int i=0;i<200;i++)
                {
            glColor3f(0.9f,0.5f,0.0f);
            float pi= 3.1416;
            float A =(i*2*pi)/200;
            float r = 0.1;
            float x= 1.70+r* cos(A);
            float y= 4.20+r* sin(A);
            glVertex2f(x,y);
                }
            glEnd();

             glBegin(GL_POLYGON);
            for( int i=0;i<200;i++)
                {
            glColor3f(0.9f,0.5f,0.0f);
            float pi= 3.1416;
            float A =(i*2*pi)/200;
            float r = 0.1;
            float x= 1.20+r* cos(A);
            float y= 5.80+r* sin(A);
            glVertex2f(x,y);
                }
            glEnd();

             //SMALL STONES1
                float dx = 1.0f;
            float dy = 0.0f;
            glColor3f(0.94f, 0.5f, 0.0f);
            glBegin(GL_POLYGON);

            // 1
            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*-1.29f + 2*(1-t)*t*-1.26f + t*t*-0.55f;
                float y = (1-t)*(1-t)* 2.36f + 2*(1-t)*t* 3.31f + t*t* 3.95f;
                glVertex2f(x + dx, y - dy);
            }

            // 2
            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*-0.55f + 2*(1-t)*t*0.1f + t*t*0.3f;
                float y = (1-t)*(1-t)* 3.95f + 2*(1-t)*t*3.58f + t*t*3.26f;
                glVertex2f(x + dx, y - dy);
            }

            // 3
            for(float t=0.0f; t<=1.0f; t+=0.02f){
                float x = (1-t)*(1-t)*0.3f + 2*(1-t)*t*0.5f + t*t*0.4f;
                float y = (1-t)*(1-t)*3.26f + 2*(1-t)*t*2.80f + t*t*2.30f;
                glVertex2f(x + dx, y - dy);
            }

            // 4
            for(float t=1.0f; t>=0.0f; t-=0.01f){
                float x = (1-t)*(1-t)*0.0f + 2*(1-t)*t*0.1f + t*t*0.4f;
                float y = (1-t)*(1-t)*2.20f + 2*(1-t)*t*2.04f + t*t*2.30f;
                glVertex2f(x + dx, y - dy);
            }

            // 5
            for(float t=1.0f; t>=0.0f; t-=0.01f){
                float x = (1-t)*(1-t)*-1.29f + 2*(1-t)*t*-0.10f + t*t*0.0f;
                float y = (1-t)*(1-t)* 2.36f + 2*(1-t)*t* 2.00f + t*t*2.20f;
                glVertex2f(x + dx, y - dy);
            }

            glEnd();
            // BORDER (Outline)
             dx = 1.0f;
             dy = 0.0f;

            glLineWidth(2.5f);
            glColor3f(0.8f, 0.4f, 0.0f);

            // 1
            glBegin(GL_LINE_STRIP);
            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*-1.29f + 2*(1-t)*t*-1.26f + t*t*-0.55f;
                float y = (1-t)*(1-t)* 2.36f + 2*(1-t)*t* 3.31f + t*t* 3.95f;
                glVertex2f(x + dx, y - dy);
            }
            glEnd();

            // 2
            glBegin(GL_LINE_STRIP);
            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*-0.55f + 2*(1-t)*t*0.1f + t*t*0.3f;
                float y = (1-t)*(1-t)* 3.95f + 2*(1-t)*t*3.58f + t*t*3.26f;
                glVertex2f(x + dx, y - dy);
            }
            glEnd();

            // 3
            glBegin(GL_LINE_STRIP);
            for(float t=0.0f; t<=1.0f; t+=0.02f){
                float x = (1-t)*(1-t)*0.3f + 2*(1-t)*t*0.5f + t*t*0.4f;
                float y = (1-t)*(1-t)*3.26f + 2*(1-t)*t*2.80f + t*t*2.30f;
                glVertex2f(x + dx, y - dy);
            }
            glEnd();

            // 5
            glBegin(GL_LINE_STRIP);
            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*0.0f + 2*(1-t)*t*0.1f + t*t*0.4f;
                float y = (1-t)*(1-t)*2.20f + 2*(1-t)*t*2.04f + t*t*2.30f;
                glVertex2f(x + dx, y - dy);
            }
            glEnd();

            // 4
            glBegin(GL_LINE_STRIP);
            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*-1.29f + 2*(1-t)*t*-0.10f + t*t*0.0f;
                float y = (1-t)*(1-t)* 2.36f + 2*(1-t)*t* 2.00f + t*t*2.20f;
                glVertex2f(x + dx, y - dy);
            }
            glEnd();


                // SMALL STONE 2
                glColor3f(0.94f, 0.5f, 0.0f);

                glBegin(GL_POLYGON);



            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*-1.29f + 2*(1-t)*t*-1.26f + t*t*-0.55f;
                float y = (1-t)*(1-t)* 2.36f + 2*(1-t)*t* 3.31f + t*t* 3.95f;
                glVertex2f(x,y);
            }


            for(float t=0.0f; t<=1.0f; t+=0.01f){
                float x = (1-t)*(1-t)*-0.55f + 2*(1-t)*t*0.1f + t*t*0.3f;
                float y = (1-t)*(1-t)* 3.95f + 2*(1-t)*t*3.58f + t*t*3.26f;
                glVertex2f(x,y);
            }


            for(float t=0.0f; t<=1.0f; t+=0.02f){
                float x = (1-t)*(1-t)*0.3f + 2*(1-t)*t*0.5f + t*t*0.4f;
                float y = (1-t)*(1-t)*3.26f + 2*(1-t)*t*2.80f + t*t*2.30f;
                glVertex2f(x,y);
            }



            for(float t=1.0f; t>=0.0f; t-=0.01f){
                float x = (1-t)*(1-t)*0.0f + 2*(1-t)*t*0.1f + t*t*0.4f;
                float y = (1-t)*(1-t)*2.20f + 2*(1-t)*t*2.04f + t*t*2.30f;
                glVertex2f(x,y);
            }

            for(float t=1.0f; t>=0.0f; t-=0.01f){
                float x = (1-t)*(1-t)*-1.29f + 2*(1-t)*t*-0.10f + t*t*0.0f;
                float y = (1-t)*(1-t)* 2.36f + 2*(1-t)*t* 2.00f + t*t*2.20f;
                glVertex2f(x,y);
            }

            glEnd();
            //SMALL STONE BORDER
                glLineWidth(3.0);
                glColor3f(0.7f, 0.4f, 0.0f);
                glBegin(GL_LINE_STRIP);

                for(float t = 0.0f; t <= 1.0f; t += 0.01f)
                {
                    float x =(1 - t)*(1 - t)*-1.29f +2*(1 - t)*t*-1.26f +t*t*-0.55f;
                    float y =(1 - t)*(1 - t)*2.36f +2*(1 - t)*t*3.31f +t*t*3.95f;
                    glVertex2f(x, y);
                }
                glEnd();

                 glLineWidth(3.0);
                glColor3f(0.8f, 0.4f, 0.0f);
                glBegin(GL_LINE_STRIP);

                for(float t = 0.0f; t <= 1.0f; t += 0.01f)
                {
                    float x =(1 - t)*(1 - t)*-0.55f +2*(1 - t)*t*0.1f +t*t*0.3f;
                    float y =(1 - t)*(1 - t)*3.95f +2*(1 - t)*t*3.58f +t*t*3.26f;
                    glVertex2f(x, y);
                }

                glEnd();
                 glLineWidth(3.0);
                glColor3f(0.8f, 0.4f, 0.0f);
                glBegin(GL_LINE_STRIP);

                for(float t = 0.0f; t <= 1.0f; t += 0.02f)
                {
                    float x =
                    (1 - t)*(1 - t)*0.3f +2*(1 - t)*t*0.5f +t*t*0.4f;
                    float y =(1 - t)*(1 - t)*3.26f +2*(1 - t)*t*2.80f +t*t*2.30f;
                    glVertex2f(x, y);
                }
                 glEnd();
                 glLineWidth(3.0);
                glColor3f(0.7f, 0.4f, 0.0f);
                glBegin(GL_LINE_STRIP);

                for(float t = 0.0f; t <= 1.0f; t += 0.01f)
                {
                    float x =(1 - t)*(1 - t)*-1.29f +2*(1 - t)*t*-0.10 +t*t*0.0f;
                    float y =(1 - t)*(1 - t)*2.36f +2*(1 - t)*t*2.00f +t*t*2.20f;
                    glVertex2f(x, y);
                }
                glEnd();

                 glLineWidth(3.0);
                glColor3f(0.7f, 0.4f, 0.0f);
                glBegin(GL_LINE_STRIP);

                for(float t = 0.0f; t <= 1.0f; t += 0.01f)
                {
                    float x =(1 - t)*(1 - t)*0.0+2*(1 - t)*t*0.1 +t*t*0.4f;
                    float y =(1 - t)*(1 - t)*2.20f +2*(1 - t)*t*2.04f +t*t*2.30f;
                    glVertex2f(x, y);
                }
                glEnd();

}

void drawStones()
{
    for (int i = 0; i < TOTAL_STONES; i++) {
        float drawX = stoneX[i] + worldX;
        float drawY = stoneY[i];

        glPushMatrix();
        glTranslatef(drawX, drawY, 0.0f);


        glScalef(STONE_SCALE*3.2, STONE_SCALE*3.2, 1.0f);
        glTranslatef(0.0f, -2.25, 0.0f);

        drawStone();
        glPopMatrix();
    }
  }
void checkStones()
{
    float charScreenX = charX + 4.75f;
    float charScreenY = charY + 0.45f;

    for (int i = 0; i < TOTAL_STONES; i++) {
        float sx = stoneX[i] + worldX;
        float sy = stoneY[i] + 0.35f;

        bool hit = centerAABB(charScreenX, charScreenY, CHAR_HB_W, CHAR_HB_H,
                              sx, sy, STONE_W * 2.0f, STONE_H * 1.5f);

        if (hit) {
            damagePlayer();
        }
    }

    finishGameOverIfNeeded();
}


void signBoard()
{
    float cx = 0.0f;
    float cy = 0.0f;   // IMPORTANT: center for rotation
    float r = 1.5f;

    // Board fill
    glColor3f(0.9f, 0.75f, 0.45f);
    glBegin(GL_POLYGON);
    for(int i = 0; i <= 360; i++)
    {
        float a = i * 3.1416f / 180.0f;
        glVertex2f(cx + r * cos(a), cy + r * sin(a));
    }
    glEnd();

    // Inner panel
    glColor3f(0.98f, 0.90f, 0.75f);
    glBegin(GL_POLYGON);
    for(int i = 0; i <= 360; i++)
    {
        float a = i * 3.1416f / 180.0f;
        glVertex2f(cx + (r - 0.3f) * cos(a), cy + (r - 0.3f) * sin(a));
    }
    glEnd();

    glLineWidth(3.0f);
    glColor3f(0.3f, 0.15f, 0.05f);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i <= 360; i++)
    {
        float a = i * 3.1416f / 180.0f;
        glVertex2f(cx + r * cos(a), cy + r * sin(a));
    }
    glEnd();

    glColor3f(0.2f, 0.2f, 0.2f);
    for(float x = -r+0.2f; x <= r-0.2f; x += 2*r-0.4f)
    {
        glBegin(GL_POLYGON);
        for(int i = 0; i < 360; i++)
        {
            float a = i * 3.1416f / 180.0f;
            glVertex2f(x + 0.06f * cos(a), cy + 0.05f + 0.06f * sin(a));
        }
        glEnd();
    }
}
void drawRotatingSignBoard(float x, float y, int i)
{
    glPushMatrix();

    glTranslatef(x, y, 0.0f);                 // obstacle position
    glRotatef(signAngle[i], 0.0f, 0.0f, 1.0f); // rotate around center

    signBoard(); // draw the object

    glPopMatrix();
}

void placeManholeFire()
{
    float minX = -7.0f;
    float maxX =  7.0f;

    for (int i = 0; i < TOTAL_FIRES; i++) {
        int sceneIndex = rand() % totalScenes;
        float localX = minX + (rand() / (float)RAND_MAX) * (maxX - minX);

        fireX[i] = sceneIndex * sceneWidth + localX;
        fireY[i] = -6.5f;          // ground  manhole position

        fireOn[i] = false;
        fireTimer[i] = 0.0f;
        fireHit[i] = false;
    }
}
void updateManholeFire()
{
    for (int i = 0; i < TOTAL_FIRES; i++) {
        fireTimer[i]++;
        if (fireOn[i]) {
            if (fireTimer[i] > FIRE_ON_TIME) {
                fireOn[i] = false;
                fireTimer[i] = 0.0f;
                fireHit[i] = false;
            }
        }else {
            if (fireTimer[i] > FIRE_OFF_TIME) {
                fireOn[i] = true;
                fireTimer[i] = 0.0f;
            }
        }
    }
}
void drawManholeFire()
{
    for (int i = 0; i < TOTAL_FIRES; i++) {

        float drawX = fireX[i] + worldX;
        float drawY = fireY[i];

        // ---- Manhole static ----
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_POLYGON);
        for (int a = 0; a < 360; a++) {
            float rad = a * 3.1416f / 180.0f;
            glVertex2f(drawX + 0.6f * cos(rad), drawY + 0.15f * sin(rad));
        }
        glEnd();

        // ---- Fire (only when ON) ----
        if (fireOn[i]) {
            float flameH = 1.2f + 0.2f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.01f);

            glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.4f, 0.0f);
            glVertex2f(drawX - 0.3f, drawY);
            glVertex2f(drawX + 0.3f, drawY);
            glColor3f(1.0f, 0.9f, 0.0f);
            glVertex2f(drawX, drawY + flameH);
            glEnd();
        }
    }
}
void checkManholeFire()
{
    float charScreenX = charX + 4.75f;
    float charScreenY = charY + 0.45f;

    for (int i = 0; i < TOTAL_FIRES; i++) {
        if (!fireOn[i]) {
            fireHit[i] = false;
            continue;
        }

        float fx = fireX[i] + worldX;
        float fy = fireY[i] + 0.65f;  // flame centre

        bool hit = centerAABB(charScreenX, charScreenY, CHAR_HB_W, CHAR_HB_H,
                              fx, fy, 1.2f, 1.3f);

        if (hit) {
            fireHit[i] = true;
            damagePlayer();
        } else {
            fireHit[i] = false;
        }
    }

    finishGameOverIfNeeded();
}


void drawBall(float x, float y) {
    int numSegments = 40;
float radius = 1.0f;
glBegin(GL_TRIANGLE_FAN);

// --- Center color (darker pink) ---
glColor3f(0.9f, 0.3f, 0.6f);
glVertex2f(x, y);   // center

for (int i = 0; i <= numSegments; i++) {
    float angle = 2.0f * 3.14159f * i / numSegments;
    float px = x + cos(angle) * radius;
    float py = y + sin(angle) * radius;

    // --- Edge color (lighter pink) ---
    glColor3f(1.0f, 0.7f, 0.85f);
    glVertex2f(px, py);
}

glEnd();
}

void drawMouse(float x, float y, float scale = 1.0f) {
    glBegin(GL_LINE_LOOP);
    glLineWidth(3);
    glColor3f(0.0f, 0.0f, 0.0f); // outline
    glVertex2f(x + (-0.4f * scale), y + (0.0f * scale));
    glVertex2f(x + (-0.04f * scale), y + (0.12f * scale));
    glVertex2f(x + (-0.05f * scale), y + (-0.15f * scale));
    glEnd();

    drawCircle(x + 0.06999f * scale, y + -0.02158f * scale, 0.177f * scale, 0.75f, 0.75f, 0.78f); // body

    glBegin(GL_POLYGON);
    glColor3f(0.75f, 0.75f, 0.78f);
    glVertex2f(x + (-0.4f * scale), y + (0.0f * scale));
    glVertex2f(x + (-0.04f * scale), y + (0.12f * scale));
    glVertex2f(x + (-0.05f * scale), y + (-0.15f * scale));
    glEnd();

    drawCircle(x + -0.09946f * scale, y + 0.15048f * scale, 0.112f * scale, 0.75f, 0.75f, 0.78f); // ear
    drawCircle(x + -0.10468f * scale, y + 0.15569f * scale, 0.055f * scale, 0.95f, 0.7f, 0.75f);  // inner ear
    drawCircle(x + -0.4f * scale, y + 0.0f * scale, 0.02f * scale, 0.0f, 0.0f, 0.0f);              // nose
    drawCircle(x + -0.2f * scale, y + 0.02f * scale, 0.02f * scale, 0.0f, 0.0f, 0.0f);            // eye

    // Tail
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(x + 0.24f * scale, y + -0.01f * scale);
        glVertex2f(x + 0.4f * scale, y + 0.06f * scale);
    glEnd();

    // Whiskers
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        glVertex2f(x + -0.05f * scale, y + -0.01f * scale); glVertex2f(x + -0.15f * scale, y + -0.01f * scale);
        glVertex2f(x + -0.05f * scale, y + -0.03f * scale); glVertex2f(x + -0.15f * scale, y + -0.03f * scale);
    glEnd();

    glFlush();
}

void drawSpikyFruit(float x, float y, float scale = 1.0f) {
    float radius = 0.2f * scale;
    int spikes = 20;

    drawCircle(x, y, radius, 0.0f, 0.0f, 0.0f); // center

    for (int i = 0; i < spikes; i++) {
        float angle = 2.0f * 3.1415926f * i / spikes;
        float spikeLength = radius * 0.5f;

        glBegin(GL_TRIANGLES);
        glColor3f(0.0f, 0.0f, 0.0f);

        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
        glVertex2f(x + radius * cos(angle + 0.1f), y + radius * sin(angle + 0.1f));
        glVertex2f(x + (radius + spikeLength) * cos(angle + 0.05f), y + (radius + spikeLength) * sin(angle + 0.05f));

        glEnd();
    }
    glFlush();
}

void placeCollectibles() {
    float minDistance = 3.0f;     // 10 cakes thats why
    float minX = 0.0f;
    float maxX = 12.0f;

    // Dora cakes (10 per level)
    for (int i = 0; i < 10; i++) {
        bool valid = false;
        while (!valid) {
            int sceneIndex = rand() % totalScenes;
            float localX = minX + (rand() / (float)RAND_MAX) * (maxX - minX);
            float candidateX = sceneIndex * sceneWidth + localX;

            valid = true;
            for (int j = 0; j < i; j++) {
                if (fabs(candidateX - doracakeX[j]) < minDistance) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                doracakeX[i] = candidateX;
                doracakeY[i] = -6.0f;
                doracakeCollected[i] = false;
            }
        }
    }

    // Hearts
    for (int i = 0; i < 3; i++) {
        bool valid = false;
        while (!valid) {
            int sceneIndex = rand() % totalScenes;
            float localX = minX + (rand() / (float)RAND_MAX) * (maxX - minX);
            float candidateX = sceneIndex * sceneWidth + localX;

            valid = true;
            for (int j = 0; j < i; j++) {
                if (fabs(candidateX - heartX[j]) < minDistance) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                heartX[i] = candidateX;
                heartY[i] = -6.0f + 0.5f;
                heartLost[i] = false;
            }
        }
    }

    doracakesCollected = 0;
    lives = totalHearts;
}

void checkCollectibles() {
    float collectRadius = 2.0f;

    float charScreenX = charX + 4.75f;
    float charScreenY = charY + 0.1f;

    // Dora cakes (10 per level)
    for (int i = 0; i < 10; i++) {
        if (!doracakeCollected[i]) {
            float cakeScreenX = doracakeX[i] + worldX;
            float cakeScreenY = doracakeY[i];

            float dx = charScreenX - cakeScreenX;
            float dy = charScreenY - cakeScreenY;

            if (dx * dx + dy * dy < collectRadius * collectRadius) {
                doracakeCollected[i] = true;
                doracakesCollected++;
                score += 100;
                levelScore += 100;
                updateHiScoreForCurrentLevel();
                playSound(SND_COLLECT);
            }
        }
    }

    // Hearts
    for (int i = 0; i < 3; i++) {
        if (!heartLost[i]) {
            float heartScreenX = heartX[i] + worldX;
            float heartScreenY = heartY[i];

            float dx = charScreenX - heartScreenX;
            float dy = charScreenY - heartScreenY;

            if (dx * dx + dy * dy < collectRadius * collectRadius) {
                heartLost[i] = true;
                if (lives < totalHearts) {
                    lives++;
                }
            }
        }
    }

    // Save the level high score and return to the main menu.
    if (doracakesCollected >= 10 && !showEndScreen && !showHome) {
        showEndScreen = true;
        gameWon = true;
        moving = false;
        jumping = false;
        updateHiScoreForCurrentLevel();
        saveHiScore();
        playSoundForce(SND_LEVELUP);
    }
}


void placeObstacles() {
    float minDistance = 6.0f;
    float minX = -3.0f;
    float maxX = 9.0f;

    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
        bool valid = false;

        while (!valid) {
            int sceneIndex = rand() % totalScenes;
            float localX = minX + (rand() / (float)RAND_MAX) * (maxX - minX);
            float candidateX = sceneIndex * sceneWidth + localX;

            valid = true;

            for (int j = 0; j < i; j++) {
                if (fabs(candidateX - obstaclePosX[j]) < minDistance) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                obstaclePosX[i] = candidateX;
                obstaclePosY[i] = -6.0f;

                obstacleLevel[i] = currentLevel;
                obstacleEnabled[i] = true;
                obstacleHit[i] = false;
                obstacleRespawnTimer[i] = 0;
                obstacleType[i] = rand() % 3;

                // Horizontal movement (Left–Right)
                if (rand() % 3 == 0) {
                    obstacleMoveSpeedX[i] = 0.03f;
                    obstacleMoveDirX[i] = 1;
                    obstacleLeftBound[i]  = sceneIndex * sceneWidth - 9.0f;
                    obstacleRightBound[i] = sceneIndex * sceneWidth + 9.0f;
                } else {
                    obstacleMoveSpeedX[i] = 0.0f;
                    obstacleMoveDirX[i] = 0;
                    obstacleLeftBound[i]  = 0;
                    obstacleRightBound[i] = 0;
                }

                // Vertical movement (Up–Down / falling)

                int verticalType = rand() % 3;
                if (verticalType == 0) {
                    obstacleMoveSpeedY[i] = 0.02f;
                    obstacleMoveDirY[i] = 1;
                    obstacleTopBound[i] = -4.0f;
                    obstacleBottomBound[i] = -7.0f;
                } else if (verticalType == 1) {
                    obstacleMoveSpeedY[i] = 0.04f;
                    obstacleMoveDirY[i] = -1;
                    obstacleTopBound[i] = obstaclePosY[i];
                    obstacleBottomBound[i] = -10.0f;
                } else { // static
                    obstacleMoveSpeedY[i] = 0.0f;
                    obstacleMoveDirY[i] = 0;
                    obstacleTopBound[i] = 0;
                    obstacleBottomBound[i] = 0;
                }
                signAngle[i] = 0.0f;
                signRotateSpeed[i] = 3.0f;
            }
        }
    }
}

void updateObstacles() {
    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
        if (!obstacleEnabled[i]) continue;
        if (obstacleLevel[i] != currentLevel) continue;

        // Random respawn

        obstacleRespawnTimer[i]++;
        float screenX = obstaclePosX[i] + worldX;
        if ((screenX < -14.0f || screenX > 14.0f) &&
            obstacleRespawnTimer[i] > RESPAWN_INTERVAL) {
            respawnObstacle(i);
            obstacleLevel[i] = currentLevel;
            obstacleEnabled[i] = true;
        }

        obstaclePosX[i] += obstacleMoveSpeedX[i] * obstacleMoveDirX[i];
        if (obstacleMoveSpeedX[i] > 0) {
            if (obstaclePosX[i] >= obstacleRightBound[i])
                obstacleMoveDirX[i] = -1;
            if (obstaclePosX[i] <= obstacleLeftBound[i])
                obstacleMoveDirX[i] = 1;
        }

        obstaclePosY[i] += obstacleMoveSpeedY[i] * obstacleMoveDirY[i];
        if (obstacleMoveSpeedY[i] > 0) {
            if (obstacleMoveDirY[i] == 1 && obstaclePosY[i] >= obstacleTopBound[i])
                obstacleMoveDirY[i] = -1;
            if (obstacleMoveDirY[i] == -1 && obstaclePosY[i] <= obstacleBottomBound[i])
                obstacleMoveDirY[i] = 1;
        }
        signAngle[i] += signRotateSpeed[i];
        if (signAngle[i] >= 360.0f)
            signAngle[i] -= 360.0f;
    }
}

void checkObstacles() {
    float charScreenX = charX + 4.75f;
    float charScreenY = charY + 0.45f;   // center-ish body point for stable hits

    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
        if (!obstacleEnabled[i]) continue;
        if (obstacleLevel[i] != currentLevel) continue;

        float obsScreenX = obstaclePosX[i] + worldX;
        float obsScreenY = obstaclePosY[i];

        float obsW, obsH;
        getObstacleHitbox(obstacleType[i], currentLevel, obsW, obsH);

        bool hit = centerAABB(charScreenX, charScreenY, CHAR_HB_W, CHAR_HB_H,
                              obsScreenX, obsScreenY, obsW, obsH);

        if (hit) {
            obstacleHit[i] = true;
            damagePlayer();
        } else {
            obstacleHit[i] = false;
        }
    }

    finishGameOverIfNeeded();
}


void drawHUD() {
    char buffer[50];

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "Cakes: %d / 10", doracakesCollected);
    drawText(-9.2f, 8.5f, buffer);

    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(-8.9f + i * 0.8f, 7.5f, 0.0f);
        glScalef(0.7f, 0.7f, 1.0f);

        if (i < lives) {
            glColor3f(0.99f, 0.85f, 0.88f);
            Heart();
        }

        glPopMatrix();
    }

    // --------------- Score display in HUD ----------

    char scoreLine[64];
    sprintf(scoreLine, "Score: %d", score);
    glColor3f(1.0f, 0.95f, 0.3f);
    drawText(5.5f, 8.5f, scoreLine);

    char hiLine[64];
    sprintf(hiLine, "Level Best: %d", hiScore);
    glColor3f(0.5f, 1.0f, 0.5f);
    drawText(5.5f, 7.7f, hiLine);

    if (!gamePaused) {
        glColor3f(0.75f, 0.85f, 1.0f);
        drawText(-9.2f, -9.2f, "P = Pause");
    }

    glLineWidth(1.0f);    //reset line width after HUD
}


void drawLighting2()
{
      //LIGHTING

         glEnable(GL_DEPTH_TEST);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT1);
        glEnable(GL_COLOR_MATERIAL);

        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glShadeModel(GL_SMOOTH);

        GLfloat lightPos[]     = { 8.0f, 14.0f, 8.0f, 1.0f };
        GLfloat lightAmbient[] = { 0.10f, 0.08f, 0.07f, 1.0f };
        GLfloat lightDiffuse[] = { 0.60f, 0.45f, 0.30f, 1.0f };

        glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT1, GL_AMBIENT,  lightAmbient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE,  lightDiffuse);
}

void drawLighting1()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    GLfloat lightPos[]     = { 0.30f, 0.30f, 0.30f, 0.0f };   // sun position
    GLfloat lightAmbient[] = { 0.55f, 0.55f, 0.55f, 1.0f };   // day ambient
    GLfloat lightDiffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };  // bright sunlight

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
}

void Level1()
{
    currentLevel = 1;
    drawLighting1();

    // Sky
    glPushMatrix();
    glTranslatef(0.0f, -2.5f, 0.0f);
    glScalef(1.8f, 1.8f, 2.0f);
    drawSky1();
    glPopMatrix();

    // Scenes
    for (int i = 0; i <= totalScenes; i++)
    {
        glPushMatrix();
        glTranslatef(worldX + i * sceneWidth, 0.0f, 0.0f);
        if (i % 2 == 0)
            houseScene();
        else
            wallScene();
        glPopMatrix();
    }

    // Doraemon
    glPushMatrix();
    glTranslatef(charX, charY, 0.0f);
    glTranslatef(((0.0f+9.5f)/2.0f), 0.0f, 0.0f);
    glScalef((float)facing * 0.5f, 0.5f, 1.0f);
    glTranslatef(-((0.0f+9.5f)/2.0f), 0.0f, 0.0f);
    Doraemon();
    glPopMatrix();

    // Init level
    if (!level1Started) {
        placeCollectibles();
        placeStones();
        for (int i = 0; i < TOTAL_OBSTACLES; i++) {
            obstacleEnabled[i] = false;
            obstacleLevel[i] = 0;
            obstacleHit[i] = false;
        }
        level1Started = true;
    }

    // Doracakes
    for (int i = 0; i < 10; i++) {
        if (!doracakeCollected[i]) {
            glPushMatrix();
            glTranslatef(doracakeX[i] + worldX, doracakeY[i], 0.0f);
            glScalef(0.5f, 1.2f, 0.0f);
            Doracake();
            glPopMatrix();
        }
    }

    // Hearts
    for (int i = 0; i < 3; i++) {
        if (!heartLost[i]) {
            glPushMatrix();
            glTranslatef(heartX[i] + worldX, heartY[i], 0.0f);
            glScalef(1.5f, 1.2f, 0.0f);
            Heart();
            glPopMatrix();
        }
    }

    // Stones only when end screen not showing
    if (!showEndScreen) {
        drawStones();
    }

    // Clouds
    glDisable(GL_LIGHTING);
    for (int ci = 0; ci < TOTAL_CLOUDS; ci++)
        drawCloud(cloudX[ci], cloudY[ci], 0.9f);

    // Ground, HUD, effects always on top
    drawGroundStrip(0.25f, 0.60f, 0.18f);
    drawHUD();
    drawHitFlash();

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    if (showEndScreen) drawEndScreen();
    if (gamePaused) drawPauseScreen();

    glutSwapBuffers();
}



void Level2()
{
    currentLevel = 2;

    // Sky
    glPushMatrix();
    glTranslatef(-9.5f, -17.2f, 0.0f);
    glScalef(1.2f, 1.5f, 1.0f);
    glNormal3f(0.0f, 0.0f, 1.0f);
    drawSky2();
    glPopMatrix();

    // Scenes
    for (int i = 0; i <= totalScenes; i++)
    {
        glPushMatrix();
        glTranslatef(worldX + i * sceneWidth, 0.0f, 0.0f);
        if (i % 2 == 0)
            houseScene();
        else
            wallScene();
        glPopMatrix();
    }

    // Doraemon
    glPushMatrix();
    glTranslatef(charX, charY, 0.0f);
    glTranslatef(((0.0f+9.5f)/2.0f), 0.0f, 0.0f);
    glScalef((float)facing * 0.5f, 0.5f, 1.0f);
    glTranslatef(-((0.0f+9.5f)/2.0f), 0.0f, 0.0f);
    Doraemon();
    glPopMatrix();

    // Init level
    if (!level2Started)
    {
        placeCollectibles();
        placeObstacles();
        placeManholeFire();
        level2Started = true;
    }

    // Obstacles only when end screen not showing
    if (!showEndScreen) {
        for (int i = 0; i < TOTAL_OBSTACLES; i++) {
            if (!obstacleEnabled[i]) continue;
            if (obstacleLevel[i] != currentLevel) continue;

            float drawX = obstaclePosX[i] + worldX;
            float drawY = obstaclePosY[i];

            drawRotatingSignBoard(drawX, drawY, i);
        }
    }

    // Doracakes
    for (int i = 0; i < 10; i++) {
        if (!doracakeCollected[i]) {
            glPushMatrix();
            glTranslatef(doracakeX[i] + worldX, doracakeY[i], 0.0f);
            glScalef(0.5f, 1.2f, 0.0f);
            Doracake();
            glPopMatrix();
        }
    }

    // Hearts
    for (int i = 0; i < 3; i++) {
        if (!heartLost[i]) {
            glPushMatrix();
            glTranslatef(heartX[i] + worldX, heartY[i], 0.0f);
            glScalef(1.5f, 1.2f, 0.0f);
            Heart();
            glPopMatrix();
        }
    }

    // Ground, fire, HUD always on top
    drawGroundStrip(0.50f, 0.42f, 0.28f);
    drawManholeFire();
    drawHUD();
    drawHitFlash();

    if (showEndScreen) drawEndScreen();
    if (gamePaused) drawPauseScreen();

    glutSwapBuffers();
}

void Level3()
{
    currentLevel = 3;
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    // Sky
    glPushMatrix();
    glTranslatef(-9.5f, -17.2f, 0.0f);
    glScalef(1.2f, 1.5f, 1.0f);
    drawSky3();
    glPopMatrix();

    // Scenes
    for (int i = 0; i <= totalScenes; i++)
    {
        glPushMatrix();
        glTranslatef(worldX + i * sceneWidth, 0.0f, 0.0f);
        if (i % 2 == 0)
            houseScene();
        else
            wallScene();
        glPopMatrix();
    }

    // Doraemon
    glPushMatrix();
    glTranslatef(charX, charY, 0.0f);
    glTranslatef(((0.0f+9.5f)/2.0f), 0.0f, 0.0f);
    glScalef((float)facing * 0.5f, 0.5f, 1.0f);
    glTranslatef(-((0.0f+9.5f)/2.0f), 0.0f, 0.0f);
    Doraemon();
    glPopMatrix();

    // Init level
    if (!level3Started) {
        placeCollectibles();
        placeObstacles();
        level3Started = true;
    }

    // Dark overlay FIRST (background atmosphere)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.2f, 0.3f);   // opacity 0.3
    glBegin(GL_QUADS);
    glVertex2f(-9.5f, -10.0f);
    glVertex2f(9.5f, -10.0f);
    glVertex2f(9.5f, 10.0f);
    glVertex2f(-9.5f, 10.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Lamp posts
    for (int i = 0; i <= totalScenes; i++) {
        glPushMatrix();
        glTranslatef(worldX + i * sceneWidth, 0.0f, 0.0f);
        if (i % 2 == 0) {
            glTranslatef(3.5f, -4.2f, 0.0f);
            glScalef(8.2f, 6.5f, -1.0f);
            drawLampPost();
        } else {
            glTranslatef(3.5f, -4.2f, 0.0f);
            glScalef(8.2f, 8.5f, 1.0f);
            drawLampPost();
        }
        glPopMatrix();
    }

    // Obstacles AFTER overlay for visibility

    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
        if (!obstacleEnabled[i]) continue;
        if (obstacleLevel[i] != currentLevel) continue;

        float drawX = obstaclePosX[i] + worldX;
        float drawY = obstaclePosY[i];

        if (obstacleType[i] == 0) {
            drawBall(drawX, drawY);
        }
        else if (obstacleType[i] == 1) {
            drawMouse(drawX, drawY, 3.0f);
        }
        else if (obstacleType[i] == 2) {
            drawSpikyFruit(drawX, drawY, 3.0f);
        }
    }

    // Doracakes AFTER overlay so they are visible
    for (int i = 0; i < 10; i++) {
        if (!doracakeCollected[i]) {
            glPushMatrix();
            glTranslatef(doracakeX[i] + worldX, doracakeY[i], 0.0f);
            glScalef(0.5f, 1.2f, 0.0f);
            Doracake();
            glPopMatrix();
        }
    }

    // Hearts AFTER overlay so they are visible
    for (int i = 0; i < 3; i++) {
        if (!heartLost[i]) {
            glPushMatrix();
            glTranslatef(heartX[i] + worldX, heartY[i], 0.0f);
            glScalef(1.5f, 1.2f, 0.0f);
            Heart();
            glPopMatrix();
        }
    }

    // Ground, HUD, effects always on top
    drawGroundStrip(0.22f, 0.20f, 0.30f);
    drawHUD();
    drawHitFlash();

    if (showEndScreen) drawEndScreen();
    if (gamePaused) drawPauseScreen();

    glPopAttrib();
    glutSwapBuffers();
}

//---------- LEVEL 4 & 5 ======== SKY

void drawSky4()      // Rainy environment
{
    glBegin(GL_QUADS);
    glColor3f(0.70f, 0.22f, 0.08f);
    glVertex2f(0.0f, 20.0f);  glVertex2f(18.0f, 20.0f);
    glColor3f(0.92f, 0.52f, 0.14f);
    glVertex2f(18.0f,18.0f);  glVertex2f(0.0f, 18.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.92f, 0.52f, 0.14f);
    glVertex2f(0.0f, 18.0f);  glVertex2f(18.0f, 18.0f);
    glColor3f(0.97f, 0.74f, 0.32f);
    glVertex2f(18.0f,13.5f);  glVertex2f(0.0f, 13.5f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.97f, 0.74f, 0.32f);
    glVertex2f(0.0f, 13.5f);  glVertex2f(18.0f, 13.5f);
    glColor3f(0.60f, 0.57f, 0.65f);
    glVertex2f(18.0f,10.0f);  glVertex2f(0.0f, 10.0f);
    glEnd();
    glFlush();
}

void drawSky5()      // Storm Night
{
    glBegin(GL_QUADS);
    glColor3f(0.01f, 0.01f, 0.06f);
    glVertex2f(0.0f, 20.0f);  glVertex2f(18.0f, 20.0f);
    glColor3f(0.03f, 0.02f, 0.13f);
    glVertex2f(18.0f,18.0f);  glVertex2f(0.0f, 18.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.03f, 0.02f, 0.13f);
    glVertex2f(0.0f, 18.0f);  glVertex2f(18.0f, 18.0f);
    glColor3f(0.07f, 0.04f, 0.20f);
    glVertex2f(18.0f,13.5f);  glVertex2f(0.0f, 13.5f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.07f, 0.04f, 0.20f);
    glVertex2f(0.0f, 13.5f);  glVertex2f(18.0f, 13.5f);
    glColor3f(0.12f, 0.07f, 0.28f);
    glVertex2f(18.0f,10.0f);  glVertex2f(0.0f, 10.0f);
    glEnd();
    glFlush();
}




// -----  FALLING ROCKS  Level 4 obstacle

void initFallingRocks() {
    for (int i = 0; i < ROCK_COUNT; i++) {
        int si = rand() % totalScenes;
        rockPosX[i]    = si * sceneWidth + (-7.0f + (rand()/(float)RAND_MAX)*14.0f);
        rockPosY[i]    = 6.0f + (rand()/(float)RAND_MAX) * 5.0f;
        rockVelocity[i]= 0.055f + (rand()/(float)RAND_MAX) * 0.065f;
        rockIsHit[i]   = false;
    }
}

void updateFallingRocks() {
    for (int i = 0; i < ROCK_COUNT; i++) {
        rockPosY[i] -= rockVelocity[i];
        if (rockPosY[i] < -8.6f) {
            int si = rand() % totalScenes;
            rockPosX[i] = si * sceneWidth + (-7.0f + (rand()/(float)RAND_MAX)*14.0f);
            rockPosY[i] = 9.5f;
            rockIsHit[i] = false;
        }
    }
}

void drawFallingRocks() {

    static const float px[5] = { 0.00f,  0.55f,  0.35f, -0.35f, -0.55f};
    static const float py[5] = { 0.55f,  0.20f, -0.45f, -0.45f,  0.20f};

    for (int i = 0; i < ROCK_COUNT; i++) {
        float dx = rockPosX[i] + worldX;
        float dy = rockPosY[i];
        if (dx < -12.0f || dx > 12.0f) continue;

        // Fill
        glColor3f(0.45f, 0.40f, 0.36f);
        glBegin(GL_POLYGON);
        for (int j = 0; j < 5; j++)
            glVertex2f(dx + px[j], dy + py[j]);
        glEnd();

        // Dark outline
        glColor3f(0.22f, 0.18f, 0.16f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < 5; j++)
            glVertex2f(dx + px[j], dy + py[j]);
        glEnd();
        glLineWidth(1.0f);

        // Crack accent
        glColor3f(0.18f, 0.14f, 0.12f);
        glBegin(GL_LINES);
            glVertex2f(dx - 0.08f, dy + 0.30f);
            glVertex2f(dx + 0.22f, dy - 0.10f);
            glVertex2f(dx + 0.22f, dy - 0.10f);
            glVertex2f(dx + 0.05f, dy - 0.35f);
        glEnd();

        // Motion blur above rock
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.40f, 0.35f, 0.30f, 0.35f);
        glBegin(GL_QUADS);
            glVertex2f(dx - 0.35f, dy + 0.55f);
            glVertex2f(dx + 0.35f, dy + 0.55f);
            glVertex2f(dx + 0.20f, dy + 1.30f);
            glVertex2f(dx - 0.20f, dy + 1.30f);
        glEnd();
        glDisable(GL_BLEND);
    }
    glLineWidth(1.0f);
}

void checkFallingRocks() {
    if (showEndScreen) return;

    float csx = charX + 4.75f;
    float csy = charY + 0.45f;

    for (int i = 0; i < ROCK_COUNT; i++) {
        float rx = rockPosX[i] + worldX;
        float ry = rockPosY[i];

        bool hit = centerAABB(csx, csy, CHAR_HB_W, CHAR_HB_H,
                              rx, ry, 1.2f, 1.2f);

        if (hit) {
            rockIsHit[i] = true;
            damagePlayer();
        } else {
            rockIsHit[i] = false;
        }
    }

    finishGameOverIfNeeded();
}


// ---------- TWINKLING STARS Level 5 sky --------------

void initStars() {
    for (int i = 0; i < STAR_COUNT; i++) {
        starPosX[i]  = -9.5f + (rand()/(float)RAND_MAX) * 19.0f;
        starPosY[i]  =  1.0f + (rand()/(float)RAND_MAX) *  9.0f;
        starPhase[i] = (rand()/(float)RAND_MAX) * 6.2832f;
    }
}

void drawStars() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.2f);
    glBegin(GL_POINTS);
    for (int i = 0; i < STAR_COUNT; i++) {
        float bright = 0.45f + 0.55f * sinf(starTime5 * 0.028f + starPhase[i]);
        glColor4f(1.0f, 0.98f, 0.88f, bright);
        glVertex2f(starPosX[i], starPosY[i]);
    }
    glEnd();

    // larger "bright" stars
    glPointSize(3.8f);
    glBegin(GL_POINTS);
    for (int i = 0; i < STAR_COUNT; i += 12) {
        float bright = 0.60f + 0.40f * sinf(starTime5 * 0.018f + starPhase[i] + 1.0f);
        glColor4f(0.96f, 0.94f, 1.0f, bright);
        glVertex2f(starPosX[i], starPosY[i]);
    }
    glEnd();
    glPointSize(1.0f);
    glDisable(GL_BLEND);
}

// LIGHTNING  Level 5

void updateLightning() {
    lightningClock += 1.0f;
    if (lightningClock >= LIGHTNING_PERIOD) {
        lightningFlash = true;
        lightningClock = 0.0f;
        lightningBoltX = -4.0f + (rand()/(float)RAND_MAX) * 8.0f;
    } else if (lightningFlash && lightningClock > 10.0f) {
        lightningFlash = false;
    }
}

void drawLightning() {
    if (!lightningFlash) return;
    float lx = lightningBoltX;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Screen-wide glow
    glColor4f(0.88f, 0.94f, 1.0f, 0.22f);
    glBegin(GL_QUADS);
        glVertex2f(-9.5f,-10.0f); glVertex2f(9.5f,-10.0f);
        glVertex2f( 9.5f, 10.0f); glVertex2f(-9.5f, 10.0f);
    glEnd();

    // Main bolt – white core
    glColor4f(1.0f, 1.0f, 1.0f, 0.92f);
    glLineWidth(3.5f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(lx + 0.00f, 10.0f);
        glVertex2f(lx + 0.40f,  6.5f);
        glVertex2f(lx - 0.25f,  3.8f);
        glVertex2f(lx + 0.50f,  1.0f);
        glVertex2f(lx - 0.20f, -1.5f);
        glVertex2f(lx + 0.30f, -4.5f);
        glVertex2f(lx + 0.05f, -7.6f);
    glEnd();

    // Branch
    glColor4f(0.8f, 0.9f, 1.0f, 0.65f);
    glLineWidth(1.8f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(lx - 0.25f,  3.8f);
        glVertex2f(lx - 0.90f,  1.5f);
        glVertex2f(lx - 1.30f, -0.5f);
    glEnd();

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

//  GHOST ENEMY --- Level 5

void resetGhost() {
    ghostPosX  = -20.0f;    // always spawn far left
    ghostPosY  = -6.5f;
    ghostIsHit = false;
}

void drawGhostEnemy(float gx, float gy) {
    float sc = 1.25f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Outer glow
    glBegin(GL_POLYGON);
    for (int i = 0; i < 40; i++) {
        float a = i * 2.0f * 3.14159f / 40;
        float br = sc * (1.25f + 0.08f * sinf(a * 3));
        glColor4f(0.55f, 0.80f, 1.0f, 0.18f);
        glVertex2f(gx + br * cosf(a), gy + br * 1.3f * sinf(a));
    }
    glEnd();

    // Body – top semicircle
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 180; i++) {
        float a = i * 3.14159f / 180.0f;
        glColor4f(0.78f, 0.92f, 1.0f, 0.80f);
        glVertex2f(gx + sc * cosf(a), gy + sc * 1.4f * sinf(a));
    }
    // Wavy hem at the bottom
    for (int i = 0; i <= 8; i++) {
        float t  = (float)i / 8.0f;
        float wx = gx + sc * (1.0f - 2.0f * t);
        float wy = gy - sc * 0.50f * (1.0f + 0.40f * sinf(t * 3.14159f * 3));
        glColor4f(0.78f, 0.92f, 1.0f, 0.80f);
        glVertex2f(wx, wy);
    }
    glEnd();

    // Eyes – dark
    for (int side = -1; side <= 1; side += 2) {
        float ex = gx + side * 0.36f * sc;
        float ey = gy + 0.38f * sc;
        glColor4f(0.05f, 0.05f, 0.15f, 0.95f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 24; i++) {
            float a = i * 2.0f * 3.14159f / 24;
            glVertex2f(ex + 0.18f * sc * cosf(a),
                       ey + 0.22f * sc * sinf(a));
        }
        glEnd();

        // Pupil gleam
        glColor4f(0.7f, 0.85f, 1.0f, 0.70f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 12; i++) {
            float a = i * 2.0f * 3.14159f / 12;
            glVertex2f(ex + 0.07f * sc * cosf(a) - 0.04f * sc,
                       ey + 0.09f * sc * sinf(a) + 0.05f * sc);
        }
        glEnd();
    }

    // Scowling mouth
    glColor4f(0.05f, 0.05f, 0.15f, 0.90f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 12; i++) {
        float t = (float)i / 12.0f;
        float mx = gx + sc * (-0.4f + 0.8f * t);
        float my = gy - 0.05f * sc + 0.18f * sc * sinf(t * 3.14159f);
        glVertex2f(mx, my);
    }
    glEnd();
    glLineWidth(1.0f);

    glDisable(GL_BLEND);
}

void updateGhost() {
    if (showEndScreen) return;
    float targetX = charX + 4.75f;
    float diff     = targetX - ghostPosX;
    float chaseSpd = (doracakesCollected >= 3) ? 0.045f : 0.030f;
    if      (diff >  0.05f) ghostPosX += chaseSpd;
    else if (diff < -0.05f) ghostPosX -= chaseSpd;
}

void checkGhost() {
    if (showEndScreen) return;

    float csx = charX + 4.75f;
    float csy = charY + 0.45f;

    // Ghost is rounded,
    float dx  = csx - ghostPosX;
    float dy  = csy - ghostPosY;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist < 1.6f) {
        ghostIsHit = true;
        damagePlayer();
    } else {
        ghostIsHit = false;
    }

    finishGameOverIfNeeded();
}


//  ADVANCED OBSTACLE PLACEMENT  Levels 4 , 5

void placeObstaclesAdvanced(float speedMult) {
    float minDist = 5.5f, minX = -3.0f, maxX = 9.0f;
    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
        bool valid = false;
        while (!valid) {
            int si     = rand() % totalScenes;
            float lx   = minX + (rand()/(float)RAND_MAX) * (maxX - minX);
            float cX   = si * sceneWidth + lx;
            valid = true;
            for (int j = 0; j < i; j++)
                if (fabs(cX - obstaclePosX[j]) < minDist) { valid = false; break; }
            if (valid) {
                obstaclePosX[i]    = cX;
                obstaclePosY[i]    = -6.0f;
                obstacleLevel[i]   = currentLevel;
                obstacleEnabled[i] = true;
                obstacleHit[i]     = false;
                obstacleRespawnTimer[i] = 0;
                obstacleType[i]    = rand() % 3;

                // Horizontal – always moving, scaled by speedMult
                obstacleMoveSpeedX[i] = 0.050f * speedMult;
                obstacleMoveDirX[i]   = (rand()%2 == 0) ? 1 : -1;
                obstacleLeftBound[i]  = si * sceneWidth - 9.0f;
                obstacleRightBound[i] = si * sceneWidth + 9.0f;

                // Vertical – bouncing
                obstacleMoveSpeedY[i] = 0.038f * speedMult;
                obstacleMoveDirY[i]   = 1;
                obstacleTopBound[i]   = -3.5f;
                obstacleBottomBound[i]= -7.5f;

                signAngle[i]      = 0.0f;
                signRotateSpeed[i]= 5.0f * speedMult;
            }
        }
    }
}

// LEVEL 4 - - Rainy env

void Level4()
{
    currentLevel = 4;

    // Sky
    glPushMatrix();
    glTranslatef(-9.5f, -17.2f, 0.0f);
    glScalef(1.2f, 1.5f, 1.0f);
    drawSky4();
    glPopMatrix();

    // Scenes
    for (int i = 0; i <= totalScenes; i++) {
        glPushMatrix();
        glTranslatef(worldX + i * sceneWidth, 0.0f, 0.0f);
        if (i % 2 == 0) houseScene(); else wallScene();
        glPopMatrix();
    }

    // Doraemon
    glPushMatrix();
    glTranslatef(charX, charY, 0.0f);
    glTranslatef(((0.0f + 9.5f) / 2.0f), 0.0f, 0.0f);
    glScalef((float)facing * 0.5f, 0.5f, 1.0f);
    glTranslatef(-((0.0f + 9.5f) / 2.0f), 0.0f, 0.0f);
    Doraemon();
    glPopMatrix();

    if (!level4Started) {
        placeCollectibles();
        placeObstaclesAdvanced(1.35f);   // faster than base
        placeManholeFire();
        initFallingRocks();
        level4Started = true;
    }

    // Obstacles- rotating signboards (same style as Level 2)
    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
        if (!obstacleEnabled[i] || obstacleLevel[i] != currentLevel) continue;
        drawRotatingSignBoard(obstaclePosX[i] + worldX, obstaclePosY[i], i);
    }

    // Collectibles
    for (int i = 0; i < 10; i++) {
        if (!doracakeCollected[i]) {
            glPushMatrix();
            glTranslatef(doracakeX[i] + worldX, doracakeY[i], 0.0f);
            glScalef(0.5f, 1.2f, 0.0f);
            Doracake();
            glPopMatrix();
        }
    }
    for (int i = 0; i < 3; i++) {
        if (!heartLost[i]) {
            glPushMatrix();
            glTranslatef(heartX[i] + worldX, heartY[i], 0.0f);
            glScalef(1.5f, 1.2f, 0.0f);
            Heart();
            glPopMatrix();
        }
    }

    drawManholeFire();
    drawFallingRocks();

    // Dark overlay for atmosphere
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.10f, 0.12f, 0.25f, 0.18f);
    glBegin(GL_QUADS);
        glVertex2f(-9.5f,-10.0f); glVertex2f(9.5f,-10.0f);
        glVertex2f( 9.5f, 10.0f); glVertex2f(-9.5f, 10.0f);
    glEnd();
    glDisable(GL_BLEND);

    drawHUD();
    if (showEndScreen) drawEndScreen();

    drawGroundStrip(0.26f, 0.30f, 0.36f);   // wet concrete
    drawHitFlash();
    if (gamePaused) drawPauseScreen();

    glutSwapBuffers();
}

// --- LEVEL 5 - Storm Night env ---

void Level5()
{
    currentLevel = 5;
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    // Sky
    glPushMatrix();
    glTranslatef(-9.5f, -17.2f, 0.0f);
    glScalef(1.2f, 1.5f, 1.0f);
    drawSky5();
    glPopMatrix();

    // Stars
    drawStars();

    // Scenes
    for (int i = 0; i <= totalScenes; i++) {
        glPushMatrix();
        glTranslatef(worldX + i * sceneWidth, 0.0f, 0.0f);
        if (i % 2 == 0) houseScene(); else wallScene();
        glPopMatrix();
    }

    // Lamp-posts same as level 3
    for (int i = 0; i <= totalScenes; i++) {
        glPushMatrix();
        glTranslatef(worldX + i * sceneWidth, 0.0f, 0.0f);
        if (i % 2 == 0) { glTranslatef(3.5f,-4.2f,0.0f); glScalef(8.2f, 6.5f,-1.0f); }
        else             { glTranslatef(3.5f,-4.2f,0.0f); glScalef(8.2f, 8.5f, 1.0f); }
        drawLampPost();
        glPopMatrix();
    }

    // Doraemon
    glPushMatrix();
    glTranslatef(charX, charY, 0.0f);
    glTranslatef(((0.0f + 9.5f) / 2.0f), 0.0f, 0.0f);
    glScalef((float)facing * 0.5f, 0.5f, 1.0f);
    glTranslatef(-((0.0f + 9.5f) / 2.0f), 0.0f, 0.0f);
    Doraemon();
    glPopMatrix();

    if (!level5Started) {
        placeCollectibles();
        placeObstaclesAdvanced(1.85f);   // faster for increasing hardness of level
        placeManholeFire();
        initStars();
        resetGhost();
        level5Started = true;
    }

    // Obstacles mixed  same as Level 3
    for (int i = 0; i < TOTAL_OBSTACLES; i++) {
        if (!obstacleEnabled[i] || obstacleLevel[i] != currentLevel) continue;
        float dx = obstaclePosX[i] + worldX;
        float dy = obstaclePosY[i];
        if      (obstacleType[i] == 0) drawBall(dx, dy);
        else if (obstacleType[i] == 1) drawMouse(dx, dy, 3.0f);
        else                            drawSpikyFruit(dx, dy, 3.0f);
    }

    // Collectibles
    for (int i = 0; i < 10; i++) {
        if (!doracakeCollected[i]) {
            glPushMatrix();
            glTranslatef(doracakeX[i] + worldX, doracakeY[i], 0.0f);
            glScalef(0.5f, 1.2f, 0.0f);
            Doracake();
            glPopMatrix();
        }
    }
    for (int i = 0; i < 3; i++) {
        if (!heartLost[i]) {
            glPushMatrix();
            glTranslatef(heartX[i] + worldX, heartY[i], 0.0f);
            glScalef(1.5f, 1.2f, 0.0f);
            Heart();
            glPopMatrix();
        }
    }

    // Heavy-night overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.22f, 0.48f);
    glBegin(GL_QUADS);
        glVertex2f(-9.5f,-10.0f); glVertex2f(9.5f,-10.0f);
        glVertex2f( 9.5f, 10.0f); glVertex2f(-9.5f, 10.0f);
    glEnd();
    glDisable(GL_BLEND);

    drawManholeFire();
    drawLightning();
    drawGhostEnemy(ghostPosX, ghostPosY);

    drawHUD();
    if (showEndScreen) drawEndScreen();

    drawGroundStrip(0.14f, 0.11f, 0.20f);   // dark stormy ground
    drawHitFlash();
    if (gamePaused) drawPauseScreen();

    glPopAttrib();
    glutSwapBuffers();
}


void specialKeyPress(int key, int x, int y) {
    if (showEndScreen || showHome || gamePaused) return;
    switch(key) {
        case GLUT_KEY_RIGHT:
            facing = 1;
            moving = true;
            break;
        case GLUT_KEY_LEFT:
            facing = -1;
            moving = true;
            break;
        case GLUT_KEY_UP:
            if (!jumping && charY <= -7.5f + 0.01f)
                jumping = true;
            break;
    }
}

void specialKeyRelease(int key, int x, int y) {
    if (showEndScreen || showHome) return;
    switch(key) {
        case GLUT_KEY_RIGHT:
        case GLUT_KEY_LEFT:
            moving = false;
            break;
    }
}


void mouseClick(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    int winW = glutGet(GLUT_WINDOW_WIDTH);
    int winH = glutGet(GLUT_WINDOW_HEIGHT);

    // Account for letterbox viewport
    float targetAspect = 16.0f / 9.0f;
    float windowAspect = (float)winW / (float)winH;

    int vpX, vpY, vpW, vpH;
    if (windowAspect > targetAspect) {
        vpH = winH;
        vpW = (int)(winH * targetAspect);
        vpX = (winW - vpW) / 2;
        vpY = 0;
    } else {
        vpW = winW;
        vpH = (int)(winW / targetAspect);
        vpX = 0;
        vpY = (winH - vpH) / 2;
    }

    // Convert mouse position
    float fx = ((float)x / winW) * 20.0f - 10.0f;
    float fy = 10.0f - ((float)y / winH) * 20.0f;

// ----- Pause menu button clicks --------
    if (gamePaused) {

        // Resume
        if (fx >= -4.0f && fx <= 4.0f &&
            fy >= 0.3f  && fy <= 1.2f)
        {
            gamePaused = false;
            glutTimerFunc(16, update, 0);
            return;
        }
        // Restart
        if (fx >= -4.0f && fx <= 4.0f &&
            fy >= -0.9f && fy <= 0.0f)
        {
            gamePaused = false;
            showEndScreen = false;
            doracakesCollected = 0;
            lives = totalHearts;
            for (int i = 0; i < 10; i++) doracakeCollected[i] = false;
            for (int i = 0; i < 3; i++)  heartLost[i] = false;
            for (int i = 0; i < TOTAL_OBSTACLES; i++) obstacleHit[i] = false;
            for (int i = 0; i < TOTAL_FIRES; i++) fireHit[i] = false;
            for (int i = 0; i < ROCK_COUNT; i++) rockIsHit[i] = false;
            ghostIsHit = false;
            placeCollectibles();
            charX = -9.5f; charY = -7.5f;
            worldX = 0.0f; sceneCount = 0;
            resetFullScoreForNewGame();
            isInvincible  = false; isFlashing    = false;
            invincTimer   = 0.0f;  hitFlashTimer = 0.0f;
            moving = false; jumping = false;
            level1Started = false; level2Started = false;
            level3Started = false; level4Started = false;
            level5Started = false;
            glutTimerFunc(16, update, 0);
            return;
        }
        // Main Menu
        if (fx >= -4.0f && fx <= 4.0f &&
            fy >= -2.1f && fy <= -1.2f)
        {
            gamePaused = false;
            doracakesCollected = 0;
            lives = totalHearts;
            charX = -9.5f; charY = -7.5f;
            worldX = 0.0f; sceneCount = 0;
            resetFullScoreForNewGame();
            showEndScreen = false;
            showHome = true;
            level1Started = false; level2Started = false;
            level3Started = false; level4Started = false;
            level5Started = false;
            glutTimerFunc(16, update, 0);
            return;
        }
        return;  // block everything while pause
    }
    // ─────────────────────────────────────────────────────



    if (showHome)
    {
        if (!bgmStarted) playBGM();

        // LEVEL 1
        if (fx >= -2.0f && fx <= 2.0f && fy <= 3.0f && fy >= 1.8f) {
            currentLevel = 1;
            resetFullScoreForNewGame();
            showHome = false;
            return;
        }
        // LEVEL 2
        if (fx >= -2.0f && fx <= 2.0f && fy <= 1.0f && fy >= -0.2f) {
            currentLevel = 2;
            resetFullScoreForNewGame();
            showHome = false;
            return;
        }
        // LEVEL 3
        if (fx >= -2.0f && fx <= 2.0f && fy <= -1.0f && fy >= -2.2f) {
            currentLevel = 3;
            resetFullScoreForNewGame();
            showHome = false;
            return;
        }
        // LEVEL 4
        if (fx >= -2.0f && fx <= 2.0f && fy <= -3.0f && fy >= -4.2f) {
            currentLevel = 4;
            resetFullScoreForNewGame();
            showHome = false;
            return;
        }
        // LEVEL 5
        if (fx >= -2.0f && fx <= 2.0f && fy <= -5.0f && fy >= -6.2f) {
            currentLevel = 5;
            resetFullScoreForNewGame();
            showHome = false;
            return;
        }
    }

    if (showEndScreen)
    {
        // Next Level button
        if (gameWon && currentLevel < 5) {
            if (fx >= -4.5f && fx <= 4.5f &&
                fy >= -0.1f && fy <= 0.8f)
            {
                showEndScreen = false;
                gameWon = false;
                advanceToNextLevel();
                return;
            }
        }

        // Main Menu button
        if (fx >= -4.5f && fx <= 4.5f &&
            fy >= -1.3f && fy <= -0.4f)
        {
            doracakesCollected = 0;
            lives = totalHearts;
            charX = -9.5f;
            charY = -7.5f;
            worldX = 0.0f;
            sceneCount = 0;
            resetFullScoreForNewGame();
            showEndScreen = false;
            showHome = true;
            return;
        }

        // Restart button
        if (fx >= -4.5f && fx <= 4.5f &&
            fy >= -2.5f && fy <= -1.6f)
        {
            showEndScreen = false;
            doracakesCollected = 0;
            lives = totalHearts;
            for (int i = 0; i < 10; i++) doracakeCollected[i] = false;
            for (int i = 0; i < 3; i++)  heartLost[i] = false;
            for (int i = 0; i < TOTAL_OBSTACLES; i++) obstacleHit[i] = false;
            for (int i = 0; i < TOTAL_FIRES; i++) fireHit[i] = false;
            for (int i = 0; i < ROCK_COUNT; i++) rockIsHit[i] = false;
            ghostIsHit = false;
            placeCollectibles();
            charX = -9.5f;
            charY = -7.5f;
            worldX = 0.0f;
            sceneCount = 0;
            resetFullScoreForNewGame();
            isInvincible  = false;
            isFlashing    = false;
            invincTimer   = 0.0f;
            hitFlashTimer = 0.0f;
            moving        = false;
            jumping       = false;
            level1Started = false;
            level2Started = false;
            level3Started = false;
            level4Started = false;
            level5Started = false;
            return;
        }
    }
}

void update(int value)
{
    // ----- Pause guard ---
    if (gamePaused) {
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }

    // --- Home screen guard -----
    if (showHome) {
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }

    if(moving && jumping)
    {
        legAngle = 0.0f;
    }

    if (moving)
    {
        legAngle += legDir * _legSwingSpeed;
        if (legAngle > _maxLegAngle || legAngle < -_maxLegAngle)
            legDir *= -1;
    }
    else
    {
        if (legAngle > 0.0f)
            legAngle -= _legResetSpeed;
        else if (legAngle < 0.0f)
            legAngle += _legResetSpeed;
        if (legAngle < 1.0f && legAngle > -1.0f)
            legAngle = 0.0f;
    }

    if (jumping)
    {
        charY += _jumpSpeed;
        if (charY >= -7.5f + 6.0f)
            jumping = false;
    }
    else if (charY > -7.5f)
    {
        charY -= _jumpSpeed;
        if (charY < -7.5f)
            charY = -7.5f;
    }

    if (moving)
    {
        if (facing == 1)
        {
            if (sceneCount < totalScenes && charX >= -6.0f)
            {
                charX = -6.0f;
                worldX -= _moveSpeed;
                if (worldX <= -(sceneCount + 1) * sceneWidth)
                    sceneCount++;
            }
            else
                charX += _moveSpeed;
        }
        else if (facing == -1)
        {
            if (sceneCount > 0 && charX <= -5.0f)
            {
                charX = -5.0f;
                worldX += _moveSpeed;
                if (worldX >= -(sceneCount - 1) * sceneWidth)
                    sceneCount--;
            }
            else
                charX -= _moveSpeed;
        }
    }

    if (sceneCount >= totalScenes)
    {
        if (charX > 3.2f) charX = 3.2f;
    }
    if (sceneCount <= 0)
    {
        if (charX < -12.5f) charX = -12.5f;
    }

    if (!showEndScreen) {
        updateObstacles();
        checkObstacles();
        checkCollectibles();
        updateManholeFire();
        checkManholeFire();
        checkStones();
    }

    // ----- Invincibility timer ---
    if (isInvincible) {
        invincTimer += 1.0f;
        if (invincTimer >= INVINC_DURATION)
            isInvincible = false;
    }

    // ---- Hit-flash countdown ---

    if (isFlashing) {
        hitFlashTimer -= 1.0f;
        if (hitFlashTimer <= 0.0f) {
            isFlashing    = false;
            hitFlashTimer = 0.0f;
        }
    }

    // Animate clouds Level 1

    if (currentLevel == 1)
        updateClouds();

    // Level 4 -  falling rocks

    if (!showEndScreen && currentLevel == 4) {
        updateFallingRocks();
        checkFallingRocks();
    }

    // Level 5- ghost + lightning + stars

    if (!showEndScreen && currentLevel == 5) {
        starTime5 += 1.0f;
        updateGhost();
        checkGhost();
        updateLightning();
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


void display() {
    if (showHome) {
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    } else {
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (showHome) {
        homeScreen();
    }
    else {
        if      (currentLevel == 1) Level1();
        else if (currentLevel == 2) Level2();
        else if (currentLevel == 3) Level3();
        else if (currentLevel == 4) Level4();
        else if (currentLevel == 5) Level5();
    }
}

void idleFunc() {
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;

    // Fill entire screen - stretch to fit the screen
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-9.50, 9.50, -10.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void init()
{
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0, 0.0, 0.0, 1.0); //leterbox bars

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-9.50, 9.50, -10.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}





int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));
    loadMusicPreference();             // load custom background music path
    loadHiScore();                     // load saved high scores for each level

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Doraemon's Treat Hunt");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeyPress);
    glutSpecialUpFunc(specialKeyRelease);
    glutMouseFunc(mouseClick);
    glutKeyboardFunc(keyPress);   // P = pause
    glutTimerFunc(16, update, 0);
    glutIdleFunc(idleFunc);


    glutMainLoop();
    return 0;
}
