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


#define MOTOR_L_F 27
#define MOTOR_L_B 25
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
	//softPwmCreate(motor.enable, 0, maxDuty);
	digitalWrite(motor.b, LOW);
	digitalWrite(motor.f, LOW);
	
	printf("Motor pins - %d %d\n", motor.f, motor.b);
	
	delay(30);
}

void setupMotors(){
	setupMotor(motor_l, MOTOR_L_B, MOTOR_L_F, MOTOR_L_E, 100, 0);
	setupMotor(motor_r, MOTOR_R_B, MOTOR_R_F, MOTOR_R_E, 100, 0);
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
			//softPwmWrite(motor_r.enable, LOW);
			//softPwmWrite(motor_l.enable, LOW);
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
		//softPwmWrite(motor_l.enable, LOW);
		stopped = 0;
	}
	if(motor_r.working){
		motor_r.working = 0;
		//softPwmWrite(motor_r.enable, LOW);
		stopped = 0;
	}
	if(!stopped){
		setDirection(STOP);
	}
}

void controlMotors(int lf, int rf, int lb, int rb){
	//softPwmWrite(motor_l.enable, motor_l.duty);
	//softPwmWrite(motor_r.enable, motor_r.duty);
	digitalWrite(motor_l.f, lf);
	digitalWrite(motor_r.f, rf);
	digitalWrite(motor_l.b, lb);
	digitalWrite(motor_r.b, rb);
	printf("%d %d %d %d\n", lf, rf, lb, rb);
	//printf("duty - %d\n", motor_l.duty);
	
}

/*void setDuty(int value){
	motor_r.duty = value;
	motor_l.duty = value;
	softPwmWrite(motor_r.enable, value);
	softPwmWrite(motor_l.enable, value);
}*/
 
void setup(){
	if(wiringPiSetup() < 0){
		exit(-1);
	}
	
	setupSensors();
	//setupMotors();
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
	FILE *filed;

	filed = fopen("measurements.csv", "w");
	
	if (filed == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	double f, l, r;
        struct timeval now;
	gettimeofday(&now, NULL);
	double startTime = now.tv_usec; 
	
	double currentPosition[] = {0, 0};
	//double oldValues[] = {0, 0};
	int xDir = 0;
	int yDir = 1;
	int oldXDir = 0;
	int oldYDir = 1;
	int oldDistance = 0;
	int ld, rd, fd;
	while(1){
		f = getAverageDistance(sensor_f);
		l = getAverageDistance(sensor_l);
		r = getAverageDistance(sensor_r);
		
		
		if(xDir != 0){
			if(oldDistance != 0){
				currentPosition[0] += xDir * (f - oldDistance);
			}
			ld = currentPosition[1] + xDir * l;
			rd = currentPosition[1] - xDir * r;
			fd = currentPosition[0] + xDir * f;
			
		}
		if(yDir != 0){
			if(oldDistance != 0){
				currentPosition[1] += yDir * (f - oldDistance);
			}
			ld = currentPosition[0] - yDir * l;
			rd = currentPosition[0] + yDir * r;
			fd = currentPosition[1] + yDir * f;
		}
		//fprintf(filed, "%f, %f, %f\n", l, f, r);
		printf("distance:%f, %f, %f\n", l, f, r);
		//printf("walls:%f, %f, %f\n", f, l, r);
		fprintf(filed, "%f, %f, %f\n", ld, fd, rd);

		if(f < 10 && l < 10 && r < 10){//turn around
			usleep(2000);
			if(xDir != 0){
				xDir = -xDir;
			} else{
				yDir = -yDir;
			}
		} else if(f < 10){//wall in front
			usleep(2000);
			if(r > 10){//turn right
				if(xDir != 0){
					xDir = 0;
					yDir = -xDir;
				} else{
					yDir = 0;
					xDir = yDir;
				}
			} else if(l > 10){//turn left
				if(xDir != 0){
					xDir = 0;
					yDir = xDir;
				} else{
					yDir = 0;
					xDir = -yDir;
				}
			}
		}
		
		oldDistance = f;

		if(oldXDir != xDir){
			oldXDir = xDir;
		}
	        if(oldYDir != yDir){
			oldYDir = yDir;
		}

		gettimeofday(&now, NULL);
		if(f > 100 && l > 100 && r > 100 || now.tv_usec - startTime > 1000000 * 10){
			break;
		}
		
		usleep(100);
	}
	fclose(filed);
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
	scan();
	/*pinMode(MOTOR_L_F, OUTPUT);
	pinMode(MOTOR_L_B, OUTPUT);
	softPwmCreate(MOTOR_L_E, 100, 100);
	digitalWrite(MOTOR_L_F, HIGH);
	digitalWrite(MOTOR_L_F, 0);
	pwmWrite(MOTOR_L_E, 100);*/
	printf("Front Distance: %fcm\n", getAverageDistance(sensor_f));
 	printf("Right Distance: %fcm\n", getAverageDistance(sensor_r));
	printf("Left Distance: %fcm\n", getAverageDistance(sensor_l));
	
    return 0;
}
