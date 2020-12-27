''' control '''

import struct
import sys
import threading

import numpy as np
import cv2

from twisted.internet import reactor
from twisted.internet.protocol import (connectionDone, ServerFactory)
from twisted.protocols.basic import LineReceiver
from twisted.python import log

from receiver import DataReceiver

IMG_WIDTH = 640
IMG_HEIGHT = 480
IMG_SIZE_8UC1 = IMG_WIDTH * IMG_HEIGHT
IMG_SIZE_8UC3 = 3 * IMG_SIZE_8UC1


def FormatAddr(addr):
    return "%s:%s" % addr


def CRC8(s: str):
    crc = 0
    for i in range(len(s)):
        extract = ord(s[i])
        for _ in range(8, 0, -1):
            sum_i = (crc ^ extract) & 0x01
            crc = crc >> 1
            if sum_i != 0:
                crc = crc ^ 0x8C
            extract = extract >> 1
    return crc


class CarSocket(LineReceiver):
    ''' CarSocket '''
    delimiter = '\n'.encode()

    def __init__(self, factory):
        self.factory = factory

    def send(self, msg):
        self.transport.write(('%s;%03d' % (msg, CRC8(msg))).encode())

    def connectionMade(self):
        log.msg("Client connection from %s" % self.transport.getPeer())
        if len(self.factory.clients) >= self.factory.clients_max:
            log.msg("Too many connections. bye !")
            self.transport.loseConnection()
        else:
            self.factory.clients.append(self)

    def connectionLost(self, reason=connectionDone):
        log.msg('Lost client connection. Reason: %s' % reason)
        self.factory.clients.remove(self)

    def lineReceived(self, line):
        line = str(line).strip()
        if line == "exit":
            self.transport.loseConnection()
        elif line == "Arton":
            log.msg("Handshake")
            self.send('Car+S1%d' % 500)
        elif line is not None:
            log.msg('Cmd received from %s: %s' %
                    (self.transport.getPeer(), line))

        return None


class RealsenseSocket(DataReceiver):
    ''' TCPSocket '''
    delimiter = b'Arton\0'

    def __init__(self, factory, window):
        self.factory = factory

        self.window = window

    def connectionMade(self):
        log.msg("Client connection from %s" % self.transport.getPeer())
        if len(self.factory.clients) >= self.factory.clients_max:
            log.msg("Too many connections. bye!")
            self.transport.loseConnection()
        else:
            self.factory.clients.append(self)

    def connectionLost(self, reason=connectionDone):
        log.msg('Lost client connection. Reason: %s' % reason)
        self.factory.clients.remove(self)

    def rawDataReceived(self, data):
        img_type = int.from_bytes(data[:4], 'little')
        second = struct.unpack('d', data[4:12])[0]
        data = data[12:]

        if img_type == 1:
            image = np.frombuffer(data, dtype=np.uint8).reshape((480, 640, 3))
            image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
        elif img_type == 2:
            image = np.frombuffer(data, dtype=np.uint8).reshape((480, 640, 1))
        elif img_type == 3:
            image = np.frombuffer(data, dtype=np.uint16).reshape((480, 640, 1))
        else:
            log.msg("Unknown type: %d", img_type)
            return None

        print(second)

        # Show image
        self.window.set_image(img_type, image, second)

        #depth_colormap = cv2.applyColorMap(cv2.convertScaleAbs(depth_image, alpha=0.03), cv2.COLORMAP_JET)
        #images = np.hstack((depth_colormap))
        #cv2.namedWindow('RealSense', cv2.WINDOW_AUTOSIZE)
        #cv2.imshow('RealSense', depth_image)
        return None


class CarSocketFactory(ServerFactory):
    protocol = CarSocket

    def __init__(self, clients_max=10):
        self.clients_max = clients_max
        self.clients = []

    def buildProtocol(self, addr):
        return CarSocket(self)


class RealsenseSocketFactory(ServerFactory):
    protocol = RealsenseSocket

    def __init__(self, clients_max=10, window=None):
        self.clients_max = clients_max
        self.clients = []
        self.window = window

    def buildProtocol(self, addr):
        return RealsenseSocket(self, self.window)


class MainThread(threading.Thread):
    ''' MainThread '''

    def __init__(self, window):
        threading.Thread.__init__(self)
        self.window = window
        self.car_factory = CarSocketFactory(20)
        self.realsense_factory = RealsenseSocketFactory(20, window)

    def send(self, msg):
        ''' send '''
        for client in self.car_factory.clients:
            client.send(msg)

    def run(self):
        ''' run '''
        reactor.listenTCP(17888, self.car_factory)
        reactor.listenTCP(17889, self.realsense_factory)
        reactor.run(installSignalHandlers=0)

    def stop(self):
        ''' stop '''
        raise Exception("Bye!")


log.startLogging(sys.stdout)
