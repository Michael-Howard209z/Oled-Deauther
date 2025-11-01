
#include "wifi_conf.h"
#include "wifi_cust_tx.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "WiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"
#include <SPI.h>
#undef max
#undef min
#include <vector>
#include <map>
#include "debug.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define BTN_DOWN PA27
#define BTN_UP PA12
#define BTN_OK PA13
typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];
  short rssi;
  uint channel;
} WiFiScanResult;
char *ssid = "xiaomi 15 ultra";
char *pass = "123456789";
int current_channel = 1;
std::vector<WiFiScanResult> scan_results;
WiFiServer server(80);
bool deauth_running = false;
uint8_t deauth_bssid[6];
uint8_t becaon_bssid[6];
uint16_t deauth_reason;
String SelectedSSID;
String SSIDCh;
unsigned long packet_count = 0;
unsigned long last_packet_time = 0;
int packet_rates[20] = {0};
int packet_buffer_index = 0;
bool packet_monitor_running = false;
int attackstate = 0;
int menustate = 0;
bool menuscroll = true;
bool okstate = true;
int scrollindex = 0;
int perdeauth = 3;
int page = 0;
#define LED_BLUE PA14
bool ledState = true;
int currentLanguage = 0;
unsigned long lastDownTime = 0;
unsigned long lastUpTime = 0;
unsigned long lastOkTime = 0;
const unsigned long DEBOUNCE_DELAY = 150;
#define MAX_SCAN_RESULTS 10
inline int myMax(int a, int b) {
  return (a > b) ? a : b;
}
unsigned long lcg_seed = 12345;
inline int lcg_rand(int min, int max) {
  lcg_seed = (1103515245 * lcg_seed + 12345) & 0x7fffffff;
  return min + (lcg_seed % (max - min + 1));
}
// Placeholder bitmaps for UI elements
static const unsigned char PROGMEM image_language_bits[] = {
  0x3C, 0x42, 0x99, 0xA5, 0xA5, 0x99, 0x42, 0x3C
};
static const unsigned char PROGMEM icon_5g_bits[] = {
  0x00, 0x00, 0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
};
static const unsigned char PROGMEM icon_2_4g_bits[] = {
  0x00, 0x00, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x00, 0x00, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C
};
static const unsigned char PROGMEM icon_signal_1_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18
};
static const unsigned char PROGMEM icon_signal_2_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C
};
static const unsigned char PROGMEM icon_signal_3_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xFF
};
static const unsigned char PROGMEM icon_signal_4_bits[] = {
  0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static const unsigned char PROGMEM image_wifi_not_connected__copy__bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x22, 0x00, 0x55, 0x80, 0xAA, 0x40, 0x55, 0xA0, 0x2A,
  0x10, 0x15, 0x88, 0x0A, 0x44, 0x05, 0xA2, 0x02, 0x11, 0x01, 0x88, 0x00, 0x44, 0x00, 0x22, 0x00
};
static const unsigned char PROGMEM image_off_text_bits[] = {
  0x00, 0x00, 0x7C, 0x44, 0x44, 0x7C, 0x00, 0x7C, 0x44, 0x44, 0x44, 0x00
};
static const unsigned char PROGMEM image_cross_contour_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x54, 0x92, 0x11, 0x00, 0x00
};

// Updated 64x32 icons (placeholders)
// Thay thế các icon 64x64 bằng icon 16x16
static const unsigned char PROGMEM icon_attack_16_bits[] = {
  0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xFF,
  0xFF, 0xFF, 0x7E, 0x7E, 0x3C, 0x3C, 0x18, 0x18
};

static const unsigned char PROGMEM icon_scan_16_bits[] = {
  0x00, 0x00, 0x07, 0xE0, 0x18, 0x18, 0x60, 0x06,
  0x80, 0x01, 0x60, 0x06, 0x18, 0x18, 0x07, 0xE0
};

static const unsigned char PROGMEM icon_select_16_bits[] = {
  0x00, 0x00, 0x03, 0xC0, 0x04, 0x20, 0x08, 0x10,
  0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x00, 0x00
};

static const unsigned char PROGMEM icon_info_16_bits[] = {
  0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x00, 0x00,
  0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x00, 0x00
};

static const unsigned char PROGMEM icon_version_16_bits[] = {
  0x00, 0x00, 0x0F, 0xF0, 0x00, 0x10, 0x00, 0x10,
  0x00, 0x10, 0x00, 0x10, 0x0F, 0xF0, 0x00, 0x00
};

static const unsigned char PROGMEM icon_settings_16_bits[] = {
  0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x07, 0xE0,
  0x02, 0x40, 0x07, 0xE0, 0x02, 0x40, 0x01, 0x80
};

static const unsigned char PROGMEM icon_reset_16_bits[] = {
  0x00, 0x00, 0x07, 0xC0, 0x08, 0x20, 0x10, 0x10,
  0x10, 0x10, 0x08, 0x20, 0x07, 0xC0, 0x00, 0x00
};

