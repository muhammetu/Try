#include "stdafx.h"
#ifdef __GROWTH_MOUNT_SYSTEM__
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "New_MountSystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"
#include "db.h"
#include "questmanager.h"
#include "New_PetSystem.h"

EVENTINFO(newmountsystem_event_info)
{
	CNewMountSystem* pMountSystem;
};

EVENTINFO(newmountsystem_event_infoe)
{
	CNewMountSystem* pMountSystem;
};

EVENTFUNC(newmountsystem_update_event)
{
	newmountsystem_event_info* info = dynamic_cast<newmountsystem_event_info*>(event->info);
	if (info == NULL)
	{
		sys_err("newmountsystem_update_event> <Factor> NULL pointer");
		return 0;
	}

	CNewMountSystem* pMountSystem = info->pMountSystem;

	if (NULL == pMountSystem)
		return 0;

	pMountSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}

EVENTFUNC(newmountsystem_expire_event)
{
	newmountsystem_event_infoe* info = dynamic_cast<newmountsystem_event_infoe*>(event->info);
	if (info == NULL)
	{
		sys_err("newmountsystem_expire_event> <Factor> NULL pointer");
		return 0;
	}

	CNewMountSystem* pMountSystem = info->pMountSystem;

	if (NULL == pMountSystem)
		return 0;

	pMountSystem->UpdateTime();
	return PASSES_PER_SEC(1);
}

const float MOUNT_COUNT_LIMIT = 3;

CNewMountActor::CNewMountActor(LPCHARACTER owner, DWORD vnum, DWORD options)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_dwlevel = 1;
	m_dwlevelstep = 0;
	m_dwExpFromMob = 0;
	m_dwExpFromItem = 0;
	m_dwexp = 0;
	m_dwexpitem = 0;
	m_dwOptions = options;
	m_dwLastActionTime = 0;

	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;

	m_dwSummonItemVID = 0;
	m_dwSummonItemID = 0;
	m_dwSummonItemVnum = 0;

	m_dwevolution = 0;
	m_dwduration = 0;
	m_dwtduration = 0;
	m_dwAgeDuration = 0;

	m_dwTimeMount = 0;
	m_dwslotimm = 0;
	m_dwImmTime = 0;

	m_dwskill[0] = 0;
	m_dwskill[1] = 0;
	m_dwskill[2] = 0;

	for (int s = 0; s < 25; ++s) {
		m_dwmountslotitem[s] = -1;
	}

	//Riferimento allo slot -1 se non disp 0 disp non set > 0 setted
	m_dwskillslot[0] = -1;
	m_dwskillslot[0] = -1;
	m_dwskillslot[0] = -1;

	for (int x = 0; x < 3; ++x) //Inizializzazione bonus del mount
	{
		int btype[3] = { APPLY_ATTBONUS_MONSTER, APPLY_ATTBONUS_STONE, APPLY_ATT_GRADE_BONUS };
		m_dwbonusmount[x][0] = btype[x];
		m_dwbonusmount[x][1] = 0;
	}

	m_dwMountType = 0;

	m_dwArtis = 0;
}

CNewMountActor::~CNewMountActor()
{
	this->Unsummon();
	m_pkOwner = 0;
}

void CNewMountActor::SetMountType(int mounttype)
{
	m_dwMountType = mounttype;
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem)
	{
		pSummonItem->SetForceAttribute(1, mounttype, m_dwbonusmount[1][1]);
	}
}

void CNewMountActor::SetName(const char* name)
{
	std::string mountName = "";

	if (0 != m_pkOwner &&
		0 == name &&
		0 != m_pkOwner->GetName())
	{
		mountName += "'s Mount";
	}
	else
		mountName += name;

	if (true == IsSummoned())
		m_pkChar->SetName(mountName);

	m_name = mountName;
}

void CNewMountActor::SetItemCube(int pos, int invpos) {
	if (m_dwmountslotitem[pos] != -1) //Controllo se l'item e' gia settato
		return;
	if (pos > 180 || pos < 0)
		return;

	m_dwmountslotitem[pos] = invpos;
}

