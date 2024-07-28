#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "PythonNonPlayer.h"
#include "../gamelib/GameLibDefines.h"

#if defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_GROWTH_MOUNT_SYSTEM)
#include "PythonNetworkStream.h"
#include "PythonPlayer.h"
#endif

PyObject* chrRaceToJob(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	return Py_BuildValue("i", RaceToJob(race));
}

PyObject* chrRaceToSex(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	return Py_BuildValue("i", RaceToSex(race));
}

PyObject* chrDestroy(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager::Instance().Destroy();
	return Py_BuildNone();
}

PyObject* chrUpdate(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager::Instance().Update();
	return Py_BuildNone();
}

PyObject* chrDeform(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager::Instance().Deform();
	return Py_BuildNone();
}

PyObject* chrRender(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager::Instance().Render();
	return Py_BuildNone();
}

// Functions For Python Test Code
PyObject* chrCreateInstance(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CPythonCharacterManager::Instance().RegisterInstance(iVirtualID);
	return Py_BuildNone();
}

PyObject* chrDeleteInstance(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CPythonCharacterManager::Instance().DeleteInstance(iVirtualID);
	return Py_BuildNone();
}

PyObject* chrSelectInstance(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CPythonCharacterManager::Instance().SelectInstance(iVirtualID);
	return Py_BuildNone();
}

PyObject* chrHasInstance(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	bool bFlag = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID) ? TRUE : FALSE;
	return Py_BuildValue("i", bFlag);
}

PyObject* chrIsEnemy(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->IsEnemy());
}

PyObject* chrIsNPC(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->IsNPC());
}

PyObject* chrIsDoor(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->IsDoor());
}

PyObject* chrIsGameMaster(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->IsGameMaster());
}

PyObject* chrIsPartyMember(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->IsPartyMember());
}

PyObject* chrSelect(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pkInst)
		return Py_BuildNone();

	switch (CPythonNonPlayer::Instance().GetEventType(pkInst->GetVirtualNumber()))
	{
	case CPythonNonPlayer::ON_CLICK_EVENT_SHOP:
		pkInst->SetAddRenderMode();
		pkInst->SetAddColor(D3DXCOLOR(0.0f, 0.3f, 0.0f, 1.0f));
		break;

	default:
		//pkInst->SetAddColor(D3DXCOLOR(0.3f, 0.0f, 0.0f, 1.0f));
		break;
	}

	return Py_BuildNone();
}

PyObject* chrUnselect(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pkInst)
		return Py_BuildNone();

	pkInst->RestoreRenderMode();
	return Py_BuildNone();
}

PyObject* chrPick(PyObject* poSelf, PyObject* poArgs)
{
	DWORD VirtualID = 0;
	if (CPythonCharacterManager::Instance().OLD_GetPickedInstanceVID(&VirtualID))
		return Py_BuildValue("i", VirtualID);
	else
		return Py_BuildValue("i", -1);
}

PyObject* chrHide(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->Hide();
	return Py_BuildNone();
}

PyObject* chrShow(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->Show();
	return Py_BuildNone();
}

PyObject* chrPickAll(PyObject* poSelf, PyObject* poArgs)
{
	DWORD VirtualID = CPythonCharacterManager::Instance().PickAll();
	return Py_BuildValue("i", VirtualID);
}

PyObject* chrSetRace(PyObject* poSelf, PyObject* poArgs)
{
	int iRace;
	if (!PyTuple_GetInteger(poArgs, 0, &iRace))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
	pkInst->SetRace(iRace, true);
#else
	pkInst->SetRace(iRace);
#endif

	return Py_BuildNone();
}

PyObject* chrSetHair(PyObject* poSelf, PyObject* poArgs)
{
	int iRace;
	if (!PyTuple_GetInteger(poArgs, 0, &iRace))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->SetHair(iRace);

	return Py_BuildNone();
}

PyObject* chrChangeHair(PyObject* poSelf, PyObject* poArgs)
{
	int iHair;

	if (!PyTuple_GetInteger(poArgs, 0, &iHair))
		return Py_BuildException();
	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->ChangeHair(iHair);
	return Py_BuildNone();
}

PyObject* chrSetArmor(PyObject* poSelf, PyObject* poArgs)
{
	int iForm;
	if (!PyTuple_GetInteger(poArgs, 0, &iForm))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->SetArmor(iForm, 0);
	pkInst->RegisterBoundingSphere();

	return Py_BuildNone();
}

PyObject* chrChangeShape(PyObject* poSelf, PyObject* poArgs)
{
	int iForm;
	if (!PyTuple_GetInteger(poArgs, 0, &iForm))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->ChangeArmor(iForm, 0);

	return Py_BuildNone();
}

PyObject* chrSetWeapon(PyObject* poSelf, PyObject* poArgs)
{
	int iForm;
	if (!PyTuple_GetInteger(poArgs, 0, &iForm))
		return Py_BuildException();
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	int iEvolution;
	if (!PyTuple_GetInteger(poArgs, 1, &iEvolution))
		return Py_BuildException();
#endif
	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	pkInst->SetWeapon(iForm, iEvolution);
#else
	pkInst->SetWeapon(iForm);
#endif
	return Py_BuildNone();
}

PyObject* chrSetVirtualID(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->SetVirtualID(iVID);
	return Py_BuildNone();
}

PyObject* chrSetNameString(PyObject* poSelf, PyObject* poArgs)
{
	char* c_szName;
	if (!PyTuple_GetString(poArgs, 0, &c_szName))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->SetNameString(c_szName, strlen(c_szName));
	return Py_BuildNone();
}

PyObject* chrRefresh(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

	if (!pkInst)
		return Py_BuildNone();

	//pkInst->Refresh(CRaceMotionData::NAME_WAIT, true);
	return Py_BuildNone();
}

PyObject* chrRevive(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->Revive();
	return Py_BuildNone();
}

PyObject* chrSetMotionMode(PyObject* poSelf, PyObject* poArgs)
{
	int iMotionMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->SetMotionMode(iMotionMode);
	return Py_BuildNone();
}

PyObject* chrSetLoopMotion(PyObject* poSelf, PyObject* poArgs)
{
	int iMotionIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->SetLoopMotion(WORD(iMotionIndex));

	return Py_BuildNone();
}

PyObject* chrBlendLoopMotion(PyObject* poSelf, PyObject* poArgs)
{
	int iMotionIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
		return Py_BuildException();
	float fBlendTime;
	if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->SetLoopMotion(WORD(iMotionIndex), fBlendTime);

	return Py_BuildNone();
}

PyObject* chrPushOnceMotion(PyObject* poSelf, PyObject* poArgs)
{
	int iMotionIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
		return Py_BuildException();

	float fBlendTime;
	if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
	{
		fBlendTime = 0.1f;
	}

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->PushOnceMotion(WORD(iMotionIndex), fBlendTime);

	return Py_BuildNone();
}

PyObject* chrPushLoopMotion(PyObject* poSelf, PyObject* poArgs)
{
	int iMotionIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
		return Py_BuildException();

	float fBlendTime;
	if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
	{
		fBlendTime = 0.1f;
	}

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();
	pkInst->PushLoopMotion(WORD(iMotionIndex), fBlendTime);

	return Py_BuildNone();
}

