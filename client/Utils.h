#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <string>
#include "Constants.h"

// receives id in Bytes form (32 bytes) and returns it in Hex form (contains 32 characters)
std::string idToHex(const Bytes&);
// receives id in Hex form (represented by 32 characters) and returns it in Bytes form (16 Bytes)
Bytes idToBytes(const std::string&);
// returns user id from info file. if the file is missing returns empty Bytes the size of id_size
// exit the program with error if there is an error with reading file or with info size
Bytes getID();
// receives a pointer to char array, reads the next 4 bytes (int size) as little endian int and returns it
int bytesToInt(const unsigned char*);
// receives Bytes and returns a string representation of them
std::string vecToStr(const Bytes&);
// receives a string and returns a Bytes representation of it
Bytes strToVec(const std::string&);

#endif