static const unsigned char PROGMEM icon_turnoff_16_bits[] = {
  0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x00, 0x00,
  0x03, 0xC0, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM icon_packet_16_bits[] = {
  0x00, 0x00, 0x0F, 0xF0, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0x08, 0x10, 0x0F, 0xF0, 0x00, 0x00
};

static const unsigned char PROGMEM icon_tetris_16_bits[] = {
  0x00, 0x00, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0xE0,
  0x07, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x00, 0x00
};

static const unsigned char PROGMEM icon_snake_16_bits[] = {
  0x00, 0x00, 0x03, 0xC0, 0x04, 0x20, 0x04, 0x20,
  0x02, 0x40, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM icon_flappy_16_bits[] = {
  0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20,
  0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM icon_pong_16_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM icon_space_16_bits[] = {
  0x00, 0x00, 0x03, 0xC0, 0x02, 0x40, 0x02, 0x40,
  0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Snake Game
#define SNAKE_GRID_WIDTH 32
#define SNAKE_GRID_HEIGHT 16
#define SNAKE_CELL_SIZE 4
#define SNAKE_GRID_X 0
#define SNAKE_GRID_Y 8
struct SnakeSegment {
  int x, y;
};
void SnakeGame() {
  std::vector<SnakeSegment> snake = {{SNAKE_GRID_WIDTH / 2, SNAKE_GRID_HEIGHT / 2}};
  int dx = 1, dy = 0;
  int foodX = lcg_rand(0, SNAKE_GRID_WIDTH - 1);
  int foodY = lcg_rand(0, SNAKE_GRID_HEIGHT - 1);
  int score = 0;
  bool gameOver = false;
  bool running = true;
  unsigned long lastMoveTime = 0;
  unsigned long lastInputTime = 0;
  const unsigned long moveDelay = 100;
  const unsigned long inputDelay = 150;

  auto drawGame = [&]() {
    display.clearDisplay();
    drawFrame();
    drawStatusBar(currentLanguage == 0 ? "SNAKE" : "RAN SAN MOI");
    for (const auto &segment : snake) {
      display.fillRect(SNAKE_GRID_X + segment.x * SNAKE_CELL_SIZE, SNAKE_GRID_Y + segment.y * SNAKE_CELL_SIZE, SNAKE_CELL_SIZE, SNAKE_CELL_SIZE, WHITE);
    }
    display.fillCircle(SNAKE_GRID_X + foodX * SNAKE_CELL_SIZE + SNAKE_CELL_SIZE / 2, SNAKE_GRID_Y + foodY * SNAKE_CELL_SIZE + SNAKE_CELL_SIZE / 2, SNAKE_CELL_SIZE / 2, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(score);
    if (gameOver) {
      display.setCursor(30, 30);
      display.print(currentLanguage == 0 ? F("Game Over") : F("Ket Thuc"));
      display.setCursor(20, 40);
      display.print(currentLanguage == 0 ? F("OK: Restart") : F("OK: Choi Lai"));
    }
    display.display();
  };

  while (running) {
    unsigned long currentTime = millis();
    if (currentTime - lastInputTime > inputDelay && !gameOver) {
      if (digitalRead(BTN_UP) == LOW && dy == 0) {
        dx = 0; dy = -1;
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_DOWN) == LOW && dy == 0) {
        dx = 0; dy = 1;
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_OK) == LOW && dx == 0) {
        dx = dy == -1 ? -1 : 1; dy = 0;
        lastInputTime = currentTime;
      }
    }
    if (!gameOver && currentTime - lastMoveTime > moveDelay) {
      SnakeSegment newHead = {snake[0].x + dx, snake[0].y + dy};
      if (newHead.x < 0 || newHead.x >= SNAKE_GRID_WIDTH || newHead.y < 0 || newHead.y >= SNAKE_GRID_HEIGHT) {
        gameOver = true;
      } else {
        for (size_t i = 1; i < snake.size(); i++) {
          if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
            gameOver = true;
            break;
          }
        }
      }
      if (!gameOver) {
        snake.insert(snake.begin(), newHead);
        if (newHead.x == foodX && newHead.y == foodY) {
          score += 10;
          foodX = lcg_rand(0, SNAKE_GRID_WIDTH - 1);
          foodY = lcg_rand(0, SNAKE_GRID_HEIGHT - 1);
        } else {
          snake.pop_back();
        }
      }
      lastMoveTime = currentTime;
    }
    if (digitalRead(BTN_OK) == LOW && currentTime - lastInputTime > inputDelay) {
      if (gameOver) {
        snake = {{SNAKE_GRID_WIDTH / 2, SNAKE_GRID_HEIGHT / 2}};
        dx = 1; dy = 0;
        score = 0;
        gameOver = false;
        foodX = lcg_rand(0, SNAKE_GRID_WIDTH - 1);
        foodY = lcg_rand(0, SNAKE_GRID_HEIGHT - 1);
      } else {
        running = false;
      }
      lastInputTime = currentTime;
    }
    drawGame();
    delay(10);
  }
}

// Flappy Bird Game
#define FLAPPY_GROUND_Y 60
#define FLAPPY_BIRD_SIZE 8
#define PIPE_WIDTH 12
#define PIPE_GAP 24
struct Pipe {
  int x;
  int gapY;
};
void FlappyBirdGame() {
  int birdX = 30;
  int birdY = SCREEN_HEIGHT / 2;
  float velocity = 0;
  std::vector<Pipe> pipes;
  int score = 0;
  bool gameOver = false;
  bool running = true;
  unsigned long lastUpdateTime = 0;
  unsigned long lastInputTime = 0;
  const unsigned long updateDelay = 50;
  const unsigned long inputDelay = 150;

  auto spawnPipe = [&]() {
    Pipe pipe;
    pipe.x = SCREEN_WIDTH;
    pipe.gapY = lcg_rand(20, FLAPPY_GROUND_Y - PIPE_GAP - 20);
    pipes.push_back(pipe);
  };

  auto drawGame = [&]() {
    display.clearDisplay();
    drawFrame();
    drawStatusBar(currentLanguage == 0 ? "FLAPPY BIRD" : "CHIM BAY");
    display.drawLine(0, FLAPPY_GROUND_Y, SCREEN_WIDTH, FLAPPY_GROUND_Y, WHITE);
    display.fillCircle(birdX, birdY, FLAPPY_BIRD_SIZE / 2, WHITE);
    for (const auto &pipe : pipes) {
      display.fillRect(pipe.x, 0, PIPE_WIDTH, pipe.gapY, WHITE);
      display.fillRect(pipe.x, pipe.gapY + PIPE_GAP, PIPE_WIDTH, FLAPPY_GROUND_Y - (pipe.gapY + PIPE_GAP), WHITE);
    }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(score);
    if (gameOver) {
      display.setCursor(30, 30);
      display.print(currentLanguage == 0 ? F("Game Over") : F("Ket Thuc"));
      display.setCursor(20, 40);
      display.print(currentLanguage == 0 ? F("OK: Restart") : F("OK: Choi Lai"));
    }
    display.display();
  };

  spawnPipe();
  while (running) {
    unsigned long currentTime = millis();
    if (digitalRead(BTN_OK) == LOW && currentTime - lastInputTime > inputDelay && !gameOver) {
      velocity = -3;
      lastInputTime = currentTime;
    }
    if (!gameOver && currentTime - lastUpdateTime > updateDelay) {
      velocity += 0.2;
      birdY += velocity;
      if (birdY < 0 || birdY > FLAPPY_GROUND_Y - FLAPPY_BIRD_SIZE) {
        gameOver = true;
      }
      for (auto &pipe : pipes) {
        pipe.x -= 2;
        if (pipe.x + PIPE_WIDTH < 0) {
          pipe.x = SCREEN_WIDTH;
          pipe.gapY = lcg_rand(20, FLAPPY_GROUND_Y - PIPE_GAP - 20);
          score += 10;
        }
        if (birdX + FLAPPY_BIRD_SIZE > pipe.x && birdX < pipe.x + PIPE_WIDTH) {
          if (birdY < pipe.gapY || birdY + FLAPPY_BIRD_SIZE > pipe.gapY + PIPE_GAP) {
            gameOver = true;
          }
        }
      }
      if (pipes.back().x < SCREEN_WIDTH - 60) {
        spawnPipe();
      }
      // Manual removal instead of std::remove_if
      for (auto it = pipes.begin(); it != pipes.end();) {
        if (it->x + PIPE_WIDTH < 0) {
          it = pipes.erase(it);
        } else {
          ++it;
        }
      }
      lastUpdateTime = currentTime;
    }
    if (digitalRead(BTN_OK) == LOW && currentTime - lastInputTime > inputDelay) {
      if (gameOver) {
        birdY = SCREEN_HEIGHT / 2;
        velocity = 0;
        pipes.clear();
        spawnPipe();
        score = 0;
        gameOver = false;
      } else {
        running = false;
      }
      lastInputTime = currentTime;
    }
    drawGame();
    delay(10);
  }
}

// Pong Game
#define PONG_PADDLE_WIDTH 16
#define PONG_PADDLE_HEIGHT 4
#define PONG_BALL_SIZE 4
void PongGame() {
  int paddleX = SCREEN_WIDTH / 2 - PONG_PADDLE_WIDTH / 2;
  int ballX = SCREEN_WIDTH / 2;
  int ballY = SCREEN_HEIGHT / 2;
  int ballDX = lcg_rand(0, 1) ? 2 : -2;
  int ballDY = -2;
  int score = 0;
  bool gameOver = false;
  bool running = true;
  unsigned long lastUpdateTime = 0;
  unsigned long lastInputTime = 0;
  const unsigned long updateDelay = 50;
  const unsigned long inputDelay = 150;

  auto drawGame = [&]() {
    display.clearDisplay();
    drawFrame();
    drawStatusBar(currentLanguage == 0 ? "PONG" : "BONG BAN");
    display.fillRect(paddleX, SCREEN_HEIGHT - PONG_PADDLE_HEIGHT - 4, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT, WHITE);
    display.fillCircle(ballX, ballY, PONG_BALL_SIZE / 2, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(score);
    if (gameOver) {
      display.setCursor(30, 30);
      display.print(currentLanguage == 0 ? F("Game Over") : F("Ket Thuc"));
      display.setCursor(20, 40);
      display.print(currentLanguage == 0 ? F("OK: Restart") : F("OK: Choi Lai"));
    }
    display.display();
  };

  while (running) {
    unsigned long currentTime = millis();
    if (currentTime - lastInputTime > inputDelay && !gameOver) {
      if (digitalRead(BTN_OK) == LOW) {
        paddleX = myMax(0, paddleX - 8);
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_UP) == LOW) {
        paddleX = myMax(0, myMax(SCREEN_WIDTH - PONG_PADDLE_WIDTH, paddleX + 8));
        lastInputTime = currentTime;
      }
    }
    if (!gameOver && currentTime - lastUpdateTime > updateDelay) {
      ballX += ballDX;
      ballY += ballDY;
      if (ballX <= PONG_BALL_SIZE / 2 || ballX >= SCREEN_WIDTH - PONG_BALL_SIZE / 2) {
        ballDX = -ballDX;
      }
      if (ballY <= PONG_BALL_SIZE / 2) {
        ballDY = -ballDY;
        score += 10;
      }
      if (ballY >= SCREEN_HEIGHT - PONG_PADDLE_HEIGHT - 4 - PONG_BALL_SIZE / 2 &&
          ballX >= paddleX && ballX <= paddleX + PONG_PADDLE_WIDTH) {
        ballDY = -ballDY;
      }
      if (ballY >= SCREEN_HEIGHT - PONG_BALL_SIZE / 2) {
        gameOver = true;
      }
      lastUpdateTime = currentTime;
    }
    if (digitalRead(BTN_DOWN) == LOW && currentTime - lastInputTime > inputDelay) {
      if (gameOver) {
        paddleX = SCREEN_WIDTH / 2 - PONG_PADDLE_WIDTH / 2;
        ballX = SCREEN_WIDTH / 2;
        ballY = SCREEN_HEIGHT / 2;
        ballDX = lcg_rand(0, 1) ? 2 : -2;
        ballDY = -2;
        score = 0;
        gameOver = false;
      } else {
        running = false;
      }
      lastInputTime = currentTime;
    }
    drawGame();
    delay(10);
  }
}

// Space Invaders Game
#define INVADERS_GRID_WIDTH 8
#define INVADERS_GRID_HEIGHT 4
#define INVADERS_CELL_SIZE 8
#define PLAYER_WIDTH 12
#define BULLET_SIZE 4
struct Invader {
  int x, y;
  bool alive;
};
struct Bullet {
  int x, y;
  bool active;
};
void SpaceInvadersGame() {
  Invader invaders[INVADERS_GRID_WIDTH * INVADERS_GRID_HEIGHT];
  for (int i = 0; i < INVADERS_GRID_HEIGHT; i++)
    for (int j = 0; j < INVADERS_GRID_WIDTH; j++)
      invaders[i * INVADERS_GRID_WIDTH + j] = {j * INVADERS_CELL_SIZE + 16, i * INVADERS_CELL_SIZE + 8, true};
  int playerX = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
  Bullet bullet = {0, 0, false};
  int score = 0;
  bool gameOver = false;
  bool running = true;
  unsigned long lastUpdateTime = 0;
  unsigned long lastInputTime = 0;
  const unsigned long updateDelay = 100;
  const unsigned long inputDelay = 150;
  int invaderDX = 1;
  unsigned long lastInvaderMove = 0;

  auto drawGame = [&]() {
    display.clearDisplay();
    drawFrame();
    drawStatusBar(currentLanguage == 0 ? "SPACE INVADERS" : "XAM LANG VU TRU");
    display.fillTriangle(playerX, SCREEN_HEIGHT - 8, playerX + PLAYER_WIDTH / 2, SCREEN_HEIGHT - 16, playerX + PLAYER_WIDTH, SCREEN_HEIGHT - 8, WHITE);
    for (int i = 0; i < INVADERS_GRID_WIDTH * INVADERS_GRID_HEIGHT; i++) {
      if (invaders[i].alive) {
        display.fillRect(invaders[i].x, invaders[i].y, INVADERS_CELL_SIZE, INVADERS_CELL_SIZE, WHITE);
      }
    }
    if (bullet.active) {
      display.fillRect(bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE, WHITE);
    }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(score);
    if (gameOver) {
      display.setCursor(30, 30);
      display.print(currentLanguage == 0 ? F("Game Over") : F("Ket Thuc"));
      display.setCursor(20, 40);
      display.print(currentLanguage == 0 ? F("OK: Restart") : F("OK: Choi Lai"));
    }
    display.display();
  };

  while (running) {
    unsigned long currentTime = millis();
    if (currentTime - lastInputTime > inputDelay && !gameOver) {
      if (digitalRead(BTN_OK) == LOW) {
        playerX = myMax(0, playerX - 8);
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_UP) == LOW) {
        playerX = myMax(0, myMax(SCREEN_WIDTH - PLAYER_WIDTH, playerX + 8));
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_DOWN) == LOW && !bullet.active) {
        bullet = {playerX + PLAYER_WIDTH / 2 - BULLET_SIZE / 2, SCREEN_HEIGHT - 16, true};
        lastInputTime = currentTime;
      }
    }
    if (!gameOver && currentTime - lastUpdateTime > updateDelay) {
      if (bullet.active) {
        bullet.y -= 4;
        if (bullet.y < 0) {
          bullet.active = false;
        } else {
          for (int i = 0; i < INVADERS_GRID_WIDTH * INVADERS_GRID_HEIGHT; i++) {
            if (invaders[i].alive &&
                bullet.x >= invaders[i].x && bullet.x <= invaders[i].x + INVADERS_CELL_SIZE &&
                bullet.y >= invaders[i].y && bullet.y <= invaders[i].y + INVADERS_CELL_SIZE) {
              invaders[i].alive = false;
              bullet.active = false;
              score += 10;
              break;
            }
          }
        }
      }
      if (currentTime - lastInvaderMove > 1000) {
        bool hitEdge = false;
        for (int i = 0; i < INVADERS_GRID_WIDTH * INVADERS_GRID_HEIGHT; i++) {
          if (invaders[i].alive) {
            invaders[i].x += invaderDX;
            if (invaders[i].x <= 0 || invaders[i].x >= SCREEN_WIDTH - INVADERS_CELL_SIZE) {
              hitEdge = true;
            }
            if (invaders[i].y >= SCREEN_HEIGHT - 16) {
              gameOver = true;
            }
          }
        }
        if (hitEdge) {
          invaderDX = -invaderDX;
          for (int i = 0; i < INVADERS_GRID_WIDTH * INVADERS_GRID_HEIGHT; i++) {
            if (invaders[i].alive) {
              invaders[i].y += INVADERS_CELL_SIZE;
            }
          }
        }
        lastInvaderMove = currentTime;
      }
      bool allDead = true;
      for (int i = 0; i < INVADERS_GRID_WIDTH * INVADERS_GRID_HEIGHT; i++) {
        if (invaders[i].alive) {
          allDead = false;
          break;
        }
      }
      if (allDead) {
        score += 100;
        for (int i = 0; i < INVADERS_GRID_HEIGHT; i++)
          for (int j = 0; j < INVADERS_GRID_WIDTH; j++)
            invaders[i * INVADERS_GRID_WIDTH + j] = {j * INVADERS_CELL_SIZE + 16, i * INVADERS_CELL_SIZE + 8, true};
      }
      lastUpdateTime = currentTime;
    }
    if (digitalRead(BTN_OK) == LOW && currentTime - lastInputTime > inputDelay) {
      if (gameOver) {
        for (int i = 0; i < INVADERS_GRID_HEIGHT; i++)
          for (int j = 0; j < INVADERS_GRID_WIDTH; j++)
            invaders[i * INVADERS_GRID_WIDTH + j] = {j * INVADERS_CELL_SIZE + 16, i * INVADERS_CELL_SIZE + 8, true};
        playerX = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
        bullet = {0, 0, false};
        score = 0;
        gameOver = false;
        invaderDX = 1;
      } else {
        running = false;
      }
      lastInputTime = currentTime;
    }
    drawGame();
    delay(10);
  }
}