void CNewMountActor::ItemCubeFeed(int type)
{
	for (int i = 0; i < 25; i++) {
		if (m_dwmountslotitem[i] != -1) {
			LPITEM itemxp = m_pkOwner->GetInventoryItem(m_dwmountslotitem[i]);
			if (!itemxp)
				continue;

#ifdef __BEGINNER_ITEM__
			if (itemxp->IsBasicItem())
			{
				m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_BASIC_ITEM"));
				continue;
			}
#endif

#ifdef __SLOT_MARKING__
			if (IS_SET(itemxp->GetAntiFlag(), ITEM_ANTIFLAG_PETFEED))
			{
				m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("WARNING_ANTIFLAG_MOUNTFEED"));
				continue;
			}
#endif

			if (itemxp->GetID() == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetID())
				continue;

			if (quest::CQuestManager::instance().GetPCForce(m_pkOwner->GetPlayerID())->IsRunning() == true)
				continue;

			if (!m_pkOwner->CanAct(true, true, VAR_TIME_NONE, nullptr, 0))
				continue;

			if (type == 1)
			{
				continue;
			}
			else if (type == 3)
			{
				if (itemxp->GetType() == 1 || itemxp->GetType() == 2)
				{
					if (GetNextExpFromItem() - GetExpI() > 0)
					{
						//SetExp(itemxp->GetShopBuyPrice() / 2, 1);
						ITEM_MANAGER::instance().RemoveItem(itemxp);
					}
					else
						m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_NO_MORE_ITEM_EXP"));
				}
				else if (itemxp->GetVnum() == 55221)
				{
					if (GetNextExpFromItem() - GetExpI() > 0)
					{
						SetExp(itemxp->GetShopBuyPrice() / 10, 1);
						ITEM_MANAGER::instance().RemoveItem(itemxp);
					}
					else
						m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("MOUNT_NO_MORE_ITEM_EXP"));
				}
			}
		}
	}

	for (int s = 0; s < 25; ++s) {
		m_dwmountslotitem[s] = -1;
	}
}

bool CNewMountActor::ResetSkill(int skill) {
	if (m_dwskillslot[skill] > 0)
	{
		m_dwskillslot[skill] = 0;
		m_dwskill[skill] = 0;
		return true;
	}
	return false;
}

bool CNewMountActor::IncreaseMountSkill(int skill) {
	if (GetSkillCount() == 0 && GetLevel() < 40)
		return false;
	else if (GetSkillCount() == 1 && GetLevel() < 60)
		return false;
	else if (GetSkillCount() == 2 && GetLevel() < 80)
		return false;

	for (int i = 0; i < 3; ++i) { //Itero gli slot per cercare la skill
		if (m_dwskillslot[i] == skill) {  //Se trova la skill o la aumenta oppure e' gi?max
			if (m_dwskill[i] < 20) {
				m_dwskill[i] += 1;
				m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Beceri duzeyine ulasti:%d"), m_dwskill[i]);
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", i, m_dwskillslot[i], m_dwskill[i]);
				return true;
			}
			else {
				m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Beceri zaten maksimum seviyeye ulasti!"));
				return false;
			}
		}
	}

	for (int i = 0; i < 3; ++i) {
		if (m_dwskillslot[i] == 0) { //Controllo se trovo uno slot vuoto abilitato
			m_dwskillslot[i] = skill;
			m_dwskill[i] = 1;
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Evcil hayvaniniz yeni bir beceri ogrendi!"));
			m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", i, m_dwskillslot[i], m_dwskill[i]);
			return true;
		}
	}

	/* Qualora il mount non soddisfi le condizioni precedenti
	   Allora tutti gli slot sono pieni e quind non pu?
	   imparare nuove skill
	*/
	m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Evcil hayvaninizin diger becerileri ogrenmek mumkun degil!"));
	return false;
}

bool CNewMountActor::IncreaseMountEvolution() {
	if (m_dwevolution < 3) {
		if ((GetLevel() == 40 && m_dwevolution < 1) || (GetLevel() == 60 && m_dwevolution < 2) || (GetLevel() == 80 && m_dwevolution < 3)) {
			m_dwevolution += 1;
			m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountEvolution %d", m_dwevolution);
			if (m_dwevolution == 3) {
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 0, m_dwskillslot[0], m_dwskill[0]);
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 1, m_dwskillslot[1], m_dwskill[1]);
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 2, m_dwskillslot[2], m_dwskill[2]);
			}
		}
		else
			return false;
	}
	else {
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Evcil hayvan maksimum evrime ulasti!"));
		return false;
	}
	return true;
}

