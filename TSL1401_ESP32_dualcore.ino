//clock pin D23, clkP
//si pin D27, siP
//Anolog input A36, anIn
#define F_CPU 240000000UL
#define L_ARRAY 128
#define L_ARRAY_1 129
#include <Arduino_FreeRTOS.h>
#include <Arduino.h>
#include <avr.io>

//pin declaration
const int clkP = 23;
const int siP = 27;
const int anIn = 36;

//global variables
uint8_t CLKcycleCounter = 0; // counts the number of clock cycles expired
volatile uint8_t pixels[L_ARRAY] = {0}; // pixel buffer - holds data from ADC
uint8_t i, send_pixels[L_ARRAY_1] = {0}; // i is an index for loops, m is the mimimum pixel value
bool endF = 0; //end of frame indicator
unsigned int maxIndex = 0; //max index value for a frame
int FmaxIndex = 0; //Final max index retained for serial transmission

//Task handles declaration
Taskhandle_t acquisition;
Taskhandle_t treatement;

void setup() {
  Serial.begin(2000000);
  pinMode(clkP, OUTPUT);
  pinMode(siP, OUTPUT);
  pinMode(anIn, INPUT);
  digitalWrite(clkP, LOW);//clk = 0V
  digitalWrite(siP, LOW);//SI = 0V

  //tasks core assignment and set up
  xTaskCreatePinnedToCore(acquisition, acquisition, 10000, NULL, 1, &acquisition, 0);
  xTaskCreatePinnedToCore(treatement, treatement, 10000, NULL, 1, &treatement, 1);

  //sarts first frame
  digitalWrite(siP, HIGH);//SI = 3.3V
  digitalWrite(clkP, HIGH);//clk = 3.3V
  digitalWrite(siP, LOW);//SI = 0V
  digitalWrite(clkP, LOW);//clk = 0V
}

//Core 0
void acquisition() { //acquires pixels values and output an array
  if (CLKcycleCounter < L_ARRAY) {
    digitalWrite(clkP, HIGH);//clk = 3.3V
    digitalWrite(clkP, LOW);//clk = 0V
    pixels[CLKcycleCounter] = analogRead(anIn);//gets value from ADC
    CLKcycleCounter++;//iterates clock counter
  }
  else if (CLKcycleCounter == L_ARRAY | CLKcycleCounter == L_ARRAY_1) {
    digitalWrite(clkP, HIGH);//clk = 3.3V
    digitalWrite(clkP, LOW);//clk = 0V
    CLKcycleCounter++;//iterates clock counter
    endF = 1;
  }
  else { //start next frame by sending a pulse on siP
    digitalWrite(siP, HIGH);//SI = 3.3V
    digitalWrite(clkP, HIGH);//clk = 3.3V
    digitalWrite(siP, LOW);//SI = 0V
    digitalWrite(clkP, LOW);//clk = 0V
    CLKcycleCounter = 0;
  }
  if (endF == 1) { // copy volatile array into non-volatile array
    for (i = 0; i < L_ARRAY; ++i) { //scans through the array
      send_pixels[i] = pixels[i];
    }
    endF = 0; //resets end of frame flag
  }
}

//Core 1
void treatment(pixels[]) { //finds maximum value in array and output it's index
  unsigned int maxP = pixels[0];//temporarily stores the maximum value
  for (int i = 1; i < L_ARRAY; i++) {
    if (maxP < pixels[i]) {
      maxP = pixels[i];
      maxIndex = i;
    }
  }
  if (maxP < 220) {
    Serial.write(-1);
  }
  else {
    Serial.write(maxIndex);
  }
}
