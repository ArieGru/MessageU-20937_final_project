#include <sstream>
#include <iostream>
#include "UsersList.h"
#include "MyExceptions.h"

// Constructor
UsersList::UsersList() {
	list = new std::vector<User>();
}

// Destructor
UsersList::~UsersList() {
	delete list;
}

// receives name and id and checks if user is in this users list (by id). if not adds it to the list
void UsersList::addUser(const std::string& name, const Bytes& id) {
	// check if user is in the list
	try {
		getUserName(id);
	}
	// if user is not in the list, add it
	catch (UserNotInList& e) {
		User newUser(name, id);
		list->push_back(newUser);
	}
}

// receives user name and returns is id if it in the list. otherwise throws UserNotInList
Bytes UsersList::getUserID(const std::string& name) const {
	std::vector<User>::iterator it = list->begin();
	while (it != list->end()) {
		if (it->uName == name)
			return it->uId;
		it++;
	}
	// error
	throw UserNotInList();
}

// receives id and returns user name of the user with this id.
std::string UsersList::getUserName(const Bytes& id) {
	std::vector<User>::iterator it = list->begin();
	while (it != list->end()) {
		if (it->uId == id)
			return it->uName;
		it++;
	}
	// error
	throw UserNotInList();
}

// prints users list
void UsersList::printList() {
	std::stringstream listBuf;
	listBuf << "\n\n--------USERS LIST FROM SERVER: --------\n" << std::endl;
	std::vector<User>::iterator it = list->begin();

	while (it != list->end()) {
		listBuf << it->uName << std::endl;
		it++;
	}
	listBuf << "\n\n" << std::endl;

	std::cout << listBuf.str();
}

// receives id and public key and adds the public key to the user with the given id. 
// throws UserNotInList if there is no user with this id
void UsersList::addPubKey(const Bytes& id, const Bytes& key) {
	std::vector<User>::iterator it = list->begin();
	while (it != list->end() && it->uId != id)
		it++;
	if (it == list->end())
		throw UserNotInList();
	else
		it->uPubKey = key;
}

// receives name and returns the public key of the user with this name.
// throws UserNotInList if there is no user with this name
Bytes UsersList::getPubKey(const Bytes& id)
{
	std::vector<User>::iterator it = list->begin();
	while (it != list->end()) {
		if (it->uId == id) {
			if (it->uPubKey.size() != PUB_KEY_SIZE)
				throw KeyNotAvailable();
			else
				return it->uPubKey;
		}
		it++;
	}
	// error
	throw UserNotInList();
}

// receives id and a symmetric key and adds the symmetric key to the user with this id. 
// throws UserNotInList if there is no user with this id
void UsersList::saveSymKey(const Bytes& id, const Bytes& key) {
	std::vector<User>::iterator it = list->begin();
	while (it != list->end() && it->uId != id)
		it++;
	if (it == list->end())
		throw UserNotInList();
	else
		it->uSymKey = key;
}

// receives id and returns the symmetric key of the user with this id.
// throws UserNotInList if there is no user with this id
Bytes UsersList::getSymKey(const Bytes& id) {
	std::vector<User>::iterator it = list->begin();
	while (it != list->end()) {
		if (it->uId == id) {
			if (it->uSymKey.size() != SYM_KEY_SIZE)
				throw KeyNotAvailable();
			else
				return it->uSymKey;
		}
		it++;
	}
	// error
	throw UserNotInList();
}

// Constructor. makes a new user with name and id, with empty public and symmetric keys
UsersList::User::User(std::string name, Bytes id) {
	uName = name;
	uId = id;
	uPubKey = Bytes();
	uSymKey = Bytes();
}

