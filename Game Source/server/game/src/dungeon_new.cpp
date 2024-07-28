#include "stdafx.h"
#include "dungeon_new.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "packet.h"
#include "desc.h"
#include "config.h"
#include "regen.h"
#include "start_position.h"
#include "item.h"
#include "item_manager.h"
#include "utils.h"
#include "questmanager.h"
#include "dungeon_info.h"

#ifdef __DUNGEON_QUEUE__
#include "questevent.h"
#endif

CDungeon::CDungeon(IdType id, long lOriginalMapIndex, long lMapIndex)
	: m_id(id),
	m_lOrigMapIndex(lOriginalMapIndex),
	m_lMapIndex(lMapIndex)
#ifdef __DUNGEON_TEST_MODE__
	,isTest(false)
#endif // __DUNGEON_TEST_MODE__
{
	Initialize();
	//sys_log(0,"DUNGEON create orig %d real %d", lOriginalMapIndex, lMapIndex);
}

CDungeon::~CDungeon()
{
	sys_log(0,"DUNGEON destroy orig %d real %d", m_lOrigMapIndex, m_lMapIndex	);
	ClearRegen();
	event_cancel(&deadEvent);
	event_cancel(&exit_all_event_);
}

void CDungeon::Initialize()
{
	deadEvent = NULL;
	// <Factor>
	exit_all_event_ = NULL;
	regen_id_ = 0;

	m_bUsePotion = false;
	m_bUseRevive = false;

	m_iMonsterCount = 0;
	m_iNpcCount = 0;

	m_lJoinX = 0;
	m_lJoinY = 0;

	m_stRegenFile = "";
}

