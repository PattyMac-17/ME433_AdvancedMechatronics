import time
import board
import pwmio

from adafruit_motor import servo

print("alive")

pwm = pwmio.PWMOut(board.GP15, duty_cycle=65536, frequency = 50)

my_servo = servo.Servo(pwm)

while True:
    my_servo.angle = 0
    time.sleep(1)
    my_servo.angle = 180
    time.sleep(1)