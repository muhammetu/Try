#include "Stdafx.h"
#include "../effectLib/EffectManager.h"

#include "FlyingData.h"

CDynamicPool<CFlyingData> CFlyingData::ms_kPool;

CFlyingData::CFlyingData()
{
	__Initialize();
}

void CFlyingData::__Initialize()
{
	m_v3AngVel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_bIsHoming = false;
	m_fInitVel = 200.0f;
	m_fConeAngle = (0.0f);
	m_fRollAngle = 0.0f;
	m_fRange = 500.0f;
	m_v3Accel = D3DXVECTOR3(0, 0, 0.0f);// gravity direction :-z
	m_fHomingMaxAngle = 3.0f;
	m_fHomingStartTime = 0.0f;
	m_fGravity = 0;
	m_bSpreading = false;
	m_bMaintainParallel = false;

	m_bHitOnBackground = false;
	m_bHitOnAnotherMonster = false;
	m_iPierceCount = 0;
	m_fCollisionSphereRadius = 0.0f;

	m_fBombRange = 10.0f;
	SetBombEffect("");
}

void CFlyingData::Destroy()
{
	m_AttachDataVector.clear();

	__Initialize();
}

CFlyingData::~CFlyingData()
{
	Destroy();
}

void CFlyingData::SetBombEffect(const char* szEffectName)
{
	m_strBombEffectName = szEffectName;
	if (m_strBombEffectName.empty())
	{
		m_dwBombEffectID = 0;
		return;
	}
	StringPath(m_strBombEffectName);
	if (CEffectManager::Instance().RegisterEffect(m_strBombEffectName.c_str(), true))
	{
		m_dwBombEffectID = GetCaseCRC32(m_strBombEffectName.c_str(), m_strBombEffectName.size());
	}
	else
	{
		m_dwBombEffectID = 0;
	}
}

CFlyingData::TFlyingAttachData& CFlyingData::GetAttachDataReference(int iIndex)
{
	return m_AttachDataVector[iIndex];
}

DWORD CFlyingData::AttachFlyEffect(int iType, const std::string& strFilename, float fRoll, float fArg1, float fArg2)
{
	TFlyingAttachData fad;
	memset(&fad, 0, sizeof(fad));
	fad.iType = FLY_ATTACH_EFFECT;
	fad.iFlyType = iType;
	fad.fPeriod = 1.0f;
	fad.strFilename = strFilename;
	fad.fRoll = fRoll;
	fad.dwTailColor = 0xffffffff;
	fad.fTailLength = 1.0f;
	fad.fTailSize = 10.0f;

	switch (iType)
	{
	case FLY_ATTACH_TYPE_MULTI_LINE:
		fad.fDistance = fArg1;
		break;
	case FLY_ATTACH_TYPE_SINE:
	case FLY_ATTACH_TYPE_EXP:
		fad.fPeriod = fArg1;
		fad.fAmplitude = fArg2;
		break;
	case FLY_ATTACH_TYPE_NONE:
	case FLY_ATTACH_TYPE_LINE:
	default:
		// Do nothing
		break;
	}

	m_AttachDataVector.push_back(fad);
	return m_AttachDataVector.size() - 1;
}

void CFlyingData::RemoveAttach(int iIndex)
{
	assert(0 <= iIndex && iIndex < (int)m_AttachDataVector.size());
	m_AttachDataVector.erase(m_AttachDataVector.begin() + iIndex);
}

void CFlyingData::RemoveAllAttach()
{
	m_AttachDataVector.clear();
}

void CFlyingData::DuplicateAttach(int iIndex)
{
	assert(0 <= iIndex && iIndex < (int)m_AttachDataVector.size());
	m_AttachDataVector.push_back(m_AttachDataVector[iIndex]);
}

