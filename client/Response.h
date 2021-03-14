#ifndef RESPONSE_HEADER
#define RESPONSE_HEADER

#include "Constants.h"

class Response {
private:
	int sVersion;	// server version
	int responseCode;
	int payloadSize;
	Bytes *payload;
	// set the response code from the given response header
	void setCode(unsigned char*);
	// reads and sets the payload size from the given header
	void setPayloadSize(unsigned char*);

public:
	// Constructor. 
	// Constructs response header from a given header
	Response(unsigned char*);
	~Response();
	// returns this response payload size
	int getPayloadSIze();
	// returns response code
	int getResponseCode();
	// pushes Bytes into payload
	void pushPayload(unsigned char*, int);
	// returns a pointer to this response payload
	const Bytes& getPayload();
};

#endif