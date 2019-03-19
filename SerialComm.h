#ifndef SERIALCOMM_H_
#define SERIALCOMM_H_

#include <windows.h>
#include <Setupapi.h>


class SerialComm{
private:
	HANDLE handle;
	char* port;
public:
	SerialComm();
	bool findPortbyPIDVID(char* PID, char* VID);
	bool connect();
	bool init_param();
	bool write(char* src, DWORD len);
	bool read(char* dest, DWORD len);
	bool close();

};




#endif
