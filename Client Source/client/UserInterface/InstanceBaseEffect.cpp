#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonTextTail.h"
#include "AbstractApplication.h"
#include "AbstractPlayer.h"
#include "PythonPlayer.h"
#include "PythonSystem.h"

#include "../EffectLib/EffectManager.h"
#include "../EffectLib/ParticleSystemData.h"
#include "../EterLib/Camera.h"

float CInstanceBase::ms_fDustGap;
float CInstanceBase::ms_fHorseDustGap;
DWORD CInstanceBase::ms_adwCRCAffectEffect[CInstanceBase::EFFECT_NUM];
std::string CInstanceBase::ms_astAffectEffectAttachBone[EFFECT_NUM];

#define BYTE_COLOR_TO_D3DX_COLOR(r, g, b) D3DXCOLOR(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, 1.0f)
D3DXCOLOR g_akD3DXClrTitle[CInstanceBase::TITLE_NUM];
D3DXCOLOR g_akD3DXClrName[CInstanceBase::NAMECOLOR_NUM];

std::map<int, std::string> g_TitleNameMap;
#ifdef ENABLE_LANDRANK_SYSTEM
std::map<int, std::string> g_TitleLandRankNameMap = {
	{1, "Adana"},
	{2, "Adýyaman"},
	{3, "Afyonkarahisar"},
	{4, "Aðrý"},
	{5, "Amasya"},
	{6, "Ankara"},
	{7, "Antalya"},
	{8, "Artvin"},
	{9, "Aydýn"},
	{10, "Balýkesir"},
	{11, "Bilecik"},
	{12, "Bingöl"},
	{13, "Bitlis"},
	{14, "Bolu"},
	{15, "Burdur"},
	{16, "Bursa"},
	{17, "Çanakkale"},
	{18, "Çankýrý"},
	{19, "Çorum"},
	{20, "Denizli"},
	{21, "Diyarbakýr"},
	{22, "Edirne"},
	{23, "Elazýð"},
	{24, "Erzincan"},
	{25, "Erzurum"},
	{26, "Eskiþehir"},
	{27, "Gaziantep"},
	{28, "Giresun"},
	{29, "Gümüþhane"},
	{30, "Hakkâri"},
	{31, "Hatay"},
	{32, "Isparta"},
	{33, "Mersin"},
	{34, "Ýstanbul"},
	{35, "Ýzmir"},
	{36, "Kars"},
	{37, "Kastamonu"},
	{38, "Kayseri"},
	{39, "Kýrklareli"},
	{40, "Kýrþehir"},
	{41, "Ýzmit"},
	// {41, "Kocaeli"},
	{42, "Konya"},
	{43, "Kütahya"},
	{44, "Malatya"},
	{45, "Manisa"},
	{46, "Kahramanmaraþ"},
	{47, "Mardin"},
	{48, "Muðla"},
	{49, "Muþ"},
	{50, "Nevþehir"},
	{51, "Niðde"},
	{52, "Ordu"},
	{53, "Rize"},
	{54, "Sakarya"},
	{55, "Samsun"},
	{56, "Siirt"},
	{57, "Sinop"},
	{58, "Sivas"},
	{59, "Tekirdað"},
	{60, "Tokat"},
	{61, "Trabzon"},
	{62, "Tunceli"},
	{63, "Þanlýurfa"},
	{64, "Uþak"},
	{65, "Van"},
	{66, "Yozgat"},
	{67, "Zonguldak"},
	{68, "Aksaray"},
	{69, "Bayburt"},
	{70, "Karaman"},
	{71, "Kýrkkale"},
	{72, "Batman"},
	{73, "Þýrnak"},
	{74, "Bartýn"},
	{75, "Ardahan"},
	{76, "Iðdýr"},
	{77, "Yalova"},
	{78, "Karabük"},
	{79, "Kilis"},
	{80, "Osmaniye"},
	{81, "Düzce"},
	{82, "Kýbrýs"},
	// {82, "Gebze"},
	// max 99!
};
std::map<char, char> g_TitleLandRankNameMap2 = {
	{'a', 'ý'},
	{'e', 'i'},
	{'ý', 'ý'},
	{'i', 'i'},
	{'o', 'u'},
	{'ö', 'ü'},
	{'u', 'u'},
	{'ü', 'ü'},
};
#endif

std::set<DWORD> g_kSet_dwPVPReadyKey;
std::set<DWORD> g_kSet_dwPVPKey;
std::set<DWORD> g_kSet_dwGVGKey;
std::set<DWORD> g_kSet_dwDUELKey;

bool g_isEmpireNameMode = false;

void  CInstanceBase::SetEmpireNameMode(bool isEnable)
{
	g_isEmpireNameMode = isEnable;

	if (isEnable)
	{
		g_akD3DXClrName[NAMECOLOR_MOB] = g_akD3DXClrName[NAMECOLOR_EMPIRE_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC] = g_akD3DXClrName[NAMECOLOR_EMPIRE_NPC];
		g_akD3DXClrName[NAMECOLOR_PC] = g_akD3DXClrName[NAMECOLOR_NORMAL_PC];

		for (UINT uEmpire = 1; uEmpire < EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC + uEmpire] = g_akD3DXClrName[NAMECOLOR_EMPIRE_PC + uEmpire];
	}
	else
	{
		g_akD3DXClrName[NAMECOLOR_MOB] = g_akD3DXClrName[NAMECOLOR_NORMAL_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC] = g_akD3DXClrName[NAMECOLOR_NORMAL_NPC];

		for (UINT uEmpire = 0; uEmpire < EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC + uEmpire] = g_akD3DXClrName[NAMECOLOR_NORMAL_PC];
	}
}

const D3DXCOLOR& CInstanceBase::GetIndexedNameColor(UINT eNameColor)
{
	if (eNameColor >= NAMECOLOR_NUM)
	{
		static D3DXCOLOR s_kD3DXClrNameDefault(0xffffffff);
		return s_kD3DXClrNameDefault;
	}

	return g_akD3DXClrName[eNameColor];
}
#ifdef ENABLE_DAMAGE_LIMIT_REWORK
#include "bigInt.h"
void CInstanceBase::AddDamageEffect(unsigned long long damage, BYTE flag, BOOL bSelf, BOOL bTarget)
#else
void CInstanceBase::AddDamageEffect(DWORD damage, BYTE flag, BOOL bSelf, BOOL bTarget)
#endif
{
	if (CPythonSystem::Instance().IsShowDamage())
	{
		SEffectDamage sDamage;
		sDamage.bSelf = bSelf;
		sDamage.bTarget = bTarget;
		sDamage.damage = damage;
		sDamage.flag = flag;
		m_DamageQueue.push_back(sDamage);
	}
}

