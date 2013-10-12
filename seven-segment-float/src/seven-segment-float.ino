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

byte displayNum[] = {off, off, off, off};  //array for display numbers from msb to lsb

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

  Serial.begin(9600);
}

void loop() {
  //current timer variables to reset at each iteration
  unsigned long currentRefresh = micros();  //get current microseconds for refresh counter

  clearLEDs();

  //display refresh timer loop, cycles through each digit at the interval of refreshInterval
  if (currentRefresh - previousRefresh > refreshInterval) {
    previousRefresh = currentRefresh;
    
    refreshLoop++;

    if (refreshLoop == 4)
      refreshLoop = 0;

    pickDigit(refreshLoop);
    displayNumber();
  }

  //testCaseAllNum();
  splitNumber(1000);
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
  updateShiftRegister(displayNum[refreshLoop]);
}

void splitNumber(double n) {
  //takes the input n and puts into the array num, split into 4 digits from lsb to msb
  double fractional;
  double integral;
  int digitsLeft = 0;
  int index = 0;

  int splitLeft[4]={-1,-1,-1,-1};
  int splitRight[4]={0,0,0,0};

  //clear previous values of the num array
  for (int i=0; i<4; i++)
  {
    displayNum[i] = off;
    negative = false;
  }

  if (n < 0) {
    n = abs(n);
    negative = true;
  }

  fractional = modf(n, &integral);
  int whole = int(integral);
  digitsLeft = numToArray(whole, splitLeft);
  int decimal = int(fractional * (pow(10,digitsLeft)));
  numToArray(decimal, splitRight);

  for (int i=0; i<digitsLeft; i++) {
      displayNum[i] = numbers[splitRight[i]];
      index++;
  }
  
  for (int j=0; j<4; j++) {
    if (splitLeft[j]!=-1)
      displayNum[j+index] = numbers[splitLeft[j]];
  }

  if (digitsLeft > 0)
    displayNum[digitsLeft]++;
}

int numToArray(int x, int store[]) {
  int left = 0;

  // if x is 0 to 9
  if (x < 10) {
    store[0] = x;
    left = 3;
  }
  //else if x is 10-99
  else if (x < 100) {
    store[1] = x / 10;
    store[0] = x % 10;
    left = 2;
  }
  //else if x is 100-999
  else if (x < 1000) {
    store[2] = x / 100;
    x = x % 100;
    store[1] = x / 10;
    store[0] = x % 10;
    left = 1;
  }
  //else if x is 1000-9999
  else if (x < 10000) {
    store[3] = x / 1000;
    x = x % 1000;
    store[2] = x / 100;
    x = x % 100;
    store[1] = x / 10;
    store[0] = x % 10;
    left = 0;
  }

  return left;
}

void updateShiftRegister(byte leds) {
  //sends byte to 8-bit shift register to turn on specific LEDs
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}


//global variables for the test function
const long testInterval = 500; //counter delay in milliseconds

long previousTestTime = 0; //previous count time
double testLoop = 0; //counter for counting loop

void testCaseAllNum() {
  //test case, a loop timer that runs through each integer from 0 to 9999
  unsigned long currentTestTime = millis();  //get current milliseconds for counting counter

  if (currentTestTime - previousTestTime > testInterval) {
    previousTestTime = currentTestTime;

    splitNumber(testLoop);

    testLoop+=0.05;

    //reset loop to 0 at end of displayable integers
    if (testLoop > 9999)
      testLoop = 0;
  }
}
