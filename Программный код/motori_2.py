import RPi.GPIO as GPIO
import time

key = 23

#GPIO.setup(key, GPIO.IN)
print('motori2_start')

lefts = []
rights = []
left = 0
right = 1



def setup_stepper_motor(delay=0.003, steps=512, pins_left = [11,12,13,15], pins_right = [29,31,32,33]):
    global step_sequence_forward
    global step_sequence_backward
    GPIO.setmode(GPIO.BOARD)
    

    for i in pins_left:
        GPIO.setup(i, GPIO.OUT)
        lefts.append(i)
    for i in pins_right: 
        GPIO.setup(i, GPIO.OUT)
        rights.append(i)

    step_sequence_forward = [
        [1, 0, 0, 0],
        [1, 1, 0, 0],
        [0, 1, 0, 0],
        [0, 1, 1, 0],
        [0, 0, 1, 0],
        [0, 0, 1, 1],
        [0, 0, 0, 1],
        [1, 0, 0, 1],
    ]
    step_sequence_backward = [w for w in step_sequence_forward[::-1]]


def set_step(mot_id, w):
    if mot_id == left:
        for pin in range(len(lefts)):
            GPIO.output(lefts[pin], w[pin])
    elif mot_id == right:
        for pin in range(len(rights)):
            GPIO.output(rights[pin], w[pin])
            


def rotate_motors(delay, steps):
    if steps > 0:
        step_sequence_0 = step_sequence_forward
        step_sequence_1 = step_sequence_backward
    else:
        step_sequence_1 = step_sequence_forward
        step_sequence_0 = step_sequence_backward
            
    for _ in range(abs(steps)):
        for step in range(len(step_sequence_1)):
            set_step(left,step_sequence_0[step])
            set_step(right,step_sequence_1[step])
            time.sleep(delay)

    #try:
    #    rotate_motors(delay, steps)
        
   # except KeyboardInterrupt:
    #    print("Остановка двигателя.")
    #finally:
     #   GPIO.cleanup()


#if __name__ == '__main__':
 #   try:
  #      while True:
   #             control_stepper_motor(delay=0.001, steps=-256, direction="forward")
                #time.sleep(1)

    #except KeyboardInterrupt:
     #   print("Программа остановлена.")
      #  GPIO.cleanup()

