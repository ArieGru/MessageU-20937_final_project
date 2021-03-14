from constants import *
import struct

class Request:
    def __init__(self, header):
        self.uid, self.cversion, self.rcode, self.plsize = \
            struct.unpack(REQ_HEADER_FORMAT, header)

    def setPayload(self, pl):
        self.payload = pl
        