bool CFlyingData::LoadScriptFile(const char* c_szFilename)
{
	Destroy();

	m_strFilename = c_szFilename;
	StringPath(m_strFilename);
	CTextFileLoader TextFileLoader;
	if (!TextFileLoader.Load(m_strFilename.c_str()))
		return false;

	TextFileLoader.SetTop();
	int temp;

	if (!TextFileLoader.GetTokenInteger("spreadingflag", &temp))
	{
		m_bSpreading = false;
	}
	else
	{
		m_bSpreading = temp ? true : false;
	}

	if (!TextFileLoader.GetTokenInteger("maintainparallelflag", &temp))
	{
		m_bMaintainParallel = false;
	}
	else
	{
		m_bMaintainParallel = temp ? true : false;
	}

	if (!TextFileLoader.GetTokenFloat("initialvelocity", &m_fInitVel))
	{
		return false;
	}

	TextFileLoader.GetTokenFloat("coneangle", &m_fConeAngle);
	//if (!TextFileLoader.GetTokenFloat("coneangle", &m_fConeAngle))
	//	m_fConeAngle = 0.0f;

	TextFileLoader.GetTokenFloat("rollangle", &m_fRollAngle);
	//if (!TextFileLoader.GetTokenFloat("rollangle", &m_fRollAngle))
	//	m_fRollAngle = 0.0f;

	TextFileLoader.GetTokenVector3("angularvelocity", &m_v3AngVel);
	//if (!TextFileLoader.GetTokenVector3("angularvelocity",&m_v3AngVel))
	//	m_v3AngVel = D3DXVECTOR3(0.0f,0.0f,0.0f);

	TextFileLoader.GetTokenFloat("gravity", &m_fGravity);
	//if (!TextFileLoader.GetTokenFloat("gravity",&m_fGravity))
		//m_fGravity = 0.0f;

	if (TextFileLoader.GetTokenInteger("hitonbackground", &temp))
		m_bHitOnBackground = temp ? true : false;
	else
		m_bHitOnBackground = false;

	if (TextFileLoader.GetTokenInteger("hitonanothermonster", &temp))
		m_bHitOnAnotherMonster = temp ? true : false;
	else
		m_bHitOnAnotherMonster = false;

	if (!TextFileLoader.GetTokenInteger("piercecount", &m_iPierceCount))
		m_iPierceCount = 0;

	//	if (!TextFileLoader.GetTokenFloat("collisionsphereradius",&m_fCollisionSphereRadius))
	//		m_fCollisionSphereRadius = 0.0f;

	TextFileLoader.GetTokenFloat("bombrange", &m_fBombRange);
	//if (!TextFileLoader.GetTokenFloat("bombrange",&m_fBombRange))
	//	m_fBombRange = 10.0f;

	if (!TextFileLoader.GetTokenString("bombeffect", &m_strBombEffectName))
	{
		m_strBombEffectName = "";
	}
	else if (!m_strBombEffectName.empty())
	{
		if (!IsGlobalFileName(m_strBombEffectName.c_str()))
			m_strBombEffectName = GetOnlyPathName(TextFileLoader.GetFileName()) + m_strBombEffectName;
		CEffectManager::Instance().RegisterEffect2(m_strBombEffectName.c_str(), &m_dwBombEffectID);
	}

	if (!TextFileLoader.GetTokenInteger("homingflag", &temp))
		m_bIsHoming = false;
	else
		m_bIsHoming = temp ? true : false;

	if (!TextFileLoader.GetTokenFloat("homingstarttime", &m_fHomingStartTime))
		m_fHomingStartTime = 0.0f;

	if (!TextFileLoader.GetTokenFloat("homingmaxangle", &m_fHomingMaxAngle))
		m_fHomingMaxAngle = 0.0f;

	if (!TextFileLoader.GetTokenFloat("range", &m_fRange))
		return false;

	if (!TextFileLoader.GetTokenVector3("acceleration", &m_v3Accel))
		m_v3Accel = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	DWORD i;
	for (i = 0; i < TextFileLoader.GetChildNodeCount(); i++)
	{
		CTextFileLoader::CGotoChild GotoChild(&TextFileLoader, i);

		std::string strNodeName;

		TextFileLoader.GetCurrentNodeName(&strNodeName);

		if (strNodeName == "attachdata")
		{
			TFlyingAttachData fad;

			if (!TextFileLoader.GetTokenInteger("type", &fad.iType))
				continue;
			TextFileLoader.GetTokenInteger("flytype", &fad.iFlyType);
			if (!TextFileLoader.GetTokenString("attachfile", &fad.strFilename))
			{
				fad.strFilename = "";
			}
			else if (!fad.strFilename.empty())
			{
				if (!IsGlobalFileName(fad.strFilename.c_str()))
					fad.strFilename = GetOnlyPathName(TextFileLoader.GetFileName()) + fad.strFilename;
				CEffectManager::Instance().RegisterEffect(fad.strFilename.c_str());
			}

			TextFileLoader.GetTokenInteger("tailflag", &temp);
			if (temp)
			{
				fad.bHasTail = true;
				TextFileLoader.GetTokenDoubleWord("tailcolor", &fad.dwTailColor);
				TextFileLoader.GetTokenFloat("taillength", &fad.fTailLength);
				TextFileLoader.GetTokenFloat("tailsize", &fad.fTailSize);
				TextFileLoader.GetTokenInteger("tailshaperect", &temp);
				fad.bRectShape = temp ? true : false;
			}
			else
			{
				fad.bHasTail = false;
			}

			TextFileLoader.GetTokenFloat("roll", &fad.fRoll);
			TextFileLoader.GetTokenFloat("distance", &fad.fDistance);
			TextFileLoader.GetTokenFloat("period", &fad.fPeriod);
			TextFileLoader.GetTokenFloat("amplitude", &fad.fAmplitude);

			m_AttachDataVector.push_back(fad);
		}
		else
		{
			TraceError("CFlyingData::LoadScriptFile Wrong Data : %s", m_strFilename.c_str());
		}
	}

	return true;
}