void CDungeon::RegisterUser(LPCHARACTER ch)
{
	m_registeredPlayers.insert(ch->GetPlayerID());
}
long long unsigned CDungeon::SaveReport(LPCHARACTER ch, char* comment)
{
	long long unsigned now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	// open target file
	char szFileName[500];
	sprintf(szFileName, "%s/game_api/dungeon_report/%llu.txt", g_stQuestDir.c_str(), now);

	FILE* fp = 0;
	fp = fopen(szFileName, "w");

	if (fp == nullptr)
		return 0;

	/*                                           FILE START                                        */
#define FILE_NL fprintf(fp, "\n");
#define FILE_INT_INFO(value) fprintf(fp, "%s: %d\n", #value, value);
#define FILE_STR_INFO(value) fprintf(fp, "%s: %s\n", #value, value);
#define FILE_STR2_INFO(value) fprintf(fp, "%s: %s\n", #value, value.c_str());
#define FILE_ITER_INT_INFO(value) \
		fprintf(fp, "%s:\n", #value); \
		for (auto &&i : value) \
		{ \
			fprintf(fp, "\t%d\n", i); \
		}
#define FILE_ITER_STR_INFO(value) \
		fprintf(fp, "%s:\n", #value); \
		for (auto &&i : value) \
		{ \
			fprintf(fp, "\t%s\n", i.c_str()); \
		}
#define FILE_ITER_MAP_INFO(value) \
		fprintf(fp, "%s:\n", #value); \
		for (auto &&i : value) \
		{ \
			fprintf(fp, "\t%s: %d\n", i.first.c_str(), i.second); \
		}
#define FILE_ITER_TIMER_INFO(value) \
		fprintf(fp, "%s:\n", #value); \
		for (auto &&i : value) \
		{ \
			if (i.first.second == m_lMapIndex) { \
				fprintf(fp, "\t%s: %d second\n", i.first.first.c_str(), event_time(i.second) / passes_per_sec); \
			} \
		}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

// header
	fprintf(fp, "-- DUNGEON REPORT START --\n");
	FILE_NL
		FILE_STR_INFO(comment)
		FILE_NL

		// character info
		if (ch)
		{
			fprintf(fp, "REPORTTER INFO:\n");
			FILE_NL
				FILE_INT_INFO(ch->GetAID())
				FILE_INT_INFO(ch->GetPlayerID())
				FILE_STR_INFO(ch->GetName())
				FILE_INT_INFO(ch->GetRaceNum())
				FILE_INT_INFO(ch->GetX())
				FILE_INT_INFO(ch->GetY())
				FILE_INT_INFO(ch->GetMapIndex())
				FILE_INT_INFO(ch->GetDungeon())
				FILE_NL
		}

	// date info
	char date_string[100];
	time_t iTime; time(&iTime); tm* pTimeInfo = localtime(&iTime);
	strftime(date_string, sizeof(date_string), "%d.%m.%Y-%H:%M:%S", pTimeInfo);
	fprintf(fp, "DATE INFO: %s\n", date_string);
	FILE_NL

		fprintf(fp, "DUNGEON INFO:\n");
	FILE_NL
		// private value info
		FILE_INT_INFO(m_id)
		FILE_INT_INFO(m_lOrigMapIndex)
		FILE_INT_INFO(m_lMapIndex)
		FILE_INT_INFO(g_bChannel)

		FILE_INT_INFO(deadEvent)
		if (exit_all_event_)
		{
			FILE_INT_INFO(exit_all_event_)
				FILE_INT_INFO(event_time(exit_all_event_) / passes_per_sec)
		}

		FILE_INT_INFO(m_bUsePotion)
		FILE_INT_INFO(m_bUseRevive)

		FILE_INT_INFO(CountRealMonster())
		FILE_INT_INFO(m_iMonsterCount)
		FILE_INT_INFO(m_iNpcCount)

		FILE_INT_INFO(m_lJoinX)
		FILE_INT_INFO(m_lJoinY)

		FILE_ITER_INT_INFO(m_registeredPlayers)
		FILE_ITER_INT_INFO(m_rewardPlayers)

		FILE_STR2_INFO(m_stRegenFile)

		FILE_ITER_MAP_INFO(m_map_Flag)
		FILE_ITER_TIMER_INFO(quest::CQuestManager::instance().m_mapServerTimer)

		FILE_NL
		fprintf(fp, "-- DUNGEON REPORT END --");

#undef FILE_NL
#undef FILE_INT_INFO
#undef FILE_STR_INFO
#undef FILE_STR2_INFO
#undef FILE_ITER_INT_INFO
#undef FILE_ITER_STR_INFO
#undef FILE_ITER_MAP_INFO
#pragma GCC diagnostic pop
	/*                                           FILE END                                        */

	//close file
	fclose(fp);
	return now;
}

void CDungeon::SetFlag(std::string name, int value)
{
	itertype(m_map_Flag) it = m_map_Flag.find(name);
	if (it != m_map_Flag.end())
		it->second = value;
	else
		m_map_Flag.insert(make_pair(name, value));
}

int CDungeon::GetFlag(std::string name)
{
	itertype(m_map_Flag) it = m_map_Flag.find(name);
	if (it != m_map_Flag.end())
		return it->second;
	else
		return 0;
}

struct FSendDestPosition
{
	FSendDestPosition(long x, long y)
	{
		p1.bHeader = HEADER_GC_DUNGEON;
		p1.subheader = DUNGEON_SUBHEADER_GC_DESTINATION_POSITION;
		p2.x = x;
		p2.y = y;
		p1.size = sizeof(p1) + sizeof(p2);
	}

	void operator()(LPCHARACTER ch)
	{
		ch->GetDesc()->BufferedPacket(&p1, sizeof(TPacketGCDungeon));
		ch->GetDesc()->Packet(&p2, sizeof(TPacketGCDungeonDestPosition));
	}

	TPacketGCDungeon p1;
	TPacketGCDungeonDestPosition p2;
};

struct FWarpToDungeon
{
	FWarpToDungeon(long lMapIndex, LPDUNGEON d)
		: m_lMapIndex(lMapIndex), m_pkDungeon(d)
	{
		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);
		m_x = pkSectreeMap->m_setting.posSpawn.x;
		m_y = pkSectreeMap->m_setting.posSpawn.y;
	}

	void operator () (LPCHARACTER ch)
	{
		ch->SaveExitLocation();
		ch->WarpSet(m_x, m_y, m_lMapIndex);
		//m_pkDungeon->IncPartyMember(ch->GetParty());
	}

	long m_lMapIndex;
	long m_x;
	long m_y;
	LPDUNGEON m_pkDungeon;
};

void CDungeon::Join(LPCHARACTER ch)
{
	if (SECTREE_MANAGER::instance().GetMap(m_lMapIndex) == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return;
	}
	FWarpToDungeon(m_lMapIndex, this) (ch);
}

EVENTINFO(dungeon_id_info)
{
	CDungeon::IdType dungeon_id;

	dungeon_id_info()
		: dungeon_id(0)
	{
	}
};

struct FCheckForDead
{
	bool ret;
	FCheckForDead()
		: ret(true)
	{}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER)) {
			ret = false;
			return;
		}
	}
};

