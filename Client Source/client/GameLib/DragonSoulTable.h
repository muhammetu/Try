#pragma once

#ifdef ENABLE_DS_SET
#include "ItemData.h"

#define DRAGON_SOUL_STRENGTH_MAX 7

const std::string g_astGradeName[] =
{
	"grade_normal",
	"grade_brilliant",
	"grade_rare",
	"grade_ancient",
	"grade_legendary",
#ifdef ENABLE_DS_GRADE_MYTH
	"grade_myth",
#endif
#ifdef ENABLE_DS_GRADE_EPIC
	"grade_epic",
#endif
#ifdef ENABLE_DS_GRADE_GODLIKE
	"grade_godlike",
#endif
};

const std::string g_astStepName[] =
{
	"step_lowest",
	"step_low",
	"step_mid",
	"step_high",
	"step_highest",
#ifdef ENABLE_NEW_DRAGON_SOUL_SINIF
	"step_new1",
	"step_new2",
	"step_new3",
	"step_new4",
	"step_new5",
#endif
};

class CDragonSoulTable
{
	enum EDragonSoulGradeTypes
	{
		DRAGON_SOUL_GRADE_NORMAL,
		DRAGON_SOUL_GRADE_BRILLIANT,
		DRAGON_SOUL_GRADE_RARE,
		DRAGON_SOUL_GRADE_ANCIENT,
		DRAGON_SOUL_GRADE_LEGENDARY,
#ifdef ENABLE_DS_GRADE_MYTH
		DRAGON_SOUL_GRADE_MYTH,
#endif
#ifdef ENABLE_DS_GRADE_EPIC
		DRAGON_SOUL_GRADE_EPIC,
#endif
#ifdef ENABLE_DS_GRADE_GODLIKE
		DRAGON_SOUL_GRADE_GODLIKE,
#endif
		DRAGON_SOUL_GRADE_MAX,
	};

	enum EDragonSoulStepTypes
	{
		DRAGON_SOUL_STEP_LOWEST,
		DRAGON_SOUL_STEP_LOW,
		DRAGON_SOUL_STEP_MID,
		DRAGON_SOUL_STEP_HIGH,
		DRAGON_SOUL_STEP_HIGHEST,
#ifdef ENABLE_NEW_DRAGON_SOUL_SINIF
		DRAGON_SOUL_STEP_NEW1,
		DRAGON_SOUL_STEP_NEW2,
		DRAGON_SOUL_STEP_NEW3,
		DRAGON_SOUL_STEP_NEW4,
		DRAGON_SOUL_STEP_NEW5,
#endif
		DRAGON_SOUL_STEP_MAX,
	};

protected:
	CTextFileLoader TextFileLoader;

	struct DragonSoulBasicApply
	{
		std::string stApplyName;
		uint16_t iApplyType;
		uint16_t iApplyValue;
	};

	struct DragonSoulAdditionalApply
	{
		std::string stApplyName;
		uint16_t iApplyType;
		uint16_t iApplyValue;
		uint8_t iProb;
	};

	struct DragonSoulItem
	{
		uint8_t iType;
		std::vector<DragonSoulBasicApply> vec_BasicApplys;
		std::vector<DragonSoulAdditionalApply> vec_AdditionalApplys;
	};

	std::map<uint8_t, std::string> m_map_VnumMapper;
	std::map<std::string, DragonSoulItem> m_map_DragonSoulTable;
	uint8_t WeightTable[DRAGON_SOUL_GRADE_MAX][DRAGON_SOUL_STEP_MAX][DRAGON_SOUL_STRENGTH_MAX];

public:
	CDragonSoulTable();
	~CDragonSoulTable();

	bool Load(const char* szDragonSoulTable);
	uint8_t GetDSSetWeight(uint8_t iSetGrade);
	uint8_t GetDSBasicApplyCount(uint8_t iDSType);
	uint16_t GetDSBasicApplyValue(uint8_t iDSType, uint8_t iApplyType);
	uint16_t GetDSAdditionalApplyValue(uint8_t iDSType, uint8_t iApplyType);
	uint16_t GetApplyType(std::string stApplyName);

private:
	bool LoadVnumMapper();
	bool LoadBasicApplys();
	bool LoadAdditionalApplys();
	bool LoadWeightTable();
};

#endif