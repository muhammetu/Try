#pragma once
#include <math.h>

#define IS_SET(flag, bit)		((flag) & (bit))
#define SET_BIT(var, bit)		((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) = (var) ^ (bit))

inline float DISTANCE_SQRT(long dx, long dy)
{
	return ::sqrt((float)dx * dx + (float)dy * dy);
}

inline int DISTANCE_APPROX(int dx, int dy)
{
	int min, max;

	if (dx < 0)
		dx = -dx;

	if (dy < 0)
		dy = -dy;

	if (dx < dy)
	{
		min = dx;
		max = dy;
	}
	else
	{
		min = dy;
		max = dx;
	}

	// coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
	return (((max << 8) + (max << 3) - (max << 4) - (max << 1) +
		(min << 7) - (min << 5) + (min << 3) - (min << 1)) >> 8);
}

#ifndef __WIN32__
inline WORD MAKEWORD(BYTE a, BYTE b)
{
	return static_cast<WORD>(a) | (static_cast<WORD>(b) << 8);
}
#endif

extern void set_global_time(time_t t);
extern time_t get_global_time();

#include <string>
std::string mysql_hash_password(const char* tmp_pwd);

extern int	dice(int number, int size);
extern size_t str_lower(const char* src, char* dest, size_t dest_size);

extern void	skip_spaces(char** string);

extern void auto_argument(const char* argument, std::vector<std::string>& outArgs);
#define AUTO_ARGUMENT()                 \
	std::vector<std::string> arguments; \
	auto_argument(argument, arguments);

#define AUTO_ARGUMENT_SIZE_CONTROL(n) \
	if (arguments.size() < n) \
	{ \
		ch->ChatPacket(CHAT_TYPE_INFO, "Wrong argument count (expected min %d, given %d)", n, arguments.size()); \
		return; \
	}
#define AUTO_ARGUMENT_TEST(n) \
	for (auto &&i : arguments) \
	{ \
		ch->ChatPacket(CHAT_TYPE_INFO, "%s", i.c_str()); \
	}
#define AUTO_ARGUMENT_NUMBER(valueName, index) str_to_number(valueName, arguments.at(index).c_str());

extern const char* one_argument(const char* argument, char* first_arg, size_t first_size);
extern const char* two_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size);
extern const char* three_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size);
extern const char* four_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size, char* four_flag, size_t four_size);
extern const char* five_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size, char* four_flag, size_t four_size, char* five_flag, size_t five_size);
extern const char* six_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size, char* four_flag, size_t four_size, char* five_flag, size_t five_size, char* six_flag, size_t six_size);
extern const char* first_cmd(const char* argument, char* first_arg, size_t first_arg_size, size_t* first_arg_len_result);

extern int CalculateDuration(int iSpd, int iDur);

extern float gauss_random(float avg = 0, float sigma = 1);

extern int parse_time_str(const char* str);

extern bool WildCaseCmp(const char* w, const char* s);
extern bool is_digits(const std::string& str);

/*int find_char_position(char array[], char character)
{
	for (int i = 0; i < sizeof(array); ++i)
	{
		if (array[i] == character)
			return i;
	}

	return -1;
}

std::string replace_char_array(char array[], char character, char replace)
{
	for (int i = 0; i < sizeof(array); ++i)
	{
		if (array[i] == character)
			array[i] = replace;
	}

	return std::string(array);
}*/