EVENTFUNC(dungeon_dead_event)
{
	dungeon_id_info* info = dynamic_cast<dungeon_id_info*>(event->info);

	if (info == NULL)
	{
		sys_err("dungeon_dead_event> <Factor> Null pointer");
		return 0;
	}

	LPDUNGEON pDungeon = CDungeonManager::instance().Find(info->dungeon_id);
	if (pDungeon == NULL) {
		return 0;
	}

	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(pDungeon->GetMapIndex());
	if (pMap == NULL) {
		return 0;
	}

	{
		FCheckForDead f;
		pMap->for_each(f);
		if (f.ret == false)
		{
			//sys_log(0, "Dungen %ld cannot ready for dead, waiting %ld sec", pDungeon->GetMapIndex(), processing_time);
			pDungeon->ExitAll();
			pDungeon->Purge(nullptr);
			return processing_time;
		}
	}

	pDungeon->deadEvent = NULL;

	sys_log(0, "DUNGEON dungeon_dead_event : map index %u", pDungeon->GetMapIndex());

	CDungeonManager::instance().Destroy(info->dungeon_id);
	return 0;
}

LPCHARACTER CDungeon::GetRandomMember()
{
	if (m_registeredPlayers.size() == 0)
		return nullptr;

	for (auto&& i : m_registeredPlayers)
	{
		auto ch = CHARACTER_MANAGER::instance().FindByPID(i);
		if (ch && ch->GetMapIndex() == GetMapIndex())
			return ch;
	}

	return nullptr;
}

EVENTINFO(jump_all_with_delay_event_info)
{
	int		m_lMapIndex;
	int		m_x;
	int		m_y;
	const char* msg;

	jump_all_with_delay_event_info()
		: m_lMapIndex(0)
		, m_x(0)
		, m_y(0)
		, msg(0)
	{
	}
};

struct FWarpToPosition
{
	long lMapIndex;
	long x;
	long y;
	FWarpToPosition(long lMapIndex, long x, long y)
		: lMapIndex(lMapIndex), x(x), y(y)
	{}

	void operator()(LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER)) {
			return;
		}
		LPCHARACTER ch = (LPCHARACTER)ent;
		if (!ch->IsPC()) {
			return;
		}
		if (ch->GetMapIndex() == lMapIndex)
		{
			ch->Show(lMapIndex, x, y, 0);
			ch->Stop();
		}
		else
		{
			ch->WarpSet(x, y, lMapIndex);
		}
	}
};

struct FWarpToPositionForce
{
	long lMapIndex;
	long x;
	long y;
	FWarpToPositionForce(long lMapIndex, long x, long y)
		: lMapIndex(lMapIndex), x(x), y(y)
	{}

	void operator()(LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER)) {
			return;
		}
		LPCHARACTER ch = (LPCHARACTER)ent;
		if (!ch->IsPC()) {
			return;
		}
		ch->WarpSet(x, y, lMapIndex);
	}
};

void CDungeon::JumpAll(long lFromMapIndex, int x, int y)
{
	x *= 100;
	y *= 100;

	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(lFromMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", lFromMapIndex);
		return;
	}

	FWarpToPosition f(m_lMapIndex, x, y);

	// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
	pMap->for_each(f);
}

void CDungeon::WarpAll(long lFromMapIndex, int x, int y)
{
	x *= 100;
	y *= 100;

	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(lFromMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", lFromMapIndex);
		return;
	}

	FWarpToPositionForce f(m_lMapIndex, x, y);

	// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
	pMap->for_each(f);
}

void CDungeonManager::Destroy(CDungeon::IdType dungeon_id)
{
	// sys_log(0, "DUNGEON destroy : map index %u", dungeon_id);
	LPDUNGEON pDungeon = Find(dungeon_id);
	if (pDungeon == NULL) {
		return;
	}
	m_map_pkDungeon.erase(dungeon_id);

	long lMapIndex = pDungeon->m_lMapIndex;
	m_map_pkMapDungeon.erase(lMapIndex);

	DWORD server_timer_arg = lMapIndex;
	quest::CQuestManager::instance().CancelServerTimers(server_timer_arg);

	SECTREE_MANAGER::instance().DestroyPrivateMap(lMapIndex);
	M2_DELETE(pDungeon);
}

LPDUNGEON CDungeonManager::Find(CDungeon::IdType dungeon_id)
{
	itertype(m_map_pkDungeon) it = m_map_pkDungeon.find(dungeon_id);
	if (it != m_map_pkDungeon.end())
		return it->second;
	return NULL;
}

LPDUNGEON CDungeonManager::FindByMapIndex(long lMapIndex)
{
	itertype(m_map_pkMapDungeon) it = m_map_pkMapDungeon.find(lMapIndex);
	if (it != m_map_pkMapDungeon.end()) {
		return it->second;
	}
	return NULL;
}

