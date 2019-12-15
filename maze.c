#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>
#include <softPwm.h>
#include <unistd.h>

#define TRIG_F 21
#define ECHO_F 22

#define TRIG_R 24
#define ECHO_R 26

#define TRIG_L 0
#define ECHO_L 3


#define MOTOR_L_F 25
#define MOTOR_L_B 27
#define MOTOR_L_E 5

#define MOTOR_R_F 29
#define MOTOR_R_B 28
#define MOTOR_R_E 2

#define FORWARD 'f'
#define BACK 'b'
#define LEFT 'l'
#define RIGHT 'r'
#define STOP 's'

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
	sensor_l.trig = TRIG_L;
	sensor_l.echo = ECHO_L;
	setupSensor(sensor_l);
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
void controlMotors(int lf, int rf, int lb, int rb);
void setDuty(int value);
double getAverageDistance(struct Sensor sensor);

void setupMotor(struct Motor motor, int b, int f, int enable, int maxDuty, int working){
	motor.b = b;
	motor.f = f;
	motor.duty = maxDuty;
	motor.enable = enable;
	motor.working = working;
	
	pinMode(motor.b, OUTPUT);
    pinMode(motor.f, OUTPUT);
	pinMode(motor.enable, PWM_OUTPUT);
	softPwmCreate(motor.enable, 0, maxDuty);
	digitalWrite(motor.b, LOW);
	digitalWrite(motor.f, LOW);
	
	delay(30);
	stopMotors();
}

void setupMotors(){
	setupMotor(motor_l, 36, 37, 18, 100, 0);
	setupMotor(motor_r, 38, 40, 13, 100, 0);
}

void setDirection(char direction){
	printf("Direction: %c\n", direction);
	switch(direction){
		case 'f': 
			controlMotors(HIGH, HIGH, LOW, LOW);
			break;
		case 'r': 
			controlMotors(HIGH, LOW, LOW, HIGH);
			break;
		case 'l': 
			controlMotors(LOW, HIGH, HIGH, LOW);
			break;
		case 'b': 
			controlMotors(LOW, LOW, HIGH, HIGH);
			break;
		case 's':
			controlMotors(LOW, LOW, LOW, LOW);
			softPwmWrite(motor_r.enable, LOW);
			softPwmWrite(motor_l.enable, LOW);
			break;
	default :
		exit(-2);
	}
}

void goForward(){
	motor_l.working = 1;
	motor_r.working = 1;
	setDirection(FORWARD);
}

void goBack(){
	motor_l.working = 0;
	motor_r.working = 0;
	setDirection(BACK);
}

void goRight(){
	motor_l.working = 1;
	motor_r.working = 0;
	setDirection(RIGHT);
}

void goLeft(){
	motor_l.working = 0;
	motor_r.working = 1;
	setDirection(LEFT);
}

void stopMotors(){
	int stopped = 1;
	if(motor_l.working){
		motor_l.working = 0;
		softPwmWrite(motor_l.enable, LOW);
		stopped = 0;
	}
	if(motor_r.working){
		motor_r.working = 0;
		softPwmWrite(motor_r.enable, LOW);
		stopped = 0;
	}
	if(!stopped){
		setDirection(STOP);
	}
}

void controlMotors(int lf, int rf, int lb, int rb){
	digitalWrite(motor_l.f, lf);
	digitalWrite(motor_r.f, rf);
	digitalWrite(motor_l.b, lb);
	digitalWrite(motor_r.b, rb);
	printf("duty - %d\n", motor_l.duty);
	softPwmWrite(motor_l.enable, motor_l.duty);
	softPwmWrite(motor_r.enable, motor_r.duty);
}

void setDuty(int value){
	motor_r.duty = value;
	motor_l.duty = value;
	softPwmWrite(motor_r.enable, value);
	softPwmWrite(motor_l.enable, value);
}
 
void setup(){
	if(wiringPiSetup() < 0){
		exit(-1);
	}
	
	setupSensors();
	setupMotors();
}

double getAverageDistance(struct Sensor sensor){
	int i, cnt = 0;
	int threshold = 15;
	int measurementsCnt = 3;
	int average = 0;
	int measurements[10];
	int allAverage = 0;
	for(i = 0; i < measurementsCnt; i++){
		measurements[i] = measureDistance(sensor);
		if(measurements[i] < threshold){
			average += measurements[i];
			cnt ++;
		}
		allAverage += measurements[i];
	}
	
	if(cnt > 0){
		return average / cnt;
	}
	return allAverage/measurementsCnt;
}

void scan(){
	FILE *fd;

	fd = fopen("measurements.csv", "w");
	
	if (fd == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	double f, l, r;
	while(1){
		f = getAverageDistance(sensor_f);
		l = getAverageDistance(sensor_l);
		r = getAverageDistance(sensor_r);
		if(f > 50 && l > 50 && r > 50){
			break;
		}
		fprintf(fd, "%f, %f, %f\n", f, l, r);
		printf("%f, %f, %f\n", f, l, r);
		usleep(100);
	}
	fclose(fd);
}

int main(void) {
	
        setup();
	/*struct timeval now;
	gettimeofday(&now, NULL);
	double startTime = now.tv_usec; 
        printf("Distance: %fcm\n", getDistance(sensor_f));
	gettimeofday(&now, NULL);
	double endTime = now.tv_usec;
	printf("Measure duration: %f\n", endTime - startTime);*/
	setDuty(100);
	goForward();
	scan();
	printf("Front Distance: %fcm\n", getAverageDistance(sensor_f));
 	printf("Right Distance: %fcm\n", getAverageDistance(sensor_r));
	printf("Left Distance: %fcm\n", getAverageDistance(sensor_l));
        return 0;
}
