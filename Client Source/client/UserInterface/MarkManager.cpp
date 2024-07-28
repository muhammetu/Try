#include "stdafx.h"
#include "MarkManager.h"

#if _MSC_VER < 1200
#include "crc32.h"
#else
#define sys_err TraceError
#define sys_log // (n, format, ...) Tracenf(format, __VA_ARGS__)
#define thecore_memcpy memcpy
#define itertype(cont) typeof((cont).begin())
#endif

CGuildMarkImage* CGuildMarkManager::__NewImage()
{
	return new CGuildMarkImage;
}

void CGuildMarkManager::__DeleteImage(CGuildMarkImage* pkImgDel)
{
	delete pkImgDel;
}

CGuildMarkManager::CGuildMarkManager()
{
#if _MSC_VER >= 1200
	_mkdir("mark");
#endif
	for (DWORD i = 0; i < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT; ++i)
		m_setFreeMarkID.insert(i);
}

CGuildMarkManager::~CGuildMarkManager()
{
	for (std::map<DWORD, CGuildMarkImage*>::iterator it = m_mapIdx_Image.begin(); it != m_mapIdx_Image.end(); ++it)
		__DeleteImage(it->second);

	m_mapIdx_Image.clear();
}

bool CGuildMarkManager::GetMarkImageFilename(DWORD imgIdx, std::string& path) const
{
	if (imgIdx >= MAX_IMAGE_COUNT)
		return false;

	char buf[64];
	snprintf(buf, sizeof(buf), "mark/%s_%lu.tga", m_pathPrefix.c_str(), imgIdx);
	path = buf;
	return true;
}

void CGuildMarkManager::SetMarkPathPrefix(const char* prefix)
{
	m_pathPrefix = prefix;
}

bool CGuildMarkManager::LoadMarkIndex()
{
	char buf[64];
	snprintf(buf, sizeof(buf), "mark/%s_index", m_pathPrefix.c_str());
	FILE* fp = fopen(buf, "r");

	if (!fp)
		return false;

	DWORD guildID;
	DWORD markID;

	char line[256];

	while (fgets(line, sizeof(line) - 1, fp))
	{
		sscanf(line, "%lu %lu", &guildID, &markID);
		line[0] = '\0';
		AddMarkIDByGuildID(guildID, markID);
	}

	LoadMarkImages();

	fclose(fp);
	return true;
}

bool CGuildMarkManager::SaveMarkIndex()
{
	char buf[64];
	snprintf(buf, sizeof(buf), "mark/%s_index", m_pathPrefix.c_str());
	FILE* fp = fopen(buf, "w");

	if (!fp)
	{
		sys_err("MarkManager::SaveMarkData: cannot open index file.");
		return false;
	}

	for (std::map<DWORD, DWORD>::iterator it = m_mapGID_MarkID.begin(); it != m_mapGID_MarkID.end(); ++it)
		fprintf(fp, "%d %d\n", it->first, it->second);

	fclose(fp);
	sys_log(0, "MarkManager::SaveMarkData: index count %d", m_mapGID_MarkID.size());
	return true;
}

