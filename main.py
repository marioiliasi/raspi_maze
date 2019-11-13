from Sensor import Sensor
import time

def main():
    start = time.time()
    frontSensor = Sensor(29, 31)
    print frontSensor.getDist(), "cm\n"
    print (time.time() - start) * 1000000, "\n"
    frontSensor.clear()

main()