PyObject* chrSetPixelPosition(PyObject* poSelf, PyObject* poArgs)
{
	int iX;
	if (!PyTuple_GetInteger(poArgs, 0, &iX))
		return Py_BuildException();
	int iY;
	if (!PyTuple_GetInteger(poArgs, 1, &iY))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	int iZ;
	if (PyTuple_GetInteger(poArgs, 2, &iZ))
	{
		pkInst->NEW_SetPixelPosition(TPixelPosition(iX, iY, iZ));
	}
	else
	{
		pkInst->SCRIPT_SetPixelPosition(iX, iY);
	}
	return Py_BuildNone();
}

PyObject* chrSetDirection(PyObject* poSelf, PyObject* poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	pkInst->SetDirection(iDirection);
	return Py_BuildNone();
}

PyObject* chrGetPixelPosition(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pkInst)
		return Py_BuildException();

	TPixelPosition PixelPosition;
	pkInst->NEW_GetPixelPosition(&PixelPosition);

	return Py_BuildValue("fff", PixelPosition.x, PixelPosition.y, PixelPosition.z);
}

PyObject* chrSetRotation(PyObject* poSelf, PyObject* poArgs)
{
	float fRotation;
	if (!PyTuple_GetFloat(poArgs, 0, &fRotation))
		return Py_BuildException();

	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

	if (!pCharacterInstance)
		return Py_BuildNone();

	fRotation = fmod(fRotation + 180.0f, 360.0f);

	pCharacterInstance->SetRotation(fRotation);

	return Py_BuildNone();
}

PyObject* chrSetRotationAll(PyObject* poSelf, PyObject* poArgs)
{
	float fRotX;
	if (!PyTuple_GetFloat(poArgs, 0, &fRotX))
		return Py_BuildException();
	float fRotY;
	if (!PyTuple_GetFloat(poArgs, 1, &fRotY))
		return Py_BuildException();
	float fRotZ;
	if (!PyTuple_GetFloat(poArgs, 2, &fRotZ))
		return Py_BuildException();

	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pCharacterInstance)
		return Py_BuildNone();

	pCharacterInstance->GetGraphicThingInstanceRef().SetXYRotation(fRotX, fRotY);
	pCharacterInstance->GetGraphicThingInstanceRef().SetRotation(fRotZ);
	return Py_BuildNone();
}

PyObject* chrBlendRotation(PyObject* poSelf, PyObject* poArgs)
{
	float fRotation;
	if (!PyTuple_GetFloat(poArgs, 0, &fRotation))
		return Py_BuildException();
	float fBlendTime;
	if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
		return Py_BuildException();

	fRotation = fmod(720.0f - fRotation, 360.0f);

	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pCharacterInstance)
		return Py_BuildNone();
	pCharacterInstance->BlendRotation(fRotation, fBlendTime);

	return Py_BuildNone();
}

PyObject* chrGetRotation(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

	if (!pCharacterInstance)
		return Py_BuildValue("f", 0.0f);

	float fRotation = pCharacterInstance->GetRotation();

	return Py_BuildValue("f", 360.0f - fRotation);
}

PyObject* chrGetRace(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

	if (!pCharacterInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pCharacterInstance->GetRace());
}

PyObject* chrGetName(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

	if (!pCharacterInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("s", pCharacterInstance->GetNameString());
}

PyObject* chrGetNameByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("s", "None");

	return Py_BuildValue("s", pInstance->GetNameString());
}

PyObject* chrGetGuildID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->GetGuildID());
}

PyObject* chrGetProjectPosition(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();
	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("ii", -100, -100);

	TPixelPosition PixelPosition;
	pInstance->NEW_GetPixelPosition(&PixelPosition);

	CPythonGraphic& rpyGraphic = CPythonGraphic::Instance();

	float fx, fy, fz;
	rpyGraphic.ProjectPosition(PixelPosition.x,
		-PixelPosition.y,
		PixelPosition.z + float(iHeight),
		&fx, &fy, &fz);

	if (1 == int(fz))
		return Py_BuildValue("ii", -100, -100);

	return Py_BuildValue("ii", int(fx), int(fy));
}

PyObject* chrGetVirtualNumber(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (NULL != pkInst)
		return Py_BuildValue("i", pkInst->GetVirtualNumber());

	return Py_BuildValue("i", CActorInstance::TYPE_PC);
}

PyObject* chrGetInstanceType(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (NULL != pkInst)
		return Py_BuildValue("i", pkInst->GetInstanceType());

	return Py_BuildValue("i", CActorInstance::TYPE_PC);
}

#ifdef ENABLE_ACCE_SYSTEM
PyObject* chrSetAcce(PyObject* poSelf, PyObject* poArgs)
{
	int dwAcce;
	if (!PyTuple_GetInteger(poArgs, 0, &dwAcce))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->SetAcce(dwAcce, 0);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
PyObject* chrSetScale(PyObject* poSelf, PyObject* poArgs)
{
	float fScaleX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScaleX))
		return Py_BuildException();
	float fScaleY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScaleY))
		return Py_BuildException();
	float fScaleZ;
	if (!PyTuple_GetFloat(poArgs, 2, &fScaleZ))
		return Py_BuildException();

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pkInst)
		return Py_BuildNone();

	pkInst->GetGraphicThingInstanceRef().SetScaleWorld(fScaleX, fScaleY, fScaleZ);
	return Py_BuildNone();
}

PyObject* chrGetLevel(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->GetLevel());
}

PyObject* chrGetLevelByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->GetLevel());
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
PyObject* braveRequestPetName(PyObject* poSelf, PyObject* poArgs)
{
	const char* petname;

	if (!PyArg_ParseTuple(poArgs, "s", &petname)) {
		return Py_BuildException();
	}

	if (strlen(petname) > 12 || strlen(petname) < 4) {
		TraceError("RequestPetName: petname lenght not allowed!");
		return Py_BuildNone();
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.PetSetNamePacket(petname);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
PyObject* braveRequestMountName(PyObject* poSelf, PyObject* poArgs)
{
	const char* mountname;

	if (!PyArg_ParseTuple(poArgs, "s", &mountname)) {
		return Py_BuildException();
	}

	if (strlen(mountname) > 12 || strlen(mountname) < 4) {
		TraceError("RequestMountName: mountname lenght not allowed!");
		return Py_BuildNone();
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.MountSetNamePacket(mountname);
	return Py_BuildNone();
}
#endif

PyObject* chrIsStone(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->IsStone());
}

#ifdef ENABLE_TARGET_BOARD_RENEWAL
PyObject* chrGetRaceByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pInstance->GetRace());
}
#endif

