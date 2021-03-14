#ifndef USERSLIST_HEADER
#define USERSLIST_HEADER

#include <string>
#include "Constants.h"

class UsersList {
private:
	class User {
	public:
		std::string uName;
		Bytes uId;
		Bytes uPubKey;
		Bytes uSymKey;
		// Constructor. makes a new user with name and id, with empty public and symmetric keys
		User(std::string, Bytes);
	};

	std::vector<User> *list;

public:
	UsersList();
	~UsersList();
	// receives name and id and checks if user is in this users list (by id). if not adds it to the list
	void addUser(const std::string&, const Bytes&);
	// receives user name and returns is id if it in the list. otherwise throws UserNotInList
	Bytes getUserID(const std::string&) const;
	// receives id and returns user name of the user with this id.
	std::string getUserName(const Bytes&);
	// prints users list
	void printList();
	// receives id and public key and adds the public key to the user with the given id. 
	// throws UserNotInList if there is no user with this id
	void addPubKey(const Bytes&, const Bytes&);
	// receives id and returns the public key of the user with this id.
	// throws UserNotInList if there is no user with this id
	Bytes getPubKey(const Bytes&);
	// receives id and a symmetric key and adds the symmetric key to the user with this id. 
	// throws UserNotInList if there is no user with this id
	void saveSymKey(const Bytes&, const Bytes&);
	// receives id and returns the symmetric key of the user with this id.
	// throws UserNotInList if there is no user with this id
	Bytes getSymKey(const Bytes&);
};

#endif