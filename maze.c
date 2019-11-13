#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define TRIG_F 21
#define ECHO_F 22
#define TRIG_R 23
#define ECHO_R 24

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
 
int main(void) {
	
        setup();

        printf("Distance: %fcm\n", getDistance(sensor_f));
 	printf("Distance: %fcm\n", getDistance(sensor_r));
	
        return 0;
}
