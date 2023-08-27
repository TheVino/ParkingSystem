 /*
This is the code for a garage parking sensor. It uses a LED bar graph and a HC-SR04 ultrasonic distance sensor to tell you when to stop to get in the perfect position.
This code was made by Jadon Miller. You can use it in any way you want, including commercial uses. If you have any questions, e-mail me at JadonMMiller@Gmail.com

Components:
1  HC-SR04 Ultrasonic Distance Sensor
8  Green LEDs
4  Red LEDs
12 220 Ohm Resistors
*/

#include <NewPing.h> //include the library for the sonar distance sensor
#include <EEPROM.h> //this lets us access the memory

//set the led pins by the color
const int G1 = 5;	// D1
const int G2 = 4;	// D2
const int Y1 = 0;	// D3
const int Y2 = 2;	// D4
const int R1 = 14;	// D5
const int R2 = 12;	// D6
const int trigPin = A0; //set the distance sensor trigger pin to A0
const int echoPin = A1; //the sonar echo pin
const int buttonPin = A2; //the button

NewPing sonar(trigPin, echoPin, 300); //start the distance sensor

const int minDeviation = 5; //the number of inches of movement required to count as moving
const int timeout = 10; //the number of seconds to keep the leds on after movement stops
const int ledDistance = 6; //the distance in inches each led covers
const int calibrationDelay = 750; //this is the time each led will stay on during the calibration countdown

int idealDistance = 5; //this is the perfect distance to the car read from the memory
int rawSonar; //this is the raw data from the distance sensor
int distance; //the distance to the vehicle
int prevDistance = 0; //the previous distance to the vehicle
unsigned long lastMoved = 0; //the millis reading of the last movement


void setup() { //this runs once at startup
	pinMode(G1, OUTPUT); //set the leds to output
	pinMode(G2, OUTPUT);
	pinMode(Y1, OUTPUT);
	pinMode(Y2, OUTPUT);
	pinMode(R1, OUTPUT);
	pinMode(R2, OUTPUT);
	pinMode(buttonPin, INPUT_PULLUP); //set button as input with internal pull-up resitors

	if (EEPROM.read(0) != 255) { //read the memory, it will read 255 if it's never been written to
		idealDistance = EEPROM.read(0); //set the idealDistance from the memory
	}

	Serial.begin(9600);
}

void loop() { //this runs over and over again
	delay(100); //wait 100 ms to get accurate readings


	rawSonar = sonar.ping_median(5); //get 5 readings and average them
	if (rawSonar != 0) { //the distance sensor sends a 0 when nothing is in range
		distance = rawSonar / US_ROUNDTRIP_IN; //convert the raw data the inches
		distance -= idealDistance; //subtract the ideal distance from the reading, giving us the distance to go
	}
	else {//if the car isn't in range
		setLEDs(0, 0, 0, 0, 0, 0); //turn the LEDs off
	}


	if (abs(distance - prevDistance) >= minDeviation) { //if the car has moved since last read
		lastMoved = 0; //reset the sleep timer

		prevDistance = distance; //reset the distance last read

		if (distance < ledDistance * 7 && distance >= ledDistance * 6) { //if in range of the 3rd led
			setLEDs(1, 0, 0, 0, 0, 0);
		}
		else if (distance < ledDistance * 6 && distance >= ledDistance * 5) { //if in range of the 4th led
			setLEDs(1, 1, 0, 0, 0, 0);
		}
		else if (distance < ledDistance * 5 && distance >= ledDistance * 4) { //if in the 5th led
			setLEDs(1, 1, 1, 0, 0, 0);
		}
		else if (distance < ledDistance * 4 && distance >= ledDistance * 3) { //if in range of the 6th led
			setLEDs(1, 1, 1, 1, 0, 0);
		}
		else if (distance < ledDistance * 3 && distance >= ledDistance * 2) { //if in range of the 7th led
			setLEDs(1, 1, 1, 1, 1, 0);
		}
		else if (distance < ledDistance * 2 && distance >= ledDistance) { //if in range of the 8th led
			setLEDs(1, 1, 1, 1, 1, 1);
		}
		else if (distance <= idealDistance) { //if in the stop position
			setLEDs(0, 0, 1, 1, 0, 0);
		}
	}
	else { //if the car isn't moving
		if (lastMoved == 0) { //if the timer hasn't been started
			lastMoved = millis(); //set the timer
		}

		if (lastMoved != 0 && millis() - lastMoved >= timeout * 1000) { //if the timer is set and past the timeout
			setLEDs(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); //turn off the leds
			lastMoved = 0; //turn off the timer
		}
	}

	//this checks the button
	if (digitalRead(buttonPin) == LOW) { //if the button is being pressed
		calibrationCountdown(); //do the LED countdown

		rawSonar = sonar.ping_median(30); //get 30 readings and average them

		if (rawSonar != 0) { //the distance sensor sends a 0 when nothing is in range
			distance = rawSonar / US_ROUNDTRIP_IN; //convert the raw data the inches

			EEPROM.write(0, distance); //write the distance to the memory
			idealDistance = distance; //set the idealDistance

			flashGreen(); //show success
		}
		else { //if out of range
			flashRed(); //show error
		}
	}
}


void calibrationCountdown() {
	setLEDs(0, 0, 0, 0, 0, 0); //turn off all the leds
	setLEDs(1, 0, 0, 0, 0, 0); //turn on the first led
	delay(calibrationDelay); //pause for a bit
	setLEDs(1, 1, 0, 0, 0, 0); //continue the pattern...
	delay(calibrationDelay); //pause for a bit
	setLEDs(1, 1, 1, 0, 0, 0); //continue the pattern...
	delay(calibrationDelay); //pause for a bit
	setLEDs(1, 1, 1, 1, 0, 0); //continue the pattern...
	delay(calibrationDelay); //pause for a bit
	setLEDs(1, 1, 1, 1, 1, 0); //continue the pattern...
	delay(calibrationDelay); //pause for a bit
	setLEDs(1, 1, 1, 1, 1, 1); //continue the pattern...
	delay(calibrationDelay); //pause for a bit
}


void flashGreen() { //this flashes the green LEDs to show success
	setLEDs(0, 0, 0, 0, 0, 0); //clear the leds
	setLEDs(1, 1, 0, 0, 0, 0); //turn the green leds on
	delay(500); //wait
	setLEDs(0, 0, 0, 0, 0, 0); //clear the leds
	delay(500); //wait
	setLEDs(1, 1, 0, 0, 0, 0); //turn the green leds on
	delay(500); //wait
	setLEDs(0, 0, 0, 0, 0, 0); //clear the leds
}

void flashRed() { //this flashes the red LEDs to show success
	setLEDs(0, 0, 0, 0, 0, 0); //clear the leds
	setLEDs(0, 0, 0, 0, 1, 1); //turn the red leds on
	delay(500); //wait
	setLEDs(0, 0, 0, 0, 0, 0); //clear the leds
	delay(500); //wait
	setLEDs(0, 0, 0, 0, 1, 1); //turn the red leds on
	delay(500); //wait
	setLEDs(0, 0, 0, 0, 0, 0); //clear the leds
}


//this function simply sets the leds to the parameters you send it
void setLEDs(int led1, int led2, int led3, int led4, int led5, int led6) {
	digitalWrite(G1, led1);
	digitalWrite(G2, led2);
	digitalWrite(Y1, led3);
	digitalWrite(Y2, led4);
	digitalWrite(R1, led5);
	digitalWrite(R2, led6);
}
