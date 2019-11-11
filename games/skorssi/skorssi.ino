/*
This is a two player Scorched earth game for Arduino prototyping microcontroller board.
It uses arduno-tvout for video and sound.
*/ 


#include <TVout.h>
#include <fontALL.h>
TVout TV;

#define X_RESOLUTION 144
#define Y_RESOLUTION 80

// These define how the terrain looks like
#define HIGHEST_MOUNTAIN_HEIGHT 30
#define LOWEST_MOUNTAIN_HEIGHT 8
#define GROUND_POLY_ORDER 5
#define GROUND_SIN_SCALER 4

#define POWER_BAR_WIDTH 45
#define HEALTH_BAR_WIDTH 15
// at least 5
#define BAR_HEIGHT 6

#define EXPLOSION_SIZE 5
#define EXPLOSION_DURATION 10

// Controllers
#define PADDLE_DEAD_ZONE_LOW 256
// potentiometer game paddle input
#define POTPIN 2
#define BTNPIN 2

// ASSETS
PROGMEM const unsigned char TANK_SPRITE_R[] = {
7, 5,0x78, 0x86, 0xFE, 0xAA, 0x7C};
PROGMEM const unsigned char TANK_SPRITE_L[] = {
7, 5, 0x3C, 0xC2, 0xFE, 0xAA, 0x7C};
#define TANK_W 7
#define TANK_H 5
#define BARREL_LENGTH 4
#define GRAVITY 1
#define MAX_POWER 15

enum TurnPhase
{
  DRIVING,
  AIMING,
  SHOOTING,
  FLYING,
  EXPLODING,
  CLEANUP,
  NEXT,
  GAMEOVER
};


// STATE
int game_mode = 0;
float a[GROUND_POLY_ORDER];
float ground_min = +1000000.0;
float ground_max = -1000000.0;
int p1_xpos = X_RESOLUTION/5;
int player_turn = 0;
int turn_phase = AIMING; 

// tank positions
int p1x = X_RESOLUTION/4;
int p1y = 5;
int p1aim = -1.2;
int p2x = X_RESOLUTION*3/4;
int p2y = 5;
int p2aim = -1.2;

float p1_health = 1.0;
float p2_health = 1.0;

// cannon shell
long button_down_time = 0L;
int shx = 0;
int shy = 0;
int shdx = 0;
int shdy = 0;

// RE-USEABLE VARIABLES
int height = 0;
long prev_explosion_time = 0L;
float new_aim = 0.0;
float power = 0;
float interpolate = 0;
float loop_step = 0.0;
int tempx;
int tempy;
char* ptxt;

float eval_poly(float x)
{
   float value = 0.0;
   for (int i=0; i<GROUND_POLY_ORDER ; i++)
     value = a[i]*pow(x,i)+sin(x)*pow(GROUND_SIN_SCALER,GROUND_POLY_ORDER);
   return value;
}

float read_paddle_pos() {
  // max and normalization is used b/c my potentiometer was nonlinear, this trick disables the "slow" range
  int val = max(PADDLE_DEAD_ZONE_LOW, analogRead(POTPIN))-PADDLE_DEAD_ZONE_LOW;
  return ((float)val)/(1024-PADDLE_DEAD_ZONE_LOW); 
}

void setup()
{
  TV.begin(_PAL, X_RESOLUTION, Y_RESOLUTION);
  TV.clear_screen();
  
  TV.select_font(font4x6);
  
  //Serial.begin(9600);
  
  // Use internal pull-up resistor for the button
  pinMode(BTNPIN, INPUT_PULLUP);
 
  randomSeed(analogRead(0));
  
  // Randomize polynomial coefficients
  for (int i=0; i<GROUND_POLY_ORDER ; i++)
  {
    a[i]= (random(0,1000)/500.0-1.0);
  }
  
  float x,y;
  
  // Find min / max
  for (int i=0; i<X_RESOLUTION; i++)
  {
    // from -1.0 to 1.0
    x = (i-(X_RESOLUTION/2))/(float)(X_RESOLUTION/2);
    y = eval_poly(x*10.0);
    
    if (y<ground_min) ground_min=y;
    if (y>ground_max) ground_max=y;
  }
  //Serial.print("ground_min="); Serial.print(ground_min);Serial.print(", max="); Serial.println(ground_max);
    
  // Initial draw of the terrain
  for (int i=0; i<X_RESOLUTION; i++)
  {
    x = (i-(X_RESOLUTION/2))/(float)(X_RESOLUTION/2);
    y = eval_poly(x*10.0);
    
    //Serial.print("eval x="); Serial.print(x);Serial.print(", "); Serial.println(y);

    // normalize to 0.0-1.0 and then to 0-HIGHEST_MOUNTAIN_HEIGHT
    height = LOWEST_MOUNTAIN_HEIGHT + (int)( // at least this high
      ((y-ground_min)/(ground_max-ground_min))* // normalize to 0.0-1.0
      (HIGHEST_MOUNTAIN_HEIGHT-LOWEST_MOUNTAIN_HEIGHT) // range
    );
    TV.draw_column(i, Y_RESOLUTION-height, Y_RESOLUTION, 1);
    //Serial.print("x=");Serial.print(i);Serial.print(", ");
    //Serial.print("y0=");Serial.print(Y_RESOLUTION-height);Serial.print(", ");
    //Serial.print("y1=");Serial.print(Y_RESOLUTION);Serial.println(", ");
  }
}

