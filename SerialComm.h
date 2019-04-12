#ifndef SERIALCOMM_H_
#define SERIALCOMM_H_

#include <windows.h>
#include <Setupapi.h>


class SerialComm{
private:
	HANDLE handle; // handle for the connection
	char* port; // COM port address


public:
	SerialComm(); // constructor

	// base function
	bool findPortbyPIDVID(const char* PID, const char* VID);
	void setPort(char* toSet, int num_len);
	bool connect();
	bool init_param();
	bool write(char* src, DWORD len);
	bool read(char* dest, DWORD len);
	bool close();

	bool single_cycle( char* toSent, unsigned short int len);

	// utility function
	static unsigned short int fletcher16(unsigned char *data, unsigned int len);
	static void USHORT2CHARARRAY(unsigned short int value, char* array);
	static unsigned short int CHARARRAY2USHORT(char* array);


};




#endif