void CNewMountActor::IncreaseMountBonus() {
	int tmplevel = GetLevel();
	if (m_dwMountType == 0)
	{
		DWORD dwArtis = number(1, 5);

		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 8 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}
	else if (m_dwMountType == 1) // sabit
	{
		DWORD dwArtis = number(2, 3);
	
		if (tmplevel % 5 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}
	else if (m_dwMountType == 2)
	{
		DWORD dwArtis = number(1, 7);
	
		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 8 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}
	else if (m_dwMountType == 3) // sabit
	{
		DWORD dwArtis = number(3, 4);

		if (tmplevel % 5 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}
	else if (m_dwMountType == 4)
	{
		DWORD dwArtis = number(1, 9);

		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 8 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}
	else if (m_dwMountType == 5) // sabit
	{
		DWORD dwArtis = number(5, 6);

		if (tmplevel % 5 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}
	else if (m_dwMountType == 6)
	{
		DWORD dwArtis = number(1, 10);

		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 8 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}
	else if (m_dwMountType == 7) // sabit
	{
		DWORD dwArtis = number(5, 8);

		if (tmplevel % 5 == 0)
		{
			m_dwbonusmount[0][1] += dwArtis;
			m_dwbonusmount[2][1] += dwArtis;
		}

		if (tmplevel % 6 == 0)
		{
			m_dwbonusmount[1][1] += dwArtis;
		}
	}

	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountBonus %d %d %d", m_dwbonusmount[0][1], m_dwbonusmount[1][1], m_dwbonusmount[2][1]);
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
	{
		pSummonItem->SetForceAttribute(0, m_dwlevel, m_dwbonusmount[0][1]);
		pSummonItem->SetForceAttribute(1, m_dwMountType, m_dwbonusmount[1][1]);
		pSummonItem->SetForceAttribute(2, m_dwArtis, m_dwbonusmount[2][1]);
	}
}

void CNewMountActor::SetNextExp(int nextExp)
{
	m_dwExpFromMob = (nextExp / 10) * 9;
	m_dwExpFromItem = nextExp - m_dwExpFromMob;
}

void CNewMountActor::SetLevel(DWORD level) // oc mount level fixi
{
	if (GetLevel() >= 250)
		return;
	m_pkChar->SetLevel(static_cast<char>(level));
	m_dwlevel = level;
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
	{
		pSummonItem->SetForceAttribute(0, level, m_dwbonusmount[0][1]);
	}
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountLevel %d", m_dwlevel);
	SetNextExp(m_pkChar->GetMountNextExp());
	m_pkChar->ViewReencode();
}

void CNewMountActor::SetEvolution(int lv) {
	if (lv == 40)
		m_dwevolution = 1;
	else if (lv == 60)
		m_dwevolution = 2;
	else if (lv == 80)
		m_dwevolution = 3;
}

void CNewMountActor::SetArtis(int val)
{
	m_dwArtis = val;

	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
	{
		pSummonItem->SetForceAttribute(2, m_dwArtis, m_dwbonusmount[2][1]);
	}
}

bool CNewMountActor::SetExp(DWORD exp, int mode)
{
	if (GetLevel() >= 250)
		return false;
	if (exp > UINT_MAX)
		exp = MAX(m_dwexp - exp, 0);
	if (mode == 0) {
		if (GetExp() + exp >= GetNextExpFromMob() && GetExpI() >= GetNextExpFromItem())
		{
			if (GetEvolution() == 0 && GetLevel() == 40)
				return false;
			else if (GetEvolution() == 1 && GetLevel() == 60)
				return false;
			else if (GetEvolution() == 2 && GetLevel() == 80)
				return false;
		}
	}
	else if (mode == 1)
	{
		if (GetExpI() + exp >= GetNextExpFromItem() && GetExp() >= GetNextExpFromMob())
		{
			if (GetEvolution() == 0 && GetLevel() == 40)
				return false;
			else if (GetEvolution() == 1 && GetLevel() == 60)
				return false;
			else if (GetEvolution() == 2 && GetLevel() == 80)
				return false;
		}
	}

	if (mode == 0) {
		if (GetExp() + exp >= GetNextExpFromMob()) {
			if (GetExpI() >= GetNextExpFromItem()) {
				SetLevel(GetLevel() + 1);
				//m_pkChar->UpdatePacket();
				m_pkChar->SendPetLevelUpEffect(m_pkChar->GetVID(), 1, GetLevel(), 1);
				IncreaseMountBonus();
				m_dwlevelstep = 0;
				m_dwexp = 0;
				m_dwexpitem = 0;
				m_pkChar->SetExp(0);
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountExp %d %d %d", m_dwexp, m_dwexpitem, m_pkChar->GetMountNextExp());
				//SetEvolution(GetLevel());
				return true;
			}
			else {
				//m_pkChar->SendPetLevelUpEffect(m_pkChar->GetVID(), 25, GetLevel(), 1);
				m_dwlevelstep = 4;
				exp = GetNextExpFromMob() - GetExp();
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountExp %d %d %d", m_dwexp, m_dwexpitem, m_pkChar->GetMountNextExp());
			}
		}
		m_dwexp += exp;
		m_pkChar->SetExp(m_dwexp);
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountExp %d %d %d", m_dwexp, m_dwexpitem, m_pkChar->GetMountNextExp());
		if (GetLevelStep() < 4) {
			if (GetExp() >= GetNextExpFromMob() / 4 * 3 && m_dwlevelstep != 3) {
				m_dwlevelstep = 3;
				//m_pkChar->SendPetLevelUpEffect(m_pkChar->GetVID(), 25, GetLevel(), 1);
			}
			else if (GetExp() >= GetNextExpFromMob() / 4 * 2 && m_dwlevelstep != 2) {
				m_dwlevelstep = 2;
				//m_pkChar->SendPetLevelUpEffect(m_pkChar->GetVID(), 25, GetLevel(), 1);
			}
			else if (GetExp() >= GetNextExpFromMob() / 4 && m_dwlevelstep != 1) {
				m_dwlevelstep = 1;
				//m_pkChar->SendPetLevelUpEffect(m_pkChar->GetVID(), 25, GetLevel(), 1);
			}
		}
	}
	else if (mode == 1) {
		if (GetExpI() + exp >= GetNextExpFromItem()) {
			if (GetExp() >= GetNextExpFromMob()) { //Set anche exp da mob e' piena livello e conservo exp restante
				m_dwexpitem = GetExpI() + exp - GetNextExpFromItem(); //Nel caso in cui livelli setta exp in eccesso 0 se ==
				m_dwexp = 0;
				m_pkChar->SetExp(0);
				m_dwlevelstep = 0;
				SetLevel(GetLevel() + 1);
				//m_pkChar->UpdatePacket();
				m_pkChar->SendPetLevelUpEffect(m_pkChar->GetVID(), 1, GetLevel(), 1);
				IncreaseMountBonus();
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountExp %d %d %d", m_dwexp, m_dwexpitem, m_pkChar->GetMountNextExp());
				//SetEvolution(GetLevel());
				return true;
			}
			else {
				exp = GetNextExpFromItem() - GetExpI();
				m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountExp %d %d %d", m_dwexp, m_dwexpitem, m_pkChar->GetMountNextExp());
			}
		}
		m_dwexpitem += exp;
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountExp %d %d %d", m_dwexp, m_dwexpitem, m_pkChar->GetMountNextExp());
	}

	return true;
}

bool CNewMountActor::Mount()
{
	if (0 == m_pkOwner)
		return false;

	if (true == HasOption(EMountOption_Mountable))
		m_pkOwner->MountVnum(m_dwVnum);

	return m_pkOwner->GetMountVnum() == m_dwVnum;;
}

void CNewMountActor::UpdateTime()
{
	m_dwTimeMount += 1;
	if (m_dwTimeMount >= 60)
	{
		m_dwduration -= 1;
		m_dwTimeMount = 0;
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
		if (pSummonItem != NULL)
		{
			pSummonItem->SetSocket(2, m_dwduration);
			pSummonItem->SetSocket(3, m_dwtduration);
			pSummonItem->SetForceAttribute(2, m_dwArtis, m_dwbonusmount[2][1]);
		}
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountDuration %d %d", m_dwduration, m_dwtduration);
	}
}

void CNewMountActor::Unmount()
{
	if (0 == m_pkOwner)
		return;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
}

void CNewMountActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
		if (pSummonItem != NULL)
		{
			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE new_mountsystem SET level = %d, evolution=%d, exp=%d, expi=%d, bonus0=%d, bonus1=%d, bonus2=%d, skill0=%d, skill0lv= %d, skill1=%d, skill1lv= %d, skill2=%d, skill2lv= %d, duration=%d, tduration=%d, mount_type=%d, artis=%d WHERE id = '%u';", this->GetLevel(), this->m_dwevolution, this->GetExp(), this->GetExpI(), this->m_dwbonusmount[0][1], this->m_dwbonusmount[1][1], this->m_dwbonusmount[2][1], this->m_dwskillslot[0], this->m_dwskill[0], this->m_dwskillslot[1], this->m_dwskill[1], this->m_dwskillslot[2], this->m_dwskill[2], this->m_dwduration, this->m_dwtduration, this->m_dwMountType, this->m_dwArtis, ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetID()));
			this->ClearBuff();

			// Mount Bonuslari (value (0,2))
			pSummonItem->SetForceAttribute(0, m_dwlevel, m_dwbonusmount[0][1]);
			pSummonItem->SetForceAttribute(1, m_dwMountType, m_dwbonusmount[1][1]);
			pSummonItem->SetForceAttribute(2, m_dwArtis, m_dwbonusmount[2][1]);

			// Mount Skill Typeleri (type (3,5))
			pSummonItem->SetForceAttribute(3, m_dwskillslot[0], m_dwskill[0]);
			pSummonItem->SetForceAttribute(4, m_dwskillslot[1], m_dwskill[1]);
			pSummonItem->SetForceAttribute(5, m_dwskillslot[2], m_dwskill[2]);

			// Mount Evolution (type (6))
			pSummonItem->SetForceAttribute(6, 1, m_dwevolution);

			// Mount Age Duration
			pSummonItem->SetSocket(1, m_dwAgeDuration);

			// Mount Duration
			pSummonItem->SetSocket(2, m_dwduration);

			// Mount Total Duration
			pSummonItem->SetSocket(3, m_dwtduration);

			// Lock
			pSummonItem->Lock(false);
		}
		this->SetSummonItem(NULL);

		if (NULL != m_pkOwner)
			m_pkOwner->ComputePoints();

		if (NULL != m_pkChar)
			M2_DESTROY_CHARACTER(m_pkChar);

		m_pkChar = 0;
		m_dwVID = 0;
		m_dwlevel = 1;
		m_dwlevelstep = 0;
		m_dwExpFromMob = 0;
		m_dwExpFromItem = 0;
		m_dwexp = 0;
		m_dwexpitem = 0;
		m_dwTimeMount = 0;
		m_dwImmTime = 0;
		m_dwslotimm = 0;
		m_dwArtis = 0;

		for (int s = 0; s < 25; ++s) {
			m_dwmountslotitem[s] = -1;
		}
		ClearBuff();
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountUnsummon");
	}
}

DWORD CNewMountActor::Summon(const char* mountName, LPITEM pSummonItem, bool bSpawnFar)
{
	long x = m_pkOwner->GetX();
	long y = m_pkOwner->GetY();
	long z = m_pkOwner->GetZ();

	if (true == bSpawnFar)
	{
		x += (number(0, 1) * 2 - 1) * number(2000, 2500);
		y += (number(0, 1) * 2 - 1) * number(2000, 2500);
	}
	else
	{
		x += number(-100, 100);
		y += number(-100, 100);
	}

	if (0 != m_pkChar)
	{
		m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkChar->GetVID();

		return m_dwVID;
	}

	m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(
		m_dwVnum,
		m_pkOwner->GetMapIndex(),
		x, y, z,
		false, (int)(m_pkOwner->GetRotation() + 180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CNewMount::Summon] Failed to summon the mount. (vnum: %d)", m_dwVnum);
		return 0;
	}

	m_pkChar->SetNewMount();

	m_pkChar->SetEmpire(m_pkOwner->GetEmpire());

	m_dwVID = m_pkChar->GetVID();

	// fix
	ClearBuff();
#ifdef __GROWTH_PET_SYSTEM__
	if (m_pkOwner->GetNewPetSystem())
		m_pkOwner->GetNewPetSystem()->RefreshBuff();
#endif // __GROWTH_PET_SYSTEM__

	char szQuery1[1024];
	snprintf(szQuery1, sizeof(szQuery1), "SELECT name,level,exp,expi,bonus0,bonus1,bonus2,skill0,skill0lv,skill1,skill1lv,skill2,skill2lv,duration,tduration,evolution,ageduration,mount_type,artis FROM new_mountsystem WHERE id = '%u';", pSummonItem->GetID());
	std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
	if (pmsg2->Get()->uiNumRows > 0) {
		MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
		this->SetName(row[0]);
		this->SetLevel(atoi(row[1]));
		this->m_dwexp = atoi(row[2]);
		this->m_dwexpitem = atoi(row[3]);
		this->m_dwbonusmount[0][1] = atoi(row[4]);
		this->m_dwbonusmount[1][1] = atoi(row[5]);
		this->m_dwbonusmount[2][1] = atoi(row[6]);
		this->m_dwskillslot[0] = atoi(row[7]);
		this->m_dwskill[0] = atoi(row[8]);
		this->m_dwskillslot[1] = atoi(row[9]);
		this->m_dwskill[1] = atoi(row[10]);
		this->m_dwskillslot[2] = atoi(row[11]);
		this->m_dwskill[2] = atoi(row[12]);
		this->m_dwduration = atoi(row[13]);
		this->m_dwtduration = atoi(row[14]);
		this->m_dwevolution = atoi(row[15]);
		this->m_dwAgeDuration = atoi(row[16]);
		this->m_dwMountType = atoi(row[17]);
		this->m_dwArtis = atoi(row[18]);
	}
	else
		this->SetName(mountName);

	this->SetSummonItem(pSummonItem);

	m_pkOwner->ComputePoints();
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);

	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountIcon %d", m_dwSummonItemVnum);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountEvolution %d", m_dwevolution);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountName %s", m_name.c_str());
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountLevel %d", m_dwlevel);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountDuration %d %d", m_dwduration, m_dwtduration);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountAgeDuration %d", m_dwAgeDuration);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountBonus %d %d %d", m_dwbonusmount[0][1], m_dwbonusmount[1][1], m_dwbonusmount[2][1]);
	if (GetLevel() >= 80)
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 0, m_dwskillslot[0], m_dwskill[0]);
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 1, m_dwskillslot[1], m_dwskill[1]);
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 2, m_dwskillslot[2], m_dwskill[2]);
	}
	else if (GetLevel() >= 60)
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 0, m_dwskillslot[0], m_dwskill[0]);
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 1, m_dwskillslot[1], m_dwskill[1]);
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 2, -1, m_dwskill[2]);
	}
	else if (GetLevel() >= 40)
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 0, m_dwskillslot[0], m_dwskill[0]);
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 1, -1, m_dwskill[1]);
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountSkill %d %d %d", 2, -1, m_dwskill[2]);
	}

	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "MountExp %d %d %d", m_dwexp, m_dwexpitem, m_pkChar->GetMountNextExp());
	this->GiveBuff();

	// Mount Bonuslari (value (0,2))
	pSummonItem->SetForceAttribute(0, m_dwlevel, m_dwbonusmount[0][1]);
	pSummonItem->SetForceAttribute(1, m_dwMountType, m_dwbonusmount[1][1]);
	pSummonItem->SetForceAttribute(2, m_dwArtis, m_dwbonusmount[2][1]);

	// Mount Skill Valueleri (value(3,5))
	pSummonItem->SetForceAttribute(3, m_dwskillslot[0], m_dwskill[0]);
	pSummonItem->SetForceAttribute(4, m_dwskillslot[1], m_dwskill[1]);
	pSummonItem->SetForceAttribute(5, m_dwskillslot[2], m_dwskill[2]);

	// Mount Evolution (type (6))
	pSummonItem->SetForceAttribute(6, 1, m_dwevolution);

	// Mount Age Duration
	pSummonItem->SetSocket(1, m_dwAgeDuration);

	// Mount Duration
	pSummonItem->SetSocket(2, m_dwduration);

	// Mount Total Duration
	pSummonItem->SetSocket(3, m_dwtduration);

	// Lock
	pSummonItem->SetSocket(0, true);
	pSummonItem->Lock(true);
	return m_dwVID;
}

