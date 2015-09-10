// catch

#include <TVout.h>
#include <fontALL.h>
TVout TV;

#define  NREST 0
#define  NOTE_sml_b 246
#define  NOTE_c 261
#define  NOTE_d 294 
#define  NOTE_e 329 
#define  NOTE_f 349 
#define  NOTE_g 392
#define  NOTE_a 440 
#define  NOTE_b 493 
#define  NOTE_C 523

#define SCREEN_WIDTH 112
#define SCREEN_HEIGHT 112

#define INITIAL_SPEED 10
#define PADDLE_DEAD_ZONE_LOW 256

// SPRITES 
PROGMEM const unsigned char COLLECT_SPRITE[] = {
5,5, 0x88,0xD8,0xF8,0xD8,0x88};
PROGMEM const unsigned char HEART_SPRITE[] = {
7,6, 0x7C,0xFE,0xFE,0x7C,0x38,0x10};
PROGMEM const unsigned char PLAYER_SPRITE[] = {
16,16,
0x78,0x1E,0xCC,0x33,0x84,0x21,0x87,0xE1,
0xCF,0xF3,0x7F,0xFE,0x3B,0xDC,0x75,0xAE,
0xF1,0x8F,0xFB,0xDF,0xFE,0x7F,0xFE,0x7F,
0x7F,0xFE,0x70,0x0E,0x3C,0x3C,0x0F,0xF0};
PROGMEM const unsigned char POT_GUIDE[] = {
16,16,
0x08,0x10,0x0C,0x30,0x3E,0x7C,0x4C,0x32,
0x88,0x11,0x80,0x01,0x81,0x81,0x09,0x90,
0x1F,0xF8,0x0F,0xF0,0x3E,0x7C,0x3E,0x7C,
0x0F,0xF0,0x1F,0xF8,0x09,0x90,0x01,0x80};

// SONG
PROGMEM const int scorenotes[]     =
{NOTE_g, NREST, NOTE_e, NREST, 
NOTE_c, NREST, NOTE_c, NREST, NREST, NOTE_g, NREST, NOTE_e, NREST,
NOTE_c, NREST, NOTE_c, NREST, NREST, NOTE_g, NREST, NOTE_e, NREST,
NOTE_c, NREST, NOTE_c, NREST, NOTE_c, NREST, NOTE_c, NREST, NOTE_c, NREST,
NOTE_d, NREST, NOTE_e, NREST, NOTE_f, NREST, NOTE_f, NREST,
NOTE_sml_b, NREST, NOTE_sml_b, NREST, NOTE_sml_b, NREST, NOTE_sml_b, NREST, NOTE_c, NREST, NOTE_d, NREST,
NOTE_e, NREST, NREST, // custom row. remove if continue the score
//NOTE_e, NREST, NREST, NOTE_g, NREST, NOTE_e, NREST,
//NOTE_c, NREST, NOTE_c, NREST, NOTE_c, NREST, NOTE_c, NREST, NOTE_c, NREST,
//NOTE_d, NREST, NOTE_e, NREST, NOTE_f, NREST, NOTE_f, NREST, NOTE_f, NREST, NOTE_d, NREST, 
//NOTE_sml_b, NREST, NOTE_sml_b, NREST, NOTE_sml_b, NREST, NOTE_sml_b, NREST, NOTE_c, NREST, NOTE_d, NREST,
//200,           50,         90,    20,         90, 
NREST};
PROGMEM const int scoredelays[] = {100, 25, 100, 25, 
200, 50, 200, 50, 250, 100, 25, 100, 25,
200, 50, 200, 50, 250, 100, 25, 100, 25,
200, 50, 200, 50, 100, 25, 200, 50, 100, 25,
100, 25, 200, 50, 100,  25, 450, 50,
200,  50, 90,  20, 90,  50, 90, 20, 90,  50, 200,  50,
200, 50, 500, // custom row. remove if continue the score
//200, 50, 500, 100, 25, 100, 25,
//200, 50, 200, 50, 100, 25, 200, 50, 100, 25,
//100, 25, 200, 50, 100,  25, 200, 50, 90, 20, 90, 50, 
// 1000};
10};
# define NOTE_COUNT 55

// potentiometer game paddle input
#define POTPIN 2

