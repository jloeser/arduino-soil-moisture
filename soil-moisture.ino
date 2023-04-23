#include <TFT.h>
#include <SPI.h>
#include <Arduino.h>

#define UPSTREAMURL "<set-me>"

#define cs   10
#define dc   9
#define rst  8

/*
Calibration:
------------
Glas water (wet): 200
Nothing (dry)   : 522
Range           : 322 (322 / 3 = 107)
*/


#define DATAPOINTS 15
#define XOFFSET 11
#define XDISTANCE 10
#define YOFFSET 10
#define YRANGE 108

#define VALUEOFFSET 200

#define DELAYMS 1000    // 1s
#define DISPLAYOFF 10   // 10s

// Pins D20 (A6) and D21 (A7) do not work here for whatever reason!
const int btnPin = 19;
const int displayLedPin = 2;

int displayTimer = 0;

int dataRecordHours[DATAPOINTS] = { 0 };
int dataRecordOneHour[6] = { 0 };
int secondsPast = 0;

int measured;

TFT display = TFT(cs, dc, rst);

void printHours() {
  for (int i=0; i < DATAPOINTS; i++) {
    Serial.print(i, DEC);
    Serial.print(": ");
    Serial.print(dataRecordHours[i]);
    Serial.println();
  }
}

void eraseGraph() {
  display.stroke(0, 0, 0);
  for (int a = 0; a < (DATAPOINTS-1); a++) {
    display.line(XOFFSET+XDISTANCE*a, YOFFSET+YRANGE-(dataRecordHours[a]), XOFFSET+XDISTANCE*(a+1), YOFFSET+YRANGE-(dataRecordHours[a+1]));
  }
}

void drawGraph() {
  for (int a = 0; a < (DATAPOINTS-1); a++) {
    display.stroke(100,20,35);
    display.line(XOFFSET+XDISTANCE*a, YOFFSET+YRANGE-(dataRecordHours[a]), XOFFSET+XDISTANCE*(a+1), YOFFSET+YRANGE-(dataRecordHours[a+1]));
  }
}

void setup() {
  pinMode(displayLedPin, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP);

  display.begin();
  Serial.begin(9600);

  display.background(0, 0, 0);
  display.stroke(255, 255, 255);
  display.setTextSize(1);
  display.text("Bodenfeuchtigkeit", 2, 2);

  display.line(XOFFSET-1, YOFFSET+YRANGE+1, XOFFSET+DATAPOINTS*10, YOFFSET+YRANGE+1);    // x axis
  display.line(XOFFSET-1, YOFFSET, XOFFSET-1, YOFFSET+YRANGE+1);      // y axis

  display.text("t", 150, 120);
  display.text("feucht", 2, 120);
  display.text("t", 2, 15);
  display.text("r", 2, 23);
  display.text("o", 2, 31);
  display.text("c", 2, 39);
  display.text("k", 2, 47);
  display.text("e", 2, 55);
  display.text("n", 2, 63);

  display.stroke(100,20,35);
  display.fill(100,20,35);
  display.circle(50,124, 2);
  display.stroke(255, 255, 255);

  display.text("S1", 55, 120);

  Serial.println( "--- Compiled: " __DATE__ ", " __TIME__ ", " __VERSION__ ", " UPSTREAMURL);

  drawGraph();
}

void loop() {
  // displayTimer = 1; // display always on

  if(displayTimer <= 0) {
    digitalWrite(displayLedPin, LOW);
  }else{
    digitalWrite(displayLedPin, HIGH);
    displayTimer--;
  }
  if(digitalRead(btnPin) == LOW)
  {
    displayTimer = DISPLAYOFF;
    Serial.println("Button pressed");
  }

  measured = int(analogRead(A0));
  measured = int(random(201,522));

  delay(DELAYMS);

  // Data recording:
  //  - 6 values every 10min
  //  - average of these 6 value is saved
  //  - last 15 hours are saved
  secondsPast++;

  if (secondsPast % 600 == 0)   // 10m
  {
    // Scale measured value (see "Calibration")
    measured = measured - VALUEOFFSET;
    if(measured < 0){
      measured = 0;
    }
    measured = measured / 3;

    dataRecordOneHour[(secondsPast/600)-1] = measured;
    Serial.print("Value (10min): ");
    Serial.println(measured, DEC);
  }

  if (secondsPast == 3600)      // 1h
  {
    eraseGraph();

    // Shift values first
    memcpy(dataRecordHours, &dataRecordHours[1], sizeof(dataRecordHours) - sizeof(int));

    int avg = 0;
    for (int i=0; i < 6; i++) {
      avg = avg + dataRecordOneHour[i];
    }
    dataRecordHours[DATAPOINTS - 1] = int(avg / 6);

    Serial.print("Average (1h): ");
    Serial.println(dataRecordHours[DATAPOINTS - 1]);
    printHours();

    drawGraph();

    secondsPast = 0;
  }
}
