#include <osrng.h>
#include <base64.h>
#include <files.h>
#include <aes.h>
#include <modes.h>
#include <immintrin.h>	// _rdrand32_step

#include "keys.h"
#include "MyExceptions.h"
// Constructor
Keys::Keys(bool infoFileExists) {
	if (infoFileExists) {
		loadPrivKey();
	}
	else {
		CryptoPP::AutoSeededRandomPool rng;
		privateKey.Initialize(rng, PUB_KEY_BITS);
	}
}

// saves the private key into info file
void Keys::savePrivKey() {
	std::fstream infoFile(MY_INFO_FILE, std::fstream::out | std::fstream::app);
	CryptoPP::Base64Encoder privkeysink(new CryptoPP::FileSink(infoFile));

	privateKey.DEREncode(privkeysink);
	privkeysink.MessageEnd(); 
}

// gets public key and Bytes, encrypts the Bytes using the public key and returns the encrypted Bytes
Bytes Keys::encryptRSA(const Bytes& pubKey, const Bytes& orig) {

	Bytes cipher;
	CryptoPP::AutoSeededRandomPool rng;
	//Read public key
	CryptoPP::ByteQueue bytes;
	CryptoPP::VectorSource vs(pubKey, true);
	vs.TransferTo(bytes);
	bytes.MessageEnd();
	CryptoPP::RSA::PublicKey uPubKey;

	uPubKey.Load(bytes);
	
	CryptoPP::RSAES_OAEP_SHA_Encryptor e(uPubKey);
	CryptoPP::VectorSource ss2(orig, true, new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::VectorSink(cipher)));

	return cipher;
}

// gets encrypted Bytes, decrypt them using this object private key and returns the decrypted Bytes
Bytes Keys::decryptRSA(const Bytes& cipher) {
	CryptoPP::AutoSeededRandomPool rng;

	Bytes decrypted;
	CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
	CryptoPP::VectorSource ss(cipher, true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::VectorSink(decrypted)));

	return decrypted;
}

// generate and return public key using this object private key 
Bytes Keys::getPubKey() {
	Bytes pKey;

	CryptoPP::RSAFunction pubKey(privateKey);
	CryptoPP::VectorSink vs(pKey);
	pubKey.Save(vs);

	return pKey;
}

// generates symmetric key
char* Keys::generateKey(char* buff, size_t size) {
	for (size_t i = 0; i < size; i += 4)
		_rdrand32_step(reinterpret_cast<unsigned int*>(&buff[i]));
	return buff;
}

// returns Bytes with new symmetric key
Bytes Keys::makeSymKey() {
	CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];

	memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

	generateKey(reinterpret_cast<char*>(key), CryptoPP::AES::DEFAULT_KEYLENGTH);

	Bytes vec;

	for (int i = 0; i < CryptoPP::AES::DEFAULT_KEYLENGTH; i++)
		vec.push_back(key[i]);

	return vec;
}

// gets symmetric key and Bytes, encrypts the Bytes using the symmetric key
//returns a pointer to new Bytes object with the encrypted Bytes
Bytes* Keys::encryptAES(const Bytes& symKey, const Bytes& orig) {
	Bytes* cipher = new Bytes();
	CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];
	
	memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

	for (int i = 0; i < CryptoPP::AES::DEFAULT_KEYLENGTH; i++)
		key[i] = symKey.at(i);

	CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::VectorSink(*cipher));
	stfEncryptor.Put(&(orig[0]), orig.size());
	stfEncryptor.MessageEnd();

	return cipher;
}

// gets symmetric key and encrypted Bytes, decrypts the Bytes using the symmetric key 
// returns a pointer to new Bytes object with the decrypted Bytes
Bytes* Keys::decryptAES(const Bytes& symKey, const Bytes& cipher) {
	Bytes* decrypted = new Bytes();
	CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];

	memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

	for (int i = 0; i < CryptoPP::AES::DEFAULT_KEYLENGTH; i++)
		key[i] = symKey.at(i);

	
	CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::VectorSink(*decrypted));
	stfDecryptor.Put(&(cipher[0]), cipher.size());
	stfDecryptor.MessageEnd();

	return decrypted;
}

// checks if the private key in the info file is valid
void Keys::checkPrivateKey() { 
	loadPrivKey();
}

// find the number of characters from the begining of the file to the stasrt of the third line.
// throws MyInfoError if there are less than three lines
int Keys::keyOffsetInFile() {
	int counter = 0;
	int lineNum = 0;
	std::ifstream file(MY_INFO_FILE);
	while (lineNum < 2) {
		char c = (char)file.get();
		if (file.eof())
			throw MyInfoError();
		if (c == '\n')
			lineNum++;
		counter++;
	}
	file.close();
	return counter;
}

// loads private key from info file to this object
// if there is a problem reading the data will exit the program with error
void Keys::loadPrivKey() {
	try {
		CryptoPP::AutoSeededRandomPool rng;
		CryptoPP::ByteQueue bytes;
		CryptoPP::FileSource file(MY_INFO_FILE, false);
		file.Pump(keyOffsetInFile());
		file.Attach(new CryptoPP::Base64Decoder);
		file.PumpAll();
		file.TransferTo(bytes);
		bytes.MessageEnd();
		privateKey.Load(bytes);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
}