void CGuildMarkManager::LoadMarkImages()
{
	bool isMarkExists[MAX_IMAGE_COUNT];
	memset(isMarkExists, 0, sizeof(isMarkExists));

	for (std::map<DWORD, DWORD>::iterator it = m_mapGID_MarkID.begin(); it != m_mapGID_MarkID.end(); ++it)
	{
		DWORD markID = it->second;

		if (markID < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
			isMarkExists[markID / CGuildMarkImage::MARK_TOTAL_COUNT] = true;
	}

	for (DWORD i = 0; i < MAX_IMAGE_COUNT; ++i)
		if (isMarkExists[i])
			__GetImage(i);
}

void CGuildMarkManager::SaveMarkImage(DWORD imgIdx)
{
	std::string path;

	if (GetMarkImageFilename(imgIdx, path))
		if (!__GetImage(imgIdx)->Save(path.c_str()))
			sys_err("%s Save failed\n", path.c_str());
}

CGuildMarkImage* CGuildMarkManager::__GetImage(DWORD imgIdx)
{
	std::map<DWORD, CGuildMarkImage*>::iterator it = m_mapIdx_Image.find(imgIdx);

	if (it == m_mapIdx_Image.end())
	{
		std::string imagePath;

		if (GetMarkImageFilename(imgIdx, imagePath))
		{
			CGuildMarkImage* pkImage = __NewImage();
			m_mapIdx_Image.insert(std::map<DWORD, CGuildMarkImage*>::value_type(imgIdx, pkImage));
			pkImage->Load(imagePath.c_str());
			return pkImage;
		}
		else
			return NULL;
	}
	else
		return it->second;
}

bool CGuildMarkManager::AddMarkIDByGuildID(DWORD guildID, DWORD markID)
{
	if (markID >= MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
		return false;

	//sys_log(0, "MarkManager: guild_id=%d mark_id=%d", guildID, markID);
	m_mapGID_MarkID.insert(std::map<DWORD, DWORD>::value_type(guildID, markID));
	m_setFreeMarkID.erase(markID);
	return true;
}

DWORD CGuildMarkManager::GetMarkID(DWORD guildID)
{
	std::map<DWORD, DWORD>::iterator it = m_mapGID_MarkID.find(guildID);

	if (it == m_mapGID_MarkID.end())
		return INVALID_MARK_ID;

	return it->second;
}

DWORD CGuildMarkManager::__AllocMarkID(DWORD guildID)
{
	std::set<DWORD>::iterator it = m_setFreeMarkID.lower_bound(0);

	if (it == m_setFreeMarkID.end())
		return INVALID_MARK_ID;

	DWORD markID = *it;

	DWORD imgIdx = markID / CGuildMarkImage::MARK_TOTAL_COUNT;
	CGuildMarkImage* pkImage = __GetImage(imgIdx);

	if (pkImage && AddMarkIDByGuildID(guildID, markID))
		return markID;

	return INVALID_MARK_ID;
}

DWORD CGuildMarkManager::GetMarkImageCount() const
{
	return m_mapIdx_Image.size();
}

DWORD CGuildMarkManager::GetMarkCount() const
{
	return m_mapGID_MarkID.size();
}

// SERVER
void CGuildMarkManager::CopyMarkIdx(char* pcBuf) const
{
	WORD* pwBuf = (WORD*)pcBuf;

	for (std::map<DWORD, DWORD>::const_iterator it = m_mapGID_MarkID.begin(); it != m_mapGID_MarkID.end(); ++it)
	{
		*(pwBuf++) = it->first; // guild id
		*(pwBuf++) = it->second; // mark id
	}
}

// SERVER
DWORD CGuildMarkManager::SaveMark(DWORD guildID, BYTE* pbMarkImage)
{
	DWORD idMark;

	if ((idMark = GetMarkID(guildID)) == INVALID_MARK_ID)
	{
		if ((idMark = __AllocMarkID(guildID)) == INVALID_MARK_ID)
		{
			sys_err("CGuildMarkManager: cannot alloc mark id %u", guildID);
			return false;
		}
		else
			sys_log(0, "SaveMark: mark id alloc %u", idMark);
	}
	else
		sys_log(0, "SaveMark: mark id found %u", idMark);

	DWORD imgIdx = (idMark / CGuildMarkImage::MARK_TOTAL_COUNT);
	CGuildMarkImage* pkImage = __GetImage(imgIdx);

	if (pkImage)
	{
		printf("imgidx %u ", imgIdx);

		std::string pathImage;
		GetMarkImageFilename(imgIdx, pathImage);
		pkImage->Save(pathImage.c_str());

		SaveMarkIndex();
	}

	return idMark;
}

// SERVER
void CGuildMarkManager::DeleteMark(DWORD guildID)
{
	std::map<DWORD, DWORD>::iterator it = m_mapGID_MarkID.find(guildID);

	if (it == m_mapGID_MarkID.end())
		return;

	CGuildMarkImage* pkImage;

	if ((pkImage = __GetImage(it->second / CGuildMarkImage::MARK_TOTAL_COUNT)) != NULL)
		pkImage->DeleteMark(it->second % CGuildMarkImage::MARK_TOTAL_COUNT);

	m_mapGID_MarkID.erase(it);
	m_setFreeMarkID.insert(it->second);

	SaveMarkIndex();
}

// SERVER
void CGuildMarkManager::GetDiffBlocks(DWORD imgIdx, const DWORD* crcList, std::map<BYTE, const SGuildMarkBlock*>& mapDiffBlocks)
{
	mapDiffBlocks.clear();

	if (m_mapIdx_Image.end() == m_mapIdx_Image.find(imgIdx))
	{
		sys_log(0, "invalid idx %u", imgIdx); // @warme668
		return;
	}

	CGuildMarkImage* p = __GetImage(imgIdx);

	if (p)
		p->GetDiffBlocks(crcList, mapDiffBlocks);
}

// CLIENT
bool CGuildMarkManager::SaveBlockFromCompressedData(DWORD imgIdx, DWORD posBlock, const BYTE* pbBlock, DWORD dwSize)
{
	CGuildMarkImage* pkImage = __GetImage(imgIdx);

	if (pkImage)
		pkImage->SaveBlockFromCompressedData(posBlock, pbBlock, dwSize);

	return false;
}

// CLIENT
bool CGuildMarkManager::GetBlockCRCList(DWORD imgIdx, DWORD* crcList)
{
	if (m_mapIdx_Image.end() == m_mapIdx_Image.find(imgIdx))
	{
		sys_log(0, "invalid idx %u", imgIdx); // @warme668
		return false;
	}

	CGuildMarkImage* p = __GetImage(imgIdx);

	if (p)
		p->GetBlockCRCList(crcList);

	return true;
}