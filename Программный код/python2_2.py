import RPi.GPIO as GPIO
#mport cv2
from pyzbar.pyzbar import decode
from picamera2 import Picamera2
import os
import time
#from motori import control_stepper_motor
import kon2 as Home
import motori_2 as mt
#import motori_A as motorA
#import motori_B as motorB
import serial 

#PuertoSerie =serial.Serial('/dev/ttyUSB0', 9600)
#the number of pulses for full rotation
ppr = 10000

    
# Константы для области интереса
ROI_X = 0
ROI_Y = 0
ROI_WIDTH = 700
ROI_HEIGHT = 500


'''
parking_numbers = ["PARKING_A123",
                   "PARKING_B456",
                   "PARKING_C789",
                   "PARKING_D012",
                   "PARKING_E345",
                   "PARKING_F678"]
'''

# Имитация базы данных парковочных мест (номер -> QR-код)
PARKING_DATA = {
    "A": 'PARKING_A',
    "B": 'PARKING_B',
    "C": 'PARKING_C',
    "D": 'PARKING_D',
    "E":  'PARKING_E',
    "F": 'PARKING_F'
}

PARKING_SLOT = {
    "A": ["", 1],
    "B": ["", 37],
    "C": ["", 73],
    "D": ["", 107],
    "E": ["", 144],
    "F": ["", 181]
}

# Путь к sh файлу
#SH_FILE_PATH = "parking_action.sh"

def get_parking_number_by_qr(qr_data):
    '''
    Проверяет, существует ли QR-код в базе данных и возвращает номер парковочного места.
    '''
    for number, qr_code in PARKING_DATA.items():
        if qr_code == qr_data:
            return number
    return None


def scan_qr_code(frame):
    """
    Выделяет область кадра для "лазерного сканирования", считывает QR-код и возвращает его данные.
    """
    # Обрезаем кадр для имитации сканирования
    roi = frame[ROI_Y:ROI_Y + ROI_HEIGHT, ROI_X:ROI_X + ROI_WIDTH]
    try:
        decoded_objects = decode(roi)
        if decoded_objects:
           # Возвращаем данные из первого найденного QR-кода
           return decoded_objects[0].data.decode('utf-8')
    except Exception as e:
        print(f"Ошибка при сканировании QR-кода: {e}")
    return None



def main():
    print('MAIN PROGRAMM')
    error_pin = 38
    mt.setup_stepper_motor()
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(error_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    
    Home.search_home(False, error_pin)
    time.sleep(2)
    
 #   Основная функция для работы с видеокамерой.

    picam2 = Picamera2()
    config = picam2.create_preview_configuration()
    picam2.configure(config)
    picam2.start()
    
    
    
    
    while True:
        frame = picam2.capture_array()
        if not GPIO.input(error_pin):
            print('camera_find')
            time.sleep(0.5)
            if frame is None:
                print("Не удалось получить кадр с камеры.")
                break

    #Рисуем прямоугольник области сканирования.

            qr_data = scan_qr_code(frame)
            if qr_data:
                parking_number = get_parking_number_by_qr(qr_data)
                if parking_number:
                    print(f"Найдено парковочное место: {parking_number}")
                    time.sleep(0.5)
                    if parking_number in PARKING_SLOT:
                        time.sleep(0.5)
                        value = PARKING_SLOT[parking_number]
                        if value[0] == "":
                            print("Find place")
                            time.sleep(0.5)
                            PARKING_SLOT[parking_number][0] = parking_number
                            for i in range(value[1]):
                                if not GPIO.input(error_pin):
                                    mt.rotate_motors(delay=0.001, steps=(16))
                            print("Jdem zaezda mashiny")
                            time.sleep(0.5)
                            # pishem function dlya zaezda car
                            if not GPIO.input(error_pin):
                                Home.search_home(False, error_pin)
                        else:
                            PARKING_SLOT[parking_number][0] = ""
                            for i in range(value[1]):
                                if not GPIO.input(error_pin):
                                    mt.rotate_motors(delay=0.001, steps=(16))
                            print("Jdem vyezda")
                            #vyezd mashuny
                            time.sleep(0.5)
                            if not GPIO.input(error_pin):
                                Home.search_home(False, error_pin)
                    #control_stepper_motor()
                    #print(parking_number)
                    #write_to_sh(parking_number)
                    #print(f"Данные сохранены в {SH_FILE_PATH}")
                    #break  # Выходим из цикла после успешного сканирования и записи
                else:
                    print("Нет соответствия в базе данных.")
                    time.sleep(1)
                    break

            #if cv2.waitKey(1) & 0xFF == ord('q'):
                #break

        #picam2.stop()
        #picam2.close()
        #cv2.destroyAllWindows()


if __name__ == "__main__":
   main()
    
# как убрать графический интерфейс для кода в питоне :
# как соединить 2 файла в питоне с учетом того что в одном файле запуск камеры и сканируется qr kolf а в другом моторчик, который крутиться при правильном qr kolt как их совместить и что дjgbcfnm 
