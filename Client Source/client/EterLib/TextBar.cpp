#include "StdAfx.h"
#include "TextBar.h"
#include "../eterlib/Util.h"

void CTextBar::__SetFont(int fontSize, bool isBold)
{
	LOGFONT logFont;

	memset(&logFont, 0, sizeof(LOGFONT));

	logFont.lfHeight = fontSize;
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;

	if (isBold)
		logFont.lfWeight = FW_BOLD;
	else
		logFont.lfWeight = FW_NORMAL;

	logFont.lfItalic = FALSE;
	logFont.lfUnderline = FALSE;
	logFont.lfStrikeOut = FALSE;
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logFont.lfQuality = ANTIALIASED_QUALITY;
	logFont.lfPitchAndFamily = DEFAULT_PITCH;
	strcpy(logFont.lfFaceName, "Arial");
	m_hFont = CreateFontIndirect(&logFont);

	HDC hdc = m_dib.GetDCHandle();
	m_hOldFont = (HFONT)SelectObject(hdc, m_hFont);
}

void CTextBar::SetTextColor(int r, int g, int b)
{
	HDC hDC = m_dib.GetDCHandle();
	::SetTextColor(hDC, RGB(r, g, b));
}

void CTextBar::GetTextExtent(const char* c_szText, SIZE* p_size)
{
	HDC hDC = m_dib.GetDCHandle();
	GetTextExtentPoint32(hDC, c_szText, strlen(c_szText), p_size);
}

#include <regex>
void CTextBar::TextOut(int ix, int iy, const char* c_szText)// @duzenleme cff leri textbarda gozukmuyor baska yerde sikinti yaratabilir ama.
{
	const auto x = std::regex_replace(c_szText, std::regex("\\|c[a-zA-Z0-9]+|\\|[r|R|H|h]"), "");
	c_szText = x.c_str();

	// if (m_isBold) {
	// 	SIZE size{ 0,0 };
	// 	GetTextExtent(c_szText, &size);
	// 	ix = (500 - size.cx) / 2;
	// }
	m_dib.TextOut(ix, iy, c_szText);
	Invalidate();
}
void CTextBar::OnCreate()
{
	m_dib.SetBkMode(TRANSPARENT);

	__SetFont(m_fontSize, m_isBold);
}

CTextBar::CTextBar(int fontSize, bool isBold) : m_hFont(NULL)
{
	m_hOldFont = NULL;
	m_fontSize = fontSize;
	m_isBold = isBold;
}

CTextBar::~CTextBar()
{
	HDC hdc = m_dib.GetDCHandle();
	SelectObject(hdc, m_hOldFont);
}