void CInstanceBase::ProcessDamage()
{
	if (m_DamageQueue.empty())
		return;

	SEffectDamage sDamage = m_DamageQueue.front();

	m_DamageQueue.pop_front();
#ifdef ENABLE_DAMAGE_LIMIT_REWORK
	BigInt damage = sDamage.damage;
#else
	DWORD damage = sDamage.damage;
#endif
	damage *= 1312;
	if (CPythonSystem::Instance().GetLowDamageMode() == 1)
		damage /= 2669;
	else if (CPythonSystem::Instance().GetLowDamageMode() == 2)
		damage /= 5024;
	else if (CPythonSystem::Instance().GetLowDamageMode() == 3)
		damage /= 11108;
	BYTE flag = sDamage.flag;
	BOOL bSelf = sDamage.bSelf;
	BOOL bTarget = sDamage.bTarget;

	CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
	float cameraAngle = GetDegreeFromPosition2(pCamera->GetTarget().x, pCamera->GetTarget().y, pCamera->GetEye().x, pCamera->GetEye().y);

	DWORD FONT_WIDTH = 30;

	CEffectManager& rkEftMgr = CEffectManager::Instance();

	D3DXVECTOR3 v3Pos = m_GraphicThingInstance.GetPosition();

	v3Pos.z += float(m_GraphicThingInstance.GetHeight());

	D3DXVECTOR3 v3Rot = D3DXVECTOR3(0.0f, 0.0f, cameraAngle);

	float fScale = 1.0f + static_cast<float>(CPythonSystem::Instance().GetDamageSize() / 10.0f);
	FONT_WIDTH *= int(fScale);

	if ((flag & DAMAGE_DODGE) || (flag & DAMAGE_BLOCK))
	{
		if (bSelf)
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
			rkEftMgr.CreateEffectWithScale(ms_adwCRCAffectEffect[EFFECT_DAMAGE_MISS], v3Pos, v3Rot, nullptr, true, fScale);
#else
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_MISS], v3Pos, v3Rot);
#endif
		else
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
			rkEftMgr.CreateEffectWithScale(ms_adwCRCAffectEffect[EFFECT_DAMAGE_TARGETMISS], v3Pos, v3Rot, nullptr, true, fScale);
#else
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_TARGETMISS], v3Pos, v3Rot);
#endif
		//__AttachEffect(EFFECT_DAMAGE_MISS);
		return;
	}
	else if (flag & DAMAGE_CRITICAL)
	{
		//rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_CRITICAL],v3Pos,v3Rot);
	}

	string strDamageType;
	DWORD rdwCRCEft = 0;
#ifdef ENABLE_DAMAGE_PROCESS_SYSTEM
	if ((flag & DAMAGE_POISON))
	{
		strDamageType = "poison_";
		rdwCRCEft = EFFECT_DAMAGE_POISON;
	}
	else if ((flag & DAMAGE_BLEEDING))
	{
		strDamageType = "bleed_";
		rdwCRCEft = EFFECT_DAMAGE_BLEEDING;
	}
	else if ((flag & DAMAGE_FIRE))
	{
		strDamageType = "burn_";
		rdwCRCEft = EFFECT_DAMAGE_FIRE;
	}
	else
	{
		if (bSelf) {
			if (m_bDamageEffectType == 0)
				rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE;
			else
				rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE2;

			if (IS_SET(flag, DAMAGE_CRITICAL) && IS_SET(flag, DAMAGE_PENETRATE))
				strDamageType = "damage_crit_pen_";
			else if (IS_SET(flag, DAMAGE_CRITICAL))
				strDamageType = "damage_crit_";
			else if (IS_SET(flag, DAMAGE_PENETRATE))
				strDamageType = "damage_pen_";
			else
				strDamageType = "damage_";

			m_bDamageEffectType = !m_bDamageEffectType;
		}
		else if (bTarget == false) {
			strDamageType = "nontarget_";
			rdwCRCEft = EFFECT_DAMAGE_NOT_TARGET;
			return;
		}
		else {
			rdwCRCEft = EFFECT_DAMAGE_TARGET;
			if (IS_SET(flag, DAMAGE_CRITICAL) && IS_SET(flag, DAMAGE_PENETRATE))
				strDamageType = "target_crit_pen_";
			else if (IS_SET(flag, DAMAGE_CRITICAL))
				strDamageType = "target_crit_";
			else if (IS_SET(flag, DAMAGE_PENETRATE))
				strDamageType = "target_pen_";
			else
				strDamageType = "target_";
		}
	}
#else
	if (bSelf)
	{
		strDamageType = "damage_";
		if (m_bDamageEffectType == 0)
			rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE;
		else
			rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE2;
		m_bDamageEffectType = !m_bDamageEffectType;
	}
	else if (bTarget == false)
	{
		strDamageType = "nontarget_";
		rdwCRCEft = EFFECT_DAMAGE_NOT_TARGET;
	}
	else
	{
		strDamageType = "target_";
		rdwCRCEft = EFFECT_DAMAGE_TARGET;
	}
#endif
#ifdef ENABLE_DAMAGE_LIMIT_REWORK
	unsigned long long index = 0;
	unsigned long long num = 0;
#else
	DWORD index = 0;
	DWORD num = 0;
