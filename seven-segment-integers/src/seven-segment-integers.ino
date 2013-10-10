/*
4 digit 7-segment display
Digit selection controlled via Arduino digital pin, HIGH is off, LOW is on
7-segment LEDs are controlled via 74HC595 8-bit register

The display shows positive integers from 0 to 9999

Copyright (c) 2013 Bryan Dagerman
*/

/* General LED layout of seven-segment display
  ___
F| A |B 
 |___|
E| G |C
 |___| .
   D   Dp

    A   B   C   D   E   F   G   Dp  Hex    Dec
________________________________________________
0 | 1   1   1   1   1   1   0   0 | 0xfc | 252
1 | 1   1   0   0   0   0   0   0 | 0x60 | 96
2 | 1   1   0   1   1   0   1   0 | 0xda | 218
3 | 1   1   1   1   0   0   1   0 | 0xf2 | 242
4 | 0   1   1   0   0   1   1   0 | 0x66 | 102
5 | 1   0   1   1   0   1   1   0 | 0xb6 | 182
6 | 1   0   1   1   1   1   1   0 | 0xbe | 190
7 | 1   1   1   0   0   0   0   0 | 0xe0 | 224
8 | 1   1   1   1   1   1   1   0 | 0xfe | 254
9 | 1   1   1   0   0   1   1   0 | 0xe6 | 230
*/
byte numbers[] = {0xfc, 0x60, 0xda, 0xf2, 0x66, 0xb6, 0xbe, 0xe0, 0xfe, 0xe6};

//set cathode pins, controls which digit is displayed
const int digits[] = {4, 5, 6, 7};

//set pins for 74HC595 8-bit register to control anodes, controls which segments display
const int dataPin = 8;
const int latchPin = 9;
const int clockPin = 10;

const long refreshInterval = 10; //refresh delay in microseconds
long previousRefresh = 0; //previous refresh time
int refreshLoop = 0;  //counter for refreshing loop

int num[] = {0,-1,-1,-1};  //array for display numbers, {ones, tens, hundreds, thousands}

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
  //turn on the LEDs accoring to the number from 0-9
  int dispNum=num[refreshLoop];

  if (dispNum == -1)
    clearLEDs();
  else
    updateShiftRegister(numbers[dispNum]);
}

void splitNumber(int n) {
  //takes the input n and puts into the array num, split in ones, tens, hundreds, and thousands place
  
  //clear previous values of the num array, -1 value allows checking to see if a new value has been placed
  for (int i=1; i<4; i++)
    num[i] = -1;

  //if n is 0-9
  if (n < 10) {
    num[0] = n;
  }
  //else if n is 10-99
  else if (n < 100) {
    num[1] = n / 10;
    num[0] = n % 10;
  }
  //else if n is 100-999
  else if (n < 1000) {
    num[2] = n / 100;
    n = n % 100;
    num[1] = n / 10;
    num[0] = n % 10;
  }
  //else if n is 1000-9999
  else if (n < 10000) {
    num[3] = n / 1000;
    n = n % 1000;
    num[2] = n / 100;
    n = n % 100;
    num[1] = n / 10;
    num[0] = n % 10;
  }
  //else n is bigger than the display, so display 0
  else
    num[0] = 0;
}

void updateShiftRegister(byte leds) {
  //sends byte to 8-bit shift register to turn on specific LEDs
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}

//global variables for the test function
const long testInterval = 100; //counter delay in milliseconds
long previousTestTime = 0; //previous count time
int testLoop = 0; //counter for counting loop

void testCaseAllNum() {
  //test case, a loop timer that runs through each integer from 0 to 9999
  unsigned long currentTestTime = millis();  //get current milliseconds for counting counter

  if (currentTestTime - previousTestTime > testInterval) {
    previousTestTime = currentTestTime;

    splitNumber(testLoop);

    testLoop ++;

    //reset loop to 0 at end of displayable integers
    if (testLoop == 10000)
      testLoop = 0;
  }
}