void initchr()
{
	static PyMethodDef s_methods[] =
	{
		{ "Destroy",					chrDestroy,							METH_VARARGS },
		{ "Update",						chrUpdate,							METH_VARARGS },
		{ "Deform",						chrDeform,							METH_VARARGS },
		{ "Render",						chrRender,							METH_VARARGS },

		// Functions For Python Code
		{ "CreateInstance",				chrCreateInstance,					METH_VARARGS },
		{ "DeleteInstance",				chrDeleteInstance,					METH_VARARGS },
		{ "SelectInstance",				chrSelectInstance,					METH_VARARGS },

		{ "HasInstance",				chrHasInstance,						METH_VARARGS },
		{ "IsEnemy",					chrIsEnemy,							METH_VARARGS },
		{ "IsNPC",						chrIsNPC,							METH_VARARGS },
		{ "IsDoor",						chrIsDoor,							METH_VARARGS },
		{ "IsGameMaster",				chrIsGameMaster,					METH_VARARGS },
		{ "IsPartyMember",				chrIsPartyMember,					METH_VARARGS },

		{ "Select",						chrSelect,							METH_VARARGS },
		{ "Unselect",					chrUnselect,						METH_VARARGS },

		{ "Hide",						chrHide,							METH_VARARGS },
		{ "Show",						chrShow,							METH_VARARGS },
		{ "Pick",						chrPick,							METH_VARARGS },
		{ "PickAll",					chrPickAll,							METH_VARARGS },

		{ "SetArmor",					chrSetArmor,						METH_VARARGS },
		{ "SetWeapon",					chrSetWeapon,						METH_VARARGS },
		{ "ChangeShape",				chrChangeShape,						METH_VARARGS },
		{ "SetRace",					chrSetRace,							METH_VARARGS },
		{ "SetHair",					chrSetHair,							METH_VARARGS },
		{ "ChangeHair",					chrChangeHair,						METH_VARARGS },
		{ "SetVirtualID",				chrSetVirtualID,					METH_VARARGS },
		{ "SetNameString",				chrSetNameString,					METH_VARARGS },

		{ "SetPixelPosition",			chrSetPixelPosition,				METH_VARARGS },
		{ "SetDirection",				chrSetDirection,					METH_VARARGS },
		{ "Refresh",					chrRefresh,							METH_VARARGS },
		{ "Revive",						chrRevive,							METH_VARARGS },

		{ "SetMotionMode",				chrSetMotionMode,					METH_VARARGS },
		{ "SetLoopMotion",				chrSetLoopMotion,					METH_VARARGS },
		{ "BlendLoopMotion",			chrBlendLoopMotion,					METH_VARARGS },
		{ "PushOnceMotion",				chrPushOnceMotion,					METH_VARARGS },
		{ "PushLoopMotion",				chrPushLoopMotion,					METH_VARARGS },
		{ "GetPixelPosition",			chrGetPixelPosition,				METH_VARARGS },
		{ "SetRotation",				chrSetRotation,						METH_VARARGS },
		{ "SetRotationAll",				chrSetRotationAll,					METH_VARARGS },
		{ "BlendRotation",				chrBlendRotation,					METH_VARARGS },
		{ "GetRotation",				chrGetRotation,						METH_VARARGS },
		{ "GetRace",					chrGetRace,							METH_VARARGS },

		{ "GetName",					chrGetName,							METH_VARARGS },
		{ "GetNameByVID",				chrGetNameByVID,					METH_VARARGS },
		{ "GetGuildID",					chrGetGuildID,						METH_VARARGS },
		{ "GetProjectPosition",			chrGetProjectPosition,				METH_VARARGS },

		{ "GetVirtualNumber",			chrGetVirtualNumber,				METH_VARARGS },
		{ "GetInstanceType",			chrGetInstanceType,					METH_VARARGS },

		{ "RaceToJob",					chrRaceToJob,							METH_VARARGS },
		{ "RaceToSex",					chrRaceToSex,							METH_VARARGS },
#ifdef ENABLE_ACCE_SYSTEM
		{ "SetAcce",						chrSetAcce,							METH_VARARGS },
#endif
#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
		{"SetScale",						chrSetScale,						METH_VARARGS},
		{"GetLevel",						chrGetLevel,						METH_VARARGS},
		{"GetLevelByVID",					chrGetLevelByVID,					METH_VARARGS},
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		{ "RequestPetName",				braveRequestPetName,					METH_VARARGS },
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
		{ "RequestMountName",			braveRequestMountName,					METH_VARARGS },
#endif
		{"IsStone",						chrIsStone,								METH_VARARGS},
#ifdef ENABLE_TARGET_BOARD_RENEWAL
		{ "GetRaceByVID",				chrGetRaceByVID,					METH_VARARGS },
#endif
		{ NULL,								NULL,								NULL		 },
	};

	PyObject* poModule = Py_InitModule("chr", s_methods);

	// Length
	PyModule_AddIntConstant(poModule, "PLAYER_NAME_MAX_LEN", PLAYER_NAME_MAX_LEN);

	// General

	PyModule_AddIntConstant(poModule, "MOTION_WAIT", CRaceMotionData::NAME_WAIT);
	PyModule_AddIntConstant(poModule, "MOTION_INTRO_WAIT", CRaceMotionData::NAME_INTRO_WAIT);
	PyModule_AddIntConstant(poModule, "MOTION_INTRO_SELECTED", CRaceMotionData::NAME_INTRO_SELECTED);
	PyModule_AddIntConstant(poModule, "MOTION_INTRO_NOT_SELECTED", CRaceMotionData::NAME_INTRO_NOT_SELECTED);
	PyModule_AddIntConstant(poModule, "MOTION_SKILL", CRaceMotionData::NAME_SKILL);

	PyModule_AddIntConstant(poModule, "MOTION_MODE_GENERAL", CRaceMotionData::MODE_GENERAL);
#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
	PyModule_AddIntConstant(poModule, "MOTION_MODE_INTRO_GENERAL", CRaceMotionData::MODE_INTRO_GENERAL);
#endif

	PyModule_AddIntConstant(poModule, "DIR_NORTH", CInstanceBase::DIR_NORTH);
	PyModule_AddIntConstant(poModule, "DIR_NORTHEAST", CInstanceBase::DIR_NORTHEAST);
	PyModule_AddIntConstant(poModule, "DIR_EAST", CInstanceBase::DIR_EAST);
	PyModule_AddIntConstant(poModule, "DIR_SOUTHEAST", CInstanceBase::DIR_SOUTHEAST);
	PyModule_AddIntConstant(poModule, "DIR_SOUTH", CInstanceBase::DIR_SOUTH);
	PyModule_AddIntConstant(poModule, "DIR_SOUTHWEST", CInstanceBase::DIR_SOUTHWEST);
	PyModule_AddIntConstant(poModule, "DIR_WEST", CInstanceBase::DIR_WEST);
	PyModule_AddIntConstant(poModule, "DIR_NORTHWEST", CInstanceBase::DIR_NORTHWEST);

	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_ENEMY", CActorInstance::TYPE_ENEMY);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_NPC", CActorInstance::TYPE_NPC);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_STONE", CActorInstance::TYPE_STONE);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_WARP", CActorInstance::TYPE_WARP);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_DOOR", CActorInstance::TYPE_DOOR);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_PC", CActorInstance::TYPE_PC);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_HORSE", CActorInstance::TYPE_HORSE);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_GOTO", CActorInstance::TYPE_GOTO);

	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_PLAYER", CActorInstance::TYPE_PC);
	PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_OBJECT", CActorInstance::TYPE_OBJECT);

	PyModule_AddIntConstant(poModule, "AFFECT_YMIR", CInstanceBase::AFFECT_YMIR);
	PyModule_AddIntConstant(poModule, "AFFECT_INVISIBILITY", CInstanceBase::AFFECT_INVISIBILITY);
	PyModule_AddIntConstant(poModule, "AFFECT_SPAWN", CInstanceBase::AFFECT_SPAWN);
	PyModule_AddIntConstant(poModule, "AFFECT_POISON", CInstanceBase::AFFECT_POISON);
	PyModule_AddIntConstant(poModule, "AFFECT_SLOW", CInstanceBase::AFFECT_SLOW);
	PyModule_AddIntConstant(poModule, "AFFECT_STUN", CInstanceBase::AFFECT_STUN);
	PyModule_AddIntConstant(poModule, "AFFECT_DUNGEON_READY", CInstanceBase::AFFECT_DUNGEON_READY);
	PyModule_AddIntConstant(poModule, "AFFECT_MOV_SPEED_POTION", CInstanceBase::AFFECT_MOV_SPEED_POTION);
	PyModule_AddIntConstant(poModule, "AFFECT_ATT_SPEED_POTION", CInstanceBase::AFFECT_ATT_SPEED_POTION);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_MIND", CInstanceBase::AFFECT_FISH_MIND);

	PyModule_AddIntConstant(poModule, "AFFECT_JEONGWI", CInstanceBase::AFFECT_JEONGWI);
	PyModule_AddIntConstant(poModule, "AFFECT_GEOMGYEONG", CInstanceBase::AFFECT_GEOMGYEONG);
	PyModule_AddIntConstant(poModule, "AFFECT_CHEONGEUN", CInstanceBase::AFFECT_CHEONGEUN);
	PyModule_AddIntConstant(poModule, "AFFECT_GYEONGGONG", CInstanceBase::AFFECT_GYEONGGONG);
	PyModule_AddIntConstant(poModule, "AFFECT_EUNHYEONG", CInstanceBase::AFFECT_EUNHYEONG);
	PyModule_AddIntConstant(poModule, "AFFECT_GWIGEOM", CInstanceBase::AFFECT_GWIGEOM);
	PyModule_AddIntConstant(poModule, "AFFECT_GONGPO", CInstanceBase::AFFECT_GONGPO);
	PyModule_AddIntConstant(poModule, "AFFECT_JUMAGAP", CInstanceBase::AFFECT_JUMAGAP);
	PyModule_AddIntConstant(poModule, "AFFECT_HOSIN", CInstanceBase::AFFECT_HOSIN);
	PyModule_AddIntConstant(poModule, "AFFECT_BOHO", CInstanceBase::AFFECT_BOHO);
	PyModule_AddIntConstant(poModule, "AFFECT_KWAESOK", CInstanceBase::AFFECT_KWAESOK);
	PyModule_AddIntConstant(poModule, "AFFECT_HEUKSIN", CInstanceBase::AFFECT_HEUKSIN);
	PyModule_AddIntConstant(poModule, "AFFECT_MUYEONG", CInstanceBase::AFFECT_MUYEONG);
	PyModule_AddIntConstant(poModule, "AFFECT_FIRE", CInstanceBase::AFFECT_FIRE);
	PyModule_AddIntConstant(poModule, "AFFECT_GICHEON", CInstanceBase::AFFECT_GICHEON);
	PyModule_AddIntConstant(poModule, "AFFECT_JEUNGRYEOK", CInstanceBase::AFFECT_JEUNGRYEOK);
	PyModule_AddIntConstant(poModule, "AFFECT_PABEOP", CInstanceBase::AFFECT_PABEOP);
	PyModule_AddIntConstant(poModule, "AFFECT_FALLEN_CHEONGEUN", CInstanceBase::AFFECT_FALLEN_CHEONGEUN);
	PyModule_AddIntConstant(poModule, "AFFECT_CHINA_FIREWORK", CInstanceBase::AFFECT_CHINA_FIREWORK);
	PyModule_AddIntConstant(poModule, "AFFECT_WAR_FLAG1", CInstanceBase::AFFECT_WAR_FLAG1);
	PyModule_AddIntConstant(poModule, "AFFECT_WAR_FLAG2", CInstanceBase::AFFECT_WAR_FLAG2);
	PyModule_AddIntConstant(poModule, "AFFECT_WAR_FLAG3", CInstanceBase::AFFECT_WAR_FLAG3);