LPDUNGEON CDungeonManager::Create(long lOriginalMapIndex)
{
	DWORD lMapIndex = SECTREE_MANAGER::instance().CreatePrivateMap(lOriginalMapIndex);

	if (!lMapIndex)
	{
		sys_log(0, "Fail to Create Dungeon : OrginalMapindex %d NewMapindex %d", lOriginalMapIndex, lMapIndex);
		return NULL;
	}

	// <Factor> TODO: Change id assignment, or drop it
	CDungeon::IdType id = next_id_++;
	while (Find(id) != NULL) {
		id = next_id_++;
	}

	LPDUNGEON pDungeon = M2_NEW CDungeon(id, lOriginalMapIndex, lMapIndex);
	if (!pDungeon)// @eger zindan yoksa return atmasi lazim cunku bos zindani listeye kaydediyor.
	{
		sys_err("M2_NEW CDungeon failed");
		return NULL;
	}
	m_map_pkDungeon.insert(std::make_pair(id, pDungeon));
	m_map_pkMapDungeon.insert(std::make_pair(lMapIndex, pDungeon));

	return pDungeon;
}

const CDungeonManager::TMapDungeon& CDungeonManager::GetDungeonMap()
{
	return m_map_pkMapDungeon;
}

void CDungeonManager::OnShutdown()
{
	quest::CQuestManager::instance().SetEventFlag("zindanlar_aktif", 0);

	for (auto&& it : m_map_pkDungeon)
	{
		it.second->ExitAll();
	}
}

CDungeonManager::CDungeonManager()
	: next_id_(0)
{
}

CDungeonManager::~CDungeonManager()
{
}

void CDungeon::UniqueSetMaxHP(const std::string& key, HPTYPE iMaxHP)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);
	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key : %s", key.c_str());
		return;
	}
	it->second->SetMaxHP(iMaxHP);
}

void CDungeon::UniqueSetHP(const std::string& key, HPTYPE iHP)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);
	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key : %s", key.c_str());
		return;
	}
	it->second->SetHP(iHP);
}

void CDungeon::UniqueSetDefGrade(const std::string& key, int iGrade)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);
	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key : %s", key.c_str());
		return;
	}
	it->second->PointChange(POINT_DEF_GRADE, iGrade - it->second->GetPoint(POINT_DEF_GRADE));
}

void CDungeon::SetUnique(const char* key, DWORD vid)
{
	LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(vid);
	if (ch)
	{
		m_map_UniqueMob.insert(make_pair(std::string(key), ch));
		ch->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	}
}

void CDungeon::PurgeUnique(const std::string& key)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);
	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key or Dead: %s", key.c_str());
		return;
	}
	LPCHARACTER ch = it->second;
	m_map_UniqueMob.erase(it);
	M2_DESTROY_CHARACTER(ch);
}

void CDungeon::KillUnique(const std::string& key)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);
	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key or Dead: %s", key.c_str());
		return;
	}
	LPCHARACTER ch = it->second;
	m_map_UniqueMob.erase(it);
	ch->Dead();
}

DWORD CDungeon::GetUniqueVid(const std::string& key)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);
	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key or Dead: %s", key.c_str());
		return 0;
	}
	LPCHARACTER ch = it->second;
	return ch->GetVID();
}

float CDungeon::GetUniqueHpPerc(const std::string& key)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);
	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key : %s", key.c_str());
		return 0.0f; // @duzenleme float dondurmuyordu bu duzeltildi.
	}
	return (100.f * it->second->GetHP()) / it->second->GetMaxHP();
}

void CDungeon::DeadCharacter(LPCHARACTER ch)
{
	if (!ch)
		return;

	if (!ch->IsPC())
	{
		TUniqueMobMap::iterator it = m_map_UniqueMob.begin();
		while (it != m_map_UniqueMob.end())
		{
			if (it->second == ch)
			{
				//sys_log(0,"Dead unique %s", it->first.c_str());
				m_map_UniqueMob.erase(it);
				break;
			}
			++it;
		}
	}
}

bool CDungeon::IsUniqueDead(const std::string& key)
{
	TUniqueMobMap::iterator it = m_map_UniqueMob.find(key);

	if (it == m_map_UniqueMob.end())
	{
		sys_err("Unknown Key or Dead : %s", key.c_str());
		return true;
	}

	return it->second->IsDead();
}

LPCHARACTER CDungeon::SpawnMob(DWORD vnum, int x, int y, int dir)
{
	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkSectreeMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return NULL;
	}
	sys_log(0, "CDungeon::SpawnMob %u %d %d", vnum, x, y);

	LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(vnum, m_lMapIndex, pkSectreeMap->m_setting.iBaseX + x * 100, pkSectreeMap->m_setting.iBaseY + y * 100, 0, false, dir == 0 ? -1 : (dir - 1) * 45);

	if (ch)
	{
		ch->SetDungeon(this);
		sys_log(0, "CDungeon::SpawnMob name %s", ch->GetName());
	}

	return ch;
}