bool CNewMountActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	float fDist = number(fMinDist, fMaxDist);
	float r = (float)number(0, 359);
	float dest_x = GetOwner()->GetX() + fDist * cos(r);
	float dest_y = GetOwner()->GetY() + fDist * sin(r);

	//GetDeltaByDegree(m_pkChar->GetRotation(), fDist, &fx, &fy);

	//if (!(SECTREE_MANAGER::instance().IsMovablePosition(m_pkChar->GetMapIndex(), m_pkChar->GetX() + (int) fx, m_pkChar->GetY() + (int) fy)
	//			&& SECTREE_MANAGER::instance().IsMovablePosition(m_pkChar->GetMapIndex(), m_pkChar->GetX() + (int) fx/2, m_pkChar->GetY() + (int) fy/2)))
	//	return true;

	m_pkChar->SetNowWalking(true);

	//if (m_pkChar->Goto(m_pkChar->GetX() + (int) fx, m_pkChar->GetY() + (int) fy))
	//	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	if (!m_pkChar->IsStateMove() && m_pkChar->Goto(dest_x, dest_y))
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();

	return true;
}

bool CNewMountActor::_UpdateFollowAI()
{
	if (0 == m_pkChar->m_pkMobData)
	{
		return false;
	}

	if (!m_pkOwner)
		return false;

	if (0 == m_originalMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);

		if (0 != mobData)
			m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
	}
	const static float	START_FOLLOW_DISTANCE = 300.0f;
	const static float	START_RUN_DISTANCE = 900.0f;

	const static float	RESPAWN_DISTANCE = 4500.f;
	const static int	APPROACH = 290;

	bool bRun = false;

	DWORD currentTime = get_dword_time();

	long ownerX = m_pkOwner->GetX();		long ownerY = m_pkOwner->GetY();
	long charX = m_pkChar->GetX();			long charY = m_pkChar->GetY();

	float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f;
		float fx = -APPROACH * cos(fOwnerRot);
		float fy = -APPROACH * sin(fOwnerRot);
		if (m_pkChar->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
		{
			return true;
		}
	}

	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if (fDist >= START_RUN_DISTANCE)
		{
			bRun = true;
		}

		m_pkChar->SetNowWalking(!bRun);

		Follow(APPROACH);

		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	return true;
}

