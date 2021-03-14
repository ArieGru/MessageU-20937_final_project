#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "Handler.h"
#include "MyExceptions.h"
#include "Utils.h"

// Constructor
Handler::Handler() {
	request = new Request();
	usersList = new UsersList();
	keys = new Keys(isInfoFileExists());
}

// Destructor
Handler::~Handler() {
	delete request;
	delete usersList;
	delete keys;
}

// main io loop. responsible for the interaction with the user.
// returns false if the user want to quit. otherwise builds the request and returns true
bool Handler::mainIOLoop() {
	int selection;	// user selection
	bool validInput = false;
	// check info file exists and valid.
	bool registered = isInfoFileExists();
	if(registered)
		checkInfo();

	// loop while the input is invalid or the request builders failed
	while (!validInput) {
		printMenu();
		selection = getSelection();
		switch (selection) {
		case(SEL_EXIT_CLIENT):
			return false;
		case(INVALID_INPUT):
			std::cout << "Input is invalid" << std::endl;
			break;
		case(SEL_REG):
			if (registered) {
				std::cout << "You're already registered to the server" << std::endl;
				break;
			}
			else {
				validInput = buildRequest(selection);
				break;
			}
		default:
			if (!registered) {
				std::cout << "You need to be registered to the server fitst" << std::endl;
				break;
			}
			else
				validInput = buildRequest(selection);
		}
	}
	// the request is ready, return true
	return true;
}

// returns the request Bytes
const Bytes& Handler::getRequest() {
	return request->getRequest();
}

// prints the menu
void Handler::printMenu() {
	std::cout << std::endl;
	std::cout << "MessageU client at your service" << std::endl << std::endl;
	std::cout << "1) Register" << std::endl;
	std::cout << "2) Request for clients list" << std::endl;
	std::cout << "3) Request for public key" << std::endl;
	std::cout << "4) Request for waiting messages" << std::endl;
	std::cout << "5) Send a text message" << std::endl;
	std::cout << "50) Send a file" << std::endl;
	std::cout << "51) Send a request for symmetric key" << std::endl;
	std::cout << "52) Send your symmetric key" << std::endl;
	std::cout << "0) Exit client" << std::endl;
	std::cout << "?" << std::endl;
}

// read user selection and returns it. if the input isn't valid returns INVALID_INPUT
int Handler::getSelection() {
	std::string userInput;
	std::getline(std::cin, userInput);

	// check if user input is empty line
	if (userInput.size() == 0)
		return INVALID_INPUT;
	// check if user input contains only digits
	for(size_t i = 0; i < userInput.size(); i++)
		if(!std::isdigit(userInput.at(i)))
			return INVALID_INPUT;

	// convert input to int
	int selection = atoi(userInput.c_str());

	// returns selection if it valid, INVALID_INPUT otherwise
	switch (selection) {
	case(SEL_REG):
	case(SEL_GET_ULIST):
	case(SEL_GET_PUB_KEY):
	case(SEL_GET_MSG):
	case(SEL_SEND_MSG):
	case(SEL_SEND_FILE):
	case(SEL_GET_SYM_KEY):
	case(SEL_SEND_SYM_KEY):
	case(SEL_EXIT_CLIENT):
		return selection;
	default:
		return INVALID_INPUT;
	}
}

// checks if the info file exists
bool Handler::isInfoFileExists() {
	return boost::filesystem::exists(MY_INFO_FILE);
}

// exits the program if there is a problem with info file or with reading it
void Handler::checkInfo() {
	// if there is a problem with the data the following functions should exit the program with error
	getID();
	keys->checkPrivateKey();
}

