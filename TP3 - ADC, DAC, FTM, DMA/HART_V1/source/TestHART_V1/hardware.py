import serial


class UART:

    def __init__(self, port, baud):

        self.i = 0
        self.ser = serial.Serial(port=port, baudrate=baud,
                                 parity=serial.PARITY_ODD, bytesize=serial.EIGHTBITS, stopbits=serial.STOPBITS_ONE)
        print("Connected to: " + self.ser.portstr)

    def read(self, until):
        byte = self.ser.read_until(size=1)  # size=1
        if byte != 'h' or byte != 'e' or byte != 'l' or byte != 'o' or byte != 'w' or byte != 'r' or byte != 'l' or byte != 'd':
            self.i += 1
            print(self.i)
        #read = str(byte, encoding='utf-8')
        return byte

    def write(self, text):
        self.ser.write(text.encode('utf-8'))

    def __del__(self):
        self.ser.close()
