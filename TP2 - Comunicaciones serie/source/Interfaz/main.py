import interface


def main():

    # Configure before using it
    port = "COM3"
    baudrate = 9600
    game = interface.Game(port=port, baudrate=baudrate)

    '''
    The following code draw the axis of multiple boards connected by CAN protocol.
    Make sure sending id, axis and val to update board axis. The comunication ends when
    reading a 'EOF'. (Not implemented yet)
    '''

    # Main loop
    while game.stillRunning():
        game.handleEvents()
        game.update()
        game.render()


if __name__ == '__main__':
    main()
