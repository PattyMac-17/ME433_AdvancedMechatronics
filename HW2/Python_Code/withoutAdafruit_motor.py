import time
import board
import pwmio

pwm = pwmio.PWMOut(board.GP15, duty_cycle=65536, frequency = 50)

def setServoAngle(angle):
    if (angle < 0):
        angle = 0
    if (angle > 180):
        angle = 180
    duty_percent = 2.0 + (angle/180) * (11.5-2)
    duty = int(duty_percent/100 * 65535)
    pwm.duty_cycle = duty

while True:
    setServoAngle(0)
    time.sleep(0.5)
    setServoAngle(180)
    time.sleep(0.5)