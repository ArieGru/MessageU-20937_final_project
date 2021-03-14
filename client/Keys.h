#ifndef KEYS_HEADER
#define KEYS_HEADER

#include <rsa.h>
#include "Constants.h"

class Keys {
private:
	CryptoPP::RSA::PrivateKey privateKey;
	// find the number of characters from the begining of the file to the stasrt of the third line.
	// throws MyInfoError if there are less than three lines
	int keyOffsetInFile();
	// loads private key from info file to this object
	// if there is a problem reading the data will exit the program with error
	void loadPrivKey();
	// generates symmetric key
	char* generateKey(char*, size_t);
public:
	Keys(bool);
	// saves the private key into info file
	void savePrivKey();
	// returns Bytes with new symmetric key
	Bytes makeSymKey();
	// gets public key and Bytes, encrypts the Bytes using the public key and returns the encrypted Bytes
	Bytes encryptRSA(const Bytes&, const Bytes&);
	// gets encrypted Bytes, decrypt them using this object private key and returns the decrypted Bytes
	Bytes decryptRSA(const Bytes&);
	// generate and return public key using this object private key 
	Bytes getPubKey();
	// gets symmetric key and Bytes, encrypts the Bytes using the symmetric key
	//returns a pointer to new Bytes object with the encrypted Bytes
	Bytes* encryptAES(const Bytes&, const Bytes&);
	// gets symmetric key and encrypted Bytes, decrypts the Bytes using the symmetric key 
	// returns a pointer to new Bytes object with the decrypted Bytes
	Bytes* decryptAES(const Bytes&, const Bytes&);
	// checks if the private key in the info file is valid
	void checkPrivateKey();
};

#endif