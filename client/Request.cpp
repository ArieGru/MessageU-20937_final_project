#include <iostream>
#include "Request.h"
#include "Utils.h"

// Constructor
Request::Request() {
	req = new Bytes();
	rCode = 0;
}

// Destructor
Request::~Request() {
	delete req;
}

// clears request
void Request::clearRequest() {
	req->clear();
	rCode = 0;
}

// returns request code
unsigned char Request::getCode() {
	return rCode;
}

// push Bytes to the request
void Request::pushBytes(const Bytes& vec) {
	req->insert(req->end(), vec.begin(), vec.end());
}

// push one character to the request
void Request::pushChar(char c) {
	req->push_back((unsigned char)c);
}

// push an integer to the request vector as little endian integer
void Request::pushInt(unsigned int num) {
	unsigned int mask = 0x000000FF;
	for (int i = 0; i < sizeof(int); i++) {
		unsigned char tmp = (num & mask) >> (i * 8);
		req->push_back(tmp);
		mask <<= 8;
	}
}

// receives id and request code and builds request header
void Request::buildHeader(const Bytes& myID, unsigned char reqCode) {
	clearRequest();
	pushBytes(getID());
	std::cout << std::endl;
	pushChar(CLIENT_VERSION);
	pushChar(reqCode);
	rCode = reqCode;
}

// builds registration request payload with the given user name and public key
void Request::buildRegRequest(const Bytes& userName, const Bytes& pubKey) {
	// push payload size
	pushInt(MAX_USERNAME_LEN + PUB_KEY_SIZE);
	// push user name
	pushBytes(userName);
	// push 0's to get to protocol's user name size 
	int nameSize = userName.size();
	while (nameSize++ < 255)
		pushChar('\0');
	// push public key
	pushBytes(pubKey);
}

// build request with no payload. only pushes payload size (which is 0)
void Request::noPayloadRequest() {
	pushInt(0);
}

// receives user id and builds a get public key request payload
void Request::buildGetPubKeyRequest(const Bytes& userID){
	pushBytes(userID);
}

// builds payload for send message request.
void Request::buildSendMsgRequest(int plSize, const Bytes& id, unsigned char type, int msgSize, const Bytes& msg) {
	pushInt(plSize);
	pushBytes(id);
	pushChar(type);
	pushInt(msgSize);
	pushBytes(msg);
}

// returns a pointer to the request Bytes
const Bytes& Request::getRequest() {
	return *req;
}
