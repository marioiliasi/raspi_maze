import matplotlib.pyplot as plt

filepath = 'measurements2.txt'
currentPosition = [0, 0]
oldValues = []
xDir = 0
yDir = 1
oldXDir = 0
oldYDir = 1
try:
    fp = open(filepath)
    line = fp.readline()
    cnt = 1
    while line:
        if line[len(line) - 1] == '\n':
            line = line[0:len(line) - 1]
        values = line.split(", ")
        values[0] = float(values[0])
        values[1] = float(values[1])
        values[2] = float(values[2])
        print(values)

        if oldValues != []:
            if oldXDir == xDir and oldYDir == yDir:
                if yDir != 0:
                    currentPosition[1] += abs(oldValues[1] - values[1]) * yDir
                elif xDir != 0:
                    currentPosition[0] += abs(oldValues[1] - values[1]) * xDir

        if yDir != 0:
            plt.scatter(currentPosition[0] - values[0], currentPosition[1])
            plt.scatter(currentPosition[0] + values[2], currentPosition[1])
        elif xDir != 0:
            plt.scatter(currentPosition[0], currentPosition[1] - values[0])
            plt.scatter(currentPosition[0], currentPosition[1] + values[2])

        if oldXDir != xDir:
            oldXDir = xDir
        if oldYDir != yDir:
            oldYDir = yDir

        if oldValues != []:
            if values[1] <= 10:
                if values[0] >= 15:
                    if yDir > 0:
                        yDir = 0
                        xDir = -1
                        currentPosition[0] -= values[0]
                    elif yDir < 0:
                        yDir = 0
                        xDir = 1
                        currentPosition[0] += values[0]
                    elif xDir > 0:
                        xDir = 0
                        yDir = 1
                        currentPosition[1] += values[1]
                    else:
                        xDir = 0
                        yDir = -1
                        currentPosition[1] -= values[1]
                elif values[2] >= 15:
                    if yDir > 0:
                        yDir = 0
                        xDir = 1
                        currentPosition[0] += values[0]
                    elif yDir < 0:
                        yDir = 0
                        xDir = -1
                        currentPosition[0] -= values[0]
                    elif xDir > 0:
                        xDir = 0
                        yDir = -1
                        currentPosition[1] -= values[1]
                    else:
                        xDir = 0
                        yDir = 1
                        currentPosition[1] += values[1]
                else:
                    if yDir != 0:
                        yDir = -yDir
                    elif xDir != 0:
                        xDir = -xDir
        else:
            plt.scatter(currentPosition[0], currentPosition[1] + values[1])
        oldValues = values
        line = fp.readline()
        '''plt.scatter(values[0], values[1])
        plt.scatter(values[2], values[3])
        plt.scatter(values[4], values[5])'''
        cnt += 1
finally:
    fp.close()

plt.show()

