#pragma once
#if defined(ENABLE_GROWTH_MOUNT_SYSTEM)
#include "../gamelib/ItemData.h"
#include "../EterBase/Poly/Poly.h"

class CInstanceBase;

class CPythonSkillMount : public CSingleton<CPythonSkillMount>
{
public:
	enum
	{
		SKILLMOUNT_TYPE_NONE,
		SKILLMOUNT_TYPE_PASSIVE,
		SKILLMOUNT_TYPE_AUTO,
		SKILLMOUNT_TYPE_MAX_NUM,
	};

	enum ESkillMountDescTokenType
	{
		DESCMOUNT_TOKEN_TYPE_VNUM,
		DESCMOUNT_TOKEN_TYPE_NAME,
		DESCMOUNT_TOKEN_TYPE_ICON_NAME,
		DESCMOUNT_TOKEN_TYPE,
		DESCMOUNT_TOKEN_TYPE_DESCRIPTION,
		DESCMOUNT_TOKEN_TYPE_DELAY,
		DESCMOUNT_TOKEN_TYPE_MAX_NUM,

		CONDITIONMOUNT_COLUMN_COUNT = 3,
		AFFECTMOUNT_COLUMN_COUNT = 3,
		AFFECTMOUNT_STEP_COUNT = 3,
		MOUNT_SKILL_COUNT = 3,
	};

	int mountslot[MOUNT_SKILL_COUNT];

	typedef struct SSkillDataMount
	{
		// Functions
		SSkillDataMount();
		///////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////

		// Variable
		DWORD dwSkillIndex;
		std::string strName;
		std::string strIconFileName;
		BYTE byType;
		std::string strDescription;
		DWORD dwskilldelay;

		CGraphicImage* pImage;

		/////
	} TSkillDataMount;

	typedef std::map<DWORD, TSkillDataMount> TSkillDataMountMap;

public:
	CPythonSkillMount();
	virtual ~CPythonSkillMount();

	void SetSkillbySlot(int slot, int skillIndex);

	BOOL GetSkillIndex(int slot, int* skillIndex);
	BOOL GetSkillData(DWORD dwSkillIndex, TSkillDataMount** ppSkillData);
	void Destroy();
	bool RegisterSkillMount(const char* c_szFileName);

protected:
	void __RegisterNormalIconImage(TSkillDataMount& rData, const char* c_szHeader, const char* c_szImageName);

protected:
	TSkillDataMountMap m_SkillDataMountMap;
	std::map<std::string, DWORD> m_SkillMountTypeIndexMap;
};
#endif