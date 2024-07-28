#ifndef _HWIDMANAGER_H____
#define _HWIDMANAGER_H____

#include <string>
#include "../eterBase/Singleton.h"

class HWIDMANAGER : public CSingleton < HWIDMANAGER >
{
public:
	HWIDMANAGER();
	std::string getMachineGUID() { return m_MachineGUID; };
	std::string getMacAddr() { return m_MacAddr; };
	std::string getHDDSerial() { return m_HDDSerial; };
	std::string getHDDModel() { return m_HDDModel; };
	std::string getCPUid() { return m_CpuId; };
	std::string getBiosId() { return m_BiosID; };
	std::string getComputerName() { return m_ComputerName; };
	std::string getUserName() { return m_UserName; };
	std::string getOSVersion() { return m_osVersion; };

	std::string EncryptString(const char* inputString);
	std::string DecryptString(const char* inputString);

private:
	std::string m_MachineGUID;
	std::string m_MacAddr;
	std::string m_HDDSerial;
	std::string m_HDDModel;
	std::string m_CpuId;
	std::string m_BiosID;
	std::string m_ComputerName;
	std::string m_UserName;
	std::string m_osVersion;
};

#endif // HWIDMANAGER