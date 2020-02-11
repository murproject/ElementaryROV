
#include <Arduino.h>
#include "pwm.h"

SOFTPWM_DEFINE_CHANNEL(8, DDRB, PORTB, PORTB0);  //Arduino pin 8
SOFTPWM_DEFINE_CHANNEL(9, DDRB, PORTB, PORTB1);  //Arduino pin 9
SOFTPWM_DEFINE_CHANNEL(10, DDRB, PORTB, PORTB2);  //Arduino pin 10
SOFTPWM_DEFINE_CHANNEL(11, DDRB, PORTB, PORTB3);  //Arduino pin 11
SOFTPWM_DEFINE_CHANNEL(12, DDRB, PORTB, PORTB4);  //Arduino pin 12
SOFTPWM_DEFINE_CHANNEL(13, DDRB, PORTB, PORTB5);  //Arduino pin 13

SOFTPWM_DEFINE_OBJECT_WITH_PWM_LEVELS(20, 101);
SOFTPWM_DEFINE_EXTERN_OBJECT_WITH_PWM_LEVELS(20, 101);

int sign(int v) {
	if (v >= 0) return 1;
	if (v < 0) return -1;
}

const int X_OFFSET = 512;
const int Y_OFFSET = 502;
const int LOW_TRESHOLD = 5;
const int TOP_TRESHOLD = 500;

const int AXISX = A0;
const int AXISY = A1;
const int AXISZ_UP = 2;
const int AXISZ_DOWN = 4;


typedef struct Thruster_type {
	int pin1;
	int pin2;

	int initThruster(int pin1_, int pin2_) {
		pin1 = pin1_;
		pin2 = pin2_;
		Palatis::SoftPWM.set(pin1, 0);
		Palatis::SoftPWM.set(pin1, 0);
		return 0;
	}

	void setPower(int power) {
		if (power >= 0) {
			Palatis::SoftPWM.set(pin1, power);
			Palatis::SoftPWM.set(pin2, 0);
		}
		else {
			Palatis::SoftPWM.set(pin1, 0);
			Palatis::SoftPWM.set(pin2, -power);
		}
	}
} Thrusters;

Thrusters leftMotor;
Thrusters rightMotor;
Thrusters verticalMotor;

int getPowerDelimetr() {
	int delimetr = 1.2;
	if (!digitalRead(3)) delimetr = 2;
	if (!digitalRead(5)) delimetr = 1;
	return delimetr;
}


int getAxisVal(int pin, int offset) {
	int axisRaw = analogRead(pin);
	int axis = axisRaw - offset;
	if (abs(axis) < LOW_TRESHOLD) axis = 0;
	if (abs(axis) > TOP_TRESHOLD) axis = TOP_TRESHOLD * sign(axis);
	return axis / (TOP_TRESHOLD / 100);
}

int getAxisZVal() {
	int axis = 0;
	if (!digitalRead(AXISZ_DOWN)) axis = -100;
	if (!digitalRead(AXISZ_UP)) axis = 100;
	return axis;
}

const int MAXPOWER = 100;

int regulator(int pow) {
	return constrain(pow, -MAXPOWER / getPowerDelimetr(), MAXPOWER / getPowerDelimetr());
}

int vregulator(int pow) {
	static long long time = 0;
	static int signPower = 0;
	int power = 0;
	if (pow == 0) {
		time = 0;
		return 0;
	}

	time = (time == 0) || sign(signPower) != sign(pow) 
		? millis() : time;

	if (pow > 0) {
		signPower = 1;
		power = 20;
		power += ((millis() - time) / 200);
	} 
	else {
		signPower = -1;
		power = -20;
		power -= ((millis() - time) / 200);
	}
	
	return constrain(power, -MAXPOWER / getPowerDelimetr(), MAXPOWER / getPowerDelimetr());
}

#define RELEASE



void setup() {
#ifndef RELEASE
	
#endif

	Serial.begin(115200);

	
	Palatis::SoftPWM.begin(1000);

	Palatis::SoftPWM.printInterruptLoad();
	pinMode(AXISX, INPUT);
	pinMode(AXISY, INPUT);
	pinMode(AXISZ_UP, INPUT);
	pinMode(AXISZ_DOWN, INPUT);
	leftMotor.initThruster(8, 9);
	rightMotor.initThruster(10, 11);
	verticalMotor.initThruster(12, 13);
}



void loop() {
	int axisx = getAxisVal(AXISX, X_OFFSET);
	int axisy = getAxisVal(AXISY, Y_OFFSET);
	int axisz = getAxisZVal();
#ifdef RELEASE
	leftMotor.setPower(regulator(axisy - axisx));
	rightMotor.setPower(regulator(axisy + axisx));
	verticalMotor.setPower(regulator(axisz));
#else
	Serial.print("LEFT: ");
	Serial.print(regulator(axisy - axisx));
	Serial.print(" RIGHT: ");
	Serial.print(regulator(axisy + axisx));
	Serial.print(" VERTICAL: ");
	Serial.println(regulator(axisz));
	if (digitalRead(3) && digitalRead(5)) {
		leftMotor.setPower(regulator(axisy - axisx));
		rightMotor.setPower(regulator(axisy + axisx));
		verticalMotor.setPower(regulator(axisz));
	} else {
		if (!digitalRead(3)) {
			leftMotor.setPower(regulator(100));
			rightMotor.setPower(regulator(100));
			verticalMotor.setPower(regulator(100));
		}
		if (!digitalRead(5)) {
			leftMotor.setPower(regulator(-100));
			rightMotor.setPower(regulator(-100));
			verticalMotor.setPower(regulator(-100));
		}
	}
	delay(100);
#endif

}
