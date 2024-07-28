#pragma once
class CHwidManager : public singleton<CHwidManager>
{
public:
	enum ELoginStatus
	{
		HW_LOGIN_OK,
		HW_LOGIN_HWID_BANNED,
		HW_LOGIN_HDD_BANNED,
		HW_LOGIN_MACHINE_BANNED,
		HW_LOGIN_BIOS_BANNED,
	};

#ifdef __BAN_REASON_UTILITY__
	enum EProcess
	{
		PROCESS_ACCOUNT,
		PROCESS_MAC_ADDR,
		PROCESS_HDD_SERIAL,
		PROCESS_MACHINE_GUID,
		PROCESS_BIOS_ID,
	};
#endif

public:
	CHwidManager(void);
	virtual ~CHwidManager(void);

	int	IsAcceptableLogin(const std::string& hwid_key, const std::string& hdd_key, const std::string& machine_guid, const std::string& bios_id);
	bool BanByAccountData(std::string account);
	bool BanByAccountID(DWORD account_id);
	bool BanByPlayerData(std::string player);
	bool Ban(std::string hwid_key, std::string hdd_serial, std::string machine_guid, std::string bios_id);

protected:
	void	Destroy();

private:
	std::unordered_set<std::string> m_bannedHwids;
	std::unordered_set<std::string> m_bannedHdd;
	std::unordered_set<std::string> m_bannedMachine;
	std::unordered_set<std::string> m_bannedBios;

	int version;
protected:
	bool IsAcceptableHwid(const std::string& key) {return m_bannedHwids.find(key) == m_bannedHwids.end();}
	bool IsAcceptableHdd(const std::string& key) {return m_bannedHdd.find(key) == m_bannedHdd.end();}
	bool IsAcceptableMachine(const std::string& key) {return m_bannedMachine.find(key) == m_bannedMachine.end();}
	bool IsAcceptableBios(const std::string& key) {return m_bannedBios.find(key) == m_bannedBios.end();}
public:
	int ReloadBanData();
};
