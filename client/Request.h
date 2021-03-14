#ifndef REQUEST_HEADER
#define REQUEST_HEADER

#include "Constants.h"

class Request {
private:
	Bytes* req;
	unsigned char rCode;

public:
	Request();
	~Request();
	// clears request
	void clearRequest();
	// returns request code
	unsigned char getCode();
	// returns a pointer to the request Bytes
	const Bytes& getRequest();
	// push Bytes to the request
	void pushBytes(const Bytes&);
	// push one character to the request
	void pushChar(char);
	// push an integer to the request vector as little endian integer
	void pushInt(unsigned int);
	// receives id and request code and builds request header
	void buildHeader(const Bytes&, unsigned char);
	// builds registration request payload with the given user name and public key
	void buildRegRequest(const Bytes&, const Bytes&);
	// build request with no payload. only pushes payload size (which is 0)
	void noPayloadRequest();
	// receives user id and builds a get public key request payload
	void buildGetPubKeyRequest(const Bytes&);
	// builds payload for send message request.
	// parameters: 
	//	int - payload size
	//	Bytes - id (of receiver)
	//	unsigned char - message type
	//	int - message size
	//	Bytes - encrypted message
	void buildSendMsgRequest(int, const Bytes&, unsigned char, int, const Bytes&);
};

#endif