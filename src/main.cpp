#include <Arduino.h>
#include <Tone.h>
#include <EEPROM.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

int startTune[] = {NOTE_E4, NOTE_E4, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_G4};
int startTuneDuration[] = {100, 200, 100, 200, 100, 400};
int stepNote[] = {NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_A4};
int stepNoteDuration[] = {100, 100, 100, 300, 100, 300};
int tones[] = {NOTE_G3, NOTE_A3, NOTE_B3, NOTE_C4};
int ledPins[] = {8, 9, 10, 11};  // LED pins
int buttonPins[] = {2, 3, 4, 5}; // The four button input pins
int turn = 0;                    // turn counter
int randomArray[50];             // long to store up to 50 random colors
int highscore = 0;

U8G2_SSD1306_128X64_NONAME_1_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
Tone speaker;

void input();
void fail();
void setAllLEDStatus(int mode);
void playLedTone(int led);
void pushButtonTone(int button, int &step);

void setup()
{
  display.begin();
  display.setPowerSave(0);

  speaker.begin(12);

  EEPROM.get(0, highscore);

  for (int x = 0; x < 4; x++)
  {
    pinMode(ledPins[x], OUTPUT);       // LED pins are outputs
    pinMode(buttonPins[x], INPUT);     // button pins are inputs
    digitalWrite(buttonPins[x], HIGH); // enable internal pullup; buttons start in high position; logic reversed
  }

  display.firstPage();
  do
  {
    display.setFont(u8g2_font_cupcakemetoyourleader_tr);
    display.drawStr(15, 40, "Simon Says");
  } while (display.nextPage());

  randomSeed(analogRead(0)); //Added to generate "more randomness" with the randomArray for the output function

  for (int note = 0; note < 6; note++)
  {
    speaker.play(startTune[note]);
    switch (note)
    {
    case 0:
      digitalWrite(ledPins[0], HIGH);
      break;
    case 1:
      digitalWrite(ledPins[1], HIGH);
      break;
    case 2:
      digitalWrite(ledPins[0], HIGH);
      break;
    case 3:
      digitalWrite(ledPins[1], HIGH);
      break;
    case 6:
      digitalWrite(ledPins[3], HIGH);
      break;
    default:
      digitalWrite(ledPins[2], HIGH);
      break;
    }
    delay(startTuneDuration[note]);
    speaker.stop();
    setAllLEDStatus(LOW);
    delay(25);
  }
  delay(1000);
}

void loop()
{
  // Player's next move welcome music tone
  display.firstPage();
  do
  {
    display.setFont(u8g2_font_cupcakemetoyourleader_tr);
    display.drawStr(15, 40, "Simon Says");
  } while (display.nextPage());
  setAllLEDStatus(HIGH);
  for (int thisNote = 0; thisNote < 6; thisNote++)
  {
    // play the next note:
    speaker.play(stepNote[thisNote]);
    // hold the note:
    delay(stepNoteDuration[thisNote]);
    // stop for the next note:
    speaker.stop();
    delay(25);
  }
  setAllLEDStatus(LOW);
  delay(1000);

  if (highscore < turn)
  {
    highscore = turn;
    EEPROM.put(0, highscore);
  }
  // Show result on display
  display.firstPage();
  do
  {
    String stringMy = "Your score: ";
    stringMy += turn;
    String stringHigh = "Best score: ";
    stringHigh += highscore;
    display.setFont(u8g2_font_cupcakemetoyourleader_tr);
    display.drawStr(1, 25, stringMy.c_str());
    display.drawStr(1, 50, stringHigh.c_str());
  } while (display.nextPage());

  delay(1000);
  randomArray[turn] = random(0, 4); //Assigning a random number (0-3) to the randomArray[y], y being the turn count
  for (int step = 0; step <= turn; step++)
  {
    playLedTone(randomArray[step]);
  }
  input();
}

void input()
{ //Function for allowing user input and checking input against the generated array

  for (int step = 0; step <= turn;)   // num of note
  {                                   //Statement controlled by turn count
    for (int bNo = 0; bNo < 4; bNo++) // num of button
    {
      if (digitalRead(buttonPins[bNo]) == LOW)
      {
        pushButtonTone(bNo, step);
      }
    }
  }
  delay(500);
  turn++; //Increments the turn count, also the last action before starting the output function over again
}

void fail()
{ //Function used if the player fails to match the sequence
  //TODO: add current scores
  display.firstPage();
  do
  {
    display.setFont(u8g2_font_cupcakemetoyourleader_tr);
    display.drawStr(15, 30, "GAME OVER");
  } while (display.nextPage());
  for (int y = 0; y <= 3; y++)
  { //Flashes lights for failure

    setAllLEDStatus(HIGH);
    speaker.play(NOTE_G2, 300);
    delay(200);
    setAllLEDStatus(LOW);
    speaker.play(NOTE_C2, 300);
    delay(200);
  }
  delay(2000);
  display.firstPage();
  do
  {
    display.setFont(u8g2_font_cupcakemetoyourleader_tr);
    display.drawStr(15, 30, "GAME OVER");
    display.setFont(u8g2_font_8x13_mf);
    display.drawStr(0, 45, "Press any button");
    display.drawStr(25, 60, "to restart");
  } while (display.nextPage());
  {
    while (
        digitalRead(buttonPins[0]) == HIGH &&
        digitalRead(buttonPins[1]) == HIGH &&
        digitalRead(buttonPins[2]) == HIGH &&
        digitalRead(buttonPins[3]) == HIGH)
    {
      setAllLEDStatus(HIGH);
      delay(100);
      setAllLEDStatus(LOW);
      delay(100);
    }
    delay(1000);

    turn = -1; //Resets turn value so the game starts over without need for a reset button
  }
}


/**
 * Change all leds to state
 *
 * This function sets all leds pins to one state
 * it can be HIGH (all leds are ON) or LOW (all leds are OFF)
 *
 * @param mode Leds pin state.
 */
void setAllLEDStatus(int mode)
{
  for (int x = 0; x <= 3; x++)
  {
    digitalWrite(ledPins[x], mode);
  }
}
void playLedTone(int led)
{
  digitalWrite(ledPins[led], HIGH);
  speaker.play(tones[led], 100);
  delay(400);
  digitalWrite(ledPins[led], LOW);
  delay(100);
}

void pushButtonTone(int button, int &step)
{
  digitalWrite(ledPins[button], HIGH);
  speaker.play(tones[button], 100);
  delay(200);
  digitalWrite(ledPins[button], LOW);
  delay(250);
  if (randomArray[step] != button)
  {         //Checks value input by user and checks it against
    fail(); //the value in the same spot on the generated array
  }         //The fail function is called if it does not match
  step++;
}