#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "AFFECT_BLEEDING", CInstanceBase::AFFECT_BLEEDING);
	PyModule_AddIntConstant(poModule, "AFFECT_RED_POSSESSION", CInstanceBase::AFFECT_RED_POSSESSION);
	PyModule_AddIntConstant(poModule, "AFFECT_BLUE_POSSESSION", CInstanceBase::AFFECT_BLUE_POSSESSION);
#endif
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_MALL", CInstanceBase::NEW_AFFECT_MALL);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_NO_DEATH_PENALTY", CInstanceBase::NEW_AFFECT_NO_DEATH_PENALTY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SKILL_BOOK_BONUS", CInstanceBase::NEW_AFFECT_SKILL_BOOK_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SKILL_BOOK_NO_DELAY", CInstanceBase::NEW_AFFECT_SKILL_BOOK_NO_DELAY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_EXP_BONUS", CInstanceBase::NEW_AFFECT_EXP_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_EXP_BONUS_EURO_FREE", CInstanceBase::NEW_AFFECT_EXP_BONUS_EURO_FREE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15", CInstanceBase::NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15);

	PyModule_AddIntConstant(poModule, "NEW_AFFECT_ITEM_BONUS", CInstanceBase::NEW_AFFECT_ITEM_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SAFEBOX", CInstanceBase::NEW_AFFECT_SAFEBOX);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_AUTOLOOT", CInstanceBase::NEW_AFFECT_AUTOLOOT);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_FISH_MIND", CInstanceBase::NEW_AFFECT_FISH_MIND);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_GOLD_BONUS", CInstanceBase::NEW_AFFECT_GOLD_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_AUTO_HP_RECOVERY", CInstanceBase::NEW_AFFECT_AUTO_HP_RECOVERY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_AUTO_SP_RECOVERY", CInstanceBase::NEW_AFFECT_AUTO_SP_RECOVERY);

	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DRAGON_SOUL_QUALIFIED", CInstanceBase::NEW_AFFECT_DRAGON_SOUL_QUALIFIED);

	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DRAGON_SOUL_DECK1", CInstanceBase::NEW_AFFECT_DRAGON_SOUL_DECK1);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DRAGON_SOUL_DECK2", CInstanceBase::NEW_AFFECT_DRAGON_SOUL_DECK2);
