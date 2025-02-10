import RPi.GPIO as GPIO
import time
#from motori import setup_stepper_motor
#from motori import rotate_motors
import motori_2 as mt
#import motori_A as motorA
#import motori_B as motorB
#from motori import setup_stepper_motor


def search_home(flag_limit, error_pin):
	print('START_HOME')
	#time.sleep(0.25)
	endstoppin = 40
	GPIO.setmode(GPIO.BOARD)
	GPIO.setup(endstoppin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(error_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	#mt.setup_stepper_motor()
	#mt.rotate_motors(delay=0.001, steps=16)
	
	while flag_limit == False and  not GPIO.input(error_pin):
		print('FIND HOME')
		#time.sleep(1)
		if GPIO.input(endstoppin):
			mt.rotate_motors(delay=0.001, steps=-12)
		else:
			flag_limit = True
#			while GPIO.input(error_pin):
#				print('error')
#				time.sleep(1)
#			if not GPIO.input(error_pin):
#				while flag_limit == False and  not GPIO.input(error_pin):
#					print('FIND HOME')
					#time.sleep(1)
#					if GPIO.input(endstoppin):
#						mt.rotate_motors(delay=0.001, steps=-12)
			
	mt.rotate_motors(delay=0.001, steps=16)
	mt.rotate_motors(delay=0.001, steps=16)
	


