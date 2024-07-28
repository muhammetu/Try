#include "stdafx.h"
#include "char.h"
#include "p2p.h"
#include "locale_service.h"
#include "buffer_manager.h"

#include <boost/algorithm/string.hpp>

extern void LoadASBot();
void LoadTextList();

extern void StartASBot(DWORD delay);
extern void StopASBot();

extern void StartGreetBot(DWORD delay);
extern void StopGreetBot();

void ChatByAnonymous();
void GreetByAnonymous();
bool GetAnonymous(char* name, BYTE* empire, int* reborn, int* level, const char** text);

LPEVENT m_as_event = NULL;
EVENTFUNC(as_event)
{
	ChatByAnonymous();
	return processing_time;
}

LPEVENT m_greet_event = NULL;
EVENTFUNC(greet_event)
{
	GreetByAnonymous();
	return processing_time;
}

struct {
	std::vector<std::string> textList;
	std::vector<std::string> nameList;
	int levelRange[2];
	int rebornRange[2];
} AsBotData;

void StartASBot(DWORD delay)
{
	StopASBot();

	LoadTextList();

	if (AsBotData.textList.size() == 0)
		return;
	if (AsBotData.nameList.size() == 0)
		return;

	m_as_event = event_create(as_event, NULL, delay);
}
void StopASBot()
{
	if (m_as_event)
		event_cancel(&m_as_event);
}

void StartGreetBot(DWORD delay)
{
	StopGreetBot();

	LoadTextList();

	if (AsBotData.textList.size() == 0)
		return;
	if (AsBotData.nameList.size() == 0)
		return;

	m_greet_event = event_create(greet_event, NULL, delay);
}
void StopGreetBot()
{
	if (m_greet_event)
		event_cancel(&m_greet_event);
}

#include <fstream>
void LoadASBot()
{
	char file_name[256 + 1];
	snprintf(file_name, sizeof(file_name), "%s/as_bot.txt", LocaleService_GetBasePath().c_str());

	std::ifstream fp(file_name);

	if(!fp.is_open())
		return;

	std::string	one_line;

	while (getline(fp, one_line))
	{
		if (one_line.length() < 1)
			continue;
		if (one_line.length() > 12)
			continue;

		if (one_line[0] == '#')
			continue;

		AsBotData.nameList.push_back(one_line);
	}

	fp.close();

	fprintf(stdout, "LoadASBot : LOADED %u names.", AsBotData.nameList.size());
}
void LoadTextList()
{
	char file_name[256 + 1];
	snprintf(file_name, sizeof(file_name), "%s/as_bot_text.txt", LocaleService_GetBasePath().c_str());

	std::ifstream fp(file_name);

	if(!fp.is_open())
		return;

	AsBotData.textList.clear();

	std::string	one_line;

	while (getline(fp, one_line))
	{
		if (one_line.length() < 1)
			continue;

		if (one_line[0] == '#')
			continue;

		AsBotData.textList.push_back(one_line);
	}

	fp.close();

	fprintf(stdout, "LoadASBot : LOADED %u names.", AsBotData.textList.size());
}

bool GetAnonymous(char* name, BYTE* empire, int* reborn, int* level, const char** text)
{
	static std::string lastName;
	std::string _name = random_value(AsBotData.nameList);

	boost::trim(_name);

	if (_name == lastName)
		return false;
	lastName = _name;

	if (number(1, 4) == 1)
		_name = std::to_string(number(1, 100)) + _name;
	if (number(1, 4) == 1)
		_name = _name + std::to_string(number(1, 100));

	if (number(1, 2) == 1)
		boost::to_upper(_name);
	else if (number(1, 2) == 1)
		boost::to_lower(_name);

	// *name = _name.c_str();
	strcpy(name, _name.c_str());

	if (!check_name(name))
		return false;

	int empireNum = number(1, 30);
	if (empireNum == 30)
		*empire = 1;
	else if (empireNum == 29)
		*empire = 2;
	else
		*empire = 3;
	// *empire = number(1, 3);

	*reborn = 0; //number(1, 10);
	*level = number(55, 105);

	*text = (random_value(AsBotData.textList)).c_str();

	return true;
}

void ChatByAnonymous()
{
	char name[CHARACTER_NAME_MAX_LEN];
	BYTE empire;
	int reborn;
	int level;
	const char* text;
	if (!GetAnonymous(name, &empire, &reborn, &level, &text))
		return;

	char chatbuf[CHAT_MAX_LEN + 1];

	if (g_bNewChatView)
	{
		const char* kingdoms[3] = { "[1]   ","[2]   ","[3]   " };
		if (reborn > 0)
			snprintf(chatbuf, sizeof(chatbuf), "%s%s |H%s%s|h(#)|h|r - [R-%d] Sv.%d|h|r : %s", kingdoms[empire - 1], name, "whisper:", name, reborn, level, text);
		else
			snprintf(chatbuf, sizeof(chatbuf), "%s%s |H%s%s|h(#)|h|r - Sv.%d|h|r : %s", kingdoms[empire - 1], name, "whisper:", name, level, text);
	}
	else
	{
		snprintf(chatbuf, sizeof(chatbuf), "%s : %s", name, text);
	}


	TPacketGGShout p;
	p.bHeader = HEADER_GG_SHOUT;
	p.bEmpire = empire;
	strcpy(p.szText, chatbuf);

	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShout));

	SendShout(chatbuf, empire);
}

void GreetByAnonymous()
{
	char name[CHARACTER_NAME_MAX_LEN];
	BYTE empire;
	int reborn;
	int level;
	const char* text;
	if (!GetAnonymous(name, &empire, &reborn, &level, &text))
		return;

	char chatbuf[CHAT_MAX_LEN + 1];
	snprintf(chatbuf, sizeof(chatbuf), "Vatansever oyuncusu %s hoþgeldin!", name);

	TPacketGGNotice p;
	p.bHeader = HEADER_GG_NOTICE;
	p.lSize = strlen(chatbuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(chatbuf, p.lSize);

	P2P_MANAGER::instance().Send(buf.read_peek(), buf.size()); // HEADER_GG_NOTICE

	SendNotice(chatbuf, false);
}

