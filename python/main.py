from Sensor import Sensor
from EngineController import EngineController
import curses
import random
import time
import matplotlib.pyplot as plt


def carControl():

    readWindow = 200
    lastReadTime = 0
    currentPosition = [0, 0]
    
    screen = curses.initscr()
    curses.noecho()
    curses.cbreak()
    screen.keypad(True)
    
    try:
        while True:
            char = screen.getch()
        
            if char == ord('q'):
                break
            elif char == ord('x'):
                print ('\n', frontSensor.getDist(), " cm\n")
            elif char == ord('z'):
                print ('\n', leftSensor.getDist(), " cm\n")
            elif char == ord('c'):
                print ('\n', rightSensor.getDist(), " cm\n")
            elif char == ord(','):
                if controller.duty > 50:
                    print ('\n', controller.duty, "\n")
                    controller.setDuty(controller.duty - 10)
                else:
                    print ("\nduty minim\n")
            elif char == ord('.'):
                if controller.duty < 100:
                    print ('\n', controller.duty, "\n")
                    controller.setDuty(controller.duty + 10)
                else:
                    print ("\nduty maxim\n")
            
            elif char == ord('p'):
                print ("\nPAUSE\n")
                controller.stop()
            elif char == ord('w') or char == curses.KEY_UP:
                print ("F")
                controller.goForward()
            elif char == ord('s') or char == curses.KEY_DOWN:
                print ("B")
                controller.goBack()
            elif char == ord('a') or char == curses.KEY_LEFT:
                print ("L")
                controller.goLeft()
            elif char == ord('d') or char == curses.KEY_RIGHT:
                print ("R")
                controller.goRight()

            
    finally:
        frontSensor.clear()
        rightSensor.clear()
        leftSensor.clear()
        controller.clear()
        '''curses.noecho()
        curses.cbreak()
        screen.keypad(False)
        plt.show()'''

frontSensor = Sensor(29, 31)
rightSensor = Sensor(35, 32)
leftSensor = Sensor(11, 15)
controller = EngineController()

xDir = 0
yDir = 1

def turnRight():
    global controller, xDir, yDir
    controller.goRight()
    
    if xDir != 0 :
	xDir = 0
        yDir = -xDir
    else:
	yDir = 0;
	xDir = yDir
    time.sleep(0.5);
    controller.goForward()
    
def turnLeft():
    global controller, xDir, yDir
    controller.goLeft()
    
    if xDir != 0 :
        xDir = 0;
        yDir = xDir;
    else:
        yDir = 0;
	xDir = -yDir;
    time.sleep(0.5);
    controller.goForward()


def turnAround():
    global controller, xDir, yDir
    controller.goRight()
    
    if xDir != 0 :
        xDir = -xDir;
    else:
	yDir = -yDir;
    time.sleep(1);
    controller.goForward();
    

def scanMaze():
    global xDir, yDir, leftSensor
    file = open("measurements.txt", "w")
    startTime = time.time()
    currentPosition = [0, 0]
    oldDistance = 0
    ld = 0
    rd = 0
    fd = 0
    oldXDir = 0
    oldYDir = 1
    controller.setDuty(35)
    marginMinThreshold = 11
    marginMaxThreshold = 13
    turnThreshold = 30
    canTurn = False
    while True:
        f = frontSensor.getDist();
	l = leftSensor.getDist();
	r = rightSensor.getDist();
	
	if xDir != 0:
            if oldDistance != 0:
		currentPosition[0] += xDir * abs(f - oldDistance);
	    ld = currentPosition[1] + xDir * l;
	    rd = currentPosition[1] - xDir * r;
            fd = currentPosition[0] + xDir * f;
            file.write(str(currentPosition[0]) + ' ' + str(ld)  + ' ');
            file.write(str(currentPosition[0]) + ' ' + str(fd)  + ' ');
            file.write(str(currentPosition[0]) + ' ' + str(rd)  + '\n');
			
	if yDir != 0 :
	    if oldDistance != 0 :
		currentPosition[1] += yDir * abs(f - oldDistance);
		    
            ld = currentPosition[0] - yDir * l;
	    rd = currentPosition[0] + yDir * r;
	    fd = currentPosition[1] + yDir * f;
	    file.write(str(ld) + ' ' + str(currentPosition[1]) + ' ');
            file.write(str(fd) + ' ' + str(currentPosition[1]) + ' ');
            file.write(str(rd) + ' ' + str(currentPosition[1]) + '\n');
	  
	    
	#fprintf(filed, "%f, %f, %f\n", l, f, r);
	#printf("distance:%f, %f, %f\n", l, f, r);
	#printf("walls:%f, %f, %f\n", f, l, r);
	
        print(l, f, r, '\n')
        if f < 7:
            controller.goBack()
            time.sleep(1)
	if f < turnThreshold and l < turnThreshold and r < turnThreshold :#turn around
            turnAround()
	elif f < turnThreshold :#wall in front
            if r > turnThreshold :#turn right
                turnRight()
	    elif l > turnThreshold :#turn left
                turnLeft()
            time.sleep(2)
            controller.goForward()
	elif r > turnThreshold:
            turnRight()
        else:
            if r < marginMinThreshold and controller.getDirection() != 'l':
                controller.goLeft()
                time.sleep(0.03)
            elif l < marginMinThreshold and controller.getDirection() != 'r':
                controller.goRight()
                time.sleep(0.03)
            elif l < marginMaxThreshold and controller.getDirection() != 'l':
                controller.goLeft()
                time.sleep(0.03)
            elif r < marginMaxThreshold and controller.getDirection() != 'r':
                controller.goRight()
                time.sleep(0.03)
                
            controller.goForward()
            time.sleep(0.05)
                
        oldDistance = f;

	if oldXDir != xDir :
	    oldXDir = xDir;
    
	if oldYDir != yDir :
	    oldYDir = yDir;
	
	if f > 200 and l > 200 and r > 200 or time.time() - startTime > 1000000 * 10 :
	    break;
		
	time.sleep(0.01);
	
    file.close()

#carControl()
scanMaze()