#endif
	long long modIndex = 1;
	vector<string> textures;
	while (Length(damage))
	{
#define ENABLE_DAMAGE_DOT
#ifdef ENABLE_DAMAGE_DOT
		if ((index + 1) % 4 == 0 && (CPythonSystem::Instance().IsShowDamageDot() || CPythonSystem::Instance().IsShowDamageLetter()))
		{
			if (CPythonSystem::Instance().IsShowDamageDot())
				textures.push_back("d:/ymir work/effect/affect/damagevalue/dot.tga");
			// textures.push_back("d:/ymir work/effect/affect/damagevalue/" + strDamageType + "dot.png");
			else if (CPythonSystem::Instance().IsShowDamageLetter())
			{
				if (modIndex == 1)
					textures.push_back("d:/ymir work/effect/affect/damagevalue/k.tga");
				else if (modIndex == 2)
					textures.push_back("d:/ymir work/effect/affect/damagevalue/m.tga");
				else if (modIndex == 3)
					textures.push_back("d:/ymir work/effect/affect/damagevalue/t.tga");
				else if (modIndex == 4)
					textures.push_back("d:/ymir work/effect/affect/damagevalue/kt.tga");
				else if (modIndex == 5)
					textures.push_back("d:/ymir work/effect/affect/damagevalue/s.tga");
				else if (modIndex == 6)
					textures.push_back("d:/ymir work/effect/affect/damagevalue/sk.tga");
				else /* if (modIndex == 7) */
					textures.push_back("d:/ymir work/effect/affect/damagevalue/skt.tga");
				modIndex++;
			}
		}
		else
		{
#endif"
#ifdef ENABLE_DAMAGE_LIMIT_REWORK
		if (index > 100)
#else
		if (index > 7)
#endif
		{
			TraceError("ProcessDamage Possibility of endless loop");
			break;
		}
		num = damage.Last();
		damage.Pop();
		char numBuf[MAX_PATH];
		sprintf(numBuf, "%lld.dds", num);
		textures.push_back("d:/ymir work/effect/affect/damagevalue/" + strDamageType + numBuf);
#ifdef ENABLE_DAMAGE_DOT
		}
#endif

		rkEftMgr.SetEffectTextures(ms_adwCRCAffectEffect[rdwCRCEft], textures);

		D3DXMATRIX matrix, matTrans;
		D3DXMatrixIdentity(&matrix);
		matrix._41 = v3Pos.x;
		matrix._42 = v3Pos.y;
		matrix._43 = v3Pos.z;
		D3DXMatrixTranslation(&matrix, v3Pos.x, v3Pos.y, v3Pos.z);
		D3DXMatrixMultiply(&matrix, &pCamera->GetInverseViewMatrix(), &matrix);
		D3DXMatrixTranslation(&matTrans, FONT_WIDTH * index, 0, 0);
		matTrans._41 = -matTrans._41;
		matrix = matTrans * matrix;
		D3DXMatrixMultiply(&matrix, &pCamera->GetViewMatrix(), &matrix);

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		rkEftMgr.CreateEffectWithScale(ms_adwCRCAffectEffect[rdwCRCEft], D3DXVECTOR3(matrix._41, matrix._42, matrix._43), v3Rot, nullptr, true, fScale);
#else
		rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[rdwCRCEft], D3DXVECTOR3(matrix._41, matrix._42, matrix._43), v3Rot);
#endif

		textures.clear();

		index++;
	}
}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
void CInstanceBase::AttachSpecialEffect(DWORD effect, bool ignoreFrustum, float fScale)
#else
void CInstanceBase::AttachSpecialEffect(DWORD effect, float fScale)
#endif
{
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	__AttachEffect(effect, ignoreFrustum, fScale);
#else
	__AttachEffect(effect, fScale);
#endif
}

void CInstanceBase::LevelUp()
{
	__AttachEffect(EFFECT_LEVELUP);
}

void CInstanceBase::SkillUp()
{
	__AttachEffect(EFFECT_SKILLUP);
}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
void CInstanceBase::CreateSpecialEffect(DWORD iEffectIndex, bool ignoreFrustum, float fScale)
#else
void CInstanceBase::CreateSpecialEffect(DWORD iEffectIndex, float fScale)
#endif
{
	const D3DXMATRIX & c_rmatGlobal = m_GraphicThingInstance.GetTransform();

	DWORD dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	DWORD dwEffectCRC = ms_adwCRCAffectEffect[iEffectIndex];
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	CEffectManager::Instance().CreateEffectInstanceWithScale(dwEffectIndex, dwEffectCRC, NULL, ignoreFrustum, fScale);
#else
	CEffectManager::Instance().CreateEffectInstanceWithScale(dwEffectIndex, dwEffectCRC, fScale);
#endif
	CEffectManager::Instance().SelectEffectInstance(dwEffectIndex);
	CEffectManager::Instance().SetEffectInstanceGlobalMatrix(c_rmatGlobal);
}

void CInstanceBase::__EffectContainer_Destroy()
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();

	SEffectContainer::Dict::iterator i;
	for (i = rkDctEftID.begin(); i != rkDctEftID.end(); ++i)
		__DetachEffect(i->second);

	rkDctEftID.clear();
}

void CInstanceBase::DestroyContainer()
{
	__EffectContainer_Destroy();
	__StoneSmoke_Destroy();
}

void CInstanceBase::AttachContainer()
{
	__StoneSmoke_Create(0);
}

void CInstanceBase::__EffectContainer_Initialize()
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();
	rkDctEftID.clear();
}

CInstanceBase::SEffectContainer::Dict& CInstanceBase::__EffectContainer_GetDict()
{
	return m_kEffectContainer.m_kDct_dwEftID;
}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
DWORD CInstanceBase::__EffectContainer_AttachEffect(DWORD dwEftKey, bool ignoreFrustum)
#else
DWORD CInstanceBase::__EffectContainer_AttachEffect(DWORD dwEftKey)
#endif
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();
	SEffectContainer::Dict::iterator f = rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end() != f)
		return 0;

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	DWORD dwEftID = __AttachEffect(dwEftKey, ignoreFrustum, m_GraphicThingInstance.GetScale().x);
#else
	DWORD dwEftID = __AttachEffect(dwEftKey, m_GraphicThingInstance.GetScale().x);
#endif
	rkDctEftID.insert(SEffectContainer::Dict::value_type(dwEftKey, dwEftID));
	return dwEftID;
}

void CInstanceBase::__EffectContainer_DetachEffect(DWORD dwEftKey)
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();
	SEffectContainer::Dict::iterator f = rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end() == f)
		return;

	__DetachEffect(f->second);

	rkDctEftID.erase(f);
}