// Tetris Game
#define TETRIS_GRID_WIDTH 10
#define TETRIS_GRID_HEIGHT 20
#define CELL_SIZE 3
#define GRID_X 39
#define GRID_Y 2
const int8_t tetrominoes[7][4][4] PROGMEM = {
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
  {{0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0}},
  {{0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0}}
};
struct TetrisPiece {
  int8_t shape[4][4];
  int x, y;
  int type;
};
void TetrisGame() {
  uint8_t grid[TETRIS_GRID_HEIGHT][TETRIS_GRID_WIDTH] = {0};
  TetrisPiece currentPiece;
  int score = 0;
  bool gameOver = false;
  bool running = true;
  unsigned long lastMoveTime = 0;
  unsigned long lastInputTime = 0;
  const unsigned long moveDelay = 500;
  const unsigned long inputDelay = 150;

  auto spawnPiece = [&]() {
    currentPiece.type = lcg_rand(0, 6);
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        currentPiece.shape[i][j] = pgm_read_byte(&tetrominoes[currentPiece.type][i][j]);
    currentPiece.x = TETRIS_GRID_WIDTH / 2 - 2;
    currentPiece.y = 0;
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (currentPiece.shape[i][j] && grid[currentPiece.y + i][currentPiece.x + j])
          return false;
    return true;
  };

  auto checkCollision = [&](int dx, int dy, int8_t shape[4][4]) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (shape[i][j]) {
          int nx = currentPiece.x + j + dx;
          int ny = currentPiece.y + i + dy;
          if (nx < 0 || nx >= TETRIS_GRID_WIDTH || ny >= TETRIS_GRID_HEIGHT || (ny >= 0 && grid[ny][nx]))
            return true;
        }
    return false;
  };

  auto rotatePiece = [&]() {
    int8_t newShape[4][4];
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        newShape[j][3-i] = currentPiece.shape[i][j];
    if (!checkCollision(0, 0, newShape))
      for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
          currentPiece.shape[i][j] = newShape[i][j];
  };

  auto lockPiece = [&]() {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (currentPiece.shape[i][j])
          grid[currentPiece.y + i][currentPiece.x + j] = 1;
    int linesCleared = 0;
    for (int i = TETRIS_GRID_HEIGHT - 1; i >= 0; i--) {
      bool full = true;
      for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
        if (!grid[i][j]) {
          full = false;
          break;
        }
      if (full) {
        linesCleared++;
        for (int k = i; k > 0; k--)
          for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
            grid[k][j] = grid[k-1][j];
        for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
          grid[0][j] = 0;
        i++;
      }
    }
    score += linesCleared * 10;
  };

  auto drawGame = [&]() {
    display.clearDisplay();
    drawFrame();
    drawStatusBar(currentLanguage == 0 ? "TETRIS" : "XEP GACH");
    for (int i = 0; i < TETRIS_GRID_HEIGHT; i++)
      for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
        if (grid[i][j])
          display.fillRect(GRID_X + j * CELL_SIZE, GRID_Y + i * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (currentPiece.shape[i][j]) {
          int px = GRID_X + (currentPiece.x + j) * CELL_SIZE;
          int py = GRID_Y + (currentPiece.y + i) * CELL_SIZE;
          display.fillRect(px, py, CELL_SIZE, CELL_SIZE, WHITE);
        }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(score);
    if (gameOver) {
      display.setCursor(30, 30);
      display.print(currentLanguage == 0 ? F("Game Over") : F("Ket Thuc"));
      display.setCursor(20, 40);
      display.print(currentLanguage == 0 ? F("OK: Restart") : F("OK: Choi Lai"));
    }
    display.display();
  };

  if (!spawnPiece()) {
    gameOver = true;
  }

  while (running) {
    unsigned long currentTime = millis();
    if (currentTime - lastInputTime > inputDelay) {
      if (digitalRead(BTN_OK) == LOW) {
        if (gameOver) {
          memset(grid, 0, sizeof(grid));
          score = 0;
          gameOver = false;
          if (!spawnPiece()) gameOver = true;
        } else {
          if (!checkCollision(-1, 0, currentPiece.shape))
            currentPiece.x--;
        }
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_UP) == LOW && !gameOver) {
        rotatePiece();
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_DOWN) == LOW && !gameOver) {
        if (!checkCollision(0, 1, currentPiece.shape))
          currentPiece.y++;
        else {
          lockPiece();
          if (!spawnPiece()) gameOver = true;
        }
        lastInputTime = currentTime;
      }
    }
    if (!gameOver && currentTime - lastMoveTime > moveDelay) {
      if (!checkCollision(0, 1, currentPiece.shape))
        currentPiece.y++;
      else {
        lockPiece();
        if (!spawnPiece()) gameOver = true;
      }
      lastMoveTime = currentTime;
    }
    if (digitalRead(BTN_OK) == LOW && currentTime - lastInputTime > 1000) {
      running = false;
    }
    drawGame();
    delay(10);
  }
}

