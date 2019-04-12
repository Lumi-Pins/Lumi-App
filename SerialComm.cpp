/*
 * File: SerialComm.cpp
 * Description: for methods in SerialComm class
 * Author: Zelin Liu
 * Date: 3/10/2019
 * Eclipse Luna 4.4.0
 * MinGW GNU BinUtils 2.28-1, Installation Manager0.6.3, WSL 5.1
 *
 * */

#include "SerialComm.h"
#include <stdio.h>
#include <string.h>

/*
 * This is the constructor for SerialComm class
 * Parameter: none
 * Return: none
 * */
SerialComm::SerialComm(){
	this->handle = INVALID_HANDLE_VALUE;
	this->port = NULL;
}

/*
 * This function sets COM port from COMport number
 * Parameter: the COM port number, the length of the number
 * Return: none
 * */
void SerialComm::setPort(char* toSet, int num_len){
	if(num_len==1){
		this->port = (char*)realloc(this->port ,(4+num_len)*sizeof(char));
		sprintf(this->port, "COM%s", toSet);
	}else if (num_len == 2 || num_len ==3){
		this->port = (char*)realloc(this->port ,(8+num_len)*sizeof(char));
		sprintf(this->port, "COM%s", toSet);
	}else{
		return;
	}
	return;

}

/*
 * This function finds the COM port a device is connected on by the device vendor ID and product ID
 * Parameter: Vendor ID and Product ID
 * Return: true on success and false on failure
 * */
bool SerialComm::findPortbyPIDVID(const char* VID, const char* PID){

	HDEVINFO DeviceInfoSet;
	DWORD DeviceIndex =0;
	SP_DEVINFO_DATA DeviceInfoData;
	PCSTR DevEnum = "USB";
	BYTE szBuffer[1024] = {0};
	DWORD   ulPropertyType;
	DWORD dwSize = 0;
	//		DWORD Error = 0;

	char expectedID[22];
	sprintf(expectedID, "USB\\VID_%s&PID_%s", VID, PID);


	//SetupDiGetClassDevs returns a handle to a device information set
	DeviceInfoSet = SetupDiGetClassDevs(
			NULL,
			DevEnum,
			NULL,
			DIGCF_ALLCLASSES | DIGCF_PRESENT);

	if (DeviceInfoSet == INVALID_HANDLE_VALUE){
		return false;
	}

	//Fills a block of memory with zeros
	ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);



	//Receive information about an enumerated device
	while (SetupDiEnumDeviceInfo(
			DeviceInfoSet,
			DeviceIndex,
			&DeviceInfoData))
	{
		DeviceIndex++;


		//Retrieves a specified Plug and Play device property
		if (SetupDiGetDeviceRegistryProperty (DeviceInfoSet,
				&DeviceInfoData,
				SPDRP_HARDWAREID,
				&ulPropertyType,
				(BYTE*)szBuffer,
				sizeof(szBuffer),   // The size, in bytes
				&dwSize))
		{
			if(strstr((char*)szBuffer, expectedID)!=NULL){
				HKEY hDeviceRegistryKey;

				hDeviceRegistryKey = SetupDiOpenDevRegKey(DeviceInfoSet,
						&DeviceInfoData,
						DICS_FLAG_GLOBAL,
						0,
						DIREG_DEV,
						KEY_READ);
				if (hDeviceRegistryKey == INVALID_HANDLE_VALUE)
				{
					// show error code
					DWORD ERRORc = (DWORD)GetLastError();
					char buff [100];
					sprintf(buff, "SetupDiOpenDevRegKey Failed: Error %lu", ERRORc);
					return false;
				}
				else
				{
					// Read in the name of the port
					char pszPortName[30];
					DWORD dwSize = sizeof(pszPortName);
					DWORD dwType = 0;

					DWORD ret = RegQueryValueEx(hDeviceRegistryKey,
							"PortName",
							NULL,
							&dwType,
							(LPBYTE) pszPortName,
							&dwSize);

					if( ret == ERROR_SUCCESS)
					{
						// check if port name has COM in it.
						if(strstr(pszPortName,"COM") == NULL){
							return false;
						}
						this->port = (char*)realloc(this->port, (strlen(pszPortName)+1)*sizeof(char));
						strcpy(this->port, pszPortName);
					}else{
						// show error code
						char buff [100];
						sprintf(buff, "RegQueryValueEx Failed: Error %lu", ret);
						MessageBox(NULL, buff, "RegQueryValueEx", MB_OK);
						RegCloseKey(hDeviceRegistryKey);
						return false;
					}
				}
				RegCloseKey(hDeviceRegistryKey);
			}
		}

	}

	if (DeviceInfoSet)
	{
		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
	}
	return 1;
}

