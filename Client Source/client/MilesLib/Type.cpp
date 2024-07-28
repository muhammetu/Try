#include "StdAfx.h"
#include "Type.h"
#include "../EterLib/TextFileLoader.h"

std::string NSound::strResult;

const char* NSound::GetResultString()
{
	return strResult.c_str();
}

void NSound::SetResultString(const char* c_pszStr)
{
	strResult.assign(c_pszStr);
}

bool NSound::LoadSoundInformationPiece(const char* c_szFileName, NSound::TSoundDataVector& rSoundDataVector, const char* c_szPathHeader)
{
	std::string strResult;
	strResult = c_szFileName;

	CTextFileLoader* pkTextFileLoader = CTextFileLoader::Cache(c_szFileName);
	if (!pkTextFileLoader)
		return false;

	CTextFileLoader& rkTextFileLoader = *pkTextFileLoader;
	if (rkTextFileLoader.IsEmpty())
	{
		SetResultString((strResult + " Can not open file for reading").c_str());
		return false;
	}

	rkTextFileLoader.SetTop();

	int iCount;
	if (!rkTextFileLoader.GetTokenInteger("sounddatacount", &iCount))
	{
		SetResultString((strResult + " File format error, SoundDataCount Unable to find.").c_str());
		return false;
	}

#ifdef ENABLE_MILES_REWORK
	float fVolume = 0.0f;
	if (rkTextFileLoader.GetTokenFloat("soundpositionenable", &fVolume))
	{
		SetResultString((strResult + " SoundPosition has checked.").c_str());
	}
	else
	{
		SetResultString((strResult + " SoundPosition has not checked.").c_str());
	}
#endif

	rSoundDataVector.clear();
	rSoundDataVector.resize(iCount);

	char szSoundDataHeader[32 + 1];
	for (DWORD i = 0; i < rSoundDataVector.size(); ++i)
	{
		_snprintf(szSoundDataHeader, sizeof(szSoundDataHeader), "sounddata%02d", i);
		CTokenVector* pTokenVector;
		if (!rkTextFileLoader.GetTokenVector(szSoundDataHeader, &pTokenVector))
		{
			SetResultString((strResult + " File format error: " + szSoundDataHeader + " Unable to find").c_str());
			return false;
		}

		if (2 != pTokenVector->size())
		{
			SetResultString((strResult + " File format error: The size of the vector is not 2").c_str());
			return false;
		}

		rSoundDataVector[i].fTime = (float)atof(pTokenVector->at(0).c_str());
		if (c_szPathHeader)
		{
			rSoundDataVector[i].strSoundFileName = c_szPathHeader;
			rSoundDataVector[i].strSoundFileName += pTokenVector->at(1).c_str();
		}
		else
		{
			rSoundDataVector[i].strSoundFileName = pTokenVector->at(1).c_str();
		}
#ifdef ENABLE_MILES_REWORK
		rSoundDataVector[i].fSoundVolume = fVolume;
#endif
	}

	SetResultString((strResult + " Loaded").c_str());
	return true;
}

void NSound::DataToInstance(const TSoundDataVector& c_rSoundDataVector, TSoundInstanceVector* pSoundInstanceVector)
{
	if (c_rSoundDataVector.empty())
		return;

	DWORD dwFPS = 60;
	const float c_fFrameTime = 1.0f / float(dwFPS);

	pSoundInstanceVector->clear();
	pSoundInstanceVector->resize(c_rSoundDataVector.size());
	for (DWORD i = 0; i < c_rSoundDataVector.size(); ++i)
	{
		const TSoundData& c_rSoundData = c_rSoundDataVector[i];
		TSoundInstance& rSoundInstance = pSoundInstanceVector->at(i);

		rSoundInstance.dwFrame = (DWORD)(c_rSoundData.fTime / c_fFrameTime);
		rSoundInstance.strSoundFileName = c_rSoundData.strSoundFileName;
#ifdef ENABLE_MILES_REWORK
		rSoundInstance.fSoundVolume = c_rSoundData.fSoundVolume;
#endif
	}
}