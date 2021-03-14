from constants import *
import struct

def makeHeader(code, plSize):
    return struct.pack(HEADER_FORMAT, SERVER_VERSION, code, plSize)

def errorMsg():
    return makeHeader(ERROR_CODE, 0)

def registerSuccessMsg(userID):
    msg = makeHeader(REG_SUC_CODE, UID_SIZE) + userID
    return msg
    
# returns a bytearray representation of users list without the user with the given uid
def generateUList(uid):
    ret = bytearray()
    for user in userslist:
        if user.uidInBytes() != uid:
            ret = ret + user.uidInBytes() + user.name
    return ret