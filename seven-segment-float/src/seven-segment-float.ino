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
  //splitNumber(-99.99999);
  displayTemperature();

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

void updateShiftRegister(byte leds) {
  //sends byte to 8-bit shift register to turn on specific LEDs
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}

bool splitNumber(double n) {
  //takes the input n and puts into the array num, split into 4 digits from lsb to msb
  double fractional;
  double integral;
  int i;
  bool negative = false;

  int store[]={-1,-1,-1,-1};

  //clear previous values of the num array
  for (i=0; i<4; i++)
    displayNum[i] = off;

  if (n < 0.0)
    negative = true;

  n = abs(n);

  fractional = modf(n, &integral);
  fractional = round(fractional*10);
  int decimal = int(fractional);
  int whole = int(integral);

  if (decimal >= 10) {
    decimal = 0;
    whole++;
  }

  if (negative) {
    if (whole >= 100)
      return false;
    displayNum[3] = minus;
  }
  else {
    if (whole >= 1000)
      return false;
  }
  
  displayNum[0] = numbers[decimal];

  // if whole is 0 to 9
  if (whole < 10) {
    store[1] = whole;
  }
  //else if whole is 10-99
  else if (whole < 100) {
    store[2] = whole / 10;
    store[1] = whole % 10;
  }
  //else if whole is 100-999
  else if (whole < 1000) {
    store[3] = whole / 100;
    whole = whole % 100;
    store[2] = whole / 10;
    store[1] = whole % 10;
  }
  
  for (i=1; i<4; i++) {
    if (store[i]!=-1)
      displayNum[i] = numbers[store[i]];
  }

  displayNum[1]++;

  return true;
}

//global variables for the test function
const long testInterval = 50; //counter delay in milliseconds

long previousTestTime = 0; //previous count time
double testLoop = -100; //counter for counting loop

void testCaseAllNum() {
  //test case, a loop timer that runs through each integer from 0 to 9999
  unsigned long currentTestTime = millis();  //get current milliseconds for counting counter

  if (currentTestTime - previousTestTime > testInterval) {
    previousTestTime = currentTestTime;

    splitNumber(testLoop);

    testLoop+=0.1;

    //reset loop to 0 at end of displayable integers
    if (testLoop > 1000)
      testLoop = -100;
  }
}

//global variables for LM35 temperature sensor
const int tempPin = 0;  //analog pin 0
const long tempInterval = 1000;  //time to delay between refreshing the display, in ms
long previousTempRefresh = 0; //previous temperature refresh

void displayTemperature () {
  unsigned long currentTempRefresh = millis();

  if (currentTempRefresh - previousTempRefresh > tempInterval) {
    previousTempRefresh = currentTempRefresh;

    double tempC = (500.0 * analogRead(tempPin)) / 1204; //poll analog to read value from LM35 
    double tempF = c2f(tempC);

    splitNumber(tempF);
  }
}

double c2f (double c) {
  return ((9*c)/5 + 32);
}