// UI Elements
void drawFrame() {
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 4, WHITE);
  display.drawRoundRect(2, 2, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, 4, WHITE);
}

void drawStatusBar(const char *status) {
  display.fillRoundRect(0, 0, SCREEN_WIDTH, 12, 4, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(4, 2);
  display.print(status);
  display.drawBitmap(SCREEN_WIDTH - 16, 3, image_language_bits, 6, 6, BLACK);
  display.setCursor(SCREEN_WIDTH - 28, 2);
  display.print(currentLanguage == 0 ? "EN" : "VI");
  display.setTextColor(WHITE);
}

void drawNavArrows(int selectedIndex, int totalItems) {
  static unsigned long lastAnimTime = 0;
  static int animFrame = 0;
  if (millis() - lastAnimTime > 200) {
    animFrame = (animFrame + 1) % 3;
    lastAnimTime = millis();
  }
  if (selectedIndex > 0) {
    int offset = animFrame == 0 ? 0 : (animFrame == 1 ? 2 : -2);
    display.fillTriangle(10 + offset, SCREEN_HEIGHT - 10, 15 + offset, SCREEN_HEIGHT - 15, 15 + offset, SCREEN_HEIGHT - 5, WHITE);
  }
  if (selectedIndex < totalItems - 1) {
    int offset = animFrame == 0 ? 0 : (animFrame == 1 ? 2 : -2);
    display.fillTriangle(SCREEN_WIDTH - 10 + offset, SCREEN_HEIGHT - 10, SCREEN_WIDTH - 15 + offset, SCREEN_HEIGHT - 15, SCREEN_WIDTH - 15 + offset, SCREEN_HEIGHT - 5, WHITE);
  }
}

const char *menuItemsEN[] PROGMEM = {
  "Attack", "Scan WiFi", "Select WiFi", "Information",
  "Version", "Settings", "Reset", "Turn Off", "Packet Monitor",
  "Tetris", "Snake", "Flappy Bird", "Pong", "Space Invaders"
};
const char *menuItemsVI[] PROGMEM = {
  "Tan cong", "Quet WiFi", "Chon WiFi", "Thong tin",
  "Phien ban", "Cai dat", "Dat lai", "Tat may", "Giam sat goi tin",
  "Xep Gach", "Ran San Moi", "Chim Bay", "Bong Ban", "Xam Lang Vu Tru"
};
// sửa thành 64 bits cũng được
const unsigned char *featureIcons[] PROGMEM = {
  icon_attack_16_bits, icon_scan_16_bits, icon_select_16_bits, icon_info_16_bits,
  icon_version_16_bits, icon_settings_16_bits, icon_reset_16_bits, icon_turnoff_16_bits,
  icon_packet_16_bits, icon_tetris_16_bits, icon_snake_16_bits, icon_flappy_16_bits,
  icon_pong_16_bits, icon_space_16_bits
};

//hàm drawMainMenu

void drawMainMenu(int selectedIndex) {
  // TinyJoypad-style main menu for 128x64 monochrome OLED
  display.clearDisplay();
  drawFrame();
  drawStatusBar(currentLanguage == 0 ? "MENU" : "THUC DON");

  const char **menuItems = currentLanguage == 0 ? menuItemsEN : menuItemsVI;
  int totalItems = 14;

  // Visible window shows up to 5 items centered on selectedIndex
  int windowSize = 5;
  int half = windowSize / 2;
  int startIndex = selectedIndex - half;
  if (startIndex < 0) startIndex = 0;
  startIndex = (0 > (totalItems - windowSize)) ? 0 : (totalItems - windowSize);


  int itemHeight = 10;
  int leftX = 6;
  int iconX = leftX + 2;
  int iconW = 16;
  int textX = leftX + iconW + 8;
  int baseY = 14; // under status bar
  for (int i = 0; i < windowSize; i++) {
    int idx = startIndex + i;
    int y = baseY + i * itemHeight;
    // Draw selection background for selected item
    if (idx == selectedIndex) {
      // rounded selection pill
      display.fillRoundRect(leftX - 2, y - 1, SCREEN_WIDTH - (leftX*2) + 4, itemHeight + 2, 4, WHITE);
      // icon background (invert)
      display.fillRect(iconX - 1, y, iconW, iconW, BLACK);
      // text in black on white (invert)
      display.setTextColor(BLACK);
      display.setTextSize(1);
      // center text vertically within item
      String txt = menuItems[idx];
      if (txt.length() > 18) txt = txt.substring(0, 15) + "...";
      display.setCursor(textX, y + 1);
      display.print(txt);
      // draw icon inverted (white) if available
      if (featureIcons[idx] != NULL) {
        display.drawBitmap(iconX - 1, y, featureIcons[idx], iconW, iconW, WHITE);
      }
    } else {
      // draw icon outline
      display.drawRoundRect(iconX - 1, y, iconW, iconW, 3, WHITE);
      if (featureIcons[idx] != NULL) {
        display.drawBitmap(iconX, y, featureIcons[idx], iconW-2, iconW-2, WHITE);
      }
      // normal text
      display.setTextColor(WHITE);
      display.setTextSize(1);
      String txt = menuItems[idx];
      if (txt.length() > 18) txt = txt.substring(0, 15) + "...";
      display.setCursor(textX, y + 1);
      display.print(txt);
    }
  }

  // Draw small page indicator at bottom center
  int page = selectedIndex + 1;
  char buf[8];
  snprintf(buf, sizeof(buf), "%d/%d", page, totalItems);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  int tw = strlen(buf) * 6;
  display.setCursor((SCREEN_WIDTH - tw) / 2, SCREEN_HEIGHT - 10);
  display.print(buf);

  display.display();
}
void drawScanScreen() {
  display.clearDisplay();
  drawFrame();
  drawStatusBar(currentLanguage == 0 ? "SCANNING" : "DANG QUET");

  static const char *frames[] = { "/", "-", "\\", "|" };
  for (int i = 0; i < 20; i++) {
    display.setCursor(48, 30);
    display.setTextSize(1);
    display.print(currentLanguage == 0 ? F("Scanning ") : F("Dang quet "));
    display.print(frames[i % 4]);
    display.drawRect(20, 45, SCREEN_WIDTH - 40, 8, WHITE);
    display.fillRect(20, 45, (SCREEN_WIDTH - 40) * i / 20, 8, WHITE);
    display.display();
    delay(200);
  }
}

void drawNetworkList(const String &selectedSSID, int scrollIndex) {
  display.clearDisplay();
  drawStatusBar(currentLanguage == 0 ? "WIFI" : "MANG WIFI");
  drawFrame();

  String displaySSID = selectedSSID;
  if (displaySSID.length() > 12) {
    displaySSID = displaySSID.substring(0, 9) + "...";
  }
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(SCREEN_WIDTH / 2 - (displaySSID.length() * 6) / 2, 20);
  display.print(displaySSID);
  display.setTextColor(WHITE);

  if (static_cast<size_t>(scrollIndex) < scan_results.size()) {
    const unsigned char *channelIcon = scan_results[scrollIndex].channel >= 36 ? icon_5g_bits : icon_2_4g_bits;
    display.drawBitmap(SCREEN_WIDTH - 20, 15, channelIcon, 16, 16, WHITE);
  }

  static unsigned long lastAnimTime = 0;
  static int animFrame = 0;
  if (millis() - lastAnimTime > 200) {
    animFrame = (animFrame + 1) % 2;
    lastAnimTime = millis();
  }
  if (static_cast<size_t>(scrollIndex) < scan_results.size()) {
    int rssi = scan_results[scrollIndex].rssi;
    const unsigned char *signalIcon;
    if (rssi >= -50) signalIcon = icon_signal_4_bits;
    else if (rssi >= -70) signalIcon = icon_signal_3_bits;
    else if (rssi >= -90) signalIcon = icon_signal_2_bits;
    else signalIcon = icon_signal_1_bits;
    if (animFrame == 0) {
      display.drawBitmap(10, 35, signalIcon, 16, 16, WHITE);
    } else {
      display.drawBitmap(10, 34, signalIcon, 16, 16, WHITE);
    }
  }

  static int arrowAnimFrame = 0;
  if (millis() - lastAnimTime > 150) {
    arrowAnimFrame = (arrowAnimFrame + 1) % 3;
  }
  if (scrollIndex > 0) {
    if (arrowAnimFrame == 0) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 45, SCREEN_WIDTH / 2 - 2, 40, SCREEN_WIDTH / 2 + 2, 45, WHITE);
    } else if (arrowAnimFrame == 1) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 46, SCREEN_WIDTH / 2 - 2, 41, SCREEN_WIDTH / 2 + 2, 46, WHITE);
    } else {
      display.drawTriangle(SCREEN_WIDTH / 2 - 6, 45, SCREEN_WIDTH / 2 - 2, 40, SCREEN_WIDTH / 2 + 2, 45, WHITE);
    }
  }
  if (static_cast<size_t>(scrollIndex) < scan_results.size() - 1) {
    if (arrowAnimFrame == 0) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 50, SCREEN_WIDTH / 2 + 2, 50, SCREEN_WIDTH / 2 - 2, 55, WHITE);
    } else if (arrowAnimFrame == 1) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 51, SCREEN_WIDTH / 2 + 2, 51, SCREEN_WIDTH / 2 - 2, 56, WHITE);
    } else {
      display.drawTriangle(SCREEN_WIDTH / 2 - 6, 50, SCREEN_WIDTH / 2 + 2, 50, SCREEN_WIDTH / 2 - 2, 55, WHITE);
    }
  }

  display.display();
}

