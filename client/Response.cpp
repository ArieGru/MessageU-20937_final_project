#include "Response.h"
#include "Utils.h"

// set the response code from the given response header
void Response::setCode(unsigned char* buf) {
	unsigned char tmp[] = { buf[1], buf[2], 0, 0 };
	responseCode = bytesToInt(tmp);
}

// reads and sets the payload size from the given header
void Response::setPayloadSize(unsigned char* buf) {
	unsigned char tmp[] = { buf[3] , buf[4],  buf[5] , buf[6] };
	payloadSize = bytesToInt(tmp);
}

// Constructor. 
// Constructs response header from a given header
Response::Response(unsigned char* buf){
	sVersion = buf[0];
	setCode(buf);
	setPayloadSize(buf);
	payload = new Bytes();
}

// Destructor
Response::~Response() {
	delete payload;
}

// returns this response payload size
int Response::getPayloadSIze() {
	return payloadSize;
}

// returns response code
int Response::getResponseCode() {
	return responseCode;
}

// pushes Bytes into payload
void Response::pushPayload(unsigned char* buf, int size) {
	for (int i = 0; i < size; i++)
		payload->push_back(buf[i]);
}

// returns a pointer to this response payload
const Bytes& Response::getPayload() {
	return *payload;
}
