#pragma once

#include <string>

class CComputerInfo
{
public:
	CComputerInfo();
	~CComputerInfo();

	std::string getComputerName();
	std::string getUserName();
	std::string getOSVersion();
};