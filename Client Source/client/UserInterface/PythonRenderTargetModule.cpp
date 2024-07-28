#include "StdAfx.h"
#ifdef ENABLE_MODEL_RENDER_TARGET
#include "../EterPythonLib/PythonWindow.h"
#include "PythonApplication.h"

PyObject* renderTargetRemove(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	CRenderTargetManager::Instance().Remove(window);
	return Py_BuildNone();
}
PyObject* renderTargetRender(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();
	int renderWindow = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &renderWindow))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
	{
		TraceError("renderTargetRender: renderTarget not found!");
		return Py_BuildNone();
	}

	renderTarget->SetRenderingRect(&(((UI::CWindow*)renderWindow)->GetRect()));
	renderTarget->RenderTexture();
	return Py_BuildNone();
}

PyObject* renderTargetCreate(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();
	DWORD w;
	if (!PyTuple_GetUnsignedLong(poArgs, 1, &w))
		return Py_BuildException();
	DWORD h;
	if (!PyTuple_GetUnsignedLong(poArgs, 2, &h))
		return Py_BuildException();

	CRenderTargetManager::Instance().CreateRenderTarget(window, w, h);
	return Py_BuildNone();
}

PyObject* renderTargetSelectModel(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD race = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &race))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
	{
		TraceError("renderTargetSelectModel: renderTarget not found!");
		return Py_BuildNone();
	}

	renderTarget->SelectModel(race);
	return Py_BuildNone();
}

PyObject* renderTargetSetVisibility(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	bool isShow = false;
	if (!PyTuple_GetBoolean(poArgs, 1, &isShow))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
	{
		TraceError("renderTargetSetVisibility: renderTarget not found!");
		return Py_BuildNone();
	}

	renderTarget->SetVisibility(isShow);
	return Py_BuildNone();
}

PyObject* renderTargetSetBackgroundImage(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();
	char* szPathName;
	if (!PyTuple_GetString(poArgs, 1, &szPathName))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
	{
		TraceError("renderTargetSetBackgroundImage: renderTarget not found!");
		return Py_BuildNone();
	}

	renderTarget->CreateBackground(szPathName, CPythonApplication::Instance().GetWidth(), CPythonApplication::Instance().GetHeight());
	return Py_BuildNone();
}

// @@
// Instance Functions
PyObject* renderTargetChangeGuild(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD guildID = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &guildID))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->ChangeGuild(guildID);
	return Py_BuildNone();
}

PyObject* renderTargetSetArmor(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD armor = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &armor))
		return Py_BuildException();

	bool bDisableWeapon = false;
	if (!PyTuple_GetBoolean(poArgs, 2, &bDisableWeapon))
		bDisableWeapon = false;

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pInstance)
		return Py_BuildNone();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetArmor(armor);
	DWORD weaponIndex = pInstance->GetPart(CRaceData::PART_WEAPON);
#ifdef ENABLE_ACCE_SYSTEM
	DWORD acceIndex = pInstance->GetPart(CRaceData::PART_ACCE);
#endif

	if (weaponIndex > 0)
		renderTarget->SetWeapon(bDisableWeapon ? 0 : weaponIndex);
#ifdef ENABLE_ACCE_SYSTEM
	if (acceIndex > 0)
		renderTarget->SetAcce(acceIndex); // oder SetSash, je nach dem
#endif
	return Py_BuildNone();
}

PyObject* renderTargetSetWeapon(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD weapon = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &weapon))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetWeapon(0);
	renderTarget->SetWeapon(weapon);
	return Py_BuildNone();
}

PyObject* renderTargetSetHair(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD hair = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &hair))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetHair(hair);
	return Py_BuildNone();
}

#ifdef ENABLE_ACCE_SYSTEM
PyObject* renderTargetSetAcce(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD acce = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &acce))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetAcce(acce);
	return Py_BuildNone();
}

PyObject* renderTargetSetShining(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD index = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &index))
		return Py_BuildException();

	DWORD shining = 0;
	if (!PyTuple_GetDWORD(poArgs, 2, &shining))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetShining(index, shining);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
PyObject* renderTargetSetAura(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD aura = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &aura))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetAura(aura);
	return Py_BuildNone();
}
#endif

PyObject* renderTargetSetLevel(PyObject * poSelf, PyObject * poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD level = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &level))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetLevel(level);
	return Py_BuildNone();
}

PyObject* renderTargetSetAlignment(PyObject * poSelf, PyObject * poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD alignment = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &alignment))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetAlignment(alignment);
	return Py_BuildNone();
}

PyObject* renderTargetSetPKMode(PyObject * poSelf, PyObject * poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	DWORD pkMode = 0;
	if (!PyTuple_GetDWORD(poArgs, 1, &pkMode))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetPKMode(pkMode);
	return Py_BuildNone();
}