#ifdef ENABLE_POTION_AS_REWORK
	PyModule_AddIntConstant(poModule, "RED_BLEND_POTION", CInstanceBase::RED_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "ORANGE_BLEND_POTION", CInstanceBase::ORANGE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "YELLOW_BLEND_POTION", CInstanceBase::YELLOW_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "GREEN_BLEND_POTION", CInstanceBase::GREEN_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "BLUE_BLEND_POTION", CInstanceBase::BLUE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "WHITE_BLEND_POTION", CInstanceBase::WHITE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "JADE_BLEND_POTION", CInstanceBase::JADE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "DARK_BLEND_POTION", CInstanceBase::DARK_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "SAFE_BLEND_POTION", CInstanceBase::SAFE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "DEVIL_BLEND_POTION", CInstanceBase::DEVIL_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "HEAL_BLEND_POTION", CInstanceBase::HEAL_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "NORMAL_HIT_BLEND_POTION", CInstanceBase::NORMAL_HIT_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "UNDEAD_BLEND_POTION", CInstanceBase::UNDEAD_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "DEVIL_ATT_BLEND_POTION", CInstanceBase::DEVIL_ATT_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "ANIMAL_BLEND_POTION", CInstanceBase::ANIMAL_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "MILGYO_BLEND_POTION", CInstanceBase::MILGYO_BLEND_POTION);
#endif
#ifdef ENABLE_AS_REWORK
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_RAMADAN_ABILITY", CInstanceBase::NEW_AFFECT_RAMADAN_ABILITY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_NOG_POCKET_ABILITY", CInstanceBase::NEW_AFFECT_NOG_POCKET_ABILITY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_ATT_SPEED", CInstanceBase::NEW_AFFECT_ATT_SPEED);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_MOV_SPEED", CInstanceBase::NEW_AFFECT_MOV_SPEED);
#endif
#ifdef ENABLE_DRAGON_BONE_EFFECT
	PyModule_AddIntConstant(poModule, "AFFECT_DBONE1", CInstanceBase::AFFECT_DBONE1);
	PyModule_AddIntConstant(poModule, "AFFECT_DBONE2", CInstanceBase::AFFECT_DBONE2);
#endif
#ifdef ENABLE_POTION_ITEMS
	PyModule_AddIntConstant(poModule, "AFFECT_18385", CInstanceBase::AFFECT_18385);
	PyModule_AddIntConstant(poModule, "AFFECT_18386", CInstanceBase::AFFECT_18386);
	PyModule_AddIntConstant(poModule, "AFFECT_18387", CInstanceBase::AFFECT_18387);
	PyModule_AddIntConstant(poModule, "AFFECT_18388", CInstanceBase::AFFECT_18388);
	PyModule_AddIntConstant(poModule, "AFFECT_18389", CInstanceBase::AFFECT_18389);
	PyModule_AddIntConstant(poModule, "AFFECT_18390", CInstanceBase::AFFECT_18390);