// build the request from the given selection. returns true if all went ok, false otherwise.
bool Handler::buildRequest(int selection) {
	// build request header
	request->buildHeader(getID(), getReqCode(selection));

	// build the request payload
	switch (selection) {
	case(SEL_REG): {
		myUserName = getUserName();
		request->buildRegRequest(strToVec(myUserName), keys->getPubKey());
		return true;
	}
	case(SEL_GET_ULIST): {
		request->noPayloadRequest();
		return true;
	}
	case(SEL_GET_PUB_KEY): {
		request->pushInt(ID_SIZE);
		std::string userName = getUserName();
		try {
			// need to check if there is public key for the protocol?
			Bytes userID = usersList->getUserID(userName);
			request->buildGetPubKeyRequest(userID);
			return true;
		}
		catch (std::exception &e) {
			std::cout << e.what() << std::endl;
			return false;
		}
	}
	case(SEL_GET_MSG): {
		request->noPayloadRequest();
		return true;
	}
	case(SEL_SEND_MSG): 
	case(SEL_SEND_FILE):
	case(SEL_GET_SYM_KEY):
	case(SEL_SEND_SYM_KEY): {
		std::string userName;
		Bytes userID;
		Bytes *encMsg = new Bytes();
		int plSize;
		int msgSize;
		unsigned char msgType;

		// get destination user name
		userName = getUserName();
		try {
			// get user id. will throw error if user isn't in users list
			userID = usersList->getUserID(userName);

			switch (selection) {
			case(SEL_SEND_MSG):
				msgType = MSG_TYPE_SEND_MSG;
				encMsg = keys->encryptAES(usersList->getSymKey(userID), strToVec(getMessage()));
				break;
			case(SEL_SEND_FILE):
				msgType = MSG_TYPE_SEND_FILE;
				encMsg = keys->encryptAES(usersList->getSymKey(userID), loadFile(getFilePath()));
				break;
			case(SEL_GET_SYM_KEY):
				msgType = MSG_TYPE_GET_SYM_KEY;
				break;
			case(SEL_SEND_SYM_KEY):
				msgType = MSG_TYPE_SEND_SYM_KEY;
				usersList->saveSymKey(userID, keys->makeSymKey());	// make new symetric key and save it.
				encMsg = new Bytes(keys->encryptRSA(usersList->getPubKey(userID), usersList->getSymKey(userID)));
				break;
			default:
				return false;
			}	// end switch
			
			msgSize = encMsg->size();
			plSize = ID_SIZE + 1 + sizeof(int) + msgSize;
			request->buildSendMsgRequest(plSize, userID, msgType, msgSize, *encMsg);
			delete encMsg;
			return true;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return false;
		}

	}	// end send message case
	default:
		return false;

	}	// end switch
}

// returns request code of the given user selection
unsigned char Handler::getReqCode(int selection) {
	switch (selection) {
	case(SEL_REG):
		return REQ_CODE_REQ;
	case(SEL_GET_ULIST):
		return REQ_CODE_GET_ULIST;
	case(SEL_GET_PUB_KEY):
		return REQ_CODE_GET_PUBKEY;
	case(SEL_GET_MSG):
		return REQ_CODE_GET_MSG;
	case(SEL_SEND_MSG):
	case(SEL_SEND_FILE):
	case(SEL_GET_SYM_KEY):
	case(SEL_SEND_SYM_KEY):
		return REQ_CODE_SEND_MSG;
	default:
		return INVALID_INPUT;
	}
}

// get user name from user and returns it as string.
std::string Handler::getUserName() {
	std::string uName;
	bool validInput = false;
	// loop while the input isn't valid. input should be between 1 to 255 (includsive) characters long
	while (!validInput) {
		std::cout << "Please enter user name: " << std::endl;
		std::getline(std::cin, uName);
		if (uName.size() == 0 || uName.size() > MAX_USERNAME_LEN) {
			std::cout << "User name size isn't valid" << std::endl;
		}
		else {
			validInput = true;
		}
	}
	return uName;
}

// get message from user and returns it as string
std::string Handler::getMessage() {
	std::string msg;
	std::cout << "Please enter yout message: " << std::endl;
	std::getline(std::cin, msg);
	return msg;
}

// saves user name and id to info file
void Handler::saveInfo(const Bytes& responsePayload) {
	std::fstream infoFile(MY_INFO_FILE, std::fstream::out | std::fstream::app);
	infoFile << myUserName << '\n' <<  idToHex(responsePayload) << '\n';
	infoFile.close();
	keys->savePrivKey();
}

// retrieves public key and user id from the given response payload and saves it
void Handler::addPubKey(const Bytes& responsePayload) {
	Bytes idVec;
	for (int i = 0; i < ID_SIZE; i++)
		idVec.push_back(responsePayload.at(i));
	Bytes pubKey;
	for (size_t i = ID_SIZE; i < responsePayload.size(); i++) {
		unsigned char t = responsePayload.at(i);
		pubKey.push_back(t);
	}
	std::cout << std::endl;
		
	usersList->addPubKey(idVec, pubKey);
}

// gets a pointer to a response object and handles it
void Handler::handleResponse(Response* response) {
	switch (response->getResponseCode()) {
	case(RES_CODE_ERROR):
		if (request->getCode()) {
			std::cout << "User name is already taken, please try again" << std::endl;
		}
		else
			std::cout << "error from server" << std::endl;
		break;
	case(RES_CODE_REG_SUC):
		try {
			saveInfo(response->getPayload());
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl << "Can't save your info, you're not registered" << std::endl;
			// if file was created but there was some error in the proccess, delete it
			if(boost::filesystem::exists(MY_INFO_FILE))
				remove(MY_INFO_FILE);
		}
		break;
	case(RES_CODE_ULIST_RES):
		makeUsersList(response->getPayload());
		usersList->printList();
		break;
	case(RES_CODE_PUB_KEY):
		addPubKey(response->getPayload());
		break;
	case(RES_CODE_MSG_SENT):
		//std::cout << "Message sent" << std::endl;
		break;
	case(RES_CODE_RECEIVE_MSG):
		printMessages(response->getPayload());
	}
}

