/*
4 digit 7-segment display
Digit selection controlled via Arduino digital pin, HIGH is off, LOW is on
7-segment LEDs are controlled via 74HC595 8-bit register

The display shows floats up to four digits

Copyright (c) 2013 Bryan Dagerman
*/
#include <math.h>

//set cathode pins, controls which digit is displayed
const int digits[] = {4, 5, 6, 7};

//set pins for 74HC595 8-bit register to control anodes, controls which segments display
const int dataPin = 8;
const int latchPin = 9;
const int clockPin = 10;

/* General LED layout of seven-segment display
  ___
F| A |B 
 |___|
E| G |C
 |___| .
   D   Dp

      A   B   C   D   E   F   G   Dp  Hex    Dec
________________________________________________
0   | 1   1   1   1   1   1   0   0 | 0xfc | 252
1   | 1   1   0   0   0   0   0   0 | 0x60 | 96
2   | 1   1   0   1   1   0   1   0 | 0xda | 218
3   | 1   1   1   1   0   0   1   0 | 0xf2 | 242
4   | 0   1   1   0   0   1   1   0 | 0x66 | 102
5   | 1   0   1   1   0   1   1   0 | 0xb6 | 182
6   | 1   0   1   1   1   1   1   0 | 0xbe | 190
7   | 1   1   1   0   0   0   0   0 | 0xe0 | 224
8   | 1   1   1   1   1   1   1   0 | 0xfe | 254
9   | 1   1   1   0   0   1   1   0 | 0xe6 | 230
-   | 0   0   0   0   0   0   1   0 | 0x02 | 2
off | 0   0   0   0   0   0   0   0 | 0x00 | 0
*/
const byte zero = 0xfc;
const byte one = 0x60;
const byte two = 0xda;
const byte three = 0xf2;
const byte four = 0x66;
const byte five = 0xb6;
const byte six = 0xbe;
const byte seven = 0xe0;
const byte eight = 0xfe;
const byte nine = 0xe6;

const byte minus = 0x02;
const byte off = 0x00;

const byte numbers[] = {zero, one, two, three, four, five, six, seven, eight, nine};

byte num[] = {off, off, off, off};  //array for display numbers from msb to lsb
bool negative = false;

const long refreshInterval = 10; //refresh delay in microseconds
long previousRefresh = 0; //previous refresh time
int refreshLoop = 0;  //counter for refreshing loop

void setup() {
  //setup  pin modes to output
  for (int i=0; i<4; i++)
    pinMode(digits[i], OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
}

void loop() {
  //current timer variables to reset at each iteration
  unsigned long currentRefresh = micros();  //get current microseconds for refresh counter

  clearLEDs();

  //display refresh timer loop, cycles through each digit at the interval of refreshInterval
  if (currentRefresh - previousRefresh > refreshInterval) {
    previousRefresh = currentRefresh;

    if (refreshLoop == 4)
      refreshLoop = 0;

    pickDigit(refreshLoop);
    displayNumber();
    refreshLoop++;
  }

  testCaseAllNum();
}

void clearLEDs() {
  //blank out LEDs for current digit
  updateShiftRegister(0);
}

void pickDigit(int j) {
  // pick which digit to display depending on the value passed in j
  for (int i=0; i<4; i++)
    digitalWrite(digits[i], HIGH);

  digitalWrite(digits[j], LOW);
  
  clearLEDs();
}

void displayNumber() {
  //turn on the LEDs accoring to the number stored in array num on the refresh cycle
  updateShiftRegister(num[refreshLoop]);
}

void splitNumber(double n) {
  //takes the input n and puts into the array num, split into 4 digits from lsb to msb
  double decimalPart = 0.0;
  int wholePart = 0;

  //clear previous values of the num array
  for (int i=0; i<4; i++)
  {
    num[i] = off;
    negative = false;
  }

  if (n < 0) {
    n = abs(n);
    negative = true;
  }

  decimalPart = fmod(n,1.0);
  wholePart = n - decimalPart;

  int splitPosition = splitWhole(wholePart);

  if(negative)
    num[splitPosition] = minus;
}

int splitWhole(int n) {
  //takes the input n and puts into the array num, and return the array position
  int whole[] = {-1,-1,-1,-1, -1};
  int wholePosition = 0;
  int j=0;

  if (negative){
    if (n > 999) {
      n = 0;
      negative = false;
    }
    else
      j = 1;
  }

  //if n is 0-9
  if (n < 10) {
    whole[0+j] = n;
  }
  //else if n is 10-99
  else if (n < 100) {
    whole[1+j] = n / 10;
    whole[0+j] = n % 10;
  }
  //else if n is 100-999
  else if (n < 1000) {
    whole[2+j] = n / 100;
    n = n % 100;
    whole[1+j] = n / 10;
    whole[0+j] = n % 10;
  }
  //else if n is 1000-9999
  else if (n < 10000) {
    whole[3+j] = n / 1000;
    n = n % 1000;
    whole[2+j] = n / 100;
    n = n % 100;
    whole[1+j] = n / 10;
    whole[0+j] = n % 10;
  }
  //else n is bigger than the display, so display 0
  else
    whole[0] = 0;

  for(int i=0; i<4; i++) {
    if(whole[i]!=-1) {
      num[wholePosition] = numbers[whole[i]];
      wholePosition++;
    }
  }

  return wholePosition;
}

void updateShiftRegister(byte leds) {
  //sends byte to 8-bit shift register to turn on specific LEDs
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}


//global variables for the test function
const long testInterval = 10; //counter delay in milliseconds
long previousTestTime = 0; //previous count time
int testLoop = -999; //counter for counting loop
int resetTestLoop = 10000;

void testCaseAllNum() {
  //test case, a loop timer that runs through each integer from 0 to 9999
  unsigned long currentTestTime = millis();  //get current milliseconds for counting counter

  if (currentTestTime - previousTestTime > testInterval) {
    previousTestTime = currentTestTime;

    splitNumber(testLoop);

    testLoop ++;

    //reset loop to 0 at end of displayable integers
    if (testLoop == resetTestLoop)
      testLoop = -999;
  }
}