void drawPacketMonitorScreen() {
  display.clearDisplay();
  drawStatusBar(currentLanguage == 0 ? "PACKET MONITOR" : "GIAM SAT GOI TIN");
  drawFrame();

  for (int x = 0; x <= SCREEN_WIDTH; x += SCREEN_WIDTH / 4) {
    display.drawLine(x, 10, x, 50, WHITE);
  }
  for (int y = 10; y <= 50; y += 10) {
    display.drawLine(0, y, SCREEN_WIDTH, y, WHITE);
  }

  int max_rate = 0;
  for (int i = 0; i < 20; i++) {
    if (packet_rates[i] > max_rate) max_rate = packet_rates[i];
  }
  max_rate = myMax(max_rate, 10);
  for (int i = 0; i < 19; i++) {
    int x1 = i * (SCREEN_WIDTH / 20);
    int x2 = (i + 1) * (SCREEN_WIDTH / 20);
    int y1 = 50 - map(packet_rates[i], 0, max_rate, 0, 40);
    int y2 = 50 - map(packet_rates[(i + 1) % 20], 0, max_rate, 0, 40);
    display.drawLine(x1, y1, x2, y2, WHITE);
    display.drawPixel(x1, y1 - 1, WHITE);
    display.drawPixel(x1, y1 + 1, WHITE);
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(2, 52);
  display.print(F("Ch:"));
  display.print(current_channel);
  display.setCursor(40, 52);
  display.print(F("APs/s:"));
  display.print(packet_rates[packet_buffer_index]);
  display.setCursor(90, 52);
  display.print(F("Tot:"));
  display.print(packet_count);
}

void drawAttackScreen(int attackType) {
  display.clearDisplay();
  drawFrame();
  drawStatusBar(currentLanguage == 0 ? "ATTACK IN PROGRESS" : "DANG TAN CONG");

  display.setTextColor(WHITE);
  display.setCursor(10, 20);

  const char *attackTypesEN[] = { "SINGLE DEAUTH", "ALL DEAUTH", "BEACON", "BEACON+DEAUTH" };
  const char *attackTypesVI[] = { "TAN CONG DON", "TAN CONG TAT CA", "BEACON", "BEACON+DEAUTH" };
  const char **attackTypes = currentLanguage == 0 ? attackTypesEN : attackTypesVI;

  if (attackType >= 0 && attackType < 4) {
    display.print(attackTypes[attackType]);
  }

  static const char patterns[] = { '.', 'o', 'O', 'o' };
  for (size_t i = 0; i < sizeof(patterns); i++) {
    display.setCursor(10, 35);
    display.print(currentLanguage == 0 ? F("Running ") : F("Dang chay "));
    display.print(patterns[i]);
    display.display();
    delay(200);
  }
}

void titleScreen() {
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 7);
  display.print(F("nigga"));
  display.setCursor(20, 20);
  display.print(F("5 GHz"));
  display.setCursor(10, 30);
  display.print(F("NGUYEN HOANG"));
  display.setCursor(10, 40);
  display.print(F("2.4 GHz"));
  //display.drawBitmap(1, 20, image_wifi_not_connected__copy__bits, 19, 16, 1);
  //display.drawBitmap(112, 35, image_off_text_bits, 12, 5, 1);
  //display.drawBitmap(45, 19, image_wifi_not_connected__copy__bits, 19, 16, 1);
  //display.drawBitmap(24, 34, image_off_text_bits, 12, 5, 1);
  //display.drawBitmap(106, 14, image_wifi_not_connected__copy__bits, 19, 16, 1);
  //display.drawBitmap(88, 25, image_wifi_not_connected__copy__bits, 19, 16, 1);
  //display.drawBitmap(9, 35, image_cross_contour_bits, 11, 16, 1);
  display.display();
  delay(1500);
}
void Single();
void All();
void Becaon();
void BecaonDeauth();
void attackLoop() {
  int attackState = 0;
  bool running = true;
  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }


  while (running) {
    display.clearDisplay();
    drawFrame();
    drawStatusBar(currentLanguage == 0 ? "ATTACK MODE" : "CHE DO TAN CONG");

    const char *attackTypesEN[] = { "Single Deauth", "All Deauth", "Beacon", "Beacon+Deauth", "Back" };
    const char *attackTypesVI[] = { "Tan cong don", "Tan cong tat ca", "Beacon", "Beacon+Deauth", "Quay lai" };
    const char **attackTypes = currentLanguage == 0 ? attackTypesEN : attackTypesVI;

    for (int i = 0; i < 5; i++) {
      display.setCursor(10, 15 + (i * 10));
      if (i == attackState) {
        display.setTextColor(BLACK, WHITE);
      } else {
        display.setTextColor(WHITE);
      }
      display.print(attackTypes[i]);
    }
    display.display();

    if (digitalRead(BTN_OK) == LOW) {
      delay(150);
      if (attackState == 4) {
        running = false;
      } else {
        drawAttackScreen(attackState);
        switch (attackState) {
          case 0: Single(); break;
          case 1: All(); break;
          case 2: Becaon(); break;
          case 3: BecaonDeauth(); break;
        }
      }
    }

    if (digitalRead(BTN_UP) == LOW) {
      delay(150);
      if (attackState < 4) attackState++;
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      delay(150);
      if (attackState > 0) attackState--;
    }
  }
}