void CInstanceBase::__AttachEmpireEffect(DWORD eEmpire)
{
	if (!__IsExistMainInstance())
		return;

	CInstanceBase* pkInstMain = __GetMainInstancePtr();

	if (IsWarp())
		return;
	if (IsObject())
		return;
	if (IsFlag())
		return;
	if (IsResource())
		return;
	if (IsNPC())
		return;
	if (IsPet())
		return;
	if (IsMount())
		return;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (IsNewPet())
		return;
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	if (IsNewExMount())
		return;
#endif
	if (pkInstMain->IsGameMaster())
	{
	}
	else
	{
		if (pkInstMain->IsSameEmpire(*this))
			return;

		// HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
		if (IsAffect(AFFECT_EUNHYEONG))
			return;
		// END_OF_HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
	}

	if (IsGameMaster())
		return;

	__EffectContainer_AttachEffect(EFFECT_EMPIRE + eEmpire);
}

void CInstanceBase::__AttachSelectEffect()
{
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	__EffectContainer_AttachEffect(EFFECT_SELECT, true);
#else
	__EffectContainer_AttachEffect(EFFECT_SELECT);
#endif
}

void CInstanceBase::__DetachSelectEffect()
{
	__EffectContainer_DetachEffect(EFFECT_SELECT);
}

void CInstanceBase::__AttachTargetEffect()
{
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	__EffectContainer_AttachEffect(EFFECT_TARGET, true);
#else
	__EffectContainer_AttachEffect(EFFECT_TARGET);
#endif
}

void CInstanceBase::__DetachTargetEffect()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET);
}

void CInstanceBase::__StoneSmoke_Inialize()
{
	m_kStoneSmoke.m_dwEftID = 0;
}

void CInstanceBase::__StoneSmoke_Destroy()
{
	if (!m_kStoneSmoke.m_dwEftID)
		return;

	__DetachEffect(m_kStoneSmoke.m_dwEftID);
	m_kStoneSmoke.m_dwEftID = 0;
}

void CInstanceBase::__StoneSmoke_Create(DWORD eSmoke)
{
	m_kStoneSmoke.m_dwEftID = m_GraphicThingInstance.AttachSmokeEffect(eSmoke);
}

void CInstanceBase::SetAlpha(float fAlpha)
{
	__SetBlendRenderingMode();
	__SetAlphaValue(fAlpha);
}

bool CInstanceBase::UpdateDeleting()
{
	Update();
	Transform();

	IAbstractApplication& rApp = IAbstractApplication::GetSingleton();

	float fAlpha = __GetAlphaValue() - (rApp.GetGlobalElapsedTime() * 1.5f);
	__SetAlphaValue(fAlpha);

	if (fAlpha < 0.0f)
		return false;

	return true;
}

void CInstanceBase::DeleteBlendOut()
{
	__SetBlendRenderingMode();
	__SetAlphaValue(1.0f);
	DetachTextTail();

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.NotifyDeletingCharacterInstance(GetVirtualID());
}

void CInstanceBase::ClearPVPKeySystem()
{
	g_kSet_dwPVPReadyKey.clear();
	g_kSet_dwPVPKey.clear();
	g_kSet_dwGVGKey.clear();
	g_kSet_dwDUELKey.clear();
}

void CInstanceBase::InsertPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.insert(dwPVPKey);
}

void CInstanceBase::InsertPVPReadyKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPReadyKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.insert(dwPVPReadyKey);
}

void CInstanceBase::RemovePVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.erase(dwPVPKey);
}

void CInstanceBase::InsertGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID)
{
	DWORD dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.insert(dwGVGKey);
}

void CInstanceBase::RemoveGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID)
{
	DWORD dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.erase(dwGVGKey);
}

void CInstanceBase::InsertDUELKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwDUELKey.insert(dwPVPKey);
}

DWORD CInstanceBase::__GetPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	if (dwVIDSrc > dwVIDDst)
		std::swap(dwVIDSrc, dwVIDDst);

	DWORD awSrc[2];
	awSrc[0] = dwVIDSrc;
	awSrc[1] = dwVIDDst;

	const BYTE* s = (const BYTE*)awSrc;
	const BYTE* end = s + sizeof(awSrc);
	unsigned long h = 0;

	while (s < end)
	{
		h *= 16777619;
		h ^= (BYTE) * (BYTE*)(s++);
	}

	return h;
}

bool CInstanceBase::__FindPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPKey.end() == g_kSet_dwPVPKey.find(dwPVPKey))
		return false;

	return true;
}

bool CInstanceBase::__FindPVPReadyKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPReadyKey.end() == g_kSet_dwPVPReadyKey.find(dwPVPKey))
		return false;

	return true;
}
bool CInstanceBase::__FindGVGKey(DWORD dwSrcGuildID, DWORD dwDstGuildID)
{
	DWORD dwGVGKey = __GetPVPKey(dwSrcGuildID, dwDstGuildID);

	if (g_kSet_dwGVGKey.end() == g_kSet_dwGVGKey.find(dwGVGKey))
		return false;

	return true;
}
bool CInstanceBase::__FindDUELKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwDUELKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwDUELKey.end() == g_kSet_dwDUELKey.find(dwDUELKey))
		return false;

	return true;
}

bool CInstanceBase::IsPVPInstance(CInstanceBase& rkInstSel)
{
	DWORD dwVIDSrc = GetVirtualID();
	DWORD dwVIDDst = rkInstSel.GetVirtualID();

	DWORD dwGuildIDSrc = GetGuildID();
	DWORD dwGuildIDDst = rkInstSel.GetGuildID();

	if (GetDuelMode())
		return true;

	return __FindPVPKey(dwVIDSrc, dwVIDDst) || __FindGVGKey(dwGuildIDSrc, dwGuildIDDst);
	//__FindDUELKey(dwVIDSrc, dwVIDDst);
}

const D3DXCOLOR& CInstanceBase::GetNameColor()
{
	return GetIndexedNameColor(GetNameColorIndex());
}