// retrieves users list from response payload and saves it
void Handler::makeUsersList(const Bytes& responsePayload) {
	int numOfUsers = responsePayload.size() / (ID_SIZE + MAX_USERNAME_LEN);

	for (int i = 0; i < numOfUsers; i++) {
		size_t startIndex = i * (ID_SIZE + MAX_USERNAME_LEN);
		size_t curIndex = startIndex;

		// copy the id from the payload (in bytes form) to a vector
		Bytes idVec;
		while (curIndex < startIndex + ID_SIZE)
			idVec.push_back(responsePayload.at(curIndex++));

		std::string userName;

		// copy user name to a string
		while (curIndex < responsePayload.size() && responsePayload.at(curIndex))
			userName += responsePayload.at(curIndex++);

		// add user to userslist.
		usersList->addUser(userName, idVec);
	}
}

// retrieves messages from the given response payload and prints them
void Handler::printMessages(const Bytes& responsePayload) {
	size_t index = 0;
	while (index < responsePayload.size()) {
		Bytes orig;
		size_t start = index;
		while(index < start + ID_SIZE)
			orig.push_back(responsePayload.at(index++));
		// skip 4 Bytes, message id
		index += 4;
		int msgType = responsePayload.at(index++);
		// read message size into temp variable
		std::string tmp;
		for (int i = 0; i < 4; i++)
			tmp += responsePayload.at(index++);
		// convert temp variable to integer in this machines endianess
		int msgSize = bytesToInt((const unsigned char*)tmp.c_str());
		// transfer message content to Bytes variable. 
		Bytes content;
		for (int i = 0; i < msgSize; i++) {
			content.push_back(responsePayload.at(index++));
		}
		try {
			std::cout << "from:  " << std::endl;
			std::cout << usersList->getUserName(orig) << std::endl;
			std::cout << "content:" << std::endl;

			switch (msgType) {
			case(MSG_TYPE_GET_SYM_KEY): {
				std::cout << GET_SYM_KEY_MSG << std::endl;
				break;
			}
			case(MSG_TYPE_SEND_SYM_KEY): {
				usersList->saveSymKey(orig, keys->decryptRSA(content));
				std::cout << SEND_SYM_KEY_MSG << std::endl;
				break;
			}
			case(MSG_TYPE_SEND_MSG): {
				Bytes key = usersList->getSymKey(orig);
				Bytes* decMsg = keys->decryptAES(key, content);
				std::cout << vecToStr(*decMsg) << std::endl;
				delete decMsg;
				break;
			}
			case(MSG_TYPE_SEND_FILE): {
				Bytes key = usersList->getSymKey(orig);
				try {
					Bytes* decMsg = keys->decryptAES(key, content);
					std::cout << saveFile(*decMsg) << std::endl;
					delete decMsg;
				}
				catch (std::exception& e) {
					std::cout << "can't save file to disk" << std::endl << e.what() << std::endl;
				}
			}
			}
		}
		catch (KeyNotAvailable& e) {
			std::cout << CANT_DECRYPT_MSG << std::endl;
		}
		// if the user isn't in your users list we will won't try to read it
		catch (UserNotInList& e) {
			std::cout << CANT_DECRYPT_MSG << std::endl;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	
		std::cout << std::endl << "-----<EOM>-----" << std::endl << std::endl;;
	}
}

// request a file path from user and returns it as string
std::string Handler::getFilePath() { 
	std::string msg;
	std::cout << "Please enter file's full path: " << std::endl;
	std::getline(std::cin, msg);
	return msg;
}

// receive a string to a path and Bytes pointer and loads the file into Bytes.
Bytes Handler::loadFile(const std::string& filePath) {
	std::streampos fileSize;
	if (!boost::filesystem::exists(filePath))
		throw FileNotFound();
	std::ifstream file(filePath, std::ios::binary);
	// get its size:
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	// read the data:
	Bytes fileBytes((unsigned int)fileSize);
	file.read((char*)&fileBytes[0], fileSize);
	file.close();
	return fileBytes;
}

// saves a file in Bytes form to disk. returns full path to this file as a string
std::string Handler::saveFile(const Bytes& fileBytes) {
	std::string tmpDir =  + "%TMP%";
	// check if %TMP% directory exists, if not create it
	if(!boost::filesystem::exists(tmpDir))
		boost::filesystem::create_directories(tmpDir);
	// make a string of the full new file path
	std::string path = boost::filesystem::current_path().string() + '\\' +
						 tmpDir + '\\' + makeRandomString();
	std::ofstream fout(path, std::ios::out | std::ios::binary);
	fout.write((char*)&fileBytes[0], fileBytes.size() * sizeof(char));
	fout.close();

	return path;
}

// generates and returns random string RANDOM_FILENAME_LEN long
std::string Handler::makeRandomString() {
	std::string ret;
	std::string chars(
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"1234567890");
	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
	// makes random file name with RANDOM_FILENAME_LEN characters
	for (int i = 0; i < RANDOM_FILENAME_LEN; ++i) {
		ret += chars[index_dist(rng)];
	}
	return ret;
}
