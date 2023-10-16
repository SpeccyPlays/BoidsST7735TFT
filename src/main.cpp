#include <Arduino.h>
#include "TFT_ST7735.h"

TFT_ST7735 tft(128, 160);
//these are reversed for some reason, 128 should be width
byte SCREENWIDTH = 128;
byte SCREENHEIGHT= 160;
byte buffer = 20;

struct boidSingle{
  int8_t x;
  int8_t y;
  int8_t oldX;
  int8_t oldY;
  byte velocity;
  float angle;
};
boidSingle boidArray[20];
uint32_t globalAverageX = 0;
uint32_t globalAverageY = 0;
uint8_t amountOfBoids = 0;
//these two change the boid behaviour
byte maxSpeed = 5;
float avoidenceAngle = 0.2;
float aimAngle = 0.2;//used to head towards same direction as neighbours
byte avoidence = 1;
//just for ease of changing the time between screen refresh
byte loopDelay = 30;
void boidSetup(boidSingle *array);
void showBoids(boidSingle *array);
void firstRule(boidSingle *array);
void secondRule(int8_t &x, int8_t &y, float &angle, byte &velocity, boidSingle *array);
void findAngleBetweenPoints(int8_t &x, int8_t &y, float &angle, uint8_t targetX, uint8_t targetY);

void setup() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  amountOfBoids = sizeof(boidArray) / sizeof(boidArray[0]);//size of boid array
  randomSeed(analogRead(0));
  boidSetup(boidArray);
}

void loop() {
  showBoids(boidArray);
  delay(loopDelay);
}
void findAngleBetweenPoints(int8_t &x, int8_t &y, float &angle, uint8_t targetX, uint8_t targetY){
  /*
  Find the angle between the boid we're at and the center mass
  */
  angle = atan2(targetY - y, targetX - x);// * 180 / PI;
}
void firstRule(boidSingle *array){
  /*
  First boid rule is to establish the center mass of all boids
  */
  for (byte i = 0; i < amountOfBoids; i++){
    globalAverageX += array[i].x;
    globalAverageY += array[i].y;
  }
  globalAverageX = (globalAverageX / (amountOfBoids - 1));
  globalAverageY = (globalAverageY / (amountOfBoids - 1));
}
void secondRule(int8_t &x, int8_t &y, float &angle, byte &velocity, boidSingle *array){
  /*
  Second rule of boid is to avoid other boids
  */
  byte neighbourCount = 0;
  byte avgNeighbourVeloctity = 0;
  float avgNeighbourAngle = 0.0;
  for (byte i = 0; i < amountOfBoids; i++){
    if ((y + avoidence == array[i].y) && (x + avoidence == array[i].x)){
      neighbourCount ++;
      avgNeighbourVeloctity += array[i].velocity;
      angle -= avoidenceAngle;
      avgNeighbourAngle += array[i].angle;
    }
    if ((y - avoidence == array[i].y) && (x + avoidence == array[i].x)){
      angle += avoidenceAngle;
      neighbourCount ++;
      avgNeighbourVeloctity += array[i].velocity;
      avgNeighbourAngle += array[i].angle;
    }
    if((y - avoidence == array[i].y) && (x + avoidence == array[i].x)){
      angle += avoidenceAngle;
      neighbourCount ++;
      avgNeighbourVeloctity += array[i].velocity;
      avgNeighbourAngle += array[i].angle;
    }
    if ((y + avoidence == array[i].y) && (x - avoidence == array[i].x)){
      angle -= avoidenceAngle;
      neighbourCount ++;
      avgNeighbourVeloctity += array[i].velocity;
      avgNeighbourAngle += array[i].angle;
    }
  }
  if (neighbourCount > 0){
    velocity = avgNeighbourVeloctity / neighbourCount;
    if (avgNeighbourAngle > angle){
      angle += aimAngle;
    }
    else {
      angle -= aimAngle;
    }
  } 
}
void showBoids(boidSingle *array){
  /*
  Do the stuff.
  Check if we're inside or outside the 'screen' boundaries and adjust where needed
  */
	for (byte i = 0; i < amountOfBoids; i++){
    array[i].oldX = array[i].x;
    array[i].oldY = array[i].y;
    if (array[i].velocity < maxSpeed){
      array[i].velocity ++;
    }
    if (array[i].x < buffer){
			array[i].x = SCREENWIDTH - buffer - 10;
			array[i].velocity = 1;
		}
		else if (array[i].x > SCREENWIDTH - buffer){
			array[i].x = 30;
			array[i].velocity = 1;
		}
		if (array[i].y < buffer){
			array[i].y = SCREENHEIGHT - buffer - 10;
			array[i].velocity = 1; 
		}
		else if (array[i].y > SCREENHEIGHT - buffer){
			array[i].y = 30;
			array[i].velocity = 1;
		}
    firstRule(boidArray);
    /*
    Find the angle between boid current position and centre mass first
    The second rule will adjust the angle to avoid others
    */
    findAngleBetweenPoints(array[i].x, array[i].y, array[i].angle, globalAverageX, globalAverageY);
    secondRule(array[i].x, array[i].y, array[i].angle, array[i].velocity, boidArray);

    //moving toward the center mass
    array[i].y += array[i].velocity * sin(array[i].angle);
		array[i].x += array[i].velocity * cos(array[i].angle);
    tft.drawCircle(array[i].oldX, array[i].oldY, 3, TFT_BLACK);
    tft.drawCircle(array[i].x, array[i].y, 3, TFT_RED);
	}
}
void boidSetup(boidSingle *array){
  /*
  Go through all boids and set with random values
  */
  for (byte i = 0; i < amountOfBoids; i++){
    /*
    Generate random values
    */
    array[i].y = random(20, SCREENHEIGHT);
    array[i].x = random(20, SCREENWIDTH);
    array[i].velocity = random(1, maxSpeed);
    array[i].angle = random(0.0, 4.7);
  }
}