UINT CInstanceBase::GetNameColorIndex()
{
	if (IsPC())
	{
		if (m_isKiller)
		{
			return NAMECOLOR_PK;
		}

		if (__IsExistMainInstance() && !__IsMainInstance())
		{
			CInstanceBase* pkInstMain = __GetMainInstancePtr();
			if (!pkInstMain)
			{
				TraceError("CInstanceBase::GetNameColorIndex - MainInstance is NULL");
				return NAMECOLOR_PC;
			}
			DWORD dwVIDMain = pkInstMain->GetVirtualID();
			DWORD dwVIDSelf = GetVirtualID();

			if (pkInstMain->GetDuelMode())
			{
				switch (pkInstMain->GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return NAMECOLOR_PC + GetEmpireID();
				case DUEL_START:
					if (__FindDUELKey(dwVIDMain, dwVIDSelf))
						return NAMECOLOR_PVP;
					else
						return NAMECOLOR_PC + GetEmpireID();
				}
			}

			if (pkInstMain->IsSameEmpire(*this))
			{
				if (__FindPVPKey(dwVIDMain, dwVIDSelf))
				{
					return NAMECOLOR_PVP;
				}

				DWORD dwGuildIDMain = pkInstMain->GetGuildID();
				DWORD dwGuildIDSelf = GetGuildID();
				if (__FindGVGKey(dwGuildIDMain, dwGuildIDSelf))
				{
					return NAMECOLOR_PVP;
				}
				/*
				if (__FindDUELKey(dwVIDMain, dwVIDSelf))
				{
					return NAMECOLOR_PVP;
				}
				*/
			}
			else
			{
				return NAMECOLOR_PVP;
			}
		}

		IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
		if (rPlayer.IsPartyMemberByVID(GetVirtualID()))
			return NAMECOLOR_PARTY;

		return NAMECOLOR_PC + GetEmpireID();
	}
	else if (IsNPC())
	{
		return NAMECOLOR_NPC;
	}
	else if (IsEnemy())
	{
#ifdef ENABLE_BOSS_ON_MINIMAP
		if (IsBoss())
			return NAMECOLOR_BOSS;
		else
			return NAMECOLOR_MOB;
#else
		return NAMECOLOR_MOB;
#endif
	}

	return D3DXCOLOR(0xffffffff);
}

const D3DXCOLOR& CInstanceBase::GetTitleColor()
{
	UINT uGrade = GetAlignmentGrade();
	if (uGrade >= TITLE_NUM)
	{
		static D3DXCOLOR s_kD3DXClrTitleDefault(0xffffffff);
		return s_kD3DXClrTitleDefault;
	}

	return g_akD3DXClrTitle[uGrade];
}

void CInstanceBase::AttachTextTail()
{
	if (m_isTextTail)
	{
		TraceError("CInstanceBase::AttachTextTail - VID [%d] ALREADY EXIST", GetVirtualID());
		return;
	}

	m_isTextTail = true;

	DWORD dwVID = GetVirtualID();

	float fTextTailHeight = IsMountingHorse() ? 110.0f : 10.0f;

	//if (IsStone())
	//	fTextTailHeight = (10.0f) * static_cast<float>(CPythonSystem::Instance().GetStoneSize() * 2.0f);

	if (IsNewPet() || IsNewExMount())
		fTextTailHeight = (10.0f) * static_cast<float>(CPythonSystem::Instance().GetPetSize()*1.0f);

	static D3DXCOLOR s_kD3DXClrTextTail = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	CPythonTextTail::Instance().RegisterCharacterTextTail(m_dwGuildID, dwVID, s_kD3DXClrTextTail, fTextTailHeight);

	// CHARACTER_LEVEL
	if (m_dwLevel)
	{
		UpdateTextTailLevel(m_dwLevel);
	}
}

void CInstanceBase::DetachTextTail()
{
	if (!m_isTextTail)
		return;

	m_isTextTail = false;
	CPythonTextTail::Instance().DeleteCharacterTextTail(GetVirtualID());
}

void CInstanceBase::UpdateTextTailLevel(DWORD level)
{
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	static D3DXCOLOR s_kMobLevelColor = D3DCOLOR_XRGB(255, 255, 0);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	static D3DXCOLOR s_kNewPetLevelColor = D3DCOLOR_XRGB(255, 255, 0);
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	static D3DXCOLOR s_kNewMountLevelColor = D3DCOLOR_XRGB(255, 255, 0);
#endif

	D3DXCOLOR s_kPlayerLevelColor = D3DXCOLOR(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);;

	if (level > 190)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(70, 201, 198);//turkuaz
	else if (level > 180)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(102, 0, 153);//mor
	else if (level > 170)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(251, 161, 191);//pembe
	else if (level > 160)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(255, 172, 64);//turuncu
	else if (level > 150)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(247, 247, 1); // sari
	else if (level > 140)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(26, 5, 243); // mavi
	else if (level > 130)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(3, 123, 3); // yeisl
	else if (level > 120)
		s_kPlayerLevelColor = D3DCOLOR_XRGB(171, 5, 1); // kirmizi

	char szText[256];
	sprintf(szText, "Sv. %d", level);

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	if (IsPC() && level > 0)
	{
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kPlayerLevelColor);
	}
	else if ((IsEnemy() || IsStone()) && level > 0)
	{
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kMobLevelColor);
	}
#ifdef ENABLE_GROWTH_PET_SYSTEM
	else if (IsNewPet() && level > 0)
	{
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kNewPetLevelColor);
	}
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	else if (IsNewExMount() && level > 0)
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kNewMountLevelColor);
#endif
	else
		CPythonTextTail::Instance().DetachLevel(GetVirtualID());
#else
	CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kPlayerLevelColor);
#endif
}

