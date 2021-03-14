#ifndef HANDLER_HEADER
#define HANDLER_HEADER

#include <string>
#include "UsersList.h"
#include "Constants.h"
#include "Keys.h"
#include "Request.h"
#include "Response.h"

class Handler {
private:
	Request* request;
	UsersList* usersList;
	Keys* keys;
	std::string myUserName;

	// prints the menu
	void printMenu();
	// read user selection and returns it. if the input isn't valid returns INVALID_INPUT
	int getSelection();
	// checks if the info file exists
	bool isInfoFileExists();
	// exits the program if there is a problem with info file or with reading it
	void checkInfo();
	// build the request from the given selection. returns true if all went ok, false otherwise.
	bool buildRequest(int);
	// returns request code of the given user selection
	unsigned char getReqCode(int);
	// get user name from user and returns it as string.
	std::string getUserName();
	// get message from user and returns it as string
	std::string getMessage();
	// saves user name and id to info file
	void saveInfo(const Bytes&);
	// retrieves public key and user id from the given response payload and saves it
	void addPubKey(const Bytes&);
	// retrieves users list from given response payload and saves it
	void makeUsersList(const Bytes&);
	// retrieves messages from the given response payload and prints them
	void printMessages(const Bytes&);
	// request a file path from user and returns it as string
	std::string getFilePath();
	// receive a string to a path and returns a Bytes with the file in this path
	Bytes loadFile(const std::string&);
	// saves a file in Bytes form to disk. returns full path to this file as a string
	std::string saveFile(const Bytes&);
	// generates and returns random string RANDOM_FILENAME_LEN long
	std::string makeRandomString();

public:
	Handler();
	~Handler();
	// main io loop. responsible for the interaction with the user.
	// returns false if the user want to quit. otherwise builds the request and returns true
	bool mainIOLoop();
	// returns the request Bytes
	const Bytes& getRequest();
	// gets a pointer to a response object and handles it
	void handleResponse(Response*);
};

#endif
