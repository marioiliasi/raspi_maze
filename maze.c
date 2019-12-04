#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>

#define TRIG_F 21
#define ECHO_F 22

#define TRIG_R 0
#define ECHO_R 2

#define MOTOR_L_F 37
#define MOTOR_L_B 36
#define MOTOR_L_E 18

#define MOTOR_R_F 40
#define MOTOR_R_B 38
#define MOTOR_R_E 13

#define FORWARD f
#define BACK b
#define LEFT l
#define RIGHT r
#define STOP s

struct Sensor {
	int trig, echo;
} sensor_f, sensor_r, sensor_l;
 
void setupSensor(struct Sensor sensor){
	pinMode(sensor.trig, OUTPUT);
	//TRIG pin must start LOW
        digitalWrite(sensor.trig, LOW);
        pinMode(sensor.echo, INPUT);
	
        delay(30);
}

void setupSensors() {
	sensor_f.trig = TRIG_F;
	sensor_f.echo = ECHO_F;
	setupSensor(sensor_f);
	sensor_r.trig = TRIG_R;
	sensor_r.echo = ECHO_R;
	setupSensor(sensor_r);
}

void setup(){
	if(wiringPiSetup() < 0){
		exit(-1);
	}
	setupSensors();
}

double measureDistance(struct Sensor sensor){
	//Send trig pulse
        digitalWrite(sensor.trig, HIGH);
        delayMicroseconds(10);
        digitalWrite(sensor.trig, LOW);
 
        //Wait for echo start
        while(digitalRead(sensor.echo) == LOW);
 
        //Wait for echo end
        long startTime = micros();
        while(digitalRead(sensor.echo) == HIGH);
        long travelTime = micros() - startTime;
 
        //Get distance in cm
        return travelTime * 1/2 * 340.29 / 10000;
}

double getDistance(struct Sensor sensor) {
	int count = 5, i;
	double sum = 0;
	for(i = 0; i < count; i++){
		sum += measureDistance(sensor);
	}
	return sum / 5;
}

struct Motor{
	int b, f, duty, enable, working;
} motor_l, motor_r;

void setupMotor(struct Motor motor, int b, int f, int enable, int maxDuty, int working);
void setupMotors();
void setDirection(char direction);
void goForward();
void goBack();
void goRight();
void goLeft();
void stopMotors();

void setupMotor(struct Motor motor, int b, int f, int enable, int maxDuty, int working){
	motor.b = b;
	motor.f = f;
	motor.duty = maxDuty;
	motor.enable = enable;
	motor.working = working;
	
	pinMode(motor.b, OUTPUT);
        pinMode(motor.f, OUTPUT);
	pinMode(motor.enable, PWM_OUTPUT);
	digitalWrite(motor.b, LOW);
	digitalWrite(motor.f, LOW);
	softPwmCreate(motor.enable, maxDuty, maxDuty);

	stopMotors();
}

void setupMotors(){
	setupMotor(motor_l, 36, 37, 18, 100, 0);
	setupMotor(motor_r, 38, 40, 13, 100, 0);
}

void setDirection(char direction){
	switch(direction){
		case 'f': 
			controlMotors(HIGH, HIGH, LOW, LOW);
		case 'r': 
			controlMotors(HIGH, LOW, LOW, HIGH);
		case 'l': 
			controlMotors(LOW, HIGH, HIGH, LOW);
		case 'b': 
			controlMotors(LOW, LOW, HIGH, HIGH);
		case 's':
			controlMotors(LOW, LOW, LOW, LOW);
	} default {
		exit -2;
	}
}

void goForward(){
	motor_l.working = 1;
	motor_r.working = 1;
	softPwmWrite(motor_l.enable, motor.duty);
	softPwmWrite(motor_r.enable, motor.duty);
	setDirection(FORWARD);
}

void goBack(){
	motor_l.working = 0;
	motor_r.working = 0;
	softPwmWrite(motor_l.enable, motor.duty);
	softPwmWrite(motor_r.enable, motor.duty);
	setDirection(BACK);
}

void goRight(){
	motor_l.working = 1;
	motor_r.working = 0;
	softPwmWrite(motor_l.enable, motor.duty);
	softPwmWrite(motor_r.enable, motor.duty);
	setDirection(RIGHT);
}

void goLeft(){
	motor_l.working = 0;
	motor_r.working = 1;
	softPwmWrite(motor_l.enable, motor.duty);
	softPwmWrite(motor_r.enable, motor.duty);
	setDirection(LEFT);
}

void stopMotors(){
	int stopped = 1;
	if(motor_l.working){
		motor_l.working = 0;
		softPwmWrite(motor_l.enable, 0);
		stopped = 0;
	}
	if(motor_r.working){
		motor_r.working = 0;
		softPwmWrite(motor_r.enable, 0);
		stopped = 0;
	if(!stopped){
		setDirection(STOP);
	}
}

void controlMotor(int lf, int rf, int lb, int rb){
	digitalWrite(motor_l.f, lf);
	digitalWrite(motor_r.f, rf);
	digitalWrite(motor_l.b, lb);
	digitalWrite(motor_r.b, rb);
}

void setDuty(int value){
	motor_r.duty = value;
	motor_l.duty = value;
	softPwmWrite(motor_r.enable, value);
	softPwmWrite(motor_l.enable, value);
}
 
int main(void) {
	
        setup();
	struct timeval now;
	gettimeofday(&now, NULL);
	double startTime = now.tv_usec;
        printf("Distance: %fcm\n", getDistance(sensor_f));
	gettimeofday(&now, NULL);
	double endTime = now.tv_usec;
	printf("Measure duration: %f\n", endTime - startTime);

 	printf("Distance: %fcm\n", getDistance(sensor_r));
        return 0;
}
