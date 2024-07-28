#include "stdafx.h"
#include "locale_service.h"
#include <fstream>
#include <stdlib.h>

typedef std::map< std::string, std::string > LocaleStringMapType;

LocaleStringMapType localeString;

int g_iUseLocale = 0;

void locale_add(const char** strings)
{
	LocaleStringMapType::const_iterator iter = localeString.find(strings[0]);

	if (iter == localeString.end())
	{
		localeString.insert(std::make_pair(strings[0], strings[1]));
	}
}

const char* locale_find(const char* string)
{
	if (0 == g_iUseLocale)
		return (string);

	LocaleStringMapType::const_iterator iter = localeString.find(string);

	if (iter == localeString.end())
	{
		static char s_line[1024] = "@0949";
		strlcpy(s_line + 5, string, sizeof(s_line) - 5);

		sys_err("LOCALE_ERROR: \"%s\";", string);
		return s_line;
	}

	return iter->second.c_str();
}

const char* quote_find_end(const char* string)
{
	const char* tmp = string;
	int         quote = 0;

	while (*tmp)
	{
		if (quote && *tmp == '\\' && *(tmp + 1))
		{
			switch (*(tmp + 1))
			{
			case '"':
				tmp += 2;
				continue;
			}
		}
		else if (*tmp == '"')
		{
			quote = !quote;
		}
		else if (!quote && *tmp == ';')
			return (tmp);

		tmp++;
	}

	return (NULL);
}

char* locale_convert(const char* src, int len)
{
	const char* tmp;
	int		i, j;
	char* buf, * dest;
	int		start = 0;
	char	last_char = 0;

	if (!len)
		return NULL;

	buf = M2_NEW char[len + 1];

	for (j = i = 0, tmp = src, dest = buf; i < len; i++, tmp++)
	{
		if (*tmp == '"')
		{
			if (last_char != '\\')
				start = !start;
			else
				goto ENCODE;
		}
		else if (*tmp == ';')
		{
			if (last_char != '\\' && !start)
				break;
			else
				goto ENCODE;
		}
		else if (start)
		{
		ENCODE:
			if (*tmp == '\\' && *(tmp + 1) == 'n')
			{
				*(dest++) = '\n';
				tmp++;
				last_char = '\n';
			}
			else
			{
				*(dest++) = *tmp;
				last_char = *tmp;
			}

			j++;
		}
	}

	if (!j)
	{
		M2_DELETE_ARRAY(buf);
		return NULL;
	}

	*dest = '\0';
	return (buf);
}

#define NUM_LOCALES 2

void locale_init(const char* filename)
{
	auto printError = [](const char* fmt, ...) -> void {
		char szBuffer[400] = {};

		va_list args;

		va_start(args, fmt);
		vsnprintf(szBuffer, sizeof(szBuffer), fmt, args);
		va_end(args);

		fprintf(stderr, "%s\n", szBuffer);
	};

	auto isOutString = [](size_t pos) -> bool {
		return pos == std::string::npos;
	};

	auto isEmptyString = [&isOutString](const std::string& line) ->bool {
		return isOutString(line.find_first_not_of(" \t\r\n"));
	};

	auto getToken = [&isOutString](std::string& line) -> std::string {
		size_t first = line.find("\"");
		size_t last = line.find_last_of("\"");

		if (first == last || isOutString(first) || isOutString(last) || first == line.length() - 1)
			return "";

		first++;
		return line.substr(first, (last - first));
	};

	//initialize two empty container strings
	std::string header = "", line = "";

	std::ifstream localestringfile(filename);
	if (!localestringfile.is_open())
	{
		printError("CANNOT OPEN LOCALE_STRING FILE! [%s] -ERROR", filename);
		return;
	}

	int lineIndex = 0;
	while (std::getline(localestringfile, line))
	{
		lineIndex++;
		size_t commentIndex = line.find("///");

		if (!isOutString(commentIndex))
			line = line.substr(0, commentIndex);

		if (isEmptyString(line))
			continue;

		std::string token = getToken(line);
		if (isEmptyString(token))
		{
			printError("LOCALE STRING WRONG SYNTAX AT LINE %d - ERROR ", lineIndex);
			return;
		}

		if (header.empty())
			header = token;

		else
		{
			if (localeString.find(header) != localeString.end())
				printError("LOCALE STRING - DOUBLE HEADER FOUND. (header [%s] , line index %d) - WARNING", header.c_str(), lineIndex);

			//printError("header [%s]\nval [%s]\n",header.c_str(), token.c_str());

			localeString[header] = token;
			header = "";
		}
	}

	if (!header.empty())
		printError("LOCALE STRING : !HEADER.EMPTY (bad reading) -ERROR");

	else
		printError("LOCALE STRING : LOADED %u elements in %d lines .", localeString.size(), lineIndex);
}