#ifdef USE_MODEL_RENDER_TARGET_UI
PyObject* renderTargetSetMainCharacter(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();
	
	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	DWORD armor = CPythonPlayer::instance().GetItemTransmutation(TItemPos(EQUIPMENT, c_Costume_Slot_Body));
	if (armor == 0)
	{
		armor = CPythonPlayer::instance().GetItemIndex(TItemPos(EQUIPMENT, c_Costume_Slot_Body));
		if (armor == 0)
		{
			armor = CPythonPlayer::instance().GetItemTransmutation(TItemPos(EQUIPMENT, c_Equipment_Body));
			if (armor == 0)
			{
				armor = CPythonPlayer::instance().GetItemIndex(TItemPos(EQUIPMENT, c_Equipment_Body));
			}
		}
	}

	CInstanceBase* instance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (!instance)
		return Py_BuildNone();

	DWORD weapon = instance->GetPart(CRaceData::PART_WEAPON);
	DWORD hair = instance->GetPart(CRaceData::PART_HAIR);
#ifdef ENABLE_ACCE_SYSTEM
	DWORD acce = instance->GetPart(CRaceData::PART_ACCE);
	DWORD shining[CHR_SHINING_NUM];
	for (int i = 0; i < CHR_SHINING_NUM; ++i) 
	{
		shining[i] = instance->m_dwShining[i];
	}
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
	DWORD aura = instance->GetPart(CRaceData::PART_AURA);
#endif

	renderTarget->SelectModel(CPythonPlayer::instance().GetRace());
	renderTarget->SetVisibility(true);

	if (armor != 0)
		renderTarget->SetArmor(armor);

	if (weapon != 0)
		renderTarget->SetWeapon(weapon);

	if (hair != 0)
		renderTarget->SetHair(hair);
	
#ifdef ENABLE_ACCE_SYSTEM
	if (acce != 0)
		renderTarget->SetAcce(acce);
#endif

#ifdef ENABLE_AURA_COSTUME_SYSTEM
	if (aura != 0)
		renderTarget->SetAura(aura);
#endif

#ifdef ENABLE_SHINING_ITEM_SYSTEM
	for (int i = 0; i < CHR_SHINING_NUM; ++i)
		renderTarget->SetShining(i, shining[i]);
#endif

	return Py_BuildNone();
}

PyObject* renderTargetSetScale(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	float xScale;
	if (!PyTuple_GetFloat(poArgs, 1, &xScale))
		return Py_BuildException();
	float yScale;
	if (!PyTuple_GetFloat(poArgs, 2, &yScale))
		return Py_BuildException();
	float zScale;
	if (!PyTuple_GetFloat(poArgs, 3, &zScale))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetScale(zScale, yScale, xScale);
	return Py_BuildNone();
}

PyObject* renderTargetGetScale(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();

	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	const D3DXVECTOR3& scale = renderTarget->GetScale();
	return Py_BuildValue("(fff)", scale.x, scale.y, scale.z);
}
#endif

PyObject* renderTargetSetMenuWindow(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();
	
	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetMenu(true);
	return Py_BuildNone();
}

PyObject* renderTargetSetWikiWindow(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* window = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &window))
		return Py_BuildException();
	
	auto renderTarget = CRenderTargetManager::Instance().GetRenderTarget(window);
	if (renderTarget == nullptr)
		return Py_BuildException();

	renderTarget->SetWiki(true);
	return Py_BuildNone();
}

void initRenderTarget()
{
	static PyMethodDef s_methods[] =
	{
		{ "Create",							renderTargetCreate,					METH_VARARGS },
		{ "Remove",							renderTargetRemove,					METH_VARARGS },
		{ "Render",							renderTargetRender,					METH_VARARGS },
		{ "SelectModel",					renderTargetSelectModel,			METH_VARARGS },
		{ "SetVisibility",					renderTargetSetVisibility,			METH_VARARGS },
		{ "SetBackgroundImage",				renderTargetSetBackgroundImage,		METH_VARARGS },

		// @@ Instance Functions
		{ "ChangeGuild",					renderTargetChangeGuild,			METH_VARARGS },
		{ "SetArmor",						renderTargetSetArmor,				METH_VARARGS },
		{ "SetWeapon",						renderTargetSetWeapon,				METH_VARARGS },
		{ "SetHair",						renderTargetSetHair,				METH_VARARGS },
#ifdef ENABLE_ACCE_SYSTEM
		{ "SetAcce",						renderTargetSetAcce,				METH_VARARGS },
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
		{ "SetAura",						renderTargetSetAura,				METH_VARARGS },
		{ "SetShining",						renderTargetSetShining,				METH_VARARGS },
#endif

		{ "SetLevel",						renderTargetSetLevel,				METH_VARARGS },
		{ "SetAlignment",					renderTargetSetAlignment,			METH_VARARGS },
		{ "SetPKMode",						renderTargetSetPKMode,				METH_VARARGS },

#ifdef USE_MODEL_RENDER_TARGET_UI
		{ "SetMainCharacter",				renderTargetSetMainCharacter,		METH_VARARGS },
		{ "SetScale",						renderTargetSetScale,				METH_VARARGS},
		{ "GetScale",						renderTargetGetScale,				METH_VARARGS},
#endif

		{ "SetMenuWindow",				renderTargetSetMenuWindow,		METH_VARARGS},
		{ "SetWikiWindow",				renderTargetSetWikiWindow,		METH_VARARGS},
		{nullptr, nullptr, 0 },
	};

	PyObject* poModule = Py_InitModule("renderTarget", s_methods);
}
#endif