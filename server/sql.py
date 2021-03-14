from constants import *
import sqlite3
import uuid
import struct

def createDB():
    create_table_clients = """CREATE TABLE
                                clients(id VARCHAR(16) PRIMARY KEY CHECK(length(id) == 16),
                                Name VARCHAR(255) CHECK(length(Name) < 255), 
                                PublicKey VARCHAR(160) CHECK(length(PublicKey) == 160),
                                LastSeen INTEGER);"""
    # CHECK(ID < 4294967296) is to restrict message id to 4 bytes. 4294967295 is the biggest unsigned integer with 4 bytes
    create_table_messages = """CREATE TABLE
                                messages(ID INTEGER PRIMARY KEY CHECK(ID < 4294967296),
                                ToClient VARCHAR(16) , 
                                FromClient VARCHAR(16) ,
                                Type VARCHAR(1) ,
                                Content BLOB, 
                                FOREIGN KEY(ToClient) REFERENCES clients(id), 
                                FOREIGN KEY(FromClient) REFERENCES clients(FromClient));"""
    with sqlite3.connect('server.db') as conn:
        try:
            conn.executescript(create_table_clients)
            conn.executescript(create_table_messages)
        except:
            conn.close()
            raise Exception
    conn.close()

def isUsernameAvailable(name):
    query = "SELECT * FROM clients WHERE name = ?"
    usersList = []
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        cur.execute(query, [name])
        usersList = cur.fetchall()
    conn.close()
    if usersList:
        raise Exception()

# insert new client to the db.
# returns generated client id (uuid) if succeed, raises exception if user name is taken or if fails 
def insertClient(name, key):
    uid = uuid.uuid1().bytes
    query = """insert into clients values (?, ?, ?, datetime('now')) """
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        # will raise exception if user name already in list OR if id is taken
        try:
            isUsernameAvailable(name)
            cur.execute(query, [uid, name, key])
        except:
            conn.close()
            raise Exception()
    conn.close()
    return uid
    
# returns a MAX_NAME_LEN size bytearray object from the given name followed by 0's
def nameToBytes(name):
    tmp = bytes(name, 'utf-8')
    return bytearray(tmp + (MAX_NAME_LEN - len(tmp)) * bytes('\0', 'utf-8'))
        

# returns bytes representation of users list without the user with the given id, as required by the protocol
def getUsersList(uid):
    query = "SELECT * FROM clients WHERE id <> ?"
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        cur.execute(query, [uid])
        usersList = cur.fetchall()
    conn.close()
    ret = bytes()
    for user in usersList:
        ret = ret + user[0] + nameToBytes(user[1])
    return ret

# returns public key bytes representation of the given user. raises exception if fails
def getUserPubKey(uid):
    query = "SELECT PublicKey FROM clients WHERE id = ?"
    pubKey = ''
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        cur.execute(query, [uid])
        pubKey = cur.fetchall()
    conn.close()
    return pubKey[0][0]
    
# saves message in db
def saveMsg(toClient, fromClient, msgType, content):
    query = """insert into messages values (?, ?, ?, ?, ?) """
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        # will raise exception if user name already in list OR if id is taken
        # we insert None as msg id to make the db generate it automatically
        try:
            cur.execute(query, [None, toClient, fromClient, msgType, content])
            return cur.lastrowid
        except:
            raise Exception()
            
    conn.close()

# delete all messages destined for the user with the given id
def deleteMessages(uid):
    query = """DELETE FROM messages WHERE ToClient = ?"""
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        cur.execute(query, [uid])
    conn.close()

# returns a bytes representation (by the protocol) of all the messages destined to the user with the given id, and deletes them from db.
def getMessages(uid):
    query = """SELECT * FROM messages WHERE ToClient = ?"""
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        cur.execute(query, [uid])
        messages = cur.fetchall()
    conn.close()
    
    deleteMessages(uid)
    
    content = bytes()
    for msg in messages:
        content += msg[2] + \
                    struct.pack('<I', msg[0]) + \
                    msg[3] + \
                    struct.pack('<I', len(msg[4])) + \
                    msg[4]
    return content

# updates last seen column for the user with the given id
def updateConnTime(uid):
    query = "UPDATE clients SET LastSeen = datetime('now') WHERE id = ?"
    with sqlite3.connect('server.db') as conn:
        cur = conn.cursor()
        cur.execute(query, [uid])
    conn.close()