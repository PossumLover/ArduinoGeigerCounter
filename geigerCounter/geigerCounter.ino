/* 
  No radiation monitors? 
  
  ⣞⢽⢪⢣⢣⢣⢫⡺⡵⣝⡮⣗⢷⢽⢽⢽⣮⡷⡽⣜⣜⢮⢺⣜⢷⢽⢝⡽⣝
  ⠸⡸⠜⠕⠕⠁⢁⢇⢏⢽⢺⣪⡳⡝⣎⣏⢯⢞⡿⣟⣷⣳⢯⡷⣽⢽⢯⣳⣫⠇
  ⠀⠀⢀⢀⢄⢬⢪⡪⡎⣆⡈⠚⠜⠕⠇⠗⠝⢕⢯⢫⣞⣯⣿⣻⡽⣏⢗⣗⠏⠀
  ⠪⡪⡪⣪⢪⢺⢸⢢⢓⢆⢤⢀⠀⠀⠀⠀⠈⢊⢞⡾⣿⡯⣏⢮⠷⠁⠀⠀
  ⠀⠀⠈⠊⠆⡃⠕⢕⢇⢇⢇⢇⢇⢏⢎⢎⢆⢄⠀⢑⣽⣿⢝⠲⠉⠀⠀⠀⠀
  ⠀⠀⠀⠀⠀⡿⠂⠠⠀⡇⢇⠕⢈⣀⠀⠁⠡⠣⡣⡫⣂⣿⠯⢪⠰⠂⠀⠀⠀⠀
  ⠀⠀⠀⠀⡦⡙⡂⢀⢤⢣⠣⡈⣾⡃⠠⠄⠀⡄⢱⣌⣶⢏⢊⠂⠀⠀⠀⠀⠀⠀
  ⠀⠀⠀⠀⢝⡲⣜⡮⡏⢎⢌⢂⠙⠢⠐⢀⢘⢵⣽⣿⡿⠁⠁⠀⠀⠀⠀⠀⠀⠀
  ⠀⠀⠀⠀⠨⣺⡺⡕⡕⡱⡑⡆⡕⡅⡕⡜⡼⢽⡻⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
  ⠀⠀⠀⠀⣼⣳⣫⣾⣵⣗⡵⡱⡡⢣⢑⢕⢜⢕⡝⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
  ⠀⠀⠀⣴⣿⣾⣿⣿⣿⡿⡽⡑⢌⠪⡢⡣⣣⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
  ⠀⠀⠀⡟⡾⣿⢿⢿⢵⣽⣾⣼⣘⢸⢸⣞⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
  ⠀⠀⠀⠀⠁⠇⠡⠩⡫⢿⣝⡻⡮⣒⢽⠋

  Budget Geiger Counter so I don't get leukemia. 

  Interacts with the RadiationD-v1.1 (CAJOE). Wanted a straight responce from the VIN output without other items to
  serial, using an Arduino Uno.
  Displays the Counts per Minute (CPM) and micro-sieverts per hour. The conversion is based on a M4011 tube. 

  Uses a rolling weigted average filter with a window of 10 to reduce random spikes. This does have the effect
  on responce time however for the digital output, but it remains indepentent of the buzzer output. 

  Outputs total clicks and time since program start to LCD screen to allow for easy calculation of average clicks per minute.

  Author: Sebastian D'Hyon
  E-mail: sebyon2@gmail.com
  License: MIT License 

  Code edits done by James Warner :)
  
  https://github.com/PossumLover/ArduinoGeigerCounter

*/

#define Version "V1.0.0"

#include <LiquidCrystal.h>

// Pin definitions and variables
const int geigerPin = 2; // Pin number for the Geiger counter
volatile int count = 0; // Variable to keep track of the count
unsigned long previousMillis = 0; // Variable to keep track of the previous time
unsigned long totalcount = 0; // Variable to keep track of the total count
unsigned long programStartTime = 0; // Variable to store the program start time
int cpm = 0; // Variable to keep track of the counts per minute
float microsieverts = 0.0; // Variable to keep track of the microsieverts per hour

// Uses a rolling weigted average filter with a window of 10 to reduce random background spikes
const int windowSize = 10;
float weights[windowSize] = {0.0333, 0.0667, 0.1, 0.1333, 0.1667, 0.2, 0.1667, 0.1333, 0.1, 0.0667}; // Weights for the moving average filter
float weightedCpm[windowSize]; // Array to store previous CPM values for the moving average filter
int currentIndex = 0; // Index of the current value in the moving average filter

LiquidCrystal lcd(9, 8, 7, 6, 5, 3);

void setup() {
  pinMode(geigerPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(geigerPin), pulse, FALLING);

  Serial.begin(9600);
  programStartTime = millis(); // Store the program start time

  lcd.begin(16, 2);
}

// Interrupt service routine for the Geiger counter
void loop() {
  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - programStartTime; // Calculate elapsed time

  if (currentMillis - previousMillis >= 1000) { // Every second
    float cps = (float)count / ((currentMillis - previousMillis) / 1000.0); // Calculate counts per second
    cpm = (int)(cps * 60.0); // Convert counts per second to counts per minute

    // Add current CPM to weighted moving average
    weightedCpm[currentIndex] = cpm;
    currentIndex = (currentIndex + 1) % windowSize;

    // Calculate weighted moving average of CPM
    float weightedSum = 0.0;
    float weightSum = 0.0;
    for (int i = 0; i < windowSize; i++) {
      int index = (currentIndex + i) % windowSize;
      weightedSum += weightedCpm[index] * weights[i];
      weightSum += weights[i];
    }
    float weightedAverageCpm = weightedSum / weightSum;

    totalcount += count;

    microsieverts = weightedAverageCpm / 151.0; // Convert CPM to microsieverts per hour, conversion factor for this geiger tube is 151 clicks = 1 uSv/H

    // Print results via serial
    Serial.print("Time: ");
    Serial.print(elapsedTime / 1000); // Convert milliseconds to seconds
    Serial.print(", Clicks: ");
    Serial.print(totalcount);
    Serial.print(", Change: ");
    Serial.print(count);
    Serial.print(", CPM: ");
    Serial.print((int)weightedAverageCpm);
    Serial.print(", uSv/H: ");
    Serial.println(microsieverts);

    // Update LCD
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(elapsedTime / 1000);
    lcd.print("s");

    lcd.setCursor(0, 1);
    lcd.print("Clicks: ");
    lcd.print(totalcount);

    count = 0; // Reset count
    previousMillis = currentMillis; // Update previous time
  }
}

void pulse() {
  count++; // Increment count on each pulse
}
