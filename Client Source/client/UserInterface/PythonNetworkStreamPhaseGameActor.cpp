#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "NetworkActorManager.h"
#include "PythonBackground.h"

#include "PythonApplication.h"
#include "AbstractPlayer.h"
#include "../gamelib/ActorInstance.h"

void CPythonNetworkStream::__GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY)
{
	CPythonBackground& rkBgMgr = CPythonBackground::Instance();
	rkBgMgr.GlobalPositionToLocalPosition(rGlobalX, rGlobalY);
}

void CPythonNetworkStream::__LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY)
{
	CPythonBackground& rkBgMgr = CPythonBackground::Instance();
	rkBgMgr.LocalPositionToGlobalPosition(rLocalX, rLocalY);
}

bool CPythonNetworkStream::__CanActMainInstance()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	return pkInstMain->CanAct();
}

void CPythonNetworkStream::__ClearNetworkActorManager()
{
	m_rokNetActorMgr->Destroy();
}

void __SetWeaponPower(IAbstractPlayer& rkPlayer, DWORD dwWeaponID)
{
	DWORD minPower = 0;
	DWORD maxPower = 0;
	DWORD minMagicPower = 0;
	DWORD maxMagicPower = 0;
	DWORD addPower = 0;

	CItemData* pkWeapon;
	if (CItemManager::Instance().GetItemDataPointer(dwWeaponID, &pkWeapon))
	{
		if (pkWeapon->GetType() == CItemData::ITEM_TYPE_WEAPON)
		{
			minPower = pkWeapon->GetValue(3);
			maxPower = pkWeapon->GetValue(4);
			minMagicPower = pkWeapon->GetValue(1);
			maxMagicPower = pkWeapon->GetValue(2);
			addPower = pkWeapon->GetValue(5);
		}
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		else if (pkWeapon->GetType() == CItemData::ITEM_TYPE_COSTUME && pkWeapon->GetSubType() == CItemData::COSTUME_WEAPON)
		{
			CItemData* pkRealWeapon;
			if (CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, c_Equipment_Weapon)), &pkRealWeapon))
			{
				minPower = pkRealWeapon->GetValue(3);
				maxPower = pkRealWeapon->GetValue(4);
				minMagicPower = pkRealWeapon->GetValue(1);
				maxMagicPower = pkRealWeapon->GetValue(2);
				addPower = pkRealWeapon->GetValue(5);
			}
		}
#endif
	}

	rkPlayer.SetWeaponPower(minPower, maxPower, minMagicPower, maxMagicPower, addPower);
}

bool IsInvisibleRace(WORD raceNum)
{
	switch (raceNum)
	{
	case 20025:
	case 20038:
	case 20039:
		return true;
	default:
		return false;
	}
}

static SNetworkActorData s_kNetActorData;

