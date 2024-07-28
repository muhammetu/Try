#pragma once
class CBanManager : public singleton<CBanManager>
{
public:
	enum EBanType
	{
		E_BAN_TYPE_ACCOUNT,
		E_BAN_TYPE_CHAT,
		E_BAN_TYPE_MAX,
	};

	enum EStrType
	{
		E_STR_TYPE_ACCOUNT,
		E_STR_TYPE_PLAYER,
		E_STR_TYPE_MAX,
	};

	enum EBanConsts
	{
		E_AVAIL_DT_PERMA = 99,
	};

public:
	CBanManager(void);
	virtual ~CBanManager(void);

	bool	PrepareBan(BYTE bBanType, BYTE bStrType, std::string selectedStr, DWORD availDt, std::string banReason, std::string banEvidence);
	std::string	GetAccountNameByAccountID(DWORD dwAccountID);
	bool	BanByAccountName(std::string account_name, DWORD availDt, std::string banReason, std::string banEvidence);
	bool	BanByAccountID(int account_id, DWORD availDt, std::string banReason, std::string player_name, std::string banEvidence);
	bool	BanByPlayerName(std::string player_name, DWORD availDt, std::string banReason, std::string banEvidence);
	bool	ChatBanByPlayerName(std::string player_name, DWORD availDt);

protected:
	void	Destroy();
};