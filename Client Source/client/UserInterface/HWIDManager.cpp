#include "HWIDManager.h"
#include "CMachineGuid.h"
#include "CHddData.h"
#include "CCpuId.h"
#include "CMacAddr.h"
#include "CBiosID.h"
#include "CComputerInfo.h"

#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/default.h>
#include <cryptopp/base64.h>

#include <XORstr.h>

HWIDMANAGER::HWIDMANAGER()
{
	m_MachineGUID = "";
	m_MacAddr = "";
	m_HDDSerial = "";
	m_HDDModel = "";
	m_CpuId = "";
	m_BiosID = "";
	m_ComputerName = "";
	m_UserName = "";
	m_osVersion = "";

	CMachineGuid	c_MachineGuid;
	CMacAddr		c_MacAddr;
	CHddData		c_HddData;
	CCpuId			c_CpuId;
	CBiosID			c_BiosID;
	CComputerInfo	c_ComputerInfo;

	m_MachineGUID = c_MachineGuid.getMachineGUID();
	m_MacAddr = c_MacAddr.getMacAddr();
	m_HDDSerial = c_HddData.getHDDSerialNumber();
	m_HDDModel = c_HddData.getHDDModelNumber();
	m_CpuId = c_CpuId.getCpuID();
	m_BiosID = c_BiosID.getBiosID();
	m_ComputerName = c_ComputerInfo.getComputerName();
	m_UserName = c_ComputerInfo.getUserName();
	m_osVersion = c_ComputerInfo.getOSVersion();
}


std::string HWIDMANAGER::EncryptString(const char* inputString)
{
	std::string decrypted(inputString);
	std::string chain;
	std::string hwid;
	CryptoPP::SHA256 tempHash;
	std::string encrypted;

	CHddData		c_HddData;

	// create
	chain.append(c_HddData.getHDDSerialNumber()).append(XOR("TG2_Kx12kl32ksa")).append(m_UserName);
	CryptoPP::StringSource nn(chain, true, new CryptoPP::HashFilter(tempHash, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(hwid), false)));
	CryptoPP::StringSource ss1(decrypted, true, new CryptoPP::DefaultEncryptorWithMAC((byte*)hwid.data(), hwid.size(), new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encrypted), false)));

	return encrypted;
}

std::string HWIDMANAGER::DecryptString(const char* inputString)
{
	try
	{
		std::string encrypted(inputString);
		std::string chain;
		std::string hwid;
		CryptoPP::SHA256 tempHash;
		std::string decrypted;

		CHddData		c_HddData;

		// create
		chain.append(c_HddData.getHDDSerialNumber()).append(XOR("TG2_Kx12kl32ksa")).append(m_UserName);
		CryptoPP::StringSource nn(chain, true, new CryptoPP::HashFilter(tempHash, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(hwid), false)));
		CryptoPP::StringSource ss1(encrypted, true, new CryptoPP::Base64Decoder(new CryptoPP::DefaultDecryptorWithMAC((byte*)hwid.data(), hwid.size(), new CryptoPP::StringSink(decrypted))));

		return decrypted;
	}
	catch (const CryptoPP::Exception)
	{
		return "Failed.";
	}

	return "...";
}