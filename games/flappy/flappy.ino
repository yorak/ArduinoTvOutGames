#include <TVout.h>
#include <fontALL.h>
TVout TV;

const int JUMP_BTN_PIN = 2;

float fball_y = 40;
float fball_dy = -1.5;
int port_x = 0;
int port_y = 0;

bool btn_pressed = false;
bool game_over = false;
int score = 0;

void setup() {
  // put your setup code here, to run once:
  TV.begin(_PAL);
  TV.select_font(font6x8);
  pinMode(JUMP_BTN_PIN, INPUT_PULLUP);
  TV.delay(1000);
  new_port();
}

void new_port() {
  port_x=TV.hres()-1;
  port_y=random(20,TV.vres()-30); 
}

void loop() {
  // put your main code here, to run repeatedly:

  // 1. check for game end condition
  if ((int)fball_y+4>TV.vres() || (int)fball_y-3<0) game_over=true;
  if (32==port_x && (fball_y<port_y-16 || fball_y>port_y+16)) game_over=true; 

  // 2. if port leaves the screen, create new
  if (port_x==1) {
    score=score+1;
    new_port();
  }

  // 3. handle input
  if (digitalRead(JUMP_BTN_PIN)==HIGH) {
    btn_pressed = false;
  }
  else {
    // jump up only _once_ when the btn goes down
    if (!btn_pressed) {
      fball_dy=-1.5;
      btn_pressed = true;

      // start a new game with a press of a btn
      if (game_over) {  
        score = 0;
        fball_y = 40;
        new_port();
        game_over = false;
      }
    }
  }
  // do not update state if game is already over
  if (game_over) return;  

  // 4. update state by ...
  // ... accelerating fall, and
  fball_dy=fball_dy+0.04;
  fball_y=fball_y+fball_dy;
  // ... progressing wall
  port_x=port_x-1;

  // 5. wait for vsync and then redraw the screen
  TV.delay_frame(1);
  TV.clear_screen();
  TV.print(2, 2, score);
  // ports
  TV.draw_line(port_x, 0, port_x, port_y-16, WHITE);
  TV.draw_line(port_x, port_y+16, port_x, TV.vres()-1, WHITE);
  // ball
  TV.draw_circle(30, (int)fball_y, 3, WHITE, WHITE);
  // "eye" of the ball
  if (btn_pressed) TV.draw_line(30, (int)fball_y, 33, (int)fball_y, BLACK);
  else TV.draw_circle(31, (int)fball_y, 1, BLACK, BLACK);
}
