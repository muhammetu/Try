#pragma once

#include "../EffectLib/StdAfx.h"
#include "../eterlib/Resource.h"
#include "../eterlib/ResourceManager.h"

enum EResourceTypes
{
	RES_TYPE_UNKNOWN,
};

class CPythonResource : public CSingleton<CPythonResource>
{
public:
	CPythonResource();
	virtual ~CPythonResource();

	void Destroy();

protected:
	CResourceManager m_resManager;
};