bool CNewMountActor::Update(DWORD deltaTime)
{
	bool bResult = true;

	if (IsSummoned()) {
		if (m_pkOwner->IsImmortal() && Mount_Skill_Table[7][2 + m_dwskill[m_dwslotimm]] <= (get_global_time() - m_dwImmTime) * 10) {
			//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "%d - %d  diff %d  Skilltable %d", get_global_time(), m_dwImmTime, (get_global_time() - m_dwImmTime) * 10, Mount_Skill_Table[16][2 + m_dwskill[m_dwslotimm]]);
			m_dwImmTime = 0;
			m_pkOwner->SetImmortal(0);
		}
	}
	if ((IsSummoned() && m_pkChar->IsDead()) || (IsSummoned() && (m_pkOwner->GetExchange() || m_pkOwner->GetMyShop() || m_dwduration <= 0 || m_dwduration <= get_global_time()))
		|| NULL == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		this->Unsummon();
		return true;
	}

	if (this->IsSummoned() && HasOption(EMountOption_Followable))
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}

bool CNewMountActor::Follow(float fMinDistance)
{
	if (!m_pkOwner || !m_pkChar)
		return false;

	float fOwnerX = m_pkOwner->GetX();
	float fOwnerY = m_pkOwner->GetY();

	float fMountX = m_pkChar->GetX();
	float fMountY = m_pkChar->GetY();

	float fDist = DISTANCE_SQRT(fOwnerX - fMountX, fOwnerY - fMountY);
	if (fDist <= fMinDistance)
		return false;

	m_pkChar->SetRotationToXY(fOwnerX, fOwnerY);

	float fx, fy;

	float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);

	if (!m_pkChar->Goto((int)(fMountX + fx + 0.5f), (int)(fMountY + fy + 0.5f)))
		return false;

	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