void CInstanceBase::RefreshTextTail()
{
	CPythonTextTail::Instance().SetCharacterTextTailColor(GetVirtualID(), GetNameColor());

	int iAlignmentGrade = GetAlignmentGrade();
	if (TITLE_NONE == iAlignmentGrade)
	{
		CPythonTextTail::Instance().DetachTitle(GetVirtualID());
	}
	else
	{
		std::map<int, std::string>::iterator itor = g_TitleNameMap.find(iAlignmentGrade);
		if (g_TitleNameMap.end() != itor)
		{
			const std::string& c_rstrTitleName = itor->second;
			CPythonTextTail::Instance().AttachTitle(GetVirtualID(), c_rstrTitleName.c_str(), GetTitleColor());
		}
	}

#ifdef ENABLE_LANDRANK_SYSTEM
	const short sLandRankPoint = GetLandRank();

	if (!IsPC() || TITLE_NONE_LANDRANK == sLandRankPoint)
	{
		CPythonTextTail::Instance().DetachLandRank(GetVirtualID());
	}
	else
	{
		string str = "";
		GetLandRankString(str, sLandRankPoint);
		CPythonTextTail::Instance().AttachLandRank(GetVirtualID(), str.c_str());
	}
#endif

#ifdef ENABLE_REBORN_SYSTEM
	short sReborn = GetReborn();

	if (!IsPC() || TITLE_NONE_REBORN == sReborn)
		CPythonTextTail::Instance().DetachReborn(GetVirtualID());
	else
	{
		CPythonTextTail::Instance().AttachReborn(GetVirtualID(), sReborn);
	}

#endif

#ifdef ENABLE_RANK_SYSTEM
	short sRank = GetRank();

	if (IsPC())
	{
		if (sRank > 0)
			CPythonTextTail::Instance().AttachRank(GetVirtualID(), sRank);
		else
			CPythonTextTail::Instance().DeattachRank(GetVirtualID());
	}
#endif

#ifdef ENABLE_MONIKER_SYSTEM
	const char* moniker = GetMoniker();

	if (!IsPC() || strlen(moniker) == 0)
		CPythonTextTail::Instance().DetachMoniker(GetVirtualID());
	else
	{
		CPythonTextTail::Instance().AttachMoniker(GetVirtualID(), moniker);
	}

#endif

#ifdef ENABLE_TEAM_SYSTEM
	short sTeam = GetTeam();

	if (IsPC())
	{
		if (sTeam)
			CPythonTextTail::Instance().AttachTeam(GetVirtualID(), sTeam);
		else
			CPythonTextTail::Instance().DeattachTeam(GetVirtualID());
	}

#endif
#ifdef ENABLE_LOVE_SYSTEM
	const char* love1 = GetLove1();
	const char* love2 = GetLove2();

	if (!IsPC())
	{
		CPythonTextTail::Instance().DetachLove(GetVirtualID());
	}
	else
	{
		if (strlen(love1) != 0 && strlen(love2) != 0)
		{
			CPythonTextTail::Instance().AttachLove(GetVirtualID(), love1, love2);
		}
	}
#endif // ENABLE_LOVE_SYSTEM

#ifdef ENABLE_WORD_SYSTEM
	const char* word = GetWord();

	if (!IsPC())
	{
		CPythonTextTail::Instance().DetachWord(GetVirtualID());
	}
	else
	{
		if (strlen(word) != 0)
		{
			CPythonTextTail::Instance().AttachWord(GetVirtualID(), word);
		}
	}
#endif // ENABLE_WORD_SYSTEM

}

#ifdef ENABLE_LANDRANK_SYSTEM
void CInstanceBase::GetLandRankString(string& sLandRank, short val)
{
	auto it = g_TitleLandRankNameMap.find(val % 100);
	if (g_TitleLandRankNameMap.end() != it)
		sLandRank += it->second;
	if (TITLE_NUM_LANDRANK_L <= val)
		sLandRank += 'l';
	if (TITLE_NUM_LANDRANK <= val)
	{
		for (int i = sLandRank.length() - 1; i >= 0; --i)
		{
			if (sLandRank[i] == 'a' || sLandRank[i] == 'e' || sLandRank[i] == 'ý' || sLandRank[i] == 'i' ||
				sLandRank[i] == 'o' || sLandRank[i] == 'ö' || sLandRank[i] == 'u' || sLandRank[i] == 'ü')
			{
				auto it2 = g_TitleLandRankNameMap2.find(sLandRank[i]);
				if (g_TitleLandRankNameMap2.end() != it2)
					sLandRank += it2->second;
				break;
			}
		}
	}
}
#endif

void CInstanceBase::RefreshTextTailTitle()
{
	RefreshTextTail();
}

/////////////////////////////////////////////////
void CInstanceBase::__ClearAffectFlagContainer()
{
	m_kAffectFlagContainer.Clear();
}

void CInstanceBase::__ClearAffects()
{
	if (IsStone())
	{
		__StoneSmoke_Destroy();
	}
	else
	{
		for (int iAffect = 0; iAffect < AFFECT_NUM; ++iAffect)
		{
			__DetachEffect(m_adwCRCAffectEffect[iAffect]);
			m_adwCRCAffectEffect[iAffect] = 0;
		}

		__ClearAffectFlagContainer();
	}

	m_GraphicThingInstance.__OnClearAffects();
}

/////////////////////////////////////////////////

void CInstanceBase::__SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	for (int i = 0; i < CAffectFlagContainer::BIT_SIZE; ++i)
	{
		bool isOldSet = m_kAffectFlagContainer.IsSet(i);
		bool isNewSet = c_rkAffectFlagContainer.IsSet(i);

		if (isOldSet != isNewSet)
		{
			__SetAffect(i, isNewSet);

			if (isNewSet)
				m_GraphicThingInstance.__OnSetAffect(i);
			else
				m_GraphicThingInstance.__OnResetAffect(i);
		}
	}

	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);
}

void CInstanceBase::__SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);

	DWORD eSmoke;
	if (m_kAffectFlagContainer.IsSet(STONE_SMOKE8))
		eSmoke = 3;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE5) | m_kAffectFlagContainer.IsSet(STONE_SMOKE6) | m_kAffectFlagContainer.IsSet(STONE_SMOKE7))
		eSmoke = 2;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE2) | m_kAffectFlagContainer.IsSet(STONE_SMOKE3) | m_kAffectFlagContainer.IsSet(STONE_SMOKE4))
		eSmoke = 1;
	else
		eSmoke = 0;

	__StoneSmoke_Destroy();
	__StoneSmoke_Create(eSmoke);
}

void CInstanceBase::SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	if (IsStone())
		__SetStoneSmokeFlagContainer(c_rkAffectFlagContainer);
	else
		__SetNormalAffectFlagContainer(c_rkAffectFlagContainer);
}

void CInstanceBase::SCRIPT_SetAffect(UINT eAffect, bool isVisible)
{
	__SetAffect(eAffect, isVisible);
}

void CInstanceBase::__SetReviveInvisibilityAffect(bool isVisible)
{
	if (isVisible)
	{
		m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
	}
	else
	{
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
	}
}

void CInstanceBase::__Assassin_SetEunhyeongAffect(bool isVisible)
{
	if (isVisible)
	{
		if (__IsMainInstance() || __MainCanSeeHiddenThing())
		{
			m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
		}
		else
		{
			m_GraphicThingInstance.BlendAlphaValue(0.0f, 1.0f);
			m_GraphicThingInstance.HideAllAttachingEffect();
		}
	}
	else
	{
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_INVISIBILITY) && __MainCanSeeHiddenThing())
			return;
#endif
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
		m_GraphicThingInstance.ShowAllAttachingEffect();
	}
}

