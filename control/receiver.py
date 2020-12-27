''' socket '''

from twisted.internet import protocol


def empty(arr):
    ''' array is empty '''
    return arr is None or len(arr) <= 0


class _PauseableMixin:
    paused = False

    def pauseProducing(self):
        self.paused = True
        self.transport.pauseProducing()

    def resumeProducing(self):
        self.paused = False
        self.transport.resumeProducing()
        self.dataReceived(b'')

    def stopProducing(self):
        self.paused = True
        self.transport.stopProducing()


class DataReceiver(protocol.Protocol, _PauseableMixin):
    '''
    A protocol that receives raw data
    '''
    _buffer = b''
    _length = 0
    _wait_body = False
    _busyReceiving = False
    delimiter = b'\r\n'

    def dataReceived(self, data):
        """
        Protocol.dataReceived.
        Translates bytes into lines, and calls lineReceived (or
        rawDataReceived, depending on mode.)
        """
        if self._busyReceiving:
            self._buffer += data
            return

        try:
            self._busyReceiving = True
            self._buffer += data
            while not empty(self._buffer) and not self.paused:
                data = None

                if not self._wait_body:
                    # Locate header
                    pos = self._buffer.find(self.delimiter)
                    if pos == -1:
                        # Not found
                        print("Not found")
                        self._buffer = b''
                        break
                    pos += len(self.delimiter)

                    # Check header
                    if len(self._buffer[pos:]) <= 4:
                        # Wait for complete header
                        print("Wait")
                        break

                    # Get body length
                    self._length = int.from_bytes(
                        self._buffer[pos:pos+4], 'little')
                    self._buffer = self._buffer[pos+4:]

                if len(self._buffer) < self._length:
                    # Wait for complete body
                    self._wait_body = True
                    break
                else:
                    # Deal with body data
                    self._wait_body = False
                    data = self._buffer[:self._length]

                    # Write extra data to buffer, for next package
                    if len(self._buffer) > self._length:
                        self._buffer = self._buffer[self._length:]
                    else:
                        self._buffer = b''

                why = self.rawDataReceived(data)
                if why:
                    return why
        finally:
            self._busyReceiving = False

    def rawDataReceived(self, data):
        '''
        Override this for when raw data is received.
        '''
        raise NotImplementedError