#endif
#ifdef ENABLE_DRAGON_BONE_EFFECT
	PyModule_AddIntConstant(poModule, "AFFECT_DBONE_1", CInstanceBase::AFFECT_DBONE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_DBONE_1", CInstanceBase::AFFECT_DBONE_1);
#endif
#ifdef ENABLE_DSS_ACTIVE_EFFECT
	PyModule_AddIntConstant(poModule, "AFFECT_DS", CInstanceBase::AFFECT_DS);
#endif
#ifdef ENABLE_DS_SET
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DS_SET", CInstanceBase::NEW_AFFECT_DS_SET);
#endif
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_ATT_GRADE", CInstanceBase::NEW_AFFECT_ATT_GRADE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_INVISIBILITY", CInstanceBase::NEW_AFFECT_INVISIBILITY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_STR", CInstanceBase::NEW_AFFECT_STR);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DEX", CInstanceBase::NEW_AFFECT_DEX);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_CON", CInstanceBase::NEW_AFFECT_CON);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_INT", CInstanceBase::NEW_AFFECT_INT);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_FISH_MIND_PILL", CInstanceBase::NEW_AFFECT_FISH_MIND_PILL);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_POISON", CInstanceBase::NEW_AFFECT_POISON);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_STUN", CInstanceBase::NEW_AFFECT_STUN);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SLOW", CInstanceBase::NEW_AFFECT_SLOW);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DUNGEON_READY", CInstanceBase::NEW_AFFECT_DUNGEON_READY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DUNGEON_UNIQUE", CInstanceBase::NEW_AFFECT_DUNGEON_UNIQUE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_REVIVE_INVISIBLE", CInstanceBase::NEW_AFFECT_REVIVE_INVISIBLE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_FIRE", CInstanceBase::NEW_AFFECT_FIRE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_CAST_SPEED", CInstanceBase::NEW_AFFECT_CAST_SPEED);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_HP_RECOVER_CONTINUE", CInstanceBase::NEW_AFFECT_HP_RECOVER_CONTINUE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SP_RECOVER_CONTINUE", CInstanceBase::NEW_AFFECT_SP_RECOVER_CONTINUE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_MOUNT", CInstanceBase::NEW_AFFECT_MOUNT);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_WAR_FLAG", CInstanceBase::NEW_AFFECT_WAR_FLAG);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_BLOCK_CHAT", CInstanceBase::NEW_AFFECT_BLOCK_CHAT);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_CHINA_FIREWORK", CInstanceBase::NEW_AFFECT_CHINA_FIREWORK);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_BOW_DISTANCE", CInstanceBase::NEW_AFFECT_BOW_DISTANCE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_EXP_BONUS", CInstanceBase::NEW_AFFECT_EXP_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_ITEM_BONUS", CInstanceBase::NEW_AFFECT_ITEM_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SAFEBOX", CInstanceBase::NEW_AFFECT_SAFEBOX);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_AUTOLOOT", CInstanceBase::NEW_AFFECT_AUTOLOOT);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_FISH_MIND", CInstanceBase::NEW_AFFECT_FISH_MIND);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_GOLD_BONUS", CInstanceBase::NEW_AFFECT_GOLD_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_MALL", CInstanceBase::NEW_AFFECT_MALL);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_NO_DEATH_PENALTY", CInstanceBase::NEW_AFFECT_NO_DEATH_PENALTY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SKILL_BOOK_BONUS", CInstanceBase::NEW_AFFECT_SKILL_BOOK_BONUS);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_SKILL_BOOK_NO_DELAY", CInstanceBase::NEW_AFFECT_SKILL_BOOK_NO_DELAY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_EXP_BONUS_EURO_FREE", CInstanceBase::NEW_AFFECT_EXP_BONUS_EURO_FREE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15", CInstanceBase::NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_UNIQUE_ABILITY", CInstanceBase::NEW_AFFECT_UNIQUE_ABILITY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_AUTO_HP_RECOVERY", CInstanceBase::NEW_AFFECT_AUTO_HP_RECOVERY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_AUTO_SP_RECOVERY", CInstanceBase::NEW_AFFECT_AUTO_SP_RECOVERY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DRAGON_SOUL_QUALIFIED", CInstanceBase::NEW_AFFECT_DRAGON_SOUL_QUALIFIED);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DRAGON_SOUL_DECK1", CInstanceBase::NEW_AFFECT_DRAGON_SOUL_DECK1);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DRAGON_SOUL_DECK2", CInstanceBase::NEW_AFFECT_DRAGON_SOUL_DECK2);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DS_SET", CInstanceBase::NEW_AFFECT_DS_SET);
	PyModule_AddIntConstant(poModule, "AFFECT_NEW_PET", CInstanceBase::AFFECT_NEW_PET);
	PyModule_AddIntConstant(poModule, "AFFECT_ICECREAM_ABILITY", CInstanceBase::AFFECT_ICECREAM_ABILITY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_RAMADAN_ABILITY", CInstanceBase::NEW_AFFECT_RAMADAN_ABILITY);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_RAMADAN_RING", CInstanceBase::NEW_AFFECT_RAMADAN_RING);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_NOG_POCKET_ABILITY", CInstanceBase::NEW_AFFECT_NOG_POCKET_ABILITY);
	PyModule_AddIntConstant(poModule, "RED_BLEND_POTION", CInstanceBase::RED_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "ORANGE_BLEND_POTION", CInstanceBase::ORANGE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "YELLOW_BLEND_POTION", CInstanceBase::YELLOW_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "GREEN_BLEND_POTION", CInstanceBase::GREEN_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "BLUE_BLEND_POTION", CInstanceBase::BLUE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "WHITE_BLEND_POTION", CInstanceBase::WHITE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "JADE_BLEND_POTION", CInstanceBase::JADE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "DARK_BLEND_POTION", CInstanceBase::DARK_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "SAFE_BLEND_POTION", CInstanceBase::SAFE_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "DEVIL_BLEND_POTION", CInstanceBase::DEVIL_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "HEAL_BLEND_POTION", CInstanceBase::HEAL_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "NORMAL_HIT_BLEND_POTION", CInstanceBase::NORMAL_HIT_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "NORMAL_HIT_BLEND_POTION", CInstanceBase::NORMAL_HIT_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "UNDEAD_BLEND_POTION", CInstanceBase::UNDEAD_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "DEVIL_ATT_BLEND_POTION", CInstanceBase::DEVIL_ATT_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "ANIMAL_BLEND_POTION", CInstanceBase::ANIMAL_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "MILGYO_BLEND_POTION", CInstanceBase::MILGYO_BLEND_POTION);
	PyModule_AddIntConstant(poModule, "AFFECT_DBONE_1", CInstanceBase::AFFECT_DBONE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_DBONE_2", CInstanceBase::AFFECT_DBONE_2);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_1", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_2", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_2);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_3", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_3);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_4", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_4);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_5", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_5);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_6", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_6);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_7", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_7);
	PyModule_AddIntConstant(poModule, "AFFECT_ALIGN_BONUS_TYPE_8", CInstanceBase::AFFECT_ALIGN_BONUS_TYPE_8);
	PyModule_AddIntConstant(poModule, "AFFECT_18385", CInstanceBase::AFFECT_18385);
	PyModule_AddIntConstant(poModule, "AFFECT_18386", CInstanceBase::AFFECT_18386);
	PyModule_AddIntConstant(poModule, "AFFECT_18387", CInstanceBase::AFFECT_18387);
	PyModule_AddIntConstant(poModule, "AFFECT_18388", CInstanceBase::AFFECT_18388);
	PyModule_AddIntConstant(poModule, "AFFECT_18389", CInstanceBase::AFFECT_18389);
	PyModule_AddIntConstant(poModule, "AFFECT_18390", CInstanceBase::AFFECT_18390);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_1", CInstanceBase::AFFECT_RANK_BONUS_TYPE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_2", CInstanceBase::AFFECT_RANK_BONUS_TYPE_2);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_3", CInstanceBase::AFFECT_RANK_BONUS_TYPE_3);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_4", CInstanceBase::AFFECT_RANK_BONUS_TYPE_4);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_5", CInstanceBase::AFFECT_RANK_BONUS_TYPE_5);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_6", CInstanceBase::AFFECT_RANK_BONUS_TYPE_6);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_7", CInstanceBase::AFFECT_RANK_BONUS_TYPE_7);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_8", CInstanceBase::AFFECT_RANK_BONUS_TYPE_8);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_9", CInstanceBase::AFFECT_RANK_BONUS_TYPE_9);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_10", CInstanceBase::AFFECT_RANK_BONUS_TYPE_10);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_11", CInstanceBase::AFFECT_RANK_BONUS_TYPE_11);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_12", CInstanceBase::AFFECT_RANK_BONUS_TYPE_12);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_13", CInstanceBase::AFFECT_RANK_BONUS_TYPE_13);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_14", CInstanceBase::AFFECT_RANK_BONUS_TYPE_14);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_15", CInstanceBase::AFFECT_RANK_BONUS_TYPE_15);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_16", CInstanceBase::AFFECT_RANK_BONUS_TYPE_16);
	PyModule_AddIntConstant(poModule, "AFFECT_RANK_BONUS_TYPE_17", CInstanceBase::AFFECT_RANK_BONUS_TYPE_17);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_1", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_2", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_2);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_3", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_3);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_4", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_4);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_5", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_5);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_6", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_6);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_7", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_7);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_8", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_8);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_9", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_9);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_10", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_10);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_11", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_11);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_12", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_12);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_13", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_13);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_14", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_14);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_15", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_15);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_16", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_16);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_17", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_17);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_18", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_18);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_19", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_19);
	PyModule_AddIntConstant(poModule, "AFFECT_LANDRANK_BONUS_TYPE_20", CInstanceBase::AFFECT_LANDRANK_BONUS_TYPE_20);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_1", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_2", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_2);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_3", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_3);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_4", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_4);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_5", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_5);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_6", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_6);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_7", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_7);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_8", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_8);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_9", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_9);
	PyModule_AddIntConstant(poModule, "AFFECT_REBORN_BONUS_TYPE_10", CInstanceBase::AFFECT_REBORN_BONUS_TYPE_10);
	PyModule_AddIntConstant(poModule, "AFFECT_MONIKER_BONUS_TYPE_1", CInstanceBase::AFFECT_MONIKER_BONUS_TYPE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_MONIKER_BONUS_TYPE_2", CInstanceBase::AFFECT_MONIKER_BONUS_TYPE_2);
	PyModule_AddIntConstant(poModule, "AFFECT_MONIKER_BONUS_TYPE_3", CInstanceBase::AFFECT_MONIKER_BONUS_TYPE_3);
	PyModule_AddIntConstant(poModule, "AFFECT_MONIKER_BONUS_TYPE_4", CInstanceBase::AFFECT_MONIKER_BONUS_TYPE_4);
	PyModule_AddIntConstant(poModule, "AFFECT_MONIKER_BONUS_TYPE_5", CInstanceBase::AFFECT_MONIKER_BONUS_TYPE_5);
	PyModule_AddIntConstant(poModule, "AFFECT_TEAM_BONUS_TYPE_1", CInstanceBase::AFFECT_TEAM_BONUS_TYPE_1);
	PyModule_AddIntConstant(poModule, "AFFECT_TEAM_BONUS_TYPE_2", CInstanceBase::AFFECT_TEAM_BONUS_TYPE_2);
	PyModule_AddIntConstant(poModule, "AFFECT_TEAM_BONUS_TYPE_3", CInstanceBase::AFFECT_TEAM_BONUS_TYPE_3);
	PyModule_AddIntConstant(poModule, "AFFECT_TEAM_BONUS_TYPE_4", CInstanceBase::AFFECT_TEAM_BONUS_TYPE_4);
	PyModule_AddIntConstant(poModule, "AFFECT_TEAM_BONUS_TYPE_5", CInstanceBase::AFFECT_TEAM_BONUS_TYPE_5);
	PyModule_AddIntConstant(poModule, "AFFECT_SKILL_BOOK_BONUS2", CInstanceBase::AFFECT_SKILL_BOOK_BONUS2);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_ITEM_DURATION", CInstanceBase::AFFECT_BIO_ITEM_DURATION);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_PACKET_DURATION", CInstanceBase::AFFECT_BIO_PACKET_DURATION);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS1", CInstanceBase::AFFECT_BIO_BONUS1);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS2", CInstanceBase::AFFECT_BIO_BONUS2);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS3", CInstanceBase::AFFECT_BIO_BONUS3);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS4", CInstanceBase::AFFECT_BIO_BONUS4);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS5", CInstanceBase::AFFECT_BIO_BONUS5);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS6", CInstanceBase::AFFECT_BIO_BONUS6);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS7", CInstanceBase::AFFECT_BIO_BONUS7);
	PyModule_AddIntConstant(poModule, "AFFECT_BIO_BONUS8", CInstanceBase::AFFECT_BIO_BONUS8);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_EXP_BONUS", CInstanceBase::AFFECT_FISH_EXP_BONUS);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_CAST_SPEED", CInstanceBase::AFFECT_FISH_CAST_SPEED);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_MAX_HP", CInstanceBase::AFFECT_FISH_MAX_HP);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_MAX_SP", CInstanceBase::AFFECT_FISH_MAX_SP);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_UNDEAD", CInstanceBase::AFFECT_FISH_ATTBONUS_UNDEAD);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_ANIMAL", CInstanceBase::AFFECT_FISH_ATTBONUS_ANIMAL);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_MILGYO", CInstanceBase::AFFECT_FISH_ATTBONUS_MILGYO);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_ORC", CInstanceBase::AFFECT_FISH_ATTBONUS_ORC);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_DEVIL", CInstanceBase::AFFECT_FISH_ATTBONUS_DEVIL);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_INSECT", CInstanceBase::AFFECT_FISH_ATTBONUS_INSECT);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_BOSS", CInstanceBase::AFFECT_FISH_ATTBONUS_BOSS);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_MONSTER", CInstanceBase::AFFECT_FISH_ATTBONUS_MONSTER);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_HUMAN", CInstanceBase::AFFECT_FISH_ATTBONUS_HUMAN);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_ATTBONUS_STONE", CInstanceBase::AFFECT_FISH_ATTBONUS_STONE);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_NORMAL_HIT_BONUS", CInstanceBase::AFFECT_FISH_NORMAL_HIT_BONUS);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_APPLY_STR", CInstanceBase::AFFECT_FISH_APPLY_STR);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_APPLY_DEX", CInstanceBase::AFFECT_FISH_APPLY_DEX);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_APPLY_CON", CInstanceBase::AFFECT_FISH_APPLY_CON);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_APPLY_INT", CInstanceBase::AFFECT_FISH_APPLY_INT);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_APPLY_CAST_SPEED", CInstanceBase::AFFECT_FISH_APPLY_CAST_SPEED);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_APPLY_ATT_GRADE", CInstanceBase::AFFECT_FISH_APPLY_ATT_GRADE);
	PyModule_AddIntConstant(poModule, "AFFECT_FISH_APPLY_DEF_GRADE", CInstanceBase::AFFECT_FISH_APPLY_DEF_GRADE);
	PyModule_AddIntConstant(poModule, "AFFECT_NEW_PET_DURATION", CInstanceBase::AFFECT_NEW_PET_DURATION);
	PyModule_AddIntConstant(poModule, "AFFECT_SKILL_BOOK_BONUS_EXPERT", CInstanceBase::AFFECT_SKILL_BOOK_BONUS_EXPERT);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_QUEST_START_IDX", CInstanceBase::NEW_AFFECT_QUEST_START_IDX);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_BLEEDING", CInstanceBase::NEW_AFFECT_BLEEDING);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DEF_GRADE", CInstanceBase::NEW_AFFECT_DEF_GRADE);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_BLEND", CInstanceBase::NEW_AFFECT_BLEND);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_HORSE_NAME", CInstanceBase::NEW_AFFECT_HORSE_NAME);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_MOUNT_BONUS", CInstanceBase::NEW_AFFECT_MOUNT_BONUS);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_1", CInstanceBase::AFFECT_GM_BONUS_1);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_2", CInstanceBase::AFFECT_GM_BONUS_2);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_3", CInstanceBase::AFFECT_GM_BONUS_3);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_4", CInstanceBase::AFFECT_GM_BONUS_4);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_5", CInstanceBase::AFFECT_GM_BONUS_5);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_6", CInstanceBase::AFFECT_GM_BONUS_6);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_7", CInstanceBase::AFFECT_GM_BONUS_7);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_8", CInstanceBase::AFFECT_GM_BONUS_8);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_9", CInstanceBase::AFFECT_GM_BONUS_9);
	PyModule_AddIntConstant(poModule, "AFFECT_GM_BONUS_10", CInstanceBase::AFFECT_GM_BONUS_10);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW1", CInstanceBase::AFFECT_RAMADAN_NEW1);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW2", CInstanceBase::AFFECT_RAMADAN_NEW2);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW3", CInstanceBase::AFFECT_RAMADAN_NEW3);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW4", CInstanceBase::AFFECT_RAMADAN_NEW4);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW5", CInstanceBase::AFFECT_RAMADAN_NEW5);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW6", CInstanceBase::AFFECT_RAMADAN_NEW6);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW7", CInstanceBase::AFFECT_RAMADAN_NEW7);
	PyModule_AddIntConstant(poModule, "AFFECT_RAMADAN_NEW8", CInstanceBase::AFFECT_RAMADAN_NEW8);
	PyModule_AddIntConstant(poModule, "AFFECT_SEKILL_SET_BONUS_1", CInstanceBase::AFFECT_SEKILL_SET_BONUS_1);
	PyModule_AddIntConstant(poModule, "AFFECT_SEKILL_SET_BONUS_2", CInstanceBase::AFFECT_SEKILL_SET_BONUS_2);
	PyModule_AddIntConstant(poModule, "AFFECT_SEKILL_SET_BONUS_3", CInstanceBase::AFFECT_SEKILL_SET_BONUS_3);
	PyModule_AddIntConstant(poModule, "AFFECT_SEKILL_SET_BONUS_4", CInstanceBase::AFFECT_SEKILL_SET_BONUS_4);
	PyModule_AddIntConstant(poModule, "AFFECT_SEKILL_SET_BONUS_5", CInstanceBase::AFFECT_SEKILL_SET_BONUS_5);
	PyModule_AddIntConstant(poModule, "AFFECT_SEKILL_SET_BONUS_6", CInstanceBase::AFFECT_SEKILL_SET_BONUS_6);
	PyModule_AddIntConstant(poModule, "AFFECT_ADAMSINIZ", CInstanceBase::AFFECT_ADAMSINIZ);
	PyModule_AddIntConstant(poModule, "AFFECT_KINDER", CInstanceBase::AFFECT_KINDER);
	PyModule_AddIntConstant(poModule, "AFFECT_CIKOLATA", CInstanceBase::AFFECT_CIKOLATA);
	PyModule_AddIntConstant(poModule, "AFFECT_GUL", CInstanceBase::AFFECT_GUL);
	PyModule_AddIntConstant(poModule, "AFFECT_LOLIPOP", CInstanceBase::AFFECT_LOLIPOP);

	PyModule_AddIntConstant(poModule, "AFFECT_LEVEL_SET_BONUS_1", CInstanceBase::AFFECT_LEVEL_SET_BONUS_1);
	PyModule_AddIntConstant(poModule, "AFFECT_LEVEL_SET_BONUS_2", CInstanceBase::AFFECT_LEVEL_SET_BONUS_2);
	PyModule_AddIntConstant(poModule, "AFFECT_LEVEL_SET_BONUS_3", CInstanceBase::AFFECT_LEVEL_SET_BONUS_3);
	PyModule_AddIntConstant(poModule, "AFFECT_LEVEL_SET_BONUS_4", CInstanceBase::AFFECT_LEVEL_SET_BONUS_4);
	PyModule_AddIntConstant(poModule, "AFFECT_LEVEL_SET_BONUS_5", CInstanceBase::AFFECT_LEVEL_SET_BONUS_5);
	PyModule_AddIntConstant(poModule, "AFFECT_ITEM_SET_BONUS_1", CInstanceBase::AFFECT_ITEM_SET_BONUS_1);
	PyModule_AddIntConstant(poModule, "AFFECT_ITEM_SET_BONUS_2", CInstanceBase::AFFECT_ITEM_SET_BONUS_2);
	PyModule_AddIntConstant(poModule, "AFFECT_ITEM_SET_BONUS_3", CInstanceBase::AFFECT_ITEM_SET_BONUS_3);
	PyModule_AddIntConstant(poModule, "AFFECT_ITEM_SET_BONUS_4", CInstanceBase::AFFECT_ITEM_SET_BONUS_4);
	PyModule_AddIntConstant(poModule, "AFFECT_ITEM_SET_BONUS_5", CInstanceBase::AFFECT_ITEM_SET_BONUS_5);
	PyModule_AddIntConstant(poModule, "AFFECT_NEW_SEBO_1", CInstanceBase::AFFECT_NEW_SEBO_1);
	PyModule_AddIntConstant(poModule, "AFFECT_NEW_SEBO_2", CInstanceBase::AFFECT_NEW_SEBO_2);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS", CInstanceBase::AFFECT_BATTLE_PASS);
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_PREMIUM", CInstanceBase::AFFECT_BATTLE_PASS_PREMIUM);
#endif
#ifdef ENABLE_BATTLE_PASS_EX
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_METIN", CInstanceBase::AFFECT_BATTLE_PASS_METIN);
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_BOSS", CInstanceBase::AFFECT_BATTLE_PASS_BOSS);
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_MONSTER", CInstanceBase::AFFECT_BATTLE_PASS_MONSTER);
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_BERSERKER", CInstanceBase::AFFECT_BATTLE_PASS_BERSERKER);
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_GUCLENDIRME", CInstanceBase::AFFECT_BATTLE_PASS_GUCLENDIRME);
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_ORTALAMA", CInstanceBase::AFFECT_BATTLE_PASS_ORTALAMA);
	PyModule_AddIntConstant(poModule, "AFFECT_BATTLE_PASS_BERSERKER_2", CInstanceBase::AFFECT_BATTLE_PASS_BERSERKER_2);