LPCHARACTER CDungeon::SpawnMob_ac_dir(DWORD vnum, int x, int y, int dir)
{
	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkSectreeMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return NULL;
	}
	sys_log(0, "CDungeon::SpawnMob %u %d %d", vnum, x, y);

	LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(vnum, m_lMapIndex, pkSectreeMap->m_setting.iBaseX + x * 100, pkSectreeMap->m_setting.iBaseY + y * 100, 0, false, dir);

	if (ch)
	{
		ch->SetDungeon(this);
		sys_log(0, "CDungeon::SpawnMob name %s", ch->GetName());
	}

	return ch;
}

void CDungeon::SpawnNameMob(DWORD vnum, int x, int y, const char* name)
{
	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkSectreeMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(vnum, m_lMapIndex, pkSectreeMap->m_setting.iBaseX + x, pkSectreeMap->m_setting.iBaseY + y, 0, false, -1);
	if (ch)
	{
		ch->SetName(name);
		ch->SetDungeon(this);
	}
}

void CDungeon::SpawnGotoMob(long lFromX, long lFromY, long lToX, long lToY)
{
	const int MOB_GOTO_VNUM = 20039;

	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkSectreeMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return;
	}

	sys_log(0, "SpawnGotoMob %d %d to %d %d", lFromX, lFromY, lToX, lToY);

	lFromX = pkSectreeMap->m_setting.iBaseX + lFromX * 100;
	lFromY = pkSectreeMap->m_setting.iBaseY + lFromY * 100;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(MOB_GOTO_VNUM, m_lMapIndex, lFromX, lFromY, 0, false, -1);

	if (ch)
	{
		char buf[30 + 1];
		snprintf(buf, sizeof(buf), ". %ld %ld", lToX, lToY);

		ch->SetName(buf);
		ch->SetDungeon(this);
	}
}

LPCHARACTER CDungeon::SpawnGroup(DWORD vnum, long x, long y, float radius, bool bAggressive, int count)
{
	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkSectreeMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return NULL;
	}

	int iRadius = (int)radius;

	int sx = pkSectreeMap->m_setting.iBaseX + x - iRadius;
	int sy = pkSectreeMap->m_setting.iBaseY + y - iRadius;
	int ex = sx + iRadius;
	int ey = sy + iRadius;

	LPCHARACTER ch = NULL;

	while (count--)
	{
		LPCHARACTER chLeader = CHARACTER_MANAGER::instance().SpawnGroup(vnum, m_lMapIndex, sx, sy, ex, ey, NULL, bAggressive, this);
		if (chLeader && !ch)
			ch = chLeader;
	}

	return ch;
}

void CDungeon::SpawnRegen(const char* filename, bool bOnce)
{
	if (!filename)
	{
		sys_err("CDungeon::SpawnRegen(filename=NULL, bOnce=%d) - m_lMapIndex[%d]", bOnce, m_lMapIndex);
		return;
	}

	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (!pkSectreeMap)
	{
		sys_err("CDungeon::SpawnRegen(filename=%s, bOnce=%d) - m_lMapIndex[%d]", filename, bOnce, m_lMapIndex);
		return;
	}
	regen_do(filename, m_lMapIndex, pkSectreeMap->m_setting.iBaseX, pkSectreeMap->m_setting.iBaseY, this, bOnce);
}

void CDungeon::AddRegen(LPREGEN regen)
{
	regen->id = regen_id_++;
	m_regen.push_back(regen);
}

void CDungeon::ClearRegen()
{
	for (itertype(m_regen) it = m_regen.begin(); it != m_regen.end(); ++it)
	{
		LPREGEN regen = *it;

		event_cancel(&regen->event);
		M2_DELETE(regen);
	}
	m_regen.clear();
}

bool CDungeon::IsValidRegen(LPREGEN regen, size_t regen_id) {
	itertype(m_regen) it = std::find(m_regen.begin(), m_regen.end(), regen);
	if (it == m_regen.end()) {
		return false;
	}
	LPREGEN found = *it;
	return (found->id == regen_id);
}

