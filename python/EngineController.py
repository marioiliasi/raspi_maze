import RPi.GPIO as gpio
import time

class EngineController():
    
    def __init__(self):
        gpio.setmode(gpio.BOARD)
        gpio.setwarnings(False)

        self.leftB = 37
        self.leftF = 36
    
        self.rightB = 38
        self.rightF = 40
            
        self.Eleft = 18
        self.Eright = 13

        self.lWorking = False
        self.rWorking = False
        
        self.duty = 100

        self.motorsGPIO()
        self.motorsEGPIO()

        self.leftCount = 0
        self.rightCount = 0
        
        self.stop()
        
        self.rPWM = gpio.PWM(self.Eright, self.duty)
        self.lPWM = gpio.PWM(self.Eleft, self.duty)
        
        self.direction = ''

    def motorsGPIO(self):
        gpio.setup(self.leftB, gpio.OUT)
        gpio.setup(self.leftF, gpio.OUT)

        gpio.setup(self.rightB, gpio.OUT)
        gpio.setup(self.rightF, gpio.OUT)

    def motorsEGPIO(self):
        gpio.setup(self.Eright, gpio.OUT)
        gpio.setup(self.Eleft, gpio.OUT)

    def goForward(self):
        self.lWorking = True
        self.rWorking = True
        self.rPWM.start(self.duty)
        self.lPWM.start(self.duty)
        self.setDirection('f')

    def goLeft(self):
        self.lWorking = False
        self.rWorking = True
        self.rPWM.start(self.duty)
        self.lPWM.start(self.duty)
        #self.lPWM.stop()
        self.setDirection('l')
        self.leftCount += 1
        
    def goRight(self):
        self.lWorking = True
        self.rWorking = False
        #self.rPWM.stop()
        self.rPWM.start(self.duty)
        self.lPWM.start(self.duty)
        self.setDirection('r')
        self.rightCount += 1

    def goBack(self):
        self.lWorking = True
        self.rWorking = True
        self.rPWM.start(self.duty)
        self.lPWM.start(self.duty)
        self.setDirection('b')

    def stay(self, t):
        self.setMotors(False, False, False, False)
        time.sleep(t)

    def stop(self):
        
        if self.lWorking:
            self.lPWM.stop()
        if self.rWorking:
            self.rPWM.stop()

    def setMotors(self, lF, rF, lB, rB):
        gpio.output(self.leftF, lF)
        gpio.output(self.rightF, rF)
        gpio.output(self.leftB, lB)
        gpio.output(self.rightB, rB)
        
    def setDirection(self, direction):
        self.direction = direction
        if direction == 'f':
            self.setMotors(True, True, False, False)
        elif direction == 'r':
            self.setMotors(True, False, False, False)
        elif direction == 'l':
            self.setMotors(False, True, False, False)
        elif direction == 'b':
            self.setMotors(False, False, True, True)
        print(direction)

    def setDuty(self, value):
        self.duty = value
        self.rPWM.ChangeDutyCycle(value)
        self.lPWM.ChangeDutyCycle(value)

    def setLeftDuty(self, value):
        self.lPWM.ChangeDutyCycle(value)

    def setRightDuty(self, value):
        self.rPWM.ChangeDutyCycle(value)
        
    def clear(self):
        
        self.stop()
        gpio.cleanup()

    def getDirection(self):
        return self.direction

    def getLeftCount(self):
        return self.leftCount

    def getRightCount(self):
        return self.rightCount
    
