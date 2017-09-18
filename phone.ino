//Q Dong and A Karapostolakis, with portions by D Dubins, T Igoe, and C Maw
//2017-06-05
//Phone dialing program

#include <Keypad.h>

//Pin allocation
int latchPin = 9;
int clockPin = 10;
int dataPin = 11;
int speakerPin1 = 12;
int speakerPin2 = 13;

//Define keypad mapping (array format is [col][row])
char keymap[4][4]={{'1','4','7','*'},{'2','5','8','0'},{'3','6','9','#'},{'A','B','C','D'}};
//Keypad input pins
byte colP[4]= {4,3,2,A0};
byte rowP[4] = {8,7,6,5};
//Keypad accessor object
Keypad pad= Keypad(makeKeymap(keymap), colP, rowP, 4, 4);

//Store currently dialed number
char number[8];
int numberIndex;
//Records whether the number is composed entirely of ordinary digits or not
boolean numericNumber;

//DTMF tone definitions (X is column, Y is row - both from top left of keypad)
int toneX[] = {1209, 1336, 1477, 1633};
int toneY[] = {697, 770, 852, 941};

//Number definitions for seven-segment display
const byte ZERO = B10000001;
const byte ONE = B11001111;
const byte TWO = B10010010;
const byte THREE = B10000110;
const byte FOUR = B11001100;
const byte FIVE = B10100100;
const byte SIX = B10100000;
const byte SEVEN = B10001111;
const byte EIGHT = B10000000;
const byte NINE = B10000100;
const byte STAR = B10011100;
const byte POUND = B11001000;
const byte A = B10001000;
const byte B = B11100000;
const byte C = B10110001;
const byte D = B11000010;
const byte DASH = B11111110;
const byte CLEAR = B11111111;
const byte DEC_PT = B10000000;

//Seven-segment display timing variable
int currentDisplay;

//Tone player variables
long tone1StartTime;
long tone2StartTime;
int tone1Duration;
int tone2Duration;
boolean tone1High;
boolean tone2High;
boolean playTone;
//Tone timing variables
int toneLength;
int toneStartTime;
boolean timerOn;
boolean timerOff;

//Phone control variables
boolean offTheHook;
long lastAction;
boolean ringing;

void setup() {
  //assign output pins
  //data and clock pins for the shift register are handled by shiftOut
  pinMode(latchPin, OUTPUT);
  pinMode(speakerPin1, OUTPUT);
  pinMode(speakerPin2, OUTPUT);
  //initialise number storage array
  clearNumber();
  //initialise tone status
  playTone = false;
  //initialise display state
  currentDisplay = 0;
  //add trigger for keypad actions
  pad.addEventListener(keypadEvent);
  //phone is hung up initially
  offTheHook = false;
  //initialise random number generator
  randomSeed(analogRead(A5));
  //initialise call timeout
  lastAction = 0;
  //phone not initially ringing
  ringing = false;
}

void loop() {
  //get keypad input
  pad.getKey();
  //check if number is finished dialing (times out after 5 seconds)
  if ((millis() - lastAction > 5000) && offTheHook && number[0] != ' ') {
    ringing = true;
  }
  
  //if phone is ringing, play tones accordingly (2 s on, 4 s off)
  if (ringing) {
    if ((millis() % 6000 >= 0) && (millis() % 6000 < 2000)) {
      //ringer on
      startTone(440, 480);
    } else {
      //ringer off
      stopTone();
    }
  }
  //play current tone
  continueTone();
  //update LCD display
  updateDisplay();
}

//Clears the stored phone number
void clearNumber() {
  int i;
  for (i = 0; i < 8; i++) {
    number[i] = ' ';
  }
  numberIndex = 0;
  numericNumber = true;
}

//The startTone, continueTone, and stopTone methods have been adapted from public domain code written by David Dubins.
//Original code can be found at https://playground.arduino.cc/Main/DTMF

//Begins playing a new tone (unless tone is already playing)
void startTone(int tone1, int tone2) {
  if (!playTone) {
    playTone = true;
    //both tones start off
    tone1High = false;
    tone2High = false;
    //set toggle time for each tone in microseconds (for square wave, switch between high and low every half a period)
    tone1Duration = 500000/tone1;
    tone2Duration = 500000/tone2;
    //set start times for each tone cycle
    tone1StartTime = micros();
    tone2StartTime = micros();
  }
}

//Continues playing the current tone
void continueTone() {
  if (playTone) {
    //if half of tone1's period has passed
    if (micros() - tone1StartTime > tone1Duration) {
      //reset timer
      tone1StartTime = micros();
      //toggle tone1, update output
      tone1High = !tone1High;
      digitalWrite(speakerPin1, tone1High);
    }
    //if half of tone2's period has passed
    if (micros() - tone2StartTime > tone2Duration) {
      //reset timer
      tone2StartTime = micros();
      //toggle tone2, update output
      tone2High = !tone2High;
      digitalWrite(speakerPin2, tone2High);
    }
  }
}

//Stops the current tone
void stopTone() {
  playTone = false;
  digitalWrite(speakerPin1, LOW);
  digitalWrite(speakerPin2, LOW);
}

