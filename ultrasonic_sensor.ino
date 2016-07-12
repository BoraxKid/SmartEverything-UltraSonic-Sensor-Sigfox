#include <Wire.h>
#include <SmeSFX.h>
#include <Arduino.h>

// If TEST is enabled, no message will be sent to Sigfox, and the sensor will trigger twice a second
#define TEST false
#define RED 0
#define GREEN 1
#define BLUE 2
#define NONE 3

// Pin numbers for the sensor
int trig = 9;
int pwm = 13;

unsigned long duration;
unsigned long distance;

// Command for the sensor module
uint8_t EnPwmCmd[4]={0x44,0x02,0xbb,0x01};

void setup()
{
  int i;
  uint8_t answer;
  
  setLight(NONE);
  SerialUSB.begin(115200);
  sfxAntenna.begin();
  
  if (TEST == false)
  {
    SerialUSB.println("SFX in Command mode");
    sfxAntenna.setSfxConfigurationMode();
    i = 1;
    do
    {
      answer = sfxAntenna.hasSfxAnswer();
      if (answer)
      {
        if (i == 1)
        {
          SerialUSB.println("SFX in Data mode");
          sfxAntenna.setSfxDataMode();
          ++i;
        }
        else if (i == 2)
          ++i;
      }
    } while (i != 3);
  }
  pinMode(trig, OUTPUT);
  digitalWrite(trig, HIGH);

  pinMode(pwm, INPUT);

  i = 0;
  while (i < 4)
  {
    SerialUSB.write(EnPwmCmd[i]);
    ++i;
  }
  delay(1500);
}

// Every hour, send a '0' or a '1' if there is an object at ~10cm of the sensor
// The light will be blue while the data is being sent, green if the last checked distance was correct, red otherwise
void loop()
{
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  
  duration = pulseIn(pwm, LOW);

  if (duration >= 10200)
  {
    sendSigfox('0');
    SerialUSB.println("Invalid");
    setLight(RED);
  }
  else
  {
    distance = duration / 50;
    SerialUSB.print("Distance: ");
    SerialUSB.print(distance);
    SerialUSB.println(" cm");
    if (distance >= 9 && distance <= 11)
    {
      sendSigfox('1');
      setLight(GREEN);
    }
    else
    {
      sendSigfox('0');
      setLight(RED);
    }
  }
  if (TEST == false)
    delay(3600000);
  else
    delay(500);
}

// Code to send a char using the Sigfox network
void sendSigfox(char here)
{
  if (TEST == false)
  {
    bool answer;
    bool finished = false;
  
    setLight(BLUE);
    sfxAntenna.sfxSendData(&here, 1);
    while (!finished)
    {
      answer = sfxAntenna.hasSfxAnswer();
      if (answer)
      {
        if (sfxAntenna.getSfxMode() == sfxDataMode)
        {
          switch (sfxAntenna.sfxDataAcknoledge())
          {
            case SFX_DATA_ACK_START:
              SerialUSB.println("Waiting an answer");
              break;
  
            case SFX_DATA_ACK_PROCESSING:
              SerialUSB.print('.');
              break;
  
            case SFX_DATA_ACK_OK:
              SerialUSB.println(' ');
              SerialUSB.println("OK");
              finished = true;
              break;
  
            case SFX_DATA_ACK_KO:
              SerialUSB.println(' ');
              SerialUSB.println("KO");
              finished = true;
              break;
          }
        }
      }
    }
  }
}

void setLight(char state)
{
  if (state == RED)
  {
    ledRedLight(HIGH);
    ledGreenLight(LOW);
    ledBlueLight(LOW);
  }
  else if (state == GREEN)
  {
    ledRedLight(LOW);
    ledGreenLight(HIGH);
    ledBlueLight(LOW);
  }
  else if (state == BLUE)
  {
    ledRedLight(LOW);
    ledGreenLight(LOW);
    ledBlueLight(HIGH);
  }
  else if (state == NONE)
  {
    ledRedLight(LOW);
    ledGreenLight(LOW);
    ledBlueLight(LOW);
  }
}