namespace
{
	struct FKillSectree
	{
		void operator () (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;

				if (!ch->IsPC() && !ch->IsPet()
#ifdef __GROWTH_PET_SYSTEM__
					&& !ch->IsNewPet()
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
					&& !ch->IsMount()
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
					&& !ch->IsNewMount()
#endif
					)
					ch->Dead();
			}
		}
	};

	struct FKillMobSectree
	{
		void operator () (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;

				if (ch->IsMonster() || ch->IsStone())
					ch->Dead();
			}
		}
	};

	struct FPurgeSectree
	{
		LPCHARACTER ExceptChar;

		FPurgeSectree(LPCHARACTER p)
			: ExceptChar(p)
		{}

		void operator () (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;

				if (ch == ExceptChar)
					return;

				if (!ch->IsPC() && !ch->IsPet()
#ifdef __GROWTH_PET_SYSTEM__
					&& !ch->IsNewPet()
#endif
#ifdef __MOUNT_COSTUME_SYSTEM__
					&& !ch->IsMount()
#endif
#ifdef __GROWTH_MOUNT_SYSTEM__
					&& !ch->IsNewMount()
#endif
					)
				{
					M2_DESTROY_CHARACTER(ch);
				}
			}
			else if (ent->IsType(ENTITY_ITEM))
			{
				LPITEM item = (LPITEM)ent;
				M2_DESTROY_ITEM(item);
			}
			else
			{
				sys_err("unknown entity type %d is in dungeon", ent->GetType());
			}
		}
	};

	struct FPurgeMobSectree
	{
		LPCHARACTER ExceptChar;

		FPurgeMobSectree(LPCHARACTER p)
			: ExceptChar(p)
		{}

		void operator () (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;

				if (ch == ExceptChar)
					return;

				if (ch->IsMonster() || ch->IsStone())
					M2_DESTROY_CHARACTER(ch);
			}
			else if (ent->IsType(ENTITY_ITEM))
			{
				LPITEM item = (LPITEM)ent;
				M2_DESTROY_ITEM(item);
			}
			else
			{
				sys_err("unknown entity type %d is in dungeon", ent->GetType());
			}
		}
	};
}

void CDungeon::KillAll()
{
	LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return;
	}
	FKillSectree f;
	pkMap->for_each(f);
}
void CDungeon::KillAllMob()
{
	LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return;
	}

	FKillMobSectree f;
	pkMap->for_each(f);
}
void CDungeon::Purge(LPCHARACTER expect)
{
	LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkMap == NULL) {
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return;
	}
	FPurgeSectree f(expect);
	pkMap->for_each(f);
}
void CDungeon::PurgeMob(LPCHARACTER ExceptChar)
{
	LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
	if (pkMap == NULL)
	{
		sys_err("CDungeon: SECTREE_MAP not found for #%ld", m_lMapIndex);
		return;
	}

	FPurgeMobSectree f(ExceptChar);
	pkMap->for_each(f);
}

void CDungeon::IncKillCount(LPCHARACTER pkKiller, LPCHARACTER pkVictim)
{
	// if (pkVictim->IsStone())
	// 	m_iStoneKill++;
	// else
	// 	m_iMobKill++;
}

void CDungeon::UsePotion(LPCHARACTER ch)
{
	m_bUsePotion = true;
}

void CDungeon::UseRevive(LPCHARACTER ch)
{
	m_bUseRevive = true;
}

bool CDungeon::IsUsePotion()
{
	return m_bUsePotion;
}

bool CDungeon::IsUseRevive()
{
	return m_bUseRevive;
}

struct FExitDungeon
{
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;

			if (ch->IsPC())
			{
				ch->SetDungeon(NULL);
				ch->ExitToSavedLocation();
			}
		}
	}
};
void CDungeon::ExitAll()
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	event_cancel(&exit_all_event_);

	m_registeredPlayers.clear();

	FExitDungeon f;
	pMap->for_each(f);

	if (deadEvent == nullptr)
	{
		event_cancel(&deadEvent);
		dungeon_id_info* info = AllocEventInfo<dungeon_id_info>();
		info->dungeon_id = m_id;
		event_cancel(&exit_all_event_);
		deadEvent = event_create(dungeon_dead_event, info, PASSES_PER_SEC(1));
	}
}
EVENTFUNC(dungeon_exit_all_event)
{
	dungeon_id_info* info = dynamic_cast<dungeon_id_info*>(event->info);

	if (info == NULL)
	{
		sys_err("dungeon_exit_all_event> <Factor> Null pointer");
		return 0;
	}

	LPDUNGEON pDungeon = CDungeonManager::instance().Find(info->dungeon_id);
	pDungeon->exit_all_event_ = NULL;
	if (!pDungeon)
	{
		sys_err("cannot find dungeon with map index %u", info->dungeon_id);
		return 0;
	}

	if (pDungeon)
		pDungeon->ExitAll();

	return 0;
}
void CDungeon::ExitAllWithDelay(int delay)
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	dungeon_id_info* info = AllocEventInfo<dungeon_id_info>();
	info->dungeon_id = m_id;
	event_cancel(&exit_all_event_);
	exit_all_event_ = event_create(dungeon_exit_all_event, info, PASSES_PER_SEC(delay));

	char buf[256];
	snprintf(buf, sizeof(buf), "DugeonRemainTime %d", GetExitRemainTime());
	cmdchat(buf);
}
struct FExitDungeonToStartPosition
{
	void operator () (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;

			if (ch->IsPC())
			{
				PIXEL_POSITION posWarp;

				if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(g_start_map[ch->GetEmpire()], ch->GetEmpire(), posWarp))
					ch->WarpSet(posWarp.x, posWarp.y);
				else
					ch->ExitToSavedLocation();
			}
		}
	}
};
void CDungeon::ExitAllToStartPosition()
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	FExitDungeonToStartPosition f;

	// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
	pMap->for_each(f);
}