void beginDtmfDigit(char key) {
  switch(key) {
    case '1': startTone(toneX[0], toneY[0]); break;
    case '2': startTone(toneX[1], toneY[0]); break;
    case '3': startTone(toneX[2], toneY[0]); break;
    case 'A': startTone(toneX[3], toneY[0]); break;
    case '4': startTone(toneX[0], toneY[1]); break;
    case '5': startTone(toneX[1], toneY[1]); break;
    case '6': startTone(toneX[2], toneY[1]); break;
    case 'B': startTone(toneX[3], toneY[1]); break;
    case '7': startTone(toneX[0], toneY[2]); break;
    case '8': startTone(toneX[1], toneY[2]); break;
    case '9': startTone(toneX[2], toneY[2]); break;
    case 'C': startTone(toneX[3], toneY[2]); break;
    case '*': startTone(toneX[0], toneY[3]); break;
    case '0': startTone(toneX[1], toneY[3]); break;
    case '#': startTone(toneX[2], toneY[3]); break;
    case 'D': startTone(toneX[3], toneY[3]); break;
  }
}

//Manages keypress events
void keypadEvent(KeypadEvent key) {
  switch (pad.getState()) {
    case PRESSED:
      switch (key) {
        case 'A':
        case 'D':
          //should only be available if receiver is hung up
          if (!offTheHook) {
            //clear entry
            clearNumber();
            number[0] = key;
            //play tone
            beginDtmfDigit(key);
          }
          break;
        case 'B':
        case 'C':
          //should only be available if receiver is off the hook
          if (offTheHook) {
            //clear entry
            clearNumber();
            number[0] = key;
            //play tone
            beginDtmfDigit(key);
          }
          break;
        case '*':
        case '#':
          //number is no longer composed of only numbers
          numericNumber = false;
        default:
          //should only be able to dial if receiver is lifted
          if (offTheHook) {
            //stop previous tone
            stopTone();
            //play tone
            beginDtmfDigit(key);
            //add to dialed number
            //insert dash at 4th position if using normal number
            if (numberIndex == 3 && numericNumber) {
              number[numberIndex] = '-';
              numberIndex++;
            }
            if (numberIndex < 8) {
              //add selected digit to number
              number[numberIndex] = key;
              numberIndex++;
            }
          }
      }
      //action performed, reset timer
      lastAction = millis();
      break;
    case RELEASED:
      stopTone();
      switch (key) {
        case 'A':
          if (!offTheHook) {
            //pick up receiver
            offTheHook = true;
            ringing = false;
            //begin dial tone
            startTone(350, 440);
            clearNumber();
          }
          break;
        case 'B':
          if (offTheHook) {
            //hang up receiver
            offTheHook = false;
            ringing = false;
            //end all tones
            stopTone();
            clearNumber();
          }
          break;
        case 'C':
          if (offTheHook) {
            //clear stored number
            clearNumber();
          }
          break;
        case 'D':
          if (!offTheHook) {
            //display random number
            for(int i = 0; i < 8; i++){
              //select random character from 0 to 9 (ASCII values 48 through 57)
              number[i] = (char)(random(48,58));
              if (i == 3) {
                number[i] = '-';
                
              }
            }
            //ring phone
            ringing = true;
          }
          break;
      }
      break;
  }
}

//Outputs the currently entered phone number to the display
void updateDisplay() {
  //30 loops of the program between screen updates (to improve tone resolution)
  int DELAY = 30;
  currentDisplay = (currentDisplay + 1) % (8 * DELAY);
  if (currentDisplay % DELAY == 0) {
    digitalWrite(latchPin, 0);
    //output current number
    shiftOut(dataPin, clockPin, byteDigit(number[currentDisplay / DELAY]));
    //output segment to be lit
    shiftOut(dataPin, clockPin, bytePower(currentDisplay / DELAY));
    digitalWrite(latchPin, 1);
  }
}

//Converts an integer index indicating a position on the display into an equivalent byte for receipt by the shift register
byte bytePower(int pos){
  switch(pos) {
    case 0: return B10000000; break;
    case 1: return B01000000; break;
    case 2: return B00100000; break;
    case 3: return B00010000; break;
    case 4: return B00001000; break;
    case 5: return B00000100; break;
    case 6: return B00000010; break;
    case 7: return B00000001; break;
    //To clear display
    case -1: return B00000000; break;
  }
}

//Converts a character into its seven-segment binary representation
byte byteDigit(char key) {
  switch(key) {
    case '0': return ZERO; break;
    case '1': return ONE; break;
    case '2': return TWO; break;
    case '3': return THREE; break;
    case '4': return FOUR; break;
    case '5': return FIVE; break;
    case '6': return SIX; break;
    case '7': return SEVEN; break;
    case '8': return EIGHT; break;
    case '9': return NINE; break;
    case '#': return POUND; break;
    case '*': return STAR; break;
    case 'A': return A; break;
    case 'B': return B; break;
    case 'C': return C; break;
    case 'D': return D; break;
    case '-': return DASH; break;
    case ' ': return CLEAR; break;
  }
}

//The following method was written by Tom Igoe and Carlyn Maw.
//Original code can be found at https://www.arduino.cc/en/Tutorial/ShftOut21
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}
