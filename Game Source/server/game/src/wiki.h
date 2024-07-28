#pragma once

class CWikiManager : public singleton<CWikiManager>
{
public:
	enum EMakeTypes
	{
		WIKI_MAKE_ALL,
		WIKI_MAKE_ITEM,
		WIKI_MAKE_REFINE,
		WIKI_MAKE_CHEST,
		WIKI_MAKE_CUBE,
		WIKI_MAKE_STONE,
		WIKI_MAKE_BOSS,
		WIKI_MAKE_CHESTDROP_INFO,
		WIKI_MAKE_MOBDROP_INFO,
		WIKI_MAKE_SHOP,
	};
	enum EVnumType
	{
		VNUM_TYPE_CHEST,
		VNUM_TYPE_MOB,
	};

	CWikiManager();
	virtual ~CWikiManager();

	void	HandleCommand(LPCHARACTER ch, const char* argument);
	bool	Make(EMakeTypes type);
	void	CreateDropItemVector(CMob& mob, std::vector<std::pair<DWORD, DWORD>>& vec_item);

	void	CreateGMDropInfoVector(DWORD vnum, EVnumType vnumType, DWORD count, std::map<DWORD, DWORD>& vec_item);

protected:
	FILE *m_regenInfoFile;
public:
	void	StartRegenInfoFile();
	void	AddRegenInfoFile(const char* filename, long lMapIndex);
	void	EndRegenInfoFile();
};
