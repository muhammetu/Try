#ifndef __INC_METIN_II_GUILDLIB_MARK_MANAGER_H__
#define __INC_METIN_II_GUILDLIB_MARK_MANAGER_H__

#include "MarkImage.h"

class CGuildMarkManager : public singleton<CGuildMarkManager>
{
public:
	enum EMaxImageCount
	{
		MAX_IMAGE_COUNT = 5,
		INVALID_MARK_ID = 0xffffffff,
	};

	CGuildMarkManager();
	virtual ~CGuildMarkManager();

	//
	// Mark
	//
	void SetMarkPathPrefix(const char* prefix);

	bool LoadMarkIndex();
	bool SaveMarkIndex();

	void LoadMarkImages();
	void SaveMarkImage(DWORD imgIdx);

	bool GetMarkImageFilename(DWORD imgIdx, std::string& path) const;
	bool AddMarkIDByGuildID(DWORD guildID, DWORD markID);
	DWORD GetMarkImageCount() const;
	DWORD GetMarkCount() const;
	DWORD GetMarkID(DWORD guildID);

	// SERVER
	void CopyMarkIdx(char* pcBuf) const;
	DWORD SaveMark(DWORD guildID, BYTE* pbMarkImage);
	void DeleteMark(DWORD guildID);
	void GetDiffBlocks(DWORD imgIdx, const DWORD* crcList, std::map<BYTE, const SGuildMarkBlock*>& mapDiffBlocks);

	// CLIENT
	bool SaveBlockFromCompressedData(DWORD imgIdx, DWORD idBlock, const BYTE* pbBlock, DWORD dwSize);
	bool GetBlockCRCList(DWORD imgIdx, DWORD* crcList);

private:
	//
	// Mark
	//
	CGuildMarkImage* __NewImage();
	void __DeleteImage(CGuildMarkImage* pkImgDel);

	DWORD __AllocMarkID(DWORD guildID);

	CGuildMarkImage* __GetImage(DWORD imgIdx);

	std::map<DWORD, CGuildMarkImage*> m_mapIdx_Image; // index = image index
	std::map<DWORD, DWORD> m_mapGID_MarkID; // index = guild id

	std::set<DWORD> m_setFreeMarkID;
	std::string		m_pathPrefix;

private:
};

#endif
