// TODO: there is a bug in move_to_next_tail_piece() that causes
//  the snake to be cut in two in some situations. I tried to 
//  save memory by reading the snake form from the memory, but 
//  I did not think it through and it does not work. I guess one
//  needs to use 2 bits per snake piece. To be fixed :)

#include <TVout.h>
TVout TV;

const int TURN_LEFT_BTN_PIN = 2;
const int TURN_RIGHT_BTN_PIN = 3;
const int GAME_SPEED = 7500;

int loop_counter = 0;
bool left_turn_pending = false;
bool right_turn_pending = false;

int snake_head_x = 0;
int snake_head_y = 0;
int snake_tail_x = 0;
int snake_tail_y = 0;
int snake_dx = 1;
int snake_dy = 0;
int apple_x = 10;
int apple_y = 10;

void setup() {
  // put your setup code here, to run once:
  TV.begin(_PAL, 64, 48);
  pinMode(TURN_LEFT_BTN_PIN, INPUT_PULLUP); // float pin high
  pinMode(TURN_RIGHT_BTN_PIN, INPUT_PULLUP); // float pin high
  init_game();
}

void loop() {
  // put your main code here, to run repeatedly:
 
  // controls 
  if (digitalRead(TURN_LEFT_BTN_PIN)==LOW) left_turn_pending = true;
  if (digitalRead(TURN_RIGHT_BTN_PIN)==LOW) right_turn_pending = true;

  // ignore most of the loops
  loop_counter = loop_counter+1;
  if (loop_counter<GAME_SPEED) return;
  else loop_counter = 0;

  // turn if there are pending turns
  turn_as_needed();
  
  // head slithers on
  snake_head_x = snake_head_x+snake_dx;
  snake_head_y = snake_head_y+snake_dy; 

  // wait for the vsync to avoid sceen flicker
  TV.delay_frame(1);
  if (snake_head_x==apple_x && snake_head_y==apple_y) {
    new_apple();
  }
  else if (TV.get_pixel(snake_head_x, snake_head_y)==WHITE ) {
    // white, but not apple, ate itself or wall -> Game over
    init_game();
  }
  else {
    TV.set_pixel(snake_tail_x, snake_tail_y, BLACK);
    move_to_next_tail_piece();
  }
  TV.set_pixel(snake_head_x, snake_head_y, WHITE);
}

void turn_as_needed() {
  int new_snake_dx = 0; int new_snake_dy = 0;
  if (left_turn_pending) {
    new_snake_dx = -snake_dy; new_snake_dy = snake_dx;
    snake_dx = new_snake_dx; snake_dy = new_snake_dy;
    left_turn_pending = false;
  }
  if (right_turn_pending) {
    new_snake_dx = snake_dy; new_snake_dy = -snake_dx;
    snake_dx = new_snake_dx; snake_dy = new_snake_dy;
    right_turn_pending = false;
  }
}

void init_game() {
  // Snake starts from the middle and moves right
  snake_dx = 1;
  snake_dy = 0;
  snake_tail_x = TV.hres()/2;
  snake_tail_y = TV.vres()/2;
  snake_head_x = snake_tail_x+snake_dx;
  snake_head_y = snake_tail_y+snake_dy;
  
  // Draw snake and add the first apple
  TV.clear_screen();
  TV.draw_rect(0, 0, TV.hres()-1, TV.vres()-1, WHITE, BLACK); 
  TV.set_pixel(snake_tail_x, snake_tail_y, WHITE);
  TV.set_pixel(snake_head_x, snake_head_y, WHITE);
  new_apple();  
}

void new_apple() {
  while (true) {
    apple_x = random(1, TV.hres());
    apple_y = random(1, TV.vres());
    // can put apple here, exit the while loop
    if (TV.get_pixel(apple_x, apple_y)==BLACK) break; 
  }
  TV.set_pixel(apple_x, apple_y, WHITE);
}

void move_to_next_tail_piece() {
  int dx=1; int dy=0;
  for (int i=0; i<4; i++) {
    // iterate (1,0), (0,-1), (-1,0), (0,1)
    int new_dx = 0; int new_dy = 0;
    new_dx = -dy; new_dy = dx;
    dx = new_dx; dy = new_dy;
    
    if (snake_tail_x+dx==1 || snake_tail_x+dx==TV.hres()-1) continue;
    if (snake_tail_y+dy==1 || snake_tail_y+dy==TV.vres()-1) continue;
    if (snake_tail_x+dx==apple_x && snake_tail_y+dy==apple_y) continue;
    if (TV.get_pixel(snake_tail_x+dx, snake_tail_y+dy)==WHITE) {
      snake_tail_x = snake_tail_x+dx;
      snake_tail_y = snake_tail_y+dy;
      break;
    }
  }
}