void CNewMountActor::SetSummonItem(LPITEM pItem)
{
	if (NULL == pItem)
	{
		m_dwSummonItemVID = 0;
		m_dwSummonItemID = 0;
		m_dwSummonItemVnum = 0;
		return;
	}

	m_dwSummonItemVID = pItem->GetVID();
	m_dwSummonItemID = pItem->GetID();
	m_dwSummonItemVnum = pItem->GetVnum();
}

void CNewMountActor::GiveBuff()
{
	long long cbonus[3] = { m_pkOwner->GetPoint(POINT_ATTBONUS_MONSTER),  m_pkOwner->GetPoint(POINT_ATTBONUS_STONE), m_pkOwner->GetPoint(POINT_ATT_GRADE) };
	for (int i = 0; i < 3; ++i) {
		m_pkOwner->AddAffect(AFFECT_NEW_MOUNT, aApplyInfo[m_dwbonusmount[i][0]].bPointType, (int)(cbonus[i] * m_dwbonusmount[i][1] / 1000), 0, 60 * 60 * 24 * 365, 0, false);
	}

	//Inizializzo le skill del mount inattive  No 10-17-18 No 0 no -1
	//Condizione lv > 81 evo 3 Solo Skill Passive
	if (GetLevel() > 81) {
		for (int s = 0; s < 3; s++) {
			switch (m_dwskillslot[s]) {
			case 1: //Metin Avcýsý
			case 2: //Canavar Avcýsý
			case 3: //Patron Avcýsý
			case 4: //Berserker
			case 5: //Yarý Ýnsanlara Karþý Güç
			case 6: //Yarý Ýnsanlara Karþý Direnç
			case 7: //Hýzlandýrma
				m_pkOwner->AddAffect(AFFECT_NEW_MOUNT, aApplyInfo[Mount_Skill_Table[m_dwskillslot[s] - 1][0]].bPointType, Mount_Skill_Table[m_dwskillslot[s] - 1][2 + m_dwskill[s]], 0, 60 * 60 * 24 * 365, 0, false);
				break;

			default:
				return;
			}
		}
	}
}

