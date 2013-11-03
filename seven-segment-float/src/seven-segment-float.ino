/*
4 digit 7-segment display
Digit selection controlled via Arduino digital pin, HIGH is off, LOW is on
7-segment LEDs are controlled via 74HC595 8-bit register

The display shows floats up to four digits

Copyright (c) 2013 Bryan Dagerman
*/
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(12); //the DS one-wire bus is on pin 12
DallasTemperature sensors(&oneWire);  //The temp sensor(s) on the one-wire bus

//set cathode pins, controls which digit is displayed
const int digits[] = {4, 5, 6, 7};

//set pins for 74HC595 8-bit register to control anodes, controls which segments display
const int dataPin = 8;
const int latchPin = 9;
const int clockPin = 10;

//set pin for the toggle button
const int buttonPin = 11;

//pin for the LM35 analog temp sensor
const int LM35_PIN = 0;

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
bool toggleSensor = true;
int refreshLoop = 0;  //counter for refreshing loop
int buttonState = 0;

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

void updateTemp () {
  double tempC = 0;

  if (toggleSensor)
    tempC = (500.0 * analogRead(LM35_PIN)) / 1024;
  else {
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
  }

  //double aveTempC = smoothTemp(tempC);
  double aveTempF = c2f(tempC);

  splitNumber(aveTempF);
}

double c2f (double c) {
  return ((9.0*c)/5.0 + 32.0);
}

const int numReadings = 20;
double tempReadings[numReadings];  //an array of analog readings
int smoothingIndex = 0;      //index for readings
double smoothingTotal = 0;      //the running total

double smoothTemp (double newReading) {
  smoothingTotal -= tempReadings[smoothingIndex];
  tempReadings[smoothingIndex] = newReading;
  smoothingTotal += tempReadings[smoothingIndex];
  smoothingIndex ++;

  if(smoothingIndex == numReadings)
    smoothingIndex = 0;

  return (smoothingTotal/numReadings);
}

void setup() {
  //setup  pin modes to output
  for (int i=0; i<4; i++)
    pinMode(digits[i], OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);

  Serial.begin(9600);
}

const long refreshInterval = 10; //refresh delay in microseconds
long previousRefresh = 0; //previous refresh time
const long sensorDelay = 2000;  //refresh delay for temp sensor update, in milliseconds
long prevSensorUpdate = 0;  //previous sensor update
void loop() {
  //current timer variables to reset at each iteration
  unsigned long currentRefresh = micros();  //get current microseconds for refresh counter
  unsigned long sensorUpdate = millis();
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

  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    toggleSensor = !(toggleSensor);
    updateTemp();
  }

  if(sensorUpdate - prevSensorUpdate > sensorDelay) {
    prevSensorUpdate = sensorUpdate;

    updateTemp();
  }
}