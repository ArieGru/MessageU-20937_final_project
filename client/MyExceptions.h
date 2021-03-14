#ifndef MYEXCEPTIONS_HEADER
#define MYEXCEPTIONS_HEADER

#include <exception>

class UserNotInList : public std::exception {
public:
	const char* what() const throw() {
		return "User not in users list";
	}
};

class KeyNotAvailable : public std::exception {
public:
	const char* what() const throw() {
		return "Key is not available for this user.";
	}
};

class MyInfoError : public std::exception {
public:
	const char* what() const throw() {
		return "There is some error with your info.";
	}
};

class FileNotFound : public std::exception {
public:
	const char* what() const throw() {
		return "file not found";
	}
};

#endif