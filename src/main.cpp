#include <Arduino.h>
#include "TFT_ST7735.h"

TFT_ST7735 tft(160, 128);
//these are reversed for some reason, 128 should be width
byte SCREENWIDTH = 160;
byte SCREENHEIGHT= 128;
byte buffer = 20;

struct boidSingle{
  int16_t x;
  int16_t y;
  int16_t oldX;
  int16_t oldY;
  byte velocity;
  float angle;
};
byte boidRadius = 2;
byte avoidCheck = 2;
byte neighbourCheck = 10;
boidSingle boidArray[20];
uint32_t globalAverageX = 0;
uint32_t globalAverageY = 0;
uint8_t amountOfBoids = 0;
//these two change the boid behaviour
byte maxSpeed = 10;
float avoidenceAngle = 0.02;
float aimAngle = 0.1;//used to head towards same direction as neighbours
//just for ease of changing the time between screen refresh
byte loopDelay = 30;
void boidSetup(boidSingle *array);
void showBoids(boidSingle *array);
void firstRule(boidSingle *array);
void secondRule(int16_t &x, int16_t &y, float &angle, byte &velocity, boidSingle *array);
void findAngleBetweenPoints(int16_t &x, int16_t &y, float &angle, uint16_t targetX, uint16_t targetY);
byte boidCollisionDetection(int16_t &x, int16_t &y, int16_t &currentBoidx, int16_t &currentBoidY, byte r);

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
byte boidCollisionDetection(int16_t &x, int16_t &y, int16_t &currentBoidx, int16_t &currentBoidY, byte r){
  /*
  check if first point is within radius of circle as boids as circles
  */
  int16_t xx = x -currentBoidx;
  int16_t yy = y - currentBoidY;
  if ((( xx * xx) + (yy * yy)) < (r * r) ){
    return 1;
  }
  else {
    return 0;
  }
}
void findAngleBetweenPoints(int16_t &x, int16_t &y, float &angle, uint16_t targetX, uint16_t targetY){
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
void secondRule(int16_t &x, int16_t &y, float &angle, byte &velocity, boidSingle *array){
  /*
  Second rule of boid is to avoid other boids
  */
  byte neighbourCount = 0;
  byte avgNeighbourVeloctity = 0;
  float avgNeighbourAngle = 0.0;
  for (byte i = 0; i < amountOfBoids; i++){
    /*
    I've got to make sure I'm not checking the boid against itself
    I can't find a way to do it in C++ so gone fully janky
    */
    if (!((x == array[i].x) & (y == array[i].y) & (velocity == array[i].velocity))){
      if (boidCollisionDetection(x, y, array[i].x, array[i].y, boidRadius + avoidCheck)){
        /*if (velocity > 1){
          velocity --;
        };*/
        angle += avoidenceAngle;
        //angle += array[i].angle - angle;
      }
      else if (boidCollisionDetection(x, y, array[i].x, array[i].y, boidRadius + neighbourCheck)){
        neighbourCount ++;
        avgNeighbourVeloctity += array[i].velocity;
        avgNeighbourAngle += array[i].angle;
      }
    }
  }
  if (neighbourCount > 0){
    velocity = avgNeighbourVeloctity / neighbourCount;
    avgNeighbourAngle = avgNeighbourAngle / neighbourCount;
    angle += (avgNeighbourAngle - angle) * aimAngle;
    /*if (avgNeighbourAngle > angle){
      angle += aimAngle;
    }
    else {
      angle += aimAngle;
    }*/
  }
}
void showBoids(boidSingle *array){
  /*
  Do the stuff.
  Check if we're inside or outside the 'screen' boundaries and adjust where needed
  */
	for (byte i = 0; i < amountOfBoids; i++){
    
    /*
    Find the angle between boid current position and centre mass first
    The second rule will adjust the angle to avoid others
    */
    array[i].oldX = array[i].x;
    array[i].oldY = array[i].y;
    firstRule(boidArray);
    //findAngleBetweenPoints(array[i].x, array[i].y, array[i].angle, globalAverageX, globalAverageY);
    secondRule(array[i].x, array[i].y, array[i].angle, array[i].velocity, boidArray);
    //move the boid
    array[i].y += array[i].velocity * sin(array[i].angle);
		array[i].x += array[i].velocity * cos(array[i].angle);
    //check if it's now off screen
    if (array[i].velocity < maxSpeed){
      array[i].velocity ++;
    }
    if (array[i].x < 0){
			array[i].x = SCREENWIDTH;
			//array[i].velocity = 1;
		}
		else if (array[i].x > SCREENWIDTH){
			array[i].x = 0;
			//array[i].velocity = 1;
		}
		if (array[i].y < 0){
			array[i].y = SCREENHEIGHT;
			//array[i].velocity = 1; 
		}
		else if (array[i].y > SCREENHEIGHT){
			array[i].y = 0;
			//array[i].velocity = 1;
		}
    tft.drawCircle(array[i].oldX, array[i].oldY, boidRadius, TFT_BLACK);
    tft.drawCircle(array[i].x, array[i].y, boidRadius, TFT_RED);
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
    array[i].y = random(0, SCREENHEIGHT);
    array[i].x = random(0, SCREENWIDTH);
    array[i].velocity = random(1, maxSpeed);
    array[i].angle = random(0.0, 4.7);
  }
}