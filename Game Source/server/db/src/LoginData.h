#pragma once
class CLoginData
{
public:
	CLoginData();

	TAccountTable& GetAccountRef();
	void            SetClientKey(const DWORD* c_pdwClientKey);

	const DWORD* GetClientKey();
	void            SetKey(DWORD dwKey);
	DWORD           GetKey();

	void            SetConnectedPeerHandle(DWORD dwHandle);
	DWORD		GetConnectedPeerHandle();

	void            SetLogonTime();
	DWORD		GetLogonTime();

	void		SetIP(const char* c_pszIP);
	const char* GetIP();

	void		SetPlay(bool bOn);
	bool		IsPlay();

	void		SetDeleted(bool bSet);
	bool		IsDeleted();

	time_t		GetLastPlayTime() { return m_lastPlayTime; }

	void            SetPremium(int* paiPremiumTimes);
	int             GetPremium(BYTE type);
	int* GetPremiumPtr();

	DWORD		GetLastPlayerID() const { return m_dwLastPlayerID; }
	void		SetLastPlayerID(DWORD id) { m_dwLastPlayerID = id; }

#ifdef __AUTH_LOGIN_TIMEOUT__
	time_t		GetAuthLoginTimeOut() { return time(0) - m_authLoginTime; }
	void		RefreshAuthLoginTime() { m_authLoginTime = time(0); }
#endif // __AUTH_LOGIN_TIMEOUT__

private:
	DWORD           m_dwKey;
	DWORD           m_adwClientKey[4];
	DWORD           m_dwConnectedPeerHandle;
	DWORD           m_dwLogonTime;
	char		m_szIP[MAX_HOST_LENGTH + 1];
	bool		m_bPlay;
	bool		m_bDeleted;

	time_t		m_lastPlayTime;
	int		m_aiPremiumTimes[PREMIUM_MAX_NUM];

	DWORD		m_dwLastPlayerID;

#ifdef __AUTH_LOGIN_TIMEOUT__
	time_t		m_authLoginTime;
#endif // __AUTH_LOGIN_TIMEOUT__

	TAccountTable   m_data;
};