// GAME STATE
int state = 0;
int val = 0;  // paddle position
int newval = 0;
int pxpos = SCREEN_WIDTH/2; // player position
int cxpos = random(0, SCREEN_WIDTH-COLLECT_SPRITE[0]/2);
int cypos = 0;
int cspeed = 1; // collectible fall speed
int lives = 3;
unsigned long score = 0l;
char strscore[6] = "00000";

int i; // your generic loop variable

int nextnotetime = 0;
int looptime = 0;
int noteidx = 0;
int note = 0;
unsigned long duration = 0;

void setup() {
  TV.begin(_PAL, SCREEN_WIDTH, SCREEN_HEIGHT);
  TV.clear_screen();
  TV.select_font(font6x8);
  //Serial.begin(9600);
//  TV.fill(1);
}

int read_paddle_pos()
{
    // max is used b/c my potentiometer was nonlinear, this migitates the "slow" range
  return max(PADDLE_DEAD_ZONE_LOW, analogRead(POTPIN))-PADDLE_DEAD_ZONE_LOW;  
}

void loop() { 

  // PLAY MUSIC
  if (state>0 && state<3) {
    looptime = millis();
    if (looptime>=nextnotetime)
    {
      note = pgm_read_word_near(scorenotes+noteidx);
      duration = pgm_read_word_near(scoredelays+noteidx)-cspeed*2;
//      if ( note!=0 )
      if (note>200 && note<1000 && duration>10) // FOR DEBUG (PROTECTS YOUR EARS)
      {
        TV.tone(note, duration);
      }
      //Serial.print(note);
      //Serial.print(",");
      //Serial.println(duration);
      nextnotetime = looptime+(int)duration;
      noteidx++;
      if (noteidx>NOTE_COUNT) noteidx=0;
    }
  }
  
  // GAME INTRO
  if (state==0) {
    delay(500);
    TV.clear_screen();
    TV.print(SCREEN_WIDTH/5,SCREEN_HEIGHT/4,"TI-TI' PELI");
    TV.bitmap(2*SCREEN_WIDTH/5, SCREEN_HEIGHT/2, POT_GUIDE);
    val = read_paddle_pos();
    //Serial.print(val);
    state+=1;
    return; // EXIT LOOP
  } 
  // wait user input
  else if (state==1){
    newval = read_paddle_pos();
    //Serial.print(val);
    if (abs(val-newval)>5)
    {
      state+=1;
    }
    return;
  }
  // state == 2 -> game state. Continue 
  
  // 1. END CONDITION
  if (lives==0)
  {
    TV.print(SCREEN_WIDTH-33,0, strscore);
    TV.print(SCREEN_WIDTH/4,SCREEN_HEIGHT/2,"GAME OVER");
    TV.noTone();
    state=3;
    return; // EXIT LOOP
  }
  TV.clear_screen();
  
  // 2. LIVES AND SCORE
  for (i = 0 ; i < lives ; i++)
  {
    TV.bitmap(8*i, 0, HEART_SPRITE);
  }
  TV.print(SCREEN_WIDTH-33,0, strscore);
  
  // 3. FALLING THINGS
  TV.bitmap(cxpos, cypos, COLLECT_SPRITE);
  cypos+=max(1,cspeed/4);
  
  // 4. PLAYER 
  newval = read_paddle_pos();    // read the value from the sensor
  // Prevent jitter
  if (abs(val-newval)>2)
  {
    val = newval;
    pxpos = (int)((SCREEN_WIDTH-16)*(float)val/(1024-PADDLE_DEAD_ZONE_LOW));
  }
  TV.bitmap(pxpos, SCREEN_HEIGHT-17, PLAYER_SPRITE);
    
  // if collectible has fallen down
  if (cypos>=(SCREEN_HEIGHT-COLLECT_SPRITE[1]))
  { 
    // On top of each other, player collected the object
    int d = abs((cxpos+COLLECT_SPRITE[0]/2)-(pxpos+PLAYER_SPRITE[0]/2));
    if (d<PLAYER_SPRITE[0]/2)
    {
      score+=100*cspeed;
      sprintf(strscore, "%05lu", score);
      cspeed+=1;
    }
    else
    {
      lives-=1;
      
      if (lives==0)
      {
         TV.clear_screen();
      }
    }
    
    cxpos = random(0, SCREEN_WIDTH-COLLECT_SPRITE[0]/2);
    cypos = 0;
  }
  
  delay(max(5,INITIAL_SPEED-cspeed));
}

