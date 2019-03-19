#include "SerialComm.h"
#include <stdio.h>
#include <string.h>


SerialComm::SerialComm(){
	this->handle = INVALID_HANDLE_VALUE;
	this->port = NULL;
}

bool SerialComm::findPortbyPIDVID(char* VID, char* PID){

		HDEVINFO DeviceInfoSet;
		DWORD DeviceIndex =0;
		SP_DEVINFO_DATA DeviceInfoData;
		PCSTR DevEnum = "USB";
		BYTE szBuffer[1024] = {0};
		DWORD   ulPropertyType;
		DWORD dwSize = 0;
		DWORD Error = 0;

		char expectedID[22];
		sprintf(expectedID, "USB\\VID_%s&PID_%s", VID, PID);


		//SetupDiGetClassDevs returns a handle to a device information set
		DeviceInfoSet = SetupDiGetClassDevs(
				NULL,
				DevEnum,
				NULL,
				DIGCF_ALLCLASSES | DIGCF_PRESENT);

		if (DeviceInfoSet == INVALID_HANDLE_VALUE){
			return 0;
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
//					MessageBox(NULL, (char*)szBuffer, "hw id", MB_OK);
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
//						MessageBox(NULL, buff, "SetupDiOpenDevRegKey", MB_OK);
						return 0;
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
//							MessageBox(NULL, pszPortName, "port name", MB_OK);
							// check if port name has COM in it.
							if(strstr(pszPortName,"COM") == NULL){
//								MessageBox(NULL, "port name does not have COM", "port name", MB_OK);
								return 0;
							}
							this->port = (char*)malloc(strlen(pszPortName)+sizeof(char));
							strcpy(this->port, pszPortName);
						}else{
							// show error code
							char buff [100];
							sprintf(buff, "RegQueryValueEx Failed: Error %lu", ret);
//							MessageBox(NULL, buff, "RegQueryValueEx", MB_OK);
							RegCloseKey(hDeviceRegistryKey);
							return 0;
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

bool SerialComm::connect(){
	if(this->port == NULL || this->handle != INVALID_HANDLE_VALUE){
		return 0;
	}

	this->handle = CreateFile(this->port,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);


	if(this->handle == INVALID_HANDLE_VALUE){
			return 0;
	}
	return 1;
}

bool SerialComm::init_param(){
	if(this->handle == INVALID_HANDLE_VALUE){
		return 0;
	}

	DCB dcbSerialCommParams = {0};
	if(!GetCommState(this->handle, &dcbSerialCommParams)){
		this->handle = INVALID_HANDLE_VALUE;
		return 0;
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
			return 0;
		}else{
			COMMTIMEOUTS timeouts = { 0 };
			timeouts.ReadIntervalTimeout = 50;
			timeouts.ReadTotalTimeoutConstant = 50;
			timeouts.ReadTotalTimeoutMultiplier = 10;
			timeouts.WriteTotalTimeoutConstant = 50;
			timeouts.WriteTotalTimeoutMultiplier = 10;
			if(!SetCommTimeouts(this->handle, &timeouts)){
				this->handle = INVALID_HANDLE_VALUE;
				return 0;
			}else{
				return PurgeComm(this->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
			}
		}
	}
}
bool SerialComm::write(char* src, DWORD len){
	if(this->handle == INVALID_HANDLE_VALUE){
		return 0;
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
		return 0;
	}
	if(dwBytesWritten != len){
		return 0;
	}

	return 1;
}
bool SerialComm::read(char* dest, DWORD len){
	if(this->handle == INVALID_HANDLE_VALUE){
		return 0;
	}
	DWORD dwBytesRead;
	if(!ReadFile(
			this->handle,
			(void*)dest,
			len,
			&dwBytesRead,
			NULL)){
		return 0;
	}
	if(len != dwBytesRead){
		return 0;
	}
	return 1;
}
bool SerialComm::close(){
	if(this->port != NULL){
		free(this->port);
		this->port = NULL;
	}
	bool ret = CloseHandle(this->handle);
	this->handle = INVALID_HANDLE_VALUE;
	return ret;
}