/*
 * This function attempts to establish serial connection to current port number
 * Parameter: none
 * Return: true on success false on failure
 * */
bool SerialComm::connect(){
	if(this->port == NULL || this->handle != INVALID_HANDLE_VALUE){
		return false;
	}

	this->handle = CreateFile(this->port,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);


	if(this->handle == INVALID_HANDLE_VALUE){
		return false;
	}
	return 1;
}

/*
 * This function sets some connection parameters
 * Parameter: none
 * Return: true on success false on failure
 * */
bool SerialComm::init_param(){
	if(this->handle == INVALID_HANDLE_VALUE){
		return false;
	}

	DCB dcbSerialCommParams = {0};
	if(!GetCommState(this->handle, &dcbSerialCommParams)){
		this->handle = INVALID_HANDLE_VALUE;
		return false;
	}else{
		// https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_dcb
		dcbSerialCommParams.BaudRate = CBR_9600;
		dcbSerialCommParams.ByteSize = 8;
		dcbSerialCommParams.StopBits = ONESTOPBIT;
		dcbSerialCommParams.Parity = NOPARITY;
		dcbSerialCommParams.fDtrControl = DTR_CONTROL_DISABLE; // not sure?
		// copy pasta
		dcbSerialCommParams.fBinary       = TRUE;
		dcbSerialCommParams.fParity       = FALSE;
		dcbSerialCommParams.fOutxCtsFlow  = FALSE; // TODO: TRUE?
		dcbSerialCommParams.fOutxDsrFlow  = FALSE;
		dcbSerialCommParams.fDsrSensitivity = FALSE; // TODO: should this be TRUE
		dcbSerialCommParams.fNull         = FALSE;
		dcbSerialCommParams.fOutX         = FALSE;
		dcbSerialCommParams.fInX          = FALSE;
		dcbSerialCommParams.fRtsControl   = RTS_CONTROL_DISABLE; // TODO: enable?
		dcbSerialCommParams.fAbortOnError = TRUE;
		// end copy pasta

		if(!SetCommState(this->handle, &dcbSerialCommParams)){
			this->handle = INVALID_HANDLE_VALUE;
			return false;
		}else{
			COMMTIMEOUTS timeouts = { 0 };
			if(!SetCommTimeouts(this->handle, &timeouts)){
				this->handle = INVALID_HANDLE_VALUE;
				return false;
			}

			timeouts.ReadIntervalTimeout = 100;
			timeouts.ReadTotalTimeoutConstant = 300;
			timeouts.ReadTotalTimeoutMultiplier = 10;
			timeouts.WriteTotalTimeoutConstant = 10;
			timeouts.WriteTotalTimeoutMultiplier = 10;
			if(!SetCommTimeouts(this->handle, &timeouts)){
				this->handle = INVALID_HANDLE_VALUE;
				return false;
			}

			return PurgeComm(this->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
		}
	}
}

/*
 * This function writes to current connection
 * Parameter: pointer to source address, length to write
 * Return: true on success, false on failure
 * */
bool SerialComm::write(char* src, DWORD len){
	// check for connection
	if(this->handle == INVALID_HANDLE_VALUE){
		return false;
	}

	DWORD dwBytesWritten;

	// write buffer to file
	bool write_result = WriteFile(
			this->handle,           // open file handle
			(void*)src,      // start of data to write
			len,  // number of bytes to write
			&dwBytesWritten, // number of bytes that were written
			NULL);            // no overlapped structure
	if(!write_result){
		return false;
	}
	if(dwBytesWritten != len){
		return false;
	}

	return true;
}

/*
 * This function reads from the current connection
 * Parameter: pointer to the destination of read, length to be read
 * Return: true on success, false on failure
 * */
bool SerialComm::read(char* dest, DWORD len){
	// test connection existance
	if(this->handle == INVALID_HANDLE_VALUE){
		MessageBox(NULL, "No connection opened", "Lumi-pins SyncData", MB_OK);
		return false;
	}

	DWORD dwBytesRead;
	if(!ReadFile(
			this->handle, // handle to connection
			(void*)dest, // destination
			len, // expected len
			&dwBytesRead, // actual len
			NULL)){

		MessageBox(NULL, "ReadFile() failed", "Lumi-pins SyncData", MB_OK);
		return false;
	}
	if(len != dwBytesRead){
		MessageBox(NULL, "ReadFile() failed, wrong length", "Lumi-pins SyncData", MB_OK);
		return false;
	}
	return true;
}

/*
 * This function closes the current connection
 * Parameter: none
 * Return: true on success, false on failure
 * */
bool SerialComm::close(){
	// free port if not free
	if(this->port != NULL){
		free(this->port);
		this->port = NULL;
	}
	// close connection
	bool ret = CloseHandle(this->handle);
	this->handle = INVALID_HANDLE_VALUE;
	return ret;
}


/*
 * This executes a single cycle of data transfer by send data, and receive checksum, then compare
 * Parameter: pointer to the source of data to be sent, length of the data
 * Return: true on success and the data sent is not corrupt, false otherwise
 * */
bool SerialComm::single_cycle(char* toSent, unsigned short int len){
	// computes checksum
	unsigned short int checksum = this->fletcher16((unsigned char*)toSent, (unsigned int)len);
	// send data
	if(!this->write(toSent, len)){
		return false; // if writing failed, abort?
	}
	// expect checksum
	char toRead[2];
	if(!this->read(toRead, 2)){
		//			return false;
		MessageBox(NULL, "No data read from Arduino", "Lumi-pins SyncData", MB_OK);
		return false;
	}
	// convert checksum to unsigned short int
	unsigned short int checksum_received = this->CHARARRAY2USHORT(toRead);
	// compare
	if(checksum == checksum_received){
		return true; // cycle complete
	}
	char buff[30];
	sprintf(buff, "checksum failed %d != %d, %d", checksum, checksum_received, len);
	MessageBox(NULL, buff, "Lumi-pins SyncData", MB_OK);

	return false;
}

/*
 * This function computers the fletcher16 checksum
 * Parameter: pointer to the data whose checksum is to be determined, length of the data
 * Return: the 2-byte size checksum
 * */
unsigned short int SerialComm::fletcher16(unsigned char *data, unsigned int len)
{

	unsigned short sum1 = 0;
	unsigned short sum2 = 0;
	for(unsigned short i =0; i < len; ++i){
		sum1 = (sum1 + data[i]) % 255;
		sum2 = (sum2 + sum1) % 255;
	}
	return (sum2 << 8)| sum1;
}


/*
 * This function converts a unsigned short int into a char array size of 2
 * Parameter: the unsigned short int number, and the address of destination array
 * Return: none
 * */
void SerialComm::USHORT2CHARARRAY(unsigned short int value, char* array){
	array[0] = (value >> 8) & 0xFF;
	array[1] = value & 0xFF;
}

/*
 * This function turns 2 char from a char array into a unsigned short int
 * Parameter: the address of the source char
 * Return: the unsigned short int value
 * */
unsigned short int SerialComm::CHARARRAY2USHORT(char* array){
	return (unsigned short)(
			((unsigned char)array[0]) << 8 |
			((unsigned char)array[1]));
}