void draw_tank_barrel(int tank_x, int tank_y, float angle, char color)
{
  TV.draw_line(
      tank_x+(TANK_W+1)/2, tank_y-1, // from
      tank_x+(TANK_W+1)/2+cos(angle)*BARREL_LENGTH, tank_y-1+sin(angle)*BARREL_LENGTH, // to
      color
  );
}

void shoot_shell(int tank_x, int tank_y, float angle, float power)
{
    // time to put the shell on it's way!
    shx = tank_x+(TANK_W+1)/2+cos(angle)*(BARREL_LENGTH+2);
    shy = tank_y-1+sin(angle)*(BARREL_LENGTH+2);
    shdx = (int)(cos(angle)*power*MAX_POWER);
    shdy = (int)(sin(angle)*power*MAX_POWER);
}

void draw_bar(int x0, int y0, int w, int h, float pos)
{
  TV.draw_rect(x0, y0, w, h, 1, 0);
  if (pos>0.0)
    TV.draw_rect(x0+2, y0+2, (int)(pos*(w-4)), h-4, 1, 1);
}


void update_tank(int& x, int& y, boolean points_right, float barrel_angle)
{
  boolean on_ground = false;
  for (int track_x=x ; track_x<x+TANK_W ; track_x++) {
    if (TV.get_pixel(track_x, y+TANK_H))
      on_ground = true;
  }
  if (!on_ground) {
    // Clear row under the sprite
    TV.draw_row(y, x, x+TANK_W, 0);
    draw_tank_barrel(x,y,barrel_angle,0);
    y++;
  }
  points_right ? TV.bitmap(x, y, TANK_SPRITE_R) : TV.bitmap(x, y, TANK_SPRITE_L);
  draw_tank_barrel(x,y,barrel_angle,1);
}

void explode_at(int exp_x, int exp_y)
{
  //  int exp_x = random(0, X_RESOLUTION);
  //  int exp_y = random(0, Y_RESOLUTION);

  // Explosion at px,py
  for (int i = 0 ; i<EXPLOSION_DURATION ; i++)
  {
    TV.draw_circle(exp_x, exp_y, EXPLOSION_SIZE, 1, 1);
    delay(100);
    TV.draw_circle(exp_x, exp_y, EXPLOSION_SIZE, 0, 0);
    delay(100);
  }
  
  float dp1 = sqrt(pow(exp_x-(p1x+TANK_W/2),2)+pow(exp_y-(p1y+TANK_H/2),2));
  if (dp1<EXPLOSION_SIZE+TANK_W/2)
  {
    p1_health = max(0.0, p1_health-dp1/(EXPLOSION_SIZE+TANK_W/2));
  }
  float dp2 = sqrt(pow(exp_x-(p2x+TANK_W/2),2)+pow(exp_y-(p2y+TANK_H/2),2));
  if (dp2<EXPLOSION_SIZE+TANK_W/2)
  {
    p2_health = max(0.0, p2_health-dp2/(EXPLOSION_SIZE+TANK_W/2));
  }
  
  // Modify terrain accordingly
  boolean some_erath_fell = true;
  while(some_erath_fell)
  {
    some_erath_fell=false;
    for (int px = exp_x-EXPLOSION_SIZE-1; px<exp_x+EXPLOSION_SIZE+1 ; px++)
    {
      int toppixel = 0;
      for (int py = Y_RESOLUTION-LOWEST_MOUNTAIN_HEIGHT-HIGHEST_MOUNTAIN_HEIGHT; py < Y_RESOLUTION ; py++)
      {
        // Do not check pixels on TANK!
        if ( (py > p1y && py<p1y+TANK_H && px > p1x && px<p1x+TANK_W) ||
             (py > p2y && py<p2y+TANK_H && px > p2x && px<p2x+TANK_W) )
          continue;
          
        // Serial.print(px);      Serial.print(",");      Serial.println(py);
        if (toppixel==0 && TV.get_pixel(px,py)==1) toppixel = py;
        if (toppixel!=0 && TV.get_pixel(px,py)==0)
        {
          // The pixel colunm "drops" by moving lowest to highest.
          TV.set_pixel(px, toppixel, 0);
          TV.set_pixel(px, py, 1);
          some_erath_fell=true;
          //Serial.print(toppixel);Serial.print("->");Serial.println(py);
          break;
        }
      }
    }
  }
}

