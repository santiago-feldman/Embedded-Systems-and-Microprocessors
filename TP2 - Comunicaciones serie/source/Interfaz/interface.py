import pygame
import random
from queue import Queue
from hardware import UART


class Board:

    def __init__(self):
        self.roll = 0
        self.pitch = 0
        self.yaw = 0
        self.img = pygame.image.load(
            "C:\\labo\\TPs_LaboDeMicros\\TP2- Comunicacion Serie\\source\\Interfaz\\Textures\\board.png")

    # Getters and setters
    def setRoll(self, val):
        self.roll = val

    def setPitch(self, val):
        self.pitch = val

    def setYaw(self, val):
        self.yaw = val

    def getImg(self):
        return self.img

    def getYaw(self):
        return self.yaw

    def getRoll(self):
        return self.roll

    def getPitch(self):
        return self.pitch


class Game:

    MAPWIDTH = 2
    MAPHEIGHT = 3
    GLOBALSCALE = 2

    def __init__(self, port, baudrate):

        # pygame setup
        pygame.init()
        pygame.display.set_caption("Board integration with UART")
        self.clock = pygame.time.Clock()
        self.running = True

        # self.uart = UART(port=port, baud=baudrate)
        self.boards = {"1:Board": Board(), "2:Board": Board(), "3:Board": Board(),
                       "4:Board": Board(), "5:Board": Board(), "6:Board": Board()}
        ref = self.boards["1:Board"].img
        w = ref.get_width()*self.GLOBALSCALE
        h = ref.get_height()*self.GLOBALSCALE
        self.screen = pygame.display.set_mode(
            (self.MAPWIDTH*2*w, self.MAPHEIGHT*h))
        self.uart = UART(port=port, baud=baudrate)

    def handleEvents(self):

        # Event handler
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False

    def update(self):

        idRead, axis, val = self.uart.read()  # Read UART

        if idRead is not None and axis is not None and val is not None:
            # Update board axis
            curr = f"{idRead}:Board"
            self.__setAxis(curr, axis=axis, val=val)

    def render(self):

        # fill the screen with a color to wipe away anything from last frame
        self.screen.fill("black")

        fontSize = 20
        offset = 10
        font = pygame.font.SysFont('Arial', fontSize)
        fcolor = (255, 255, 255)

        # Lo posicionamos
        for i, (name, board) in enumerate(zip(self.boards.keys(), self.boards.values())):

            column = i % 2
            row = i//2

            img = board.img
            img = pygame.transform.scale(
                img, (img.get_width()*self.GLOBALSCALE, img.get_height()*self.GLOBALSCALE))
            x_cart = (2*column+1) * img.get_width()
            y_cart = row * img.get_height()

            currName = font.render(f'{name}', False, fcolor)
            descP = font.render(f'Pitch: {board.getPitch()}°', False, fcolor)
            descR = font.render(f'Roll: {board.getRoll()}°', False, fcolor)
            descY = font.render(f'Yaw: {board.getYaw()}°', False, fcolor)
            x_font = 2*column * img.get_width()
            y_font = row * img.get_width()

            self.screen.blit(img, (x_cart, y_cart))
            self.screen.blit(
                currName, (x_font+offset, y_cart+fontSize*0+offset))
            self.screen.blit(descP, (x_font+offset, y_cart+fontSize*1+offset))
            self.screen.blit(descR, (x_font+offset, y_cart+fontSize*2+offset))
            self.screen.blit(descY, (x_font+offset, y_cart+fontSize*3+offset))

        # flip() the display to put your work on screen
        pygame.display.flip()

        self.clock.tick(60)  # limits FPS to 60

    def stillRunning(self):
        return self.running

    def __setAxis(self, name, axis, val):
        if axis == 'R':
            self.boards[name].setRoll(val)
        elif axis == 'C':
            self.boards[name].setPitch(val)
        elif axis == 'O':
            self.boards[name].setYaw(val)

    def __del__(self):
        pygame.quit()
