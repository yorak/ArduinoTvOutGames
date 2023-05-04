#include <TVout.h>
#include <fontALL.h>
TVout TV;

const int PADDLE_BTN_LEFT = 2;
const int PADDLE_BTN_RIGHT = 3;

const byte PADDLE_WIDTH = 16;
const byte PADDLE_HEIGHT = 3;
const byte PADDLE_PAD = 2;
const byte BALL_SIZE = 2;

const byte BRICK_WIDTH = 12;
const byte BRICK_HEIGHT = 5;
const byte BRICK_COLS = 10;
const byte BRICK_ROWS = 4;

byte paddle_x;
byte ball_x;
byte ball_y;
char ball_dx;
char ball_dy;

bool bricks[BRICK_COLS][BRICK_ROWS];

void reset_game() {
  paddle_x = TV.hres() / 2 - PADDLE_WIDTH / 2;
  ball_x = 30;
  ball_y = 40;
  ball_dx = 1;
  ball_dy = -1;
  for (byte i = 0; i < BRICK_COLS; i++) for (byte j = 0; j < BRICK_ROWS; j++) bricks[i][j] = true;
}

void setup() {
  TV.begin(_PAL, 112, 112);
  TV.select_font(font6x8);
  pinMode(PADDLE_BTN_LEFT, INPUT_PULLUP);
  pinMode(PADDLE_BTN_RIGHT, INPUT_PULLUP);
  TV.delay(1000);
  reset_game();
}

void loop() {
  // Handle input
  if (digitalRead(PADDLE_BTN_LEFT) == LOW && paddle_x > 0) {
    paddle_x--;
  }
  if (digitalRead(PADDLE_BTN_RIGHT) == LOW && paddle_x < TV.hres() - 1 - PADDLE_WIDTH) {
    paddle_x++;
  }

  // Update ball position
  ball_x += ball_dx;
  ball_y += ball_dy;
  // Make sure we are not out of bounds.
  ball_x = min(max(BALL_SIZE, ball_x), TV.hres()-1-BALL_SIZE);
  ball_y = min(max(0, ball_y), TV.vres()-1-BALL_SIZE);

  // Check ball collisions
  if (ball_x <= BALL_SIZE || ball_x >= TV.hres() - BALL_SIZE -1) {
    ball_dx = -ball_dx;
  }
  if (ball_y <= 0) {
    ball_dy = -ball_dy;
  }

  // Check ball-paddle collision
  if (ball_y + BALL_SIZE >= TV.vres()-PADDLE_HEIGHT-PADDLE_PAD && ball_x + BALL_SIZE >= paddle_x && ball_x <= paddle_x + PADDLE_WIDTH) {
    ball_dy = -ball_dy;
  }

  // Check ball-brick collisions
  byte col = ball_x / BRICK_WIDTH;
  byte row = ball_y / BRICK_HEIGHT;
  if (row < BRICK_ROWS && col < BRICK_COLS && bricks[col][row]) {
    bricks[col][row] = false;
    ball_dy = -ball_dy;
  }

  // Check for end game
  if (ball_y>=TV.vres()-1-BALL_SIZE) {
    TV.delay(1000);
    reset_game();
  }

  // Draw the screen
  TV.clear_screen();

  // Draw paddle
  TV.draw_rect(paddle_x, TV.vres() - PADDLE_HEIGHT - PADDLE_PAD, PADDLE_WIDTH, PADDLE_HEIGHT, WHITE, WHITE);

  // Draw ball
  TV.draw_circle(ball_x, ball_y, BALL_SIZE, WHITE, WHITE);

  // Draw bricks
  for (byte i = 0; i < BRICK_COLS; i++) {
    for (byte j = 0; j < BRICK_ROWS; j++) {
      if (bricks[i][j]) {
        TV.draw_rect(i * BRICK_WIDTH, j * BRICK_HEIGHT, BRICK_WIDTH, BRICK_HEIGHT, WHITE);
      }
    }
  }

  // Wait for vsync
  TV.delay_frame(1);
}
