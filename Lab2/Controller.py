import msvcrt
import serial
ser = serial.Serial()
#ser = serial.Serial('COM4', 9600)
ser.baudrate = 9600
ser.port = 'COM4'
ser.open()
ser.isOpen()
import keyboard
mode = "screen"
# Poll keyboard
def robot_command():
    while(not keyboard.read_key()):
        pass
    if keyboard.is_pressed("w"):
        ser.write(b'C21FE')
    if keyboard.is_pressed("q"):
        ser.write(b'C21SE')
    if keyboard.is_pressed("d"):
        ser.write(b'C21RE')
    if keyboard.is_pressed("a"):
        ser.write(b'C21LE')
    if keyboard.is_pressed("s"):
        ser.write(b'C21BE')
    if keyboard.is_pressed("j"):
        ser.write(b'C21JE')
    if keyboard.is_pressed("k"):
        ser.write(b'C21KE')
    if keyboard.is_pressed("/"):
        ser.write(b'C21/E')
        global mode
        mode = "screen"
def screen_command():
    while(not keyboard.read_key()):
        pass
    input_char = keyboard.read_key()
    if input_char == '/':
        global mode
        mode = "robot"
    else:
        pass
    message = "C21"
    if input_char == "backspace":
        input_char = '~'
    elif input_char == "space":
        input_char = ' '
    else:
        pass
    message += input_char
    message += 'E'
    transmission = bytes(message, 'ascii')
    ser.write(transmission)
    print(transmission)
while (1):
    if mode == "robot":
        robot_command()
    else:
        screen_command()