void CInstanceBase::__Shaman_SetParalysis(bool isParalysis)
{
	m_GraphicThingInstance.SetParalysis(isParalysis);
}

void CInstanceBase::__Warrior_SetGeomgyeongAffect(bool isVisible)
{
	if (isVisible)
	{
		if (m_kWarrior.m_dwGeomgyeongEffect)
			__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);

		m_GraphicThingInstance.SetReachScale(1.5f);
		if (m_GraphicThingInstance.IsTwoHandMode())
			m_kWarrior.m_dwGeomgyeongEffect = __AttachEffect(EFFECT_WEAPON + WEAPON_TWOHAND);
		else
			m_kWarrior.m_dwGeomgyeongEffect = __AttachEffect(EFFECT_WEAPON + WEAPON_ONEHAND);
	}
	else
	{
		m_GraphicThingInstance.SetReachScale(1.0f);

		__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);
		m_kWarrior.m_dwGeomgyeongEffect = 0;
	}
}

void CInstanceBase::__SetAffect(UINT eAffect, bool isVisible)
{
	switch (eAffect)
	{
	case AFFECT_YMIR:
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
			return;
#else
		if (IsAffect(AFFECT_INVISIBILITY))
			return;
#endif
		break;

	case AFFECT_CHEONGEUN:
		m_GraphicThingInstance.SetResistFallen(isVisible);
		break;
	case AFFECT_GEOMGYEONG:
		__Warrior_SetGeomgyeongAffect(isVisible);
		return;
		break;
	case AFFECT_REVIVE_INVISIBILITY:
		__Assassin_SetEunhyeongAffect(isVisible);
		break;
	case AFFECT_EUNHYEONG:
		__Assassin_SetEunhyeongAffect(isVisible);
		break;
	case AFFECT_GYEONGGONG:
	case AFFECT_KWAESOK:
		if (isVisible)
			if (!IsWalking())
				return;
		break;
	case AFFECT_INVISIBILITY:
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (__MainCanSeeHiddenThing())
		{
			if (isVisible)
				m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
			else
				m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
			break;
		}
#endif
		if (isVisible)
		{
			m_GraphicThingInstance.ClearAttachingEffect();
			__EffectContainer_Destroy();
			DetachTextTail();
		}
		else
		{
			m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
			AttachTextTail();
			RefreshTextTail();
		}
		return;
		break;
		//		case AFFECT_FAINT:
		//			m_GraphicThingInstance.SetFaint(isVisible);
		//			break;
		//		case AFFECT_SLEEP:
		//			m_GraphicThingInstance.SetSleep(isVisible);
		//			break;
	case AFFECT_STUN:
		m_GraphicThingInstance.SetSleep(isVisible);
		break;
	}

	if (eAffect >= AFFECT_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetAffect(eAffect:%d<AFFECT_NUM:%d, isVisible=%d)", GetVirtualID(), eAffect, isVisible);
		return;
	}

	if (isVisible)
	{
		if (!m_adwCRCAffectEffect[eAffect])
		{
			m_adwCRCAffectEffect[eAffect] = __AttachEffect(EFFECT_AFFECT + eAffect);
		}
	}
	else
	{
		if (m_adwCRCAffectEffect[eAffect])
		{
			__DetachEffect(m_adwCRCAffectEffect[eAffect]);
			m_adwCRCAffectEffect[eAffect] = 0;
		}
	}
}

bool CInstanceBase::IsPossibleEmoticon()
{
	CEffectManager& rkEftMgr = CEffectManager::Instance();
	for (DWORD eEmoticon = 0; eEmoticon < EMOTICON_NUM; eEmoticon++)
	{
		DWORD effectID = ms_adwCRCAffectEffect[EFFECT_EMOTICON + eEmoticon];
		if (effectID && rkEftMgr.IsAliveEffect(effectID))
			return false;
	}

	if (ELTimer_GetMSec() - m_dwEmoticonTime < 1000)
	{
#ifndef LIVE_SERVER
		TraceError("ELTimer_GetMSec() - m_dwEmoticonTime");
#endif
		return false;
	}

	return true;
}

void CInstanceBase::SetFishEmoticon()
{
	SetEmoticon(EMOTICON_FISH);
}

void CInstanceBase::SetEmoticon(UINT eEmoticon)
{
	if (eEmoticon >= EMOTICON_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetEmoticon(eEmoticon:%d<EMOTICON_NUM:%d, isVisible=%d)",
			GetVirtualID(), eEmoticon);
		return;
	}
	if (IsPossibleEmoticon())
	{
		D3DXVECTOR3 v3Pos = m_GraphicThingInstance.GetPosition();
		v3Pos.z += float(m_GraphicThingInstance.GetHeight());

		//CEffectManager& rkEftMgr=CEffectManager::Instance();
		CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();

		D3DXVECTOR3 v3Dir = (pCamera->GetEye() - v3Pos) * 9 / 10;
		v3Pos = pCamera->GetEye() - v3Dir;

		v3Pos = D3DXVECTOR3(0, 0, 0);
		v3Pos.z += float(m_GraphicThingInstance.GetHeight());

		//rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_EMOTICON+eEmoticon],v3Pos,D3DXVECTOR3(0,0,0));
		m_GraphicThingInstance.AttachEffectByID(0, NULL, ms_adwCRCAffectEffect[EFFECT_EMOTICON + eEmoticon], &v3Pos);
		m_dwEmoticonTime = ELTimer_GetMSec();
	}
}

void CInstanceBase::SetDustGap(float fDustGap)
{
	ms_fDustGap = fDustGap;
}

void CInstanceBase::SetHorseDustGap(float fDustGap)
{
	ms_fHorseDustGap = fDustGap;
}

