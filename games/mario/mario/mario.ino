#include <TVout.h>
#include <fontALL.h>

TVout TV;

const int BUTTON_LEFT = 2;
const int BUTTON_RIGHT = 3;
const int BUTTON_JUMP = 4;

const byte PLAYER_SIZE = 2;
const byte GROUND_HEIGHT = 10;
const byte PLATFORM_HEIGHT = 3;
const byte PLATFORM_WIDTH = 10;

const byte NUMBER_OF_PLATFORMS = 3;
byte platforms[NUMBER_OF_PLATFORMS][2] = {
  {20, 80},
  {40, 60},
  {60, 40}
};

byte player_x;
byte player_y;
char player_dx;
char player_dy;
bool isJumping = false;

void reset_game() {
  player_x = TV.hres() / 2;
  player_y = TV.vres() - GROUND_HEIGHT - PLAYER_SIZE;
  player_dx = 0;
  player_dy = 0;
}

void setup() {
  TV.begin(_PAL, 112, 112);
  TV.select_font(font6x8);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_JUMP, INPUT_PULLUP);
  TV.delay(1000);
  reset_game();
}

bool isOnPlatform() {
  for (byte i = 0; i < NUMBER_OF_PLATFORMS; i++) {
    if (player_x + PLAYER_SIZE >= platforms[i][0] && player_x <= platforms[i][0] + PLATFORM_WIDTH &&
        player_y + PLAYER_SIZE >= platforms[i][1] && player_y <= platforms[i][1] + PLATFORM_HEIGHT) {
      return true;
    }
  }
  return false;
}

void loop() {
  // Handle input
  if (digitalRead(BUTTON_LEFT) == LOW && player_x > 0) {
    player_dx = -1;
  } else if (digitalRead(BUTTON_RIGHT) == LOW && player_x < TV.hres() - PLAYER_SIZE) {
    player_dx = 1;
  } else {
    player_dx = 0;
  }

  // Handle jump
  if (digitalRead(BUTTON_JUMP) == LOW) {
    if (!isJumping) {
      isJumping = true;
      player_dy = -8;
    }
  } else if (isJumping) {
    player_dy+=1;
  }

  // Update player position
  player_x += player_dx;
  

  // Predict new player position
  byte new_y = player_y + player_dy;
  // Check if the new position would collide with a platform
  byte platform_y = TV.vres() - GROUND_HEIGHT - PLATFORM_HEIGHT;
  for (byte i = 0; i < NUMBER_OF_PLATFORMS; i++) {
    if (player_x + PLAYER_SIZE >= platforms[i][0] && player_x <= platforms[i][0] + PLATFORM_WIDTH &&
        new_y + PLAYER_SIZE >= platforms[i][1] && new_y <= platforms[i][1] + PLATFORM_HEIGHT) {
      platform_y = platforms[i][1];
      break;
    }
  }
  player_y = min(platform_y, player_y + player_dy);
  

  // Gravity
  if (player_y < TV.vres() - GROUND_HEIGHT - PLAYER_SIZE && !isOnPlatform()) {
    player_dy += 1;
  } else {
    player_dy = 0;
    isJumping = false;
  }

  // Draw the screen
  TV.clear_screen();

  // Draw ground
  TV.draw_line(0, TV.vres() - GROUND_HEIGHT, TV.hres(), TV.vres() - GROUND_HEIGHT, WHITE);

  // Draw platforms
  for (byte i = 0; i < NUMBER_OF_PLATFORMS; i++) {
    TV.draw_rect(platforms[i][0], platforms[i][1]+PLATFORM_HEIGHT, PLATFORM_WIDTH, PLATFORM_HEIGHT, WHITE, WHITE);
  }

  // Draw player
  TV.draw_rect(player_x, player_y, PLAYER_SIZE, PLAYER_SIZE, WHITE, WHITE);

  // Wait for vsync
  TV.delay_frame(1);
}