int CDungeon::GetExitRemainTime()
{
	if (exit_all_event_)
		return (event_time(exit_all_event_) / passes_per_sec);
	else
	{
		sys_err("no exit_all_event_ (orig map index %u)", m_lOrigMapIndex);
		return 0;
	}
}

// DUNGEON_NOTICE
namespace
{
	struct FNotice
	{
		FNotice(const char* psz) : m_psz(psz)
		{
		}

		void operator() (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;
				if (ch->IsPC()) // @duzenleme player disinsakilere paket gondermiyor.
					ch->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_psz);
			}
		}

		const char* m_psz;
	};

	struct FBig_Notice
	{
		FBig_Notice(const char* psz) : m_psz(psz)
		{
		}

		void operator() (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;
				if (ch->IsPC()) // @duzenleme player disinsakilere paket gondermiyor.
					ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", m_psz);
			}
		}
		const char* m_psz;
	};

	struct Fcmdchat
	{
		Fcmdchat(const char* psz) : m_psz(psz)
		{
		}

		void operator() (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;
				if (ch->IsPC()) // @duzenleme player disinsakilere paket gondermiyor.
					ch->ChatPacket(CHAT_TYPE_COMMAND, "%s", m_psz);
			}
		}

		const char* m_psz;
	};

	struct Fmission
	{
		Fmission(const char* psz, bool _isSub) : m_psz(psz), isSub(_isSub)
		{
		}

		void operator() (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER)ent;
				if (ch->IsPC())
				{
					if (isSub)
						ch->ChatPacket(CHAT_TYPE_DUNGEON_SUBMISSION, "%s", m_psz);
					else
						ch->ChatPacket(CHAT_TYPE_DUNGEON_MISSION, "%s", m_psz);
				}
			}
		}

		const char* m_psz;
		bool isSub;
	};
}

void CDungeon::Notice(const char* msg)
{
	// sys_log(0, "XXX Dungeon Notice %p %s", this, msg);
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	FNotice f(msg);
	pMap->for_each(f);
}

void CDungeon::Big_Notice(const char* msg)
{
	// sys_log(0, "XXX Dungeon Notice %p %s", this, msg);
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	FBig_Notice f(msg);
	pMap->for_each(f);
}

void CDungeon::cmdchat(const char* msg)
{
	sys_log(0, "XXX Dungeon cmdchat %p %s", this, msg);
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	Fcmdchat f(msg);
	pMap->for_each(f);
}

void CDungeon::SetMission(const char* msg)
{
	if (this->mission.compare(msg) == 0)
		return;

	this->mission.clear();
#ifdef __DUNGEON_TEST_MODE__
	if (IsTest())
		this->mission += "TEST MOD AKTIF:: ";
#endif // __DUNGEON_TEST_MODE__
	this->mission += msg;
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}


	sys_log(0, "XXX Dungeon Fmission %p %s", this, this->mission.c_str());
	Fmission f(this->mission.c_str(), false);
	pMap->for_each(f);
}

void CDungeon::SetSubMission(const char* msg)
{
	if (this->submission.compare(msg) == 0)
		return;

	this->submission.clear();
#ifdef __DUNGEON_TEST_MODE__
	if (IsTest())
		this->submission += "TEST MOD AKTIF:: ";
#endif // __DUNGEON_TEST_MODE__
	this->submission += msg;
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	sys_log(0, "XXX Dungeon Fsubmission %p %s", this, this->submission.c_str());
	Fmission f(this->submission.c_str(), true);
	pMap->for_each(f);
}

