# This is the server program of client-server, end-to-end encryption chat project
# we use a specified binary protocol over TCP
#    Author: Arie Gruber

from constants import *
import socket
import Request
import struct
import selectors
import sql

# and returns the name in the given bytes without the zeroes at the end of it
def nameToStr(name):
    return str(name, 'utf-8').rstrip('\0')

# returns header with the given code and payload size in bytes format
def makeHeader(code, plSize):
    return struct.pack(HEADER_FORMAT, SERVER_VERSION, code, plSize)

def errorMsg():
    return makeHeader(ERROR_CODE, 0)

def registerUser(name, pubKey):
    try:
        userID = sql.insertClient(name, pubKey)
        return makeHeader(REG_SUC_CODE, UID_SIZE) + userID
    except:
        return errorMsg()

def getUList(uid):
    sql.updateConnTime(uid)
    uList = sql.getUsersList(uid)
    msg = makeHeader(ULIST_RESP_CODE, len(uList)) + uList
    return msg

def getPubKey(request):
    uid = request.payload
    sql.updateConnTime(request.uid)
    try:
        msg = makeHeader(PUB_KEY_RESP_CODE, UID_SIZE + PUB_KEY_SIZE) + uid + sql.getUserPubKey(uid)
        return msg
    except:
        return errorMsg()
        
def saveMsg(request):
    orig = request.uid
    sql.updateConnTime(orig)
    dest = request.payload[:UID_SIZE]
    msgType = struct.pack('B', request.payload[UID_SIZE])
    txt = request.payload[MSG_CONTENT_OFFSET:]
    msgID = sql.saveMsg(dest, orig, msgType, txt)
    return makeHeader(MSG_SEND_SUC_CODE, MSG_SENT_PAYLOAD_SIZE) + request.payload[:UID_SIZE] + struct.pack('<I', msgID)
    
    
def getMessages(request): 
    sql.updateConnTime(request.uid)
    content = sql.getMessages(request.uid)
    header = makeHeader(REC_MSG_CODE, len(content))
    return header + content

def requestHandler(request):
    if request.rcode == REG_CODE:
        userName = nameToStr(request.payload[:MAX_NAME_LEN])
        pubKey = request.payload[MAX_NAME_LEN:]
        return registerUser(userName, pubKey)
    elif request.rcode == ULIST_REQ_CODE:
        return getUList(request.uid)
    elif request.rcode == PUB_KEY_REQ_CODE:
        return getPubKey(request)
    elif request.rcode == GET_MSG_CODE:
        return getMessages(request)
    elif request.rcode == SEND_MSG_CODE:
        return saveMsg(request)

sel = selectors.DefaultSelector()

def accept(sock, mask):
    conn, addr = sock.accept()
    conn.setblocking(False)
    sel.register(conn, selectors.EVENT_READ, read)
    
def read(conn, mask):
    try:
        header = conn.recv(REQ_HEADER_SIZE)
        request = Request.Request(header)
        data = bytearray()
        while len(data) < request.plsize:
            packet = conn.recv(request.plsize - len(data))
            data.extend(packet)
        request.setPayload(data)
        res = requestHandler(request)
        conn.sendall(res)
    except:
        print('something went wrong with last connection')
    finally:
        sel.unregister(conn)
        conn.close()
        
def main():
    host = ''
    #read port from file
    f = open("port.info", "r")
    port = int(f.read())
    
    #if db file doesn't exist, make it
    try:
        sql.createDB()
    except:
        print('db already exists')
    sock = socket.socket()
    sock.bind((host, port))
    sock.listen()
    sock.setblocking(False)
    sel.register(sock, selectors.EVENT_READ, accept)
    
    while True:
        events = sel.select()
        for key, mask in events:
            callback = key.data
            callback(key.fileobj, mask)
    
if __name__ == '__main__':  
    main()