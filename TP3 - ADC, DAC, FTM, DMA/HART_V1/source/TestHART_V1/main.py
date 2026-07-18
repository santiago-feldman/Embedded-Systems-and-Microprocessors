from hardware import UART
import matplotlib.pyplot as plt
import os

def main():

    # Configure before using
    port = "COM8"
    baudrate = 9600
    uart = UART(port=port, baud=baudrate)
    
    with open("./HART_V1/source/TestHART_V1/transmit.txt", "r") as file:
        text = file.read()

    uart.write(text)

    read = 0
    while (read != '~'):
        read = uart.read(b'~')
        print(read)

    with open("./HART_V1/source/TestHART_V1/received.txt", "w") as file:
        file.write(read)


if __name__ == '__main__':
    main()