void CDungeon::CheckEliminated()
{
	/*
	if (m_iMonsterCount > 0)
		return;

	if (m_iNpcCount > 0)
		return;

	if (m_bFuncAtEliminate) // TODO
	{
		sys_log(0, "CheckEliminated: warp");
		m_bWarpAtEliminate = false;

		if (m_iWarpDelay)
		{
			dungeon_id_info* info = AllocEventInfo<dungeon_id_info>();
			info->dungeon_id = m_id;

			event_cancel(&jump_to_event_);
			jump_to_event_ = event_create(dungeon_jump_to_event, info, PASSES_PER_SEC(m_iWarpDelay));
		}
		else
		{
			JumpToEliminateLocation();
		}
	}
	*/
}

struct FNearPosition
{
	long x;
	long y;
	int dist;
	bool ret;

	FNearPosition(long x, long y, int d) :
		x(x), y(y), dist(d), ret(true)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ret == false)
			return;

		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;

			if (ch->IsPC())
			{
				if (DISTANCE_APPROX(ch->GetX() - x * 100, ch->GetY() - y * 100) > dist * 100)
					ret = false;
			}
		}
	}
};

bool CDungeon::IsAllPCNearTo(int x, int y, int dist)
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return false;
	}

	FNearPosition f(x, y, dist);

	// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
	pMap->for_each(f);

	return f.ret;
}

struct FTransferMonsters
{
	LPCHARACTER m_ch;
	FTransferMonsters(LPCHARACTER ch) :
		m_ch(ch)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (!(ch->IsDead()) && (ch->IsMonster() || ch->IsStone()))
			{
				if (m_ch->GetGMLevel() > GM_PLAYER)
				{
					char buf[256];
					snprintf(buf, sizeof(buf), "name %s pos %ld %ld", ch->GetName(), ch->GetX(), ch->GetY());
					m_ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);
				}
				ch->Show(m_ch->GetMapIndex(), m_ch->GetX(), m_ch->GetY(), 0);
				ch->Stop();
			}
		}
	}
};
void CDungeon::TransferAllMonsters(LPCHARACTER ch)
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return;
	}

	FTransferMonsters f(ch);

	pMap->for_each(f);
}

void CDungeon::RegisterItem(DWORD id)
{
	m_registeredItems.insert(id);
}
bool CDungeon::IsRegisteredItem(DWORD id)
{
	itertype(m_registeredItems) it = std::find(m_registeredItems.begin(), m_registeredItems.end(), id);
	if (it == m_registeredItems.end())
		return false;
	else
		return true;
}
void CDungeonManager::FindPcRegisteredDungeon(DWORD pid, std::set<LPDUNGEON>& outSet)
{
	for (std::map<long, LPDUNGEON>::reverse_iterator it = m_map_pkMapDungeon.rbegin(); it != m_map_pkMapDungeon.rend(); ++it)
		if (it->second->IsRegisteredUser(pid))
			outSet.insert(it->second);
}
LPDUNGEON CDungeonManager::FindLastRegisteredDungeon(DWORD pid, long mapIndex)
{
	for (std::map<long, LPDUNGEON>::reverse_iterator it = m_map_pkMapDungeon.rbegin(); it != m_map_pkMapDungeon.rend(); ++it)
		if (it->second->GetOrigMapIndex() == mapIndex && it->second->IsRegisteredUser(pid))
			return it->second;

	return NULL;
}
EVENTFUNC(jump_all_with_delay_event)
{
	jump_all_with_delay_event_info* info = dynamic_cast<jump_all_with_delay_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("jump_all_with_delay_event> <Factor> NULL pointer");
		return 0;
	}

	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap(info->m_lMapIndex);
	if (pSecMap)
	{
		FWarpToPosition f(info->m_lMapIndex, info->m_x, info->m_y);
		pSecMap->for_each(f);
	}

	FNotice f(info->msg);
	pSecMap->for_each(f);

	return 0;
}

struct FCountMonsterByVnum
{
	int n;
	DWORD m_vnum;
	FCountMonsterByVnum(DWORD vnum) :
		n(0), m_vnum(vnum)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (!(ch->IsDead()) && (ch->GetRaceNum() == m_vnum))//FIX if (!ch->IsPC())
				n++;
		}
	}
};

int CDungeon::CountRealMonster(DWORD vnum)
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return 0;
	}

	if (vnum == 0)
		;//TODO

	FCountMonsterByVnum f(vnum);

	// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
	pMap->for_each(f);
	return f.n;
}

struct FIsEliminated
{
	bool is;
	FIsEliminated() : is(true)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if ((ch->IsMonster() || ch->IsStone()) && !ch->IsDead())//FIX if (!ch->IsPC())
			{
				is = false;
				return;
			}
		}
	}
};

bool CDungeon::IsEliminated()
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (!pMap)
	{
		sys_err("cannot find map by index %d", m_lMapIndex);
		return false;
	}

	FIsEliminated f;

	pMap->for_each(f);
	return f.is;
}