void CNewMountActor::ClearBuff()
{
	m_pkOwner->RemoveAffect(AFFECT_NEW_MOUNT);
	return;
}

void CNewMountActor::DoMountSkill(int skillslot) {
	if (GetLevel() < 82 || m_dwevolution < 3)
		return;
}

int CNewMountActor::GetSkillCount() {
	int skillCount = 0;
	for (int i = 0; i < 3; i++) {
		if (m_dwskillslot[i] >= 0)
			++skillCount;
	}
	return skillCount;
}

CNewMountSystem::CNewMountSystem(LPCHARACTER owner)
{
	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CNewMountSystem::~CNewMountSystem()
{
	Destroy();
}

bool CNewMountSystem::IncreaseMountSkill(int skill) {
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->IncreaseMountSkill(skill);
			}
		}
	}
	return false;
}

bool CNewMountSystem::ResetSkill(int skill) {
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->ResetSkill(skill);
			}
		}
	}
	return false;
}

bool CNewMountSystem::IncreaseMountEvolution() {
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->IncreaseMountEvolution();
			}
		}
	}
	return false;
}

void CNewMountSystem::Destroy()
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			delete mountActor;
		}
	}
	event_cancel(&m_pkNewMountSystemUpdateEvent);
	event_cancel(&m_pkNewMountSystemExpireEvent);
	m_mountActorMap.clear();
}

void CNewMountSystem::UpdateTime()
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;

		if (0 != mountActor && mountActor->IsSummoned())
		{
			mountActor->UpdateTime();
		}
	}
}

bool CNewMountSystem::Update(DWORD deltaTime)
{
	bool bResult = true;

	DWORD currentTime = get_dword_time();

	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;

	std::vector <CNewMountActor*> v_garbageActor;

	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;

		if (0 != mountActor && mountActor->IsSummoned())
		{
			LPCHARACTER pMount = mountActor->GetCharacter();

			if (NULL == CHARACTER_MANAGER::instance().Find(pMount->GetVID()))
			{
				v_garbageActor.push_back(mountActor);
			}
			else
			{
				bResult = bResult && mountActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CNewMountActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeleteMount(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CNewMountSystem::DeleteMount(DWORD mobVnum)
{
	TNewMountActorMap::iterator iter = m_mountActorMap.find(mobVnum);

	if (m_mountActorMap.end() == iter)
	{
		return;
	}

	CNewMountActor* mountActor = iter->second;

	if (0 == mountActor)
		sys_err("[CMounttSystem::DeleteMount] NULL Pointer (mountActor)");
	else
		delete mountActor;

	m_mountActorMap.erase(iter);
}

void CNewMountSystem::DeleteMount(CNewMountActor* mountActor)
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		if (iter->second == mountActor)
		{
			delete mountActor;
			m_mountActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CMountSystem::DeleteMount] Can't find mountActor(0x%x) on my list(size: %d) ", mountActor, m_mountActorMap.size());
}

void CNewMountSystem::Unsummon(DWORD vnum, bool bDeleteFromList)
{
	CNewMountActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CMountSystem::GetByVnum(%d)] NULL Pointer (mountActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeleteMount(actor);

	bool bActive = false;
	for (TNewMountActorMap::iterator it = m_mountActorMap.begin(); it != m_mountActorMap.end(); it++)
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pkNewMountSystemUpdateEvent);
		event_cancel(&m_pkNewMountSystemExpireEvent);
		m_pkNewMountSystemUpdateEvent = NULL;
		m_pkNewMountSystemExpireEvent = NULL;
	}
}

DWORD CNewMountSystem::GetNewMountITemID()
{
	DWORD itemid = 0;
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				itemid = mountActor->GetSummonItemID();
				break;
			}
		}
	}
	return itemid;
}