void loop()
{
  boolean buttonPressed = (digitalRead(BTNPIN) == LOW);
  
  // Player health and turn indicator
  draw_bar(1, 1, HEALTH_BAR_WIDTH, BAR_HEIGHT, p1_health);
  draw_bar(X_RESOLUTION-1-HEALTH_BAR_WIDTH, 1, HEALTH_BAR_WIDTH, BAR_HEIGHT, p2_health);
  if (player_turn==0)
    ptxt = "P1<";
  else
    ptxt = "P1 ";
  TV.print(1+HEALTH_BAR_WIDTH+2,2,ptxt);
  if (player_turn==0)
    ptxt = " P2";
  else
    ptxt = ">P2"; 
  TV.print(X_RESOLUTION-1-HEALTH_BAR_WIDTH-1-4*3,2,ptxt);
  
  switch (turn_phase)
  {
    case AIMING:
      new_aim = read_paddle_pos()*-3.4;
      //Serial.println(new_aim);
      if (player_turn==0)
      {
        if (new_aim!=p1aim)
          draw_tank_barrel(p1x, p1y, p1aim, 0); // erase
        p1aim = new_aim;
      }
      if (player_turn==1)
      {
        if (new_aim!=p2aim)
          draw_tank_barrel(p2x, p2y, p2aim, 0); // erase
        p2aim = new_aim;
      }
      
      if (buttonPressed)
      {
        turn_phase=SHOOTING;
        button_down_time = millis();
      }
      break; // end AIMING
    case SHOOTING:
      power = min(1.0, max(0.1, (millis()-button_down_time)/1000.0));
      draw_bar(X_RESOLUTION/2-POWER_BAR_WIDTH/2, 1, POWER_BAR_WIDTH, BAR_HEIGHT, power);
      if (!buttonPressed)
      {

        //Serial.println(power);
        if (player_turn==0)
          shoot_shell(p1x, p1y, p1aim, power);          
        if (player_turn==1)
          shoot_shell(p2x, p2y, p2aim, power);
        turn_phase=FLYING;
        
        TV.draw_rect(X_RESOLUTION/2-POWER_BAR_WIDTH/2, 1, POWER_BAR_WIDTH, BAR_HEIGHT,0,0);
      } 
      break; // end SHOOTING
    case FLYING:
      TV.set_pixel(shx, shy, 0);
      
      // Check if the shell hits anything.
      loop_step = 2.0/(abs(shdx)+abs(shdy));
      for (interpolate=loop_step ; interpolate<=1.0 ; interpolate+=loop_step)
      {
        tempx = (int)(shx+interpolate*shdx);
        tempy = (int)(shy+interpolate*shdy);
        
        // Out of bounds, next step.
        if (tempx<0 || tempx>=X_RESOLUTION || tempy<0 || tempy>Y_RESOLUTION)
        {
          turn_phase = CLEANUP;
          break;
        }
        if ( TV.get_pixel(tempx,tempy)==1 )
        {
          turn_phase=EXPLODING;
          break;
        }
      }
      
      shx+=(int)(min(1.0, interpolate)*shdx);
      shy+=(int)(min(1.0, interpolate)*shdy);
      shdy+=GRAVITY;
      
      if (turn_phase==FLYING)
        TV.set_pixel(shx, shy, 1);
      break; // end FLYING
    case EXPLODING:
      explode_at(shx, shy);
      turn_phase=CLEANUP;
      break;
    case CLEANUP:
      if (p1_health==0.0)
      {
        TV.print(X_RESOLUTION/2-3*4, Y_RESOLUTION/3-6, "P2 WINS");
        explode_at(p1x+TANK_W/2, p1y+TANK_H/2);
        turn_phase=GAMEOVER;
      }
      else if (p2_health==0.0)
      {
        TV.print(X_RESOLUTION/2-3*4, Y_RESOLUTION/3-6, "P1 WINS");
        explode_at(p2x+TANK_W/2, p2y+TANK_H/2);
        turn_phase=GAMEOVER;
      }
      else
      {
        turn_phase=NEXT;
      }
      break;
    case NEXT:
      player_turn = (player_turn+1)%2;
      turn_phase = AIMING;
      break; 
    case GAMEOVER:
      break; 
  };

  if (turn_phase!=GAMEOVER)
  {
    update_tank(p1x, p1y, true, p1aim);
    update_tank(p2x, p2y, false, p2aim);
  }

  delay(20);

  
  // Wait 2000 ms between random explosions
  //if (!buttonPressed || TV.millis()-prev_explosion_time<2000) return;  
  //prev_explosion_time = TV.millis();
}

