from serial import Serial


class UART:

    def __init__(self, port, baud):

        self.ser = Serial(port=port, baudrate=baud)
        print("Connected to: " + self.ser.portstr)

    def read(self):

        try:
            id = int(self.ser.read_until()[-2:-1])  # board id
            if id == 0:
                id = 1
            temp = self.ser.read_until()
            val = int(temp[1:])  # val
            axis = str(temp[:1], 'utf-8')  # axis
            return id, axis, val

        except:
            return None, None, None

    def __del__(self):
        self.ser.close()