void networkSelectionLoop() {
  bool running = true;
  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }

  while (running) {
    drawNetworkList(SelectedSSID, scrollindex);

    if (digitalRead(BTN_OK) == LOW) {
      delay(150);
      while (digitalRead(BTN_OK) == LOW) {
        delay(10);
      }
      running = false;
    }

    if (digitalRead(BTN_UP) == LOW) {
      delay(150);
      if (static_cast<size_t>(scrollindex) < scan_results.size() - 1) {
        scrollindex++;
        SelectedSSID = scan_results[scrollindex].ssid;
        SSIDCh = scan_results[scrollindex].channel >= 36 ? "5G" : "2.4G";
      }
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      delay(150);
      if (scrollindex > 0) {
        scrollindex--;
        SelectedSSID = scan_results[scrollindex].ssid;
        SSIDCh = scan_results[scrollindex].channel >= 36 ? "5G" : "2.4G";
      }
    }

    display.display();
    delay(50);
  }
}

void settingsMenu() {
  int settingState = 0;
  bool running = true;

  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }

  while (running) {
    display.clearDisplay();
    drawStatusBar(currentLanguage == 0 ? "SETTINGS" : "CAI DAT");
    drawFrame();

    String perDeauthText = String(F("Per Deauth: ")) + String(perdeauth);
    String ledText = String(F("LED RGB: ")) + String(ledState ? "ON" : "OFF");
    String langText = String(F("Language: ")) + String(currentLanguage == 0 ? "EN" : "VI");

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 15);
    if (settingState == 0) display.setTextColor(BLACK, WHITE);
    display.print(perDeauthText);
    display.setTextColor(WHITE);
    display.setCursor(10, 25);
    if (settingState == 1) display.setTextColor(BLACK, WHITE);
    display.print(ledText);
    display.setTextColor(WHITE);
    display.setCursor(10, 35);
    if (settingState == 2) display.setTextColor(BLACK, WHITE);
    display.print(langText);
    display.setTextColor(WHITE);
    display.setCursor(10, 45);
    if (settingState == 3) display.setTextColor(BLACK, WHITE);
    display.print(currentLanguage == 0 ? "Back" : "Quay lai");

    display.display();

    if (digitalRead(BTN_OK) == LOW) {
      delay(150);
      if (settingState == 3) {
        running = false;
      } else if (settingState == 0) {
        perdeauth = (perdeauth % 20) + 1;
      } else if (settingState == 1) {
        ledState = !ledState;
        digitalWrite(LED_BLUE, ledState ? LOW : HIGH);
      } else if (settingState == 2) {
        currentLanguage = (currentLanguage + 1) % 2;
      }
      while (digitalRead(BTN_OK) == LOW) {
        delay(10);
      }
    }

    if (digitalRead(BTN_UP) == LOW) {
      delay(150);
      if (settingState < 3) settingState++;
      while (digitalRead(BTN_UP) == LOW) delay(10);
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      delay(150);
      if (settingState > 0) settingState--;
      while (digitalRead(BTN_DOWN) == LOW) delay(10);
    }
  }
}

void fadeTransition() {
  display.invertDisplay(true);
  delay(30);
  display.invertDisplay(false);
  for (int brightness = 255; brightness >= 0; brightness -= 32) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness);
    delay(20);
  }
  drawMainMenu(menustate);
  for (int brightness = 0; brightness <= 255; brightness += 32) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness);
    delay(20);
  }
}