#endif // ENABLE_BATTLE_PASS_EX
	PyModule_AddIntConstant(poModule, "AFFECT_YILBASI", CInstanceBase::AFFECT_YILBASI);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_PET_NAME", CInstanceBase::AFFECT_PB2_PET_NAME);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_MOUNT_NAME", CInstanceBase::AFFECT_PB2_MOUNT_NAME);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_CHAR_NAME", CInstanceBase::AFFECT_PB2_CHAR_NAME);
	PyModule_AddIntConstant(poModule, "AFFECT_LOVE", CInstanceBase::AFFECT_LOVE);
	PyModule_AddIntConstant(poModule, "AFFECT_WORD", CInstanceBase::AFFECT_WORD);

	PyModule_AddIntConstant(poModule, "AFFECT_OTOMATIKAV", CInstanceBase::AFFECT_OTOMATIKAV);
#ifdef ENABLE_PB2_PREMIUM_SYSTEM
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_SKILL_COLOR", CInstanceBase::AFFECT_PB2_SKILL_COLOR);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_GLOBAL_CHAT", CInstanceBase::AFFECT_PB2_GLOBAL_CHAT);
#endif // ENABLE_PB2_PREMIUM_SYSTEM
#ifdef ENABLE_PB2_NEW_TAG_SYSTEM
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_TAG_VIP", CInstanceBase::AFFECT_PB2_TAG_VIP);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_TAG_KING", CInstanceBase::AFFECT_PB2_TAG_KING);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_TAG_QUEEN", CInstanceBase::AFFECT_PB2_TAG_QUEEN);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_TAG_MAFIA", CInstanceBase::AFFECT_PB2_TAG_MAFIA);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_TAG_BARON", CInstanceBase::AFFECT_PB2_TAG_BARON);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_TAG_STONE_KING", CInstanceBase::AFFECT_PB2_TAG_STONE_KING);
	PyModule_AddIntConstant(poModule, "AFFECT_PB2_TAG_BOSS_KING", CInstanceBase::AFFECT_PB2_TAG_BOSS_KING);

	PyModule_AddIntConstant(poModule, "NEW_AFFECT_PB2_TAG_VIP", CInstanceBase::NEW_AFFECT_PB2_TAG_VIP);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_PB2_TAG_KING", CInstanceBase::NEW_AFFECT_PB2_TAG_KING);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_PB2_TAG_QUEEN", CInstanceBase::NEW_AFFECT_PB2_TAG_QUEEN);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_PB2_TAG_MAFIA", CInstanceBase::NEW_AFFECT_PB2_TAG_MAFIA);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_PB2_TAG_BARON", CInstanceBase::NEW_AFFECT_PB2_TAG_BARON);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_PB2_TAG_STONE_KING", CInstanceBase::NEW_AFFECT_PB2_TAG_STONE_KING);
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_PB2_TAG_BOSS_KING", CInstanceBase::NEW_AFFECT_PB2_TAG_BOSS_KING);
#endif // ENABLE_PB2_NEW_TAG_SYSTEM
#ifdef ENABLE_MULTIFARM_BLOCK
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_MULTI_FARM",				CInstanceBase::NEW_AFFECT_MULTI_FARM);
#endif

}