bool CNewMountSystem::IsActiveMount()
{
	bool state = false;
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				state = true;
				break;
			}
		}
	}
	return state;
}

int CNewMountSystem::GetLevelStep()
{
	int step = 4;
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				step = mountActor->GetLevelStep();
				break;
			}
		}
	}
	return step;
}

bool CNewMountSystem::SetExp(int iExp, int mode)
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->SetExp(iExp, mode);
			}
		}
	}

	return false;
}

void CNewMountSystem::SetMountType(int iType)
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				mountActor->SetMountType(iType);
				break;
			}
		}
	}
}

void CNewMountSystem::SetArtis(int val)
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				mountActor->SetArtis( val);
				break;
			}
		}
	}
}

int CNewMountSystem::GetEvolution()
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->GetEvolution();
			}
		}
	}
	return -1;
}

int CNewMountSystem::GetLevel()
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->GetLevel();
			}
		}
	}
	return -1;
}

int CNewMountSystem::GetExp()
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->GetExp();
			}
		}
	}
	return 0;
}

int CNewMountSystem::GetAge()
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned())
			{
				int tmpage = (time(0) + mountActor->GetAge()) / 86400;
				return tmpage;
			}
		}
	}
	return -1;
}

int CNewMountSystem::GetMountType()
{
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned())
			{
				return mountActor->GetMountType();
			}
		}
	}
	return -1;
}

void CNewMountSystem::SetItemCube(int pos, int invpos) {
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->SetItemCube(pos, invpos);
			}
		}
	}
}

void CNewMountSystem::ItemCubeFeed(int type) {
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->ItemCubeFeed(type);
			}
		}
	}
}

void CNewMountSystem::DoMountSkill(int skillslot) {
	for (TNewMountActorMap::iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;
		if (mountActor != 0)
		{
			if (mountActor->IsSummoned()) {
				return mountActor->DoMountSkill(skillslot);
			}
		}
	}
}

CNewMountActor* CNewMountSystem::Summon(DWORD mobVnum, LPITEM pSummonItem, const char* mountName, bool bSpawnFar, DWORD options)
{
	CNewMountActor* mountActor = this->GetByVnum(mobVnum);

	if (0 == mountActor)
	{
		mountActor = M2_NEW CNewMountActor(m_pkOwner, mobVnum, options);
		m_mountActorMap.insert(std::make_pair(mobVnum, mountActor));
	}

	mountActor->Summon(mountName, pSummonItem, bSpawnFar);

	if (NULL == m_pkNewMountSystemUpdateEvent)
	{
		newmountsystem_event_info* info = AllocEventInfo<newmountsystem_event_info>();

		info->pMountSystem = this;

		m_pkNewMountSystemUpdateEvent = event_create(newmountsystem_update_event, info, PASSES_PER_SEC(1) / 4);
	}

	if (NULL == m_pkNewMountSystemExpireEvent)
	{
		newmountsystem_event_infoe* infoe = AllocEventInfo<newmountsystem_event_infoe>();

		infoe->pMountSystem = this;

		m_pkNewMountSystemExpireEvent = event_create(newmountsystem_expire_event, infoe, PASSES_PER_SEC(1));	// 1 volata per sec
	}

	return mountActor;
}

CNewMountActor* CNewMountSystem::GetByVID(DWORD vid) const
{
	CNewMountActor* mountActor = 0;

	bool bFound = false;

	for (TNewMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		mountActor = iter->second;

		if (0 == mountActor)
		{
			sys_err("[CMountSystem::GetByVID(%d)] NULL Pointer (mountActor)", vid);
			continue;
		}

		bFound = mountActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? mountActor : 0;
}

CNewMountActor* CNewMountSystem::GetByVnum(DWORD vnum) const
{
	CNewMountActor* mountActor = 0;

	TNewMountActorMap::const_iterator iter = m_mountActorMap.find(vnum);

	if (m_mountActorMap.end() != iter)
		mountActor = iter->second;

	return mountActor;
}

size_t CNewMountSystem::CountSummoned() const
{
	size_t count = 0;

	for (TNewMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			if (mountActor->IsSummoned())
				++count;
		}
	}

	return count;
}

void CNewMountSystem::RefreshBuff()
{
	for (TNewMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			if (mountActor->IsSummoned())
			{
				mountActor->ClearBuff();
				mountActor->GiveBuff();
			}
		}
	}
}

CNewMountActor* CNewMountSystem::GetSummoned()
{
	for (TNewMountActorMap::const_iterator iter = m_mountActorMap.begin(); iter != m_mountActorMap.end(); ++iter)
	{
		CNewMountActor* mountActor = iter->second;

		if (0 != mountActor)
		{
			if (mountActor->IsSummoned())
			{
				return mountActor;
			}
		}
	}

	return nullptr;
}
#endif