bool CPythonNetworkStream::RecvCharacterAppendPacket()
{
	TPacketGCCharacterAdd chrAddPacket;
	if (!Recv(sizeof(chrAddPacket), &chrAddPacket))
		return false;

	__GlobalPositionToLocalPosition(chrAddPacket.x, chrAddPacket.y);

	SNetworkActorData kNetActorData;
	kNetActorData.m_bType = chrAddPacket.bType;
	kNetActorData.m_wMovSpd = chrAddPacket.bMovingSpeed;
	kNetActorData.m_wAtkSpd = chrAddPacket.bAttackSpeed;
	kNetActorData.m_dwRace = chrAddPacket.wRaceNum;

	kNetActorData.m_dwStateFlags = chrAddPacket.bStateFlag;
	kNetActorData.m_dwVID = chrAddPacket.dwVID;
	kNetActorData.m_fRot = chrAddPacket.angle;

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	kNetActorData.m_bLevel = chrAddPacket.dwLevel;
	kNetActorData.m_dwAIFlag = chrAddPacket.dwAIFlag;
#else
	kNetActorData.m_bLevel = 0;
#endif // ENABLE_MOB_AGGR_LVL_INFO

	// position Info
	kNetActorData.SetPosition(chrAddPacket.x, chrAddPacket.y);

	// affectflags
	kNetActorData.m_kAffectFlags.CopyData(0, sizeof(chrAddPacket.dwAffectFlag[0]), &chrAddPacket.dwAffectFlag[0]);
	kNetActorData.m_kAffectFlags.CopyData(32, sizeof(chrAddPacket.dwAffectFlag[1]), &chrAddPacket.dwAffectFlag[1]);

	// fill zero
	kNetActorData.m_stName = "";
	kNetActorData.m_alignment = 0;
	kNetActorData.m_byPKMode = 0;
	kNetActorData.m_dwGuildID = 0;
	kNetActorData.m_bEmpireID = 0;
	kNetActorData.m_dwMountVnum = 0;
	kNetActorData.m_dwArmor = 0;
	kNetActorData.m_dwWeapon = 0;
	kNetActorData.m_dwHair = 0;
#ifdef ENABLE_ACCE_SYSTEM
	kNetActorData.m_dwAcce = 0;
	kNetActorData.m_dwAcceEffect = 0;
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	kNetActorData.m_dwAura = 0;
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
	kNetActorData.m_dwQuiver = 0;
#endif // ENABLE_QUIVER_SYSTEM
#ifdef ENABLE_RANK_SYSTEM
	kNetActorData.m_sRank = 0;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	kNetActorData.m_sLandRank = 0;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	kNetActorData.m_sReborn = 0;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	kNetActorData.m_sTeam = 0;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	kNetActorData.m_Moniker = "";
#endif
#ifdef ENABLE_LOVE_SYSTEM
	kNetActorData.m_Love1 = "";
	kNetActorData.m_Love2 = "";
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	kNetActorData.m_word = "";
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	kNetActorData.m_bMemberType = 0;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	kNetActorData.m_wArmorEvolution = 0;
	kNetActorData.m_wWeaponEvolution = 0;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memset(kNetActorData.m_dwSkillColor, 0, sizeof(kNetActorData.m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	memset(kNetActorData.m_dwShining, 0, sizeof(kNetActorData.m_dwShining));
#endif

	if (kNetActorData.m_bType != CActorInstance::TYPE_PC && kNetActorData.m_bType != CActorInstance::TYPE_NPC)
	{
		const char* c_szName;
		CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
		if (rkNonPlayer.GetName(kNetActorData.m_dwRace, &c_szName))
			kNetActorData.m_stName = c_szName;

		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
	{
		s_kNetActorData = kNetActorData;
	}

	return true;
}

bool CPythonNetworkStream::RecvCharacterAdditionalInfo()
{
	TPacketGCCharacterAdditionalInfo chrInfoPacket;
	if (!Recv(sizeof(chrInfoPacket), &chrInfoPacket))
		return false;

	SNetworkActorData kNetActorData = s_kNetActorData;
	if (IsInvisibleRace(kNetActorData.m_dwRace))
		return true;

	if (kNetActorData.m_dwVID == chrInfoPacket.vid)
	{
		kNetActorData.m_stName		= chrInfoPacket.name;
		kNetActorData.m_dwGuildID	= chrInfoPacket.dwGuildID;
		kNetActorData.m_bLevel		= chrInfoPacket.bLevel;
		kNetActorData.m_byPKMode	= chrInfoPacket.bPKMode;
		kNetActorData.m_dwGuildID	= chrInfoPacket.dwGuildID;
		kNetActorData.m_bEmpireID	= chrInfoPacket.bEmpire;
		kNetActorData.m_dwMountVnum	= chrInfoPacket.dwMountVnum;
		kNetActorData.m_alignment	= chrInfoPacket.alignment;
		kNetActorData.m_dwArmor = chrInfoPacket.parts[CHR_EQUIPPART_ARMOR];
		kNetActorData.m_dwWeapon = chrInfoPacket.parts[CHR_EQUIPPART_WEAPON];
		kNetActorData.m_dwHair = chrInfoPacket.parts[CHR_EQUIPPART_HAIR];
#ifdef ENABLE_ACCE_SYSTEM
		kNetActorData.m_dwAcce = chrInfoPacket.parts[CHR_EQUIPPART_ACCE];
		kNetActorData.m_dwAcceEffect = chrInfoPacket.parts[CHR_EQUIPPART_ACCE_EFFECT];
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
		kNetActorData.m_dwAura = chrInfoPacket.parts[CHR_EQUIPPART_AURA];
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
		kNetActorData.m_dwQuiver = chrInfoPacket.parts[CHR_EQUIPPART_ARROW];
#endif // ENABLE_QUIVER_SYSTEM
#ifdef ENABLE_RANK_SYSTEM
		kNetActorData.m_sRank		= chrInfoPacket.rank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		kNetActorData.m_sLandRank	= chrInfoPacket.land_rank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
		kNetActorData.m_sReborn		= chrInfoPacket.reborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
		kNetActorData.m_sTeam = chrInfoPacket.team;
#endif

#ifdef ENABLE_MONIKER_SYSTEM
		kNetActorData.m_Moniker = chrInfoPacket.m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
		kNetActorData.m_Love1 = chrInfoPacket.m_Love1;
		kNetActorData.m_Love2 = chrInfoPacket.m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		kNetActorData.m_word = chrInfoPacket.m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
		kNetActorData.m_bMemberType	= chrInfoPacket.guild_member_type;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		kNetActorData.m_wArmorEvolution = chrInfoPacket.itemEvolution[EVOLUTION_ARMOR];
		kNetActorData.m_wWeaponEvolution = chrInfoPacket.itemEvolution[EVOLUTION_WEAPON];
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		memcpy(kNetActorData.m_dwSkillColor, chrInfoPacket.dwSkillColor, sizeof(kNetActorData.m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
		memcpy(kNetActorData.m_dwShining, chrInfoPacket.adwShining, sizeof(kNetActorData.m_dwShining));
#endif

		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
	{
		TraceError("TPacketGCCharacterAdditionalInfo name=%s vid=%d race=%d Error", chrInfoPacket.name, chrInfoPacket.vid, kNetActorData.m_dwRace);
	}
	return true;
}

bool CPythonNetworkStream::RecvCharacterUpdatePacket()
{
	TPacketGCCharacterUpdate chrUpdatePacket;
	if (!Recv(sizeof(chrUpdatePacket), &chrUpdatePacket))
		return false;

	SNetworkUpdateActorData kNetUpdateActorData;

	kNetUpdateActorData.m_dwVID =chrUpdatePacket.vid;
	kNetUpdateActorData.m_dwArmor = chrUpdatePacket.parts[CHR_EQUIPPART_ARMOR];
	kNetUpdateActorData.m_dwWeapon = chrUpdatePacket.parts[CHR_EQUIPPART_WEAPON];
	kNetUpdateActorData.m_dwHair = chrUpdatePacket.parts[CHR_EQUIPPART_HAIR];
#ifdef ENABLE_ACCE_SYSTEM
	kNetUpdateActorData.m_dwAcce = chrUpdatePacket.parts[CHR_EQUIPPART_ACCE];
	kNetUpdateActorData.m_dwAcceEffect = chrUpdatePacket.parts[CHR_EQUIPPART_ACCE_EFFECT];
#endif // ENABLE_ACCE_SYSTEM
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	kNetUpdateActorData.m_dwAura = chrUpdatePacket.parts[CHR_EQUIPPART_AURA];
#endif // ENABLE_AURA_COSTUME_SYSTEM
#ifdef ENABLE_QUIVER_SYSTEM
	kNetUpdateActorData.m_dwQuiver = chrUpdatePacket.parts[CHR_EQUIPPART_ARROW];
#endif // ENABLE_QUIVER_SYSTEM
	kNetUpdateActorData.m_wMovSpd		= chrUpdatePacket.wMovingSpeed;
	kNetUpdateActorData.m_wAtkSpd		= chrUpdatePacket.wAttackSpeed;
	kNetUpdateActorData.m_dwStateFlags=chrUpdatePacket.bStateFlag;
	kNetUpdateActorData.m_kAffectFlags.CopyData(0, sizeof(chrUpdatePacket.dwAffectFlag[0]), &chrUpdatePacket.dwAffectFlag[0]);
	kNetUpdateActorData.m_kAffectFlags.CopyData(32, sizeof(chrUpdatePacket.dwAffectFlag[1]), &chrUpdatePacket.dwAffectFlag[1]);
	kNetUpdateActorData.m_dwGuildID		= chrUpdatePacket.dwGuildID;
	kNetUpdateActorData.m_byPKMode=chrUpdatePacket.bPKMode;
	kNetUpdateActorData.m_dwMountVnum=chrUpdatePacket.dwMountVnum;
	kNetUpdateActorData.m_bLevel=chrUpdatePacket.bLevel;
	kNetUpdateActorData.m_alignment=chrUpdatePacket.alignment;
#ifdef ENABLE_RANK_SYSTEM
	kNetUpdateActorData.m_sRank=chrUpdatePacket.rank;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	kNetUpdateActorData.m_sLandRank=chrUpdatePacket.land_rank;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	kNetUpdateActorData.m_sReborn=chrUpdatePacket.reborn;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	kNetUpdateActorData.m_sTeam = chrUpdatePacket.team;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	kNetUpdateActorData.m_Moniker = chrUpdatePacket.m_Moniker;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	kNetUpdateActorData.m_Love1 = chrUpdatePacket.m_Love1;
	kNetUpdateActorData.m_Love2 = chrUpdatePacket.m_Love2;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	kNetUpdateActorData.m_word = chrUpdatePacket.m_word;
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_GUILD_LEADER_SYSTEM
	kNetUpdateActorData.m_bMemberType=chrUpdatePacket.guild_member_type;
#endif
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	kNetUpdateActorData.m_wArmorEvolution = chrUpdatePacket.itemEvolution[EVOLUTION_ARMOR];
	kNetUpdateActorData.m_wWeaponEvolution = chrUpdatePacket.itemEvolution[EVOLUTION_WEAPON];
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memcpy(kNetUpdateActorData.m_dwSkillColor, chrUpdatePacket.dwSkillColor, sizeof(kNetUpdateActorData.m_dwSkillColor));
#endif
#ifdef ENABLE_SHINING_ITEM_SYSTEM
	memcpy(kNetUpdateActorData.m_dwShining, chrUpdatePacket.adwShining, sizeof(kNetUpdateActorData.m_dwShining));
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
	kNetUpdateActorData.m_hasAttacker = chrUpdatePacket.hasAttacker;
#endif // ENABLE_AUTO_HUNT_SYSTEM

	__RecvCharacterUpdatePacket(&kNetUpdateActorData);

	return true;
}


void CPythonNetworkStream::__RecvCharacterAppendPacket(SNetworkActorData* pkNetActorData)
{
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetActorData->m_dwVID))
	{
		rkPlayer.SetRace(pkNetActorData->m_dwRace);
		__SetWeaponPower(rkPlayer, pkNetActorData->m_dwWeapon); // TODO:

		if (rkPlayer.NEW_GetMainActorPtr())
		{
			CPythonBackground::Instance().Update(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY, 0.0f);
			CPythonCharacterManager::Instance().Update();
		}
		else
		{
			// TODO:
			__ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
		}
	}

	m_rokNetActorMgr->AppendActor(*pkNetActorData);

	if (GetMainActorVID() == pkNetActorData->m_dwVID)
	{
		rkPlayer.SetTarget(0);
		if (m_bComboSkillFlag)
			rkPlayer.SetComboSkillFlag(m_bComboSkillFlag);

		__SetGuildID(pkNetActorData->m_dwGuildID);
	}
}

void CPythonNetworkStream::__RecvCharacterUpdatePacket(SNetworkUpdateActorData* pkNetUpdateActorData)
{
	m_rokNetActorMgr->UpdateActor(*pkNetUpdateActorData);

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetUpdateActorData->m_dwVID))
	{
		__SetGuildID(pkNetUpdateActorData->m_dwGuildID);
		__SetWeaponPower(rkPlayer, pkNetUpdateActorData->m_dwWeapon); // TODO:

		__RefreshStatus();
		__RefreshAlignmentWindow();
		__RefreshEquipmentWindow();
		__RefreshInventoryWindow();
	}
	else
	{
		rkPlayer.NotifyCharacterUpdate(pkNetUpdateActorData->m_dwVID);
	}
}

bool CPythonNetworkStream::RecvCharacterDeletePacket()
{
	TPacketGCCharacterDelete chrDelPacket;

	if (!Recv(sizeof(chrDelPacket), &chrDelPacket))
	{
		TraceError("CPythonNetworkStream::RecvCharacterDeletePacket - Recv Error");
		return false;
	}

	m_rokNetActorMgr->RemoveActor(chrDelPacket.dwVID);
#ifndef ENABLE_OFFLINE_SHOP
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Disappear", Py_BuildValue("(i)", chrDelPacket.dwVID));
#endif
	return true;
}

bool CPythonNetworkStream::RecvCharacterMovePacket()
{
	TPacketGCMove kMovePacket;
	if (!Recv(sizeof(TPacketGCMove), &kMovePacket))
	{
		Tracen("CPythonNetworkStream::RecvCharacterMovePacket - PACKET READ ERROR");
		return false;
	}

	__GlobalPositionToLocalPosition(kMovePacket.lX, kMovePacket.lY);

	SNetworkMoveActorData kNetMoveActorData;
	kNetMoveActorData.m_dwArg = kMovePacket.bArg;
	kNetMoveActorData.m_dwFunc = kMovePacket.bFunc;
	kNetMoveActorData.m_dwTime = kMovePacket.dwTime;
	kNetMoveActorData.m_dwVID = kMovePacket.dwVID;
	kNetMoveActorData.m_fRot = kMovePacket.bRot * 5.0f;
	kNetMoveActorData.m_lPosX = kMovePacket.lX;
	kNetMoveActorData.m_lPosY = kMovePacket.lY;
	kNetMoveActorData.m_dwDuration = kMovePacket.dwDuration;
	m_rokNetActorMgr->MoveActor(kNetMoveActorData);
	return true;
}

bool CPythonNetworkStream::RecvOwnerShipPacket()
{
	TPacketGCOwnership kPacketOwnership;

	if (!Recv(sizeof(kPacketOwnership), &kPacketOwnership))
		return false;

	m_rokNetActorMgr->SetActorOwner(kPacketOwnership.dwOwnerVID, kPacketOwnership.dwVictimVID);
	return true;
}

bool CPythonNetworkStream::RecvSyncPositionPacket()
{
	TPacketGCSyncPosition kPacketSyncPos;
	if (!Recv(sizeof(kPacketSyncPos), &kPacketSyncPos))
		return false;

	TPacketGCSyncPositionElement kSyncPos;

	UINT uSyncPosCount = (kPacketSyncPos.wSize - sizeof(kPacketSyncPos)) / sizeof(kSyncPos);
	for (UINT iSyncPos = 0; iSyncPos < uSyncPosCount; ++iSyncPos)
	{
		if (!Recv(sizeof(TPacketGCSyncPositionElement), &kSyncPos))
			return false;

		__GlobalPositionToLocalPosition(kSyncPos.lX, kSyncPos.lY);
		m_rokNetActorMgr->SyncActor(kSyncPos.dwVID, kSyncPos.lX, kSyncPos.lY);
	}

	return true;
}