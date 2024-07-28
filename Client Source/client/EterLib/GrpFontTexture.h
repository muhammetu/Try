#pragma once

#include "GrpTexture.h"
#include "GrpImageTexture.h"
#include "GrpDIB.h"

#include <vector>
#include <map>
#include <unordered_map>

class CGraphicFontTexture : public CGraphicTexture
{
public:
	typedef std::pair<WORD, wchar_t> TCharacterKey;

	typedef struct SCharacterInfomation
	{
		short index;
		short width;
		short height;
		float left;
		float top;
		float right;
		float bottom;
		float advance;
	} TCharacterInfomation;

	typedef std::vector<TCharacterInfomation*>		TPCharacterInfomationVector;

	CGraphicFontTexture();
	virtual ~CGraphicFontTexture();

	void Destroy();
	bool Create(const std::string& fontName, int fontSize, bool bItalic);

	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	bool CheckTextureIndex(DWORD dwTexture);
#ifdef ENABLE_FIX_MOBS_LAG
	CGraphicImageTexture* GetTexture(DWORD dwTexture);
#endif
	void SelectTexture(DWORD dwTexture);

	bool UpdateTexture();

	TCharacterInfomation* GetCharacterInfomation(wchar_t code);
	TCharacterInfomation* UpdateCharacterInfomation(wchar_t code);

	bool IsEmpty() const;

private:
	typedef std::vector<CGraphicImageTexture*>				TGraphicImageTexturePointerVector;
	typedef unordered_map<wchar_t, TCharacterInfomation>	TCharacterInfomationMap;
	void Initialize();

	bool AppendTexture();

	HFONT GetFont();

	CGraphicDib	m_dib;

	HFONT	m_hFontOld;
	HFONT	m_hFont;

	TGraphicImageTexturePointerVector m_pFontTextureVector;

	TCharacterInfomationMap m_charInfoMap;

	int m_x;
	int m_y;
	int m_step;
	bool m_isDirty;

	std::string m_fontName;
	LONG	m_fontSize;
	bool	m_bItalic;
};
