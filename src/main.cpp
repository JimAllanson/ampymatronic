#include <Arduino.h>
#include <ESP32Servo.h>
#include <Arduino_GFX_Library.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

//servo closed 80
//servo open 120
#define BUTTON 5

#define AUDIO_TX 21
#define AUDIO_RX 22

#define EYELID_COLOR 0xD269
#define EYELID_SHADE 0xA986

HardwareSerial softSerial(1);
DFRobotDFPlayerMini myDFPlayer;

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_HWSPI(33 /* DC */, 32 /* CS */);
Arduino_DataBus *bus2 = new Arduino_HWSPI(25 /* DC */, 26 /* CS */);
Arduino_GFX *rightEye = new Arduino_GC9A01(bus, 21 /* RST */, 0 /* rotation */, true /* IPS */);
Arduino_GFX *leftEye = new Arduino_GC9A01(bus2, 27 /* RST */, 0 /* rotation */, true /* IPS */);

void songOne();
void songTwo();
void songThree();

void eyesOpen();
void eyesOpenXY(int x, int y);
void eyesClosed();
void eyelidsDown();
void wink();

void mouthClose();
void mouthOpen();

void idleLoop(void *pvParameters);
TaskHandle_t idleTask;

Servo servo;

int song = 0;
int isPlaying = 0;

void setup(void)
{
  softSerial.begin(9600, SERIAL_8N1, AUDIO_TX, AUDIO_RX);
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50); // Standard 50hz servo
  servo.attach(4, 500, 2400);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  delay(100);
  while (!myDFPlayer.begin(softSerial))
  {
    Serial.print(F("."));
    delay(10);
  }

  Serial.println(F("DFPlayer Mini online."));

  delay(100);

  myDFPlayer.volume(30); //Set volume value. From 0 to 30

  rightEye->begin();
  leftEye->begin();
  rightEye->fillScreen(BLACK);
  leftEye->fillScreen(BLACK);

  delay(1000);

  xTaskCreatePinnedToCore(idleLoop, "IdleTask", 10000, NULL, 1, &idleTask, 0);

  pinMode(BUTTON, INPUT_PULLUP);
}

void eyesOpen()
{
  eyesOpenXY(120, 90);
}

void eyesOpenXY(int x, int y)
{
  rightEye->fillScreen(0xffff);
  rightEye->fillEllipse(x, y, 60, 80, 0x0000);
  leftEye->fillScreen(0xffff);
  leftEye->fillEllipse(x, y, 60, 80, 0x0000);
}

void eyelidsDown()
{
  eyesOpenXY(120, 120);

  rightEye->fillTriangle(0, 30, 240, 0, 240, 120, EYELID_COLOR);
  rightEye->fillRect(0, 0, 240, 30, EYELID_COLOR);
  leftEye->fillRect(0, 0, 240, 30, EYELID_COLOR);
  leftEye->fillTriangle(0, 0, 240, 30, 0, 120, EYELID_COLOR);
}

void eyesClosed()
{
  rightEye->fillScreen(EYELID_COLOR);
  rightEye->fillRect(0, 130, 240, 15, EYELID_SHADE);
  leftEye->fillScreen(EYELID_COLOR);
  leftEye->fillRect(0, 130, 240, 15, EYELID_SHADE);
}

void songOne()
{
  isPlaying = 1;
  Serial.println("Playing song 1");
  mouthClose();

  myDFPlayer.play(1);
  delay(500);

  eyesOpen();
  mouthOpen();
  delay(2000);
  mouthClose();
  delay(300);

  eyelidsDown();
  mouthOpen();
  delay(2000);
  mouthClose();
  delay(300);

  eyesClosed();
  mouthOpen();
  delay(1700);
  mouthClose();
  delay(300);

  eyesOpen();
  mouthOpen();
  delay(1800);
  mouthClose();
  isPlaying = 0;
}

void songTwo()
{
  isPlaying = 1;
  Serial.println("Playing song 2");
  mouthClose();

  myDFPlayer.play(2);
  delay(500);

  eyelidsDown();
  mouthOpen();
  delay(3000);
  mouthClose();
  delay(300);

  eyesClosed();
  mouthOpen();
  delay(2000);
  mouthClose();
  delay(1200);

  eyelidsDown();
  mouthOpen();
  delay(3000);
  mouthClose();
  delay(300);

  eyesOpen();
  mouthOpen();
  delay(2000);
  mouthClose();
  isPlaying = 0;
}

void songThree()
{
  isPlaying = 1;
  Serial.println("Playing song 3");
  mouthClose();

  myDFPlayer.play(3);
  delay(500);

  eyesOpenXY(90, 150);
  mouthOpen();
  delay(900);
  mouthClose();
  delay(200);

  eyesOpenXY(150, 150);
  mouthOpen();
  delay(900);
  mouthClose();
  delay(200);

  eyesOpenXY(90, 90);
  mouthOpen();
  delay(800);
  mouthClose();
  delay(200);

  eyesOpenXY(150, 90);
  mouthOpen();
  delay(800);
  mouthClose();
  delay(200);

  eyelidsDown();
  mouthOpen();
  delay(1200);
  mouthClose();

  eyesClosed();
  mouthOpen();
  delay(1200);
  mouthClose();
  isPlaying = 0;
}

void mouthClose()
{
  servo.write(70);
}

void mouthOpen()
{
  servo.write(110);
}

void wink()
{
  eyesOpen();
  delay(5000);
  rightEye->fillScreen(EYELID_COLOR);
  rightEye->fillRect(0, 130, 240, 15, EYELID_SHADE);
  delay(2000);
  eyesOpen();
}

void loop()
{
  if (digitalRead(BUTTON) == 0)
  {
    switch (song)
    {
    case 0:
      song++;
      return wink();
    case 1:
      song++;
      return songOne();
    case 2:
      song++;
      return songTwo();
    case 3:
      song = 0;
      return songThree();
    }
  }
  delay(10);
}

void idleLoop(void *pvParameters)
{
  for (;;)
  {
    if (!isPlaying)
      eyesOpenXY(random(90, 150), random(90, 150));
    delay(random(2000, 5000));
    if (!isPlaying)
      eyesOpenXY(random(90, 150), random(90, 150));
    delay(random(1000, 5000));
    if (!isPlaying)
      eyesClosed();
    delay(100);
  }
}