void CInstanceBase::__DetachEffect(DWORD dwEID)
{
	m_GraphicThingInstance.DettachEffect(dwEID);
}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
DWORD CInstanceBase::__AttachEffect(UINT eEftType, bool ignoreFrustum, float fScale)
#else
DWORD CInstanceBase::__AttachEffect(UINT eEftType)
#endif
{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	switch (eEftType)
	{
	case EFFECT_AFFECT + AFFECT_GONGPO:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/fear_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_JUMAGAP:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/jumagap_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_HOSIN:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/shaman/effect/3hosin_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_BOHO:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/shaman/effect/boho_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_KWAESOK:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/shaman/effect/10kwaesok_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_HEUKSIN:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/heuksin_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_MUYEONG:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/muyeong_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_FIRE:
		RegisterEffect(eEftType, "Bip01", "d:/ymir work/effect/hit/blow_flame/flame_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_GICHEON:
		RegisterEffect(eEftType, "Bip01 R Hand", "d:/ymir work/pc/shaman/effect/6gicheon_hand.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_JEUNGRYEOK:
		RegisterEffect(eEftType, "Bip01 L Hand", "d:/ymir work/pc/shaman/effect/jeungryeok_hand.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_PABEOP:
		RegisterEffect(eEftType, "Bip01 Head", "d:/ymir work/pc/sura/effect/pabeop_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_CHEONGEUN:
	case EFFECT_AFFECT + AFFECT_FALLEN_CHEONGEUN:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/warrior/effect/gyeokgongjang_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_GWIGEOM:
		RegisterEffect(eEftType, "Bip01 R Finger2", "d:/ymir work/pc/sura/effect/gwigeom_loop.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_GYEONGGONG:
		RegisterEffect(eEftType, "", "d:/ymir work/pc/assassin/effect/gyeonggong_loop.mse", false, GetNameString());
		break;
	case EFFECT_WEAPON + WEAPON_ONEHAND:
		RegisterEffect(eEftType, "equip_right_hand", "d:/ymir work/pc/warrior/effect/geom_sword_loop.mse", false, GetNameString());
		break;
	case EFFECT_WEAPON + WEAPON_TWOHAND:
		RegisterEffect(eEftType, "equip_right_hand", "d:/ymir work/pc/warrior/effect/geom_spear_loop.mse", false, GetNameString());
		break;
#ifdef ENABLE_WOLFMAN_CHARACTER
	case EFFECT_AFFECT + AFFECT_RED_POSSESSION:
		RegisterEffect(eEftType, "Bip01", "d:/ymir work/effect/hit/blow_flame/flame_loop_w.mse", false, GetNameString());
		break;
	case EFFECT_AFFECT + AFFECT_BLUE_POSSESSION:
		RegisterEffect(eEftType, "", "d:/ymir work/pc3/common/effect/gyeokgongjang_loop_w.mse", false, GetNameString());
		break;
#endif
	default:
		break;
	}
#endif
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return 0;
#else
	if (IsAffect(AFFECT_INVISIBILITY))
		return 0;
#endif

	if (eEftType >= EFFECT_NUM)
		return 0;

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	DWORD* dwSkillColor = m_GraphicThingInstance.GetSkillColorByEffectID(eEftType);
#endif

	// ENABLE_GRAPHIC_OPTIMIZATION
	// uyarlanmadi dikkat!
	if (ms_astAffectEffectAttachBone[eEftType].empty())
	{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		return m_GraphicThingInstance.AttachEffectByID(0, NULL, ms_adwCRCAffectEffect[eEftType], NULL, dwSkillColor, ignoreFrustum, fScale);
#else
		return m_GraphicThingInstance.AttachEffectByID(0, NULL, ms_adwCRCAffectEffect[eEftType], ignoreFrustum);
#endif
	}
	else
	{
		std::string& rstrBoneName = ms_astAffectEffectAttachBone[eEftType];
		const char* c_szBoneName;
		if (0 == rstrBoneName.compare("PART_WEAPON"))
		{
			if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szBoneName))
			{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType], NULL, dwSkillColor, ignoreFrustum);
#else
				return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType], ignoreFrustum);
#endif
			}
		}
		else if (0 == rstrBoneName.compare("PART_WEAPON_LEFT"))
		{
			if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON_LEFT, &c_szBoneName))
			{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType], NULL, dwSkillColor, ignoreFrustum);
#else
				return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType], ignoreFrustum);
#endif
			}
		}
		else
		{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			return m_GraphicThingInstance.AttachEffectByID(0, rstrBoneName.c_str(), ms_adwCRCAffectEffect[eEftType], NULL, dwSkillColor, ignoreFrustum, fScale);
#else
			return m_GraphicThingInstance.AttachEffectByID(0, rstrBoneName.c_str(), ms_adwCRCAffectEffect[eEftType], ignoreFrustum);
#endif
		}
	}

	return 0;
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
bool CInstanceBase::RegisterEffect(UINT eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache, const char* name)
#else
bool CInstanceBase::RegisterEffect(UINT eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache)
#endif
{
	if (eEftType >= EFFECT_NUM)
		return false;

	ms_astAffectEffectAttachBone[eEftType] = c_szEftAttachBone;

	DWORD& rdwCRCEft = ms_adwCRCAffectEffect[eEftType];
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	if (!CEffectManager::Instance().RegisterEffect2(c_szEftName, &rdwCRCEft, isCache, name))
#else
	if (!CEffectManager::Instance().RegisterEffect2(c_szEftName, &rdwCRCEft, isCache))
#endif
	{
		TraceError("CInstanceBase::RegisterEffect(eEftType=%d, c_szEftAttachBone=%s, c_szEftName=%s, isCache=%d) - Error", eEftType, c_szEftAttachBone, c_szEftName, isCache);
		rdwCRCEft = 0;
		return false;
	}

	return true;
}

void CInstanceBase::RegisterTitleName(int iIndex, const char* c_szTitleName)
{
	g_TitleNameMap.insert(make_pair(iIndex, c_szTitleName));
}

D3DXCOLOR __RGBToD3DXColoru(UINT r, UINT g, UINT b)
{
	DWORD dwColor = 0xff; dwColor <<= 8;
	dwColor |= r; dwColor <<= 8;
	dwColor |= g; dwColor <<= 8;
	dwColor |= b;

	return D3DXCOLOR(dwColor);
}

bool CInstanceBase::RegisterNameColor(UINT uIndex, UINT r, UINT g, UINT b)
{
	if (uIndex >= NAMECOLOR_NUM)
		return false;

	g_akD3DXClrName[uIndex] = __RGBToD3DXColoru(r, g, b);
	return true;
}

bool CInstanceBase::RegisterTitleColor(UINT uIndex, UINT r, UINT g, UINT b)
{
	if (uIndex >= TITLE_NUM)
		return false;

	g_akD3DXClrTitle[uIndex] = __RGBToD3DXColoru(r, g, b);
	return true;
}