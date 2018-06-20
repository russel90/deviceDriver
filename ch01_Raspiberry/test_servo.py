import time 
from SunFounder_PCA9685 import Servo

myservo = []
for i in range(0, 16):
        myservo.append(Servo.Servo(i))  # channel 1
        Servo.Servo(i).setup()
        print 'myservo%s'%i

while True:
        for i in range(0, 180, 5):
                print i
                for channel in range(0, 16):
                        myservo[channel].write(i)
                        print '   channel%s'%channel
                time.sleep(0.1)
        for i in range(180, 0, -5):
                print i
                for channel in range(0, 16):
                        myservo[channel].write(i)
                        print '    channel%s'%channel
                time.sleep(0.1)
