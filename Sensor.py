import RPi.GPIO as gpio
import time

class Sensor():

    def __init__(self, trig, echo):
        gpio.setmode(gpio.BOARD)

        self.trig = trig
        self.echo = echo

        self.setupDistSensor()

    def setupDistSensor(self):
        gpio.setup(self.trig, gpio.OUT)
        gpio.output(self.trig, 0)
        gpio.setup(self.echo, gpio.IN)

    
    def getDist(self):
        gpio.output(self.trig, 1)
        time.sleep(0.00001)
        gpio.output(self.trig, 0)

        while gpio.input(self.echo) == 0:
            pass
        start = time.time()

        while gpio.input(self.echo) == 1:
            pass

        stop = time.time()

        return (stop - start) * 1/2 * 340 * 100

    def clear(self):
        gpio.cleanup()
