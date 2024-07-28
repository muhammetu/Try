#pragma once

#include "../EterLib/NetStream.h"
#include "MarkManager.h"

class CGuildMarkDownloader : public CNetworkStream, public CSingleton<CGuildMarkDownloader>
{
public:
	CGuildMarkDownloader();
	virtual ~CGuildMarkDownloader();

	bool Connect(const CNetworkAddress& c_rkNetAddr, DWORD dwHandle, DWORD dwRandomKey);

	void Process();

private:
	enum EGuildMarkDownloaderState
	{
		STATE_OFFLINE,
		STATE_LOGIN,
		STATE_COMPLETE,
	};

	enum EGuildMarkRecvType
	{
		TODO_RECV_NONE,
		TODO_RECV_MARK,
	};

private:
	void OnConnectFailure();
	void OnConnectSuccess();
	void OnRemoteDisconnect();
	void OnDisconnect();

	void __Initialize();
	bool __StateProcess();

	UINT __GetPacketSize(UINT header);
	bool __DispatchPacket(UINT header);

	void __OfflineState_Set();
	void __CompleteState_Set();

	void __LoginState_Set();
	bool __LoginState_Process();
	bool __LoginState_RecvPhase();
	bool __LoginState_RecvHandshake();
	bool __LoginState_RecvPing();
	bool __LoginState_RecvMarkIndex();
	bool __LoginState_RecvMarkBlock();
#ifdef ENABLE_IMPROVED_PACKET_ENCRYPTION
	bool __LoginState_RecvKeyAgreement();
	bool __LoginState_RecvKeyAgreementCompleted();
#endif
	bool __SendMarkIDXList();
	bool __SendMarkCRCList();

private:
	DWORD m_dwHandle;
	DWORD m_dwRandomKey;
	DWORD m_dwTodo;

	std::vector<DWORD> m_kVec_dwGuildID;

	UINT m_eState;

	BYTE m_currentRequestingImageIndex;

	CGuildMarkManager* m_pkMarkMgr;

	DWORD m_dwBlockIndex;
	DWORD m_dwBlockDataSize;
	DWORD m_dwBlockDataPos;
};
