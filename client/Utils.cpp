#include <sstream>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include "Utils.h"
#include "MyExceptions.h"

// receives id in Bytes form (32 bytes) and returns it in Hex form (contains 32 characters)
std::string idToHex(const Bytes& idBytes) {
	std::stringstream buf;
	for (size_t i = 0; i < idBytes.size(); i++) {
		// add leading zero if necessary
		if ((int)idBytes.at(i) < 0x10)
			buf << '0';
		std::string a;
		buf << std::hex << (int)idBytes.at(i);
	}
	return buf.str();
}

// receives id in Hex form (represented by 32 characters) and returns it in Bytes form (16 Bytes)
Bytes idToBytes(const std::string& idHex) {
	Bytes id;

	char tmpArr[3];
	tmpArr[2] = '\0';
	for (int i = 0; i < ID_SIZE; i++) {
		int tmpIndx = i * 2;
		tmpArr[0] = idHex.at(tmpIndx);
		tmpArr[1] = idHex.at(tmpIndx + 1);
		id.push_back((unsigned char)strtol(tmpArr, NULL, 16));
	}

	return id;
}

// returns user id from info file. if the file is missing returns empty Bytes the size of id_size
// exit the program with error if there is an error with reading file or with info size
Bytes getID() {
	Bytes id;
	std::string userName;
	std::string userID;
	// check if file exists
	if (!boost::filesystem::exists(MY_INFO_FILE))
		return Bytes(ID_SIZE);
	// try reading id from file. if there is an error reading the file print error message and exit the program
	try {
		std::ifstream myInfoFile(MY_INFO_FILE);
		// read first line into user name string
		std::getline(myInfoFile, userName);
		// read second line, which contains the ID
		std::getline(myInfoFile, userID);
		myInfoFile.close();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
	// check if the user id size is correct. if not exit the program
	if (userID.size() != ID_HEX_SIZE) {
		std::cout << "There is some error with your info." << std::endl;
		exit(1);
	}

	return idToBytes(userID);
}

// receives a pointer to char array, reads the next 4 bytes (int size) as little endian int and returns it
int bytesToInt(const unsigned char *ptr) {
	int ret = 0;
	ret = (unsigned char)ptr[3] << 24;
	ret += (unsigned char)ptr[2] << 16;
	ret += (unsigned char)ptr[1] << 8;
	ret += (unsigned char)ptr[0];
	return ret;
}

// receives Bytes and returns a string representation of them
std::string vecToStr(const Bytes& vec) {
	std::string str;

	for (size_t i = 0; i < vec.size(); i++)
		str += vec.at(i);
	return str;
}

// receives a string and returns a Bytes representation of it
Bytes strToVec(const std::string& str) {
	Bytes vec;
	for (size_t i = 0; i < str.size(); i++)
		vec.push_back(str.at(i));
	return vec;
}