#include <string>
//////////////////////////////////////////////////////////////////// CONFIG VARIABLES ////////////////////////////////////////////////////////

//						Maximum number of steps you can undo
const static int		MAX_UNDO_STEPS = 20; 

//						The number of aliases typically shown on the line a character's turn is shown on.
const static int		ALIASES_SHOWN = 2;
//						The 'info' command will still show all of a character's aliases, and simple display mode will only show their first name (outside of 'info')

//						Determines whether or not it shows a character's info when no other info is displayed.
const static bool		SHOW_INFO_EACH_TURN = true;

//						How long the program forces the user to wait on a complex character's turn before it lets the turn advance.
const static long 
double					SECONDS_WAITED = 2.5;

//						Determines if the "load" command should change the working directory to whatever the directory of the given file is.
const static bool		LOAD_CHANGES_WORKING_DIRECTORY = false;
//						This behavior only functions during initial character entry - not while running combat, even if this setting is enabled.


//						Used by certain display code to cap the number of characters printed horizontally
const static int		CONSOLE_WIDTH = 112;


//						When files are loaded it can be configured to read out what's happening as it does it in real time, or that feature can be disabled for faster loading.
const static bool		DISPLAY_INFO_FROM_LOADED_FILES = false;


const static
std::string				BASE_DIRECTORY_PROXY = ".../:::"; //Directories starting with this will start at the program's own directory

const static bool		WRITE_LOGS_TO_FILE = false; //Whether or not to write the ongoing log to a file.

const static
std::string				INITIAL_DIRECTORY = ""; //In case you want it to start someplace other than the program's own directory. The program's directory is still the "base" and the directory accessed with the base directory proxy.
//Resetting the directory still resets it to the  normal base directory, regardless of the initial directory.

/*
This may be some of the worst code I've ever written. 
Originally it was intended to sort initiatives and nothing else, but over time it's become a tool that handles more and more, far beyond the original scope of what
it was intended to do.
At first it started small - just tracking turns and hit points. It was only a couple commands, no need to write a sophisticated and robust system to manage them right? 
Just a quick'n'dirty hack to add a couple small features. And for a short term micro-project, comments would just be a waste of time.
But after one more week I realized I needed just a few more features to make it more useful, so I added a just few commands to rearrange initiatives.
It was then that I should have overhauled the code...but surely this time it would be the last change.

That was almost two years ago. Time and time again, I kept adding "just a little more", hack after hack, week by week, slowly growing it like a tumor.
Now it's far too late.
To rewrite it now is more work than just adding slightly to the festering heap every time, forever dooming the code to its existence as a museum of lazy and
ill-advised practices.

So beware, reader - only suffering lies ahead. Continue if you dare...

*/



#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <random>
#include <fstream>
#include <climits>
#include <thread>
#include <chrono>
#include <time.h>
#include <map>
#include <filesystem>

std::string wd = INITIAL_DIRECTORY; //Working directory to load files from

int save_dc = 10;

const static bool PRINT_DEBUG = false;
typedef size_t index_t;
static int initial_round = 1; //Global variables are a bad practice
static std::string initial_turn = ""; //Stores name of character whose turn will start
void trim(std::string& str);
bool simple_display = true; //Controls whether or not simple display mode is enabled.
static std::string execution_dir = "";

std::string get_lowercase(std::string str);

std::string replace_first(const std::string& original, const std::string& original_substring, const std::string& new_substring)
{
	size_t index = original.find(original_substring);
	if (index != std::string::npos)
	{
		std::string pre = original.substr(0, index);
		std::string post = "";
		if (index + original_substring.length() < original.length())
			post = original.substr(index + original_substring.length());

		return pre + new_substring + post;
	}
	else
	{
		return original;
	}
}

std::string replace_first(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool case_sensitive)
{
	if (case_sensitive)
	{
		return replace_first(original, original_substring, new_substring);
	}
	else
	{
		std::string lowercase_original = get_lowercase(original);
		std::string lowercase_original_substring = get_lowercase(original_substring);
		size_t index = lowercase_original.find(lowercase_original_substring);

		if (index != std::string::npos)
		{
			std::string prestring = original.substr(0, index);
			std::string post = "";
			if (index + original_substring.length() < original.length())
				post = original.substr(index + original_substring.length());

			return prestring + new_substring + post;
		}
		else
		{
			return original;
		}
	}
}

std::string replace_first(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool whole_word_only, bool case_sensitive)
{
	if (!whole_word_only)
	{
		if (case_sensitive)
		{
			return replace_first(original, original_substring, new_substring);
		}
		else
		{
			std::string lowercase_original = get_lowercase(original);
			std::string lowercase_original_substring = get_lowercase(original_substring);
			size_t index = lowercase_original.find(lowercase_original_substring);

			if (index != std::string::npos)
			{
				std::string prestring = original.substr(0, index);
				std::string post = "";
				if (index + original_substring.length() < original.length())
					post = original.substr(index + original_substring.length());

				return prestring + new_substring + post;
			}
			else
			{
				return original;
			}
		}
	}
	else
	{
		if (case_sensitive)
		{
			if (whole_word_only)
			{
				std::string returned = " " + original + " ";
				std::string substring = " " + original_substring + " ";
				size_t index = returned.find(substring);
				if (index != std::string::npos)
				{
					std::string pre = returned.substr(0, index);
					std::string post = "";
					if (index + substring.length() < returned.length())
						post = returned.substr(index + substring.length());

					returned = pre + " " + new_substring + " " + post;

					index = returned.find(substring);
				}
				if (returned[returned.size() - 1] == ' ')
					returned.resize(returned.size() - 1);
				if (returned[0] == ' ')
					returned = returned.substr(1);
				return returned;
			}
			else
			{
				std::string returned = original;
				size_t index = returned.find(original_substring);
				if (index != std::string::npos)
				{
					std::string pre = returned.substr(0, index);
					std::string post = "";
					if (index + original_substring.length() < returned.length())
						post = returned.substr(index + original_substring.length());

					returned = pre + new_substring + post;
					index = returned.find(original_substring);
				}
				return returned;
			}
		}
		else
		{
			std::string cur = original;
			if (whole_word_only)
				cur = " " + cur + " ";
			std::string lowercase_original_substring = get_lowercase(original_substring);
			if (whole_word_only)
				lowercase_original_substring = " " + lowercase_original_substring + " ";

			if (whole_word_only)
			{
				if (get_lowercase(cur).find(lowercase_original_substring) != std::string::npos)
				{
					cur = replace_first(cur, " " + original_substring + " ", " " + new_substring + " ", false);
				}


				cur = cur.substr(1);
				cur.resize(cur.size() - 1);
			}
			else
			{
				if (get_lowercase(cur).find(lowercase_original_substring) != std::string::npos)
					cur = replace_first(cur, original_substring, new_substring, false);
			}

			return cur;
		}
	}
	
}


std::string replace_all(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool whole_word_only)
{
	if (whole_word_only)
	{
		std::string returned = " " + original + " ";
		std::string substring = " " + original_substring + " ";
		size_t index = returned.find(substring);
		while (index != std::string::npos)
		{
			std::string pre = returned.substr(0, index);
			std::string post = "";
			if (index + substring.length() < returned.length())
				post = returned.substr(index + substring.length());

			returned = pre + " " + new_substring + " " + post;

			index = returned.find(substring);
		}
		if(returned[returned.size()-1]==' ')
			returned.resize(returned.size() - 1);
		if(returned[0]==' ')
			returned = returned.substr(1);
		return returned;
	}
	else
	{
		std::string returned = original;
		size_t index = returned.find(original_substring);
		while (index != std::string::npos)
		{
			std::string pre = returned.substr(0, index);
			std::string post = "";
			if (index + original_substring.length() < returned.length())
				post = returned.substr(index + original_substring.length());

			returned = pre + new_substring + post;
			index = returned.find(original_substring);
		}
		return returned;
	}
}

std::string replace_all(const std::string& original, const std::string& original_substring, const std::string& new_substring, bool whole_word_only, bool case_sensitive)
{
	if (case_sensitive)
	{
		return replace_all(original, original_substring, new_substring, whole_word_only);
	}
	else
	{
		std::string cur = original;
		if (whole_word_only)
			cur = " " + cur + " ";
		std::string lowercase_original_substring = get_lowercase(original_substring);
		if (whole_word_only)
			lowercase_original_substring = " " + lowercase_original_substring + " ";

		if (whole_word_only)
		{
			while (get_lowercase(cur).find(lowercase_original_substring) != std::string::npos)
			{
				cur = replace_first(cur, " " + original_substring + " ", " " + new_substring + " ", false);
			}


			cur = cur.substr(1);
			cur.resize(cur.size() - 1);
		}
		else
		{
			while (get_lowercase(cur).find(lowercase_original_substring) != std::string::npos)
				cur = replace_first(cur, original_substring, new_substring, false);
		}

		return cur;
	}
}

inline bool comp_substring(const std::string& first, const std::string& second, size_t chars_to_compare) {
	for (index_t i = 0; i < chars_to_compare; ++i) {
		if (first[i] != second[i])
			return false;
	}
	return true;
}

inline bool comp_substring_not_case_sensitive(const std::string& first, const std::string& second, size_t chars_to_compare) {
	for (index_t i = 0; i < chars_to_compare; ++i) {
		if (std::tolower(first[i]) != std::tolower(second[i]))
			return false;
	}
	return true;
}

inline void trim(std::string& str)
{
	size_t leading_spaces, trailing_spaces;
	for (leading_spaces = 0; leading_spaces < str.length() && str[leading_spaces] == ' '; ++leading_spaces) {}
	if (leading_spaces > 0)
		str = str.substr(leading_spaces, str.length() - leading_spaces);

	if (str.length() == 0)
		return;

	for (trailing_spaces = 0; str[str.length() - 1 - trailing_spaces] == ' ' && trailing_spaces < str.length(); ++trailing_spaces) {}
	str = str.substr(0, str.length() - trailing_spaces);


	auto double_spaces = str.find("  ");
	while (double_spaces < str.length())
	{
		str = str.substr(0, double_spaces) + str.substr(double_spaces + 1);
		double_spaces = str.find("  ");
	}
}

//Lazy but cross-platform way to "clear" the screen
inline void clear()
{
	for (int i = 0; i < 100; ++i)
		std::cout << std::endl;
}


inline bool starts_with(const std::string& base, const std::string& beginning)
{
	return comp_substring_not_case_sensitive(base, beginning, beginning.size());;
}

inline bool ends_with(const std::string& base, const std::string& ending)
{
	if (base.size() < ending.size())
		return false;
	else 
	{
		int base_i = base.size() - 1;
		for (int ending_i = ending.size() - 1; ending_i  >= 0; --ending_i)
		{
			if (base[base_i] != ending[ending_i])
				return false;
			--base_i;
		}
		return true;
	}
}

std::string get_directory(std::string filename)
{
	if (starts_with(filename, BASE_DIRECTORY_PROXY + "/"))
		filename = filename.substr(BASE_DIRECTORY_PROXY.size()+1);

	if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
	{
		size_t backi = filename.size() - 1;
		while (filename[backi] != '/' && filename[backi] != '\\')
		{
			--backi;
		}
		std::string directory = filename;
		directory.resize(backi);
		return directory;
	}
	else
		return wd;
}

bool is_absolute_directory(const std::string& path)
{
	if (starts_with(path, "C:\\") || starts_with(path, "C:/") || starts_with(path, "/C:/") || starts_with(path, "/home/") || starts_with(path, "/usr/") || starts_with(path, "\\home\\") || starts_with(path, "\\usr\\"))
		return true;
	return false;
}

//Makes an existing string lowercase in-place
inline void make_lowercase(std::string& str)
{
	for (index_t i = 0; i < str.length(); ++i)
		str[i] = std::tolower(str[i]);
}

//Returns a copy of a string that's made to be lowercase, while preserving the original
std::string get_lowercase(std::string str) //Pass by value creates a copy that I can make lowercase before returning
{
	make_lowercase(str);
	return str;
}
class creature;
bool name_is_unique(const std::string& name, const std::list<creature>& creatures);

//Who needs enums when you have ints?
const static int SHOW_ONE_NAME = 0;
const static int SHOW_SOME_NAMES = 1;
const static int SHOW_ALL_NAMES = 2;
int get_number_arg(std::string dummy_line, bool& is_signed, std::list<creature>& creatures, creature* executor);

inline void ls(std::string dir, std::string& turn_msg, bool recursive, bool linked, bool override_error, int depth, bool override_add_path, std::string info, std::vector<std::string>& shown_dirs)
{
	for (int i = 0; i < dir.size(); ++i)
	{
		if (dir[i] == '\\')
			dir[i] = '/';
	}
	char pipe_char = '|';
	char line_char = '-';
	std::string blank_pipe = "    ";
	blank_pipe[0] = pipe_char;
	std::string line = "    ";
	line[0] = pipe_char;
	std::string empty = "    ";
	for (int i = 1; i < line.size(); ++i)
	{
		line[i] = line_char;
	}
	try {
		for (int i = 0; i < execution_dir.size(); ++i)
		{
			if (execution_dir[i] == '\\')
				execution_dir[i] = '/';
		}
		if (!override_add_path && !override_error && !is_absolute_directory(dir))
		{
			dir = execution_dir + "/" + dir;
		}

		if (!override_error)
		{
			for (int i = 0; i < depth; ++i)
			{
				if (i == depth - 1)
				{
					std::cout << line;
					turn_msg += line;
				}
				else
				{
					std::cout << blank_pipe;
					turn_msg += blank_pipe;
				}
			}
			std::cout << dir << std::endl;
			turn_msg += dir + "\n";
			++depth;
		}
		bool has_shown_linked_entries = false;
		for (const auto& entry : std::filesystem::directory_iterator(dir))
		{
			std::string p = entry.path().string();
			for (int i = 0; i < p.size(); ++i)
			{
				if (p[i] == '\\')
					p[i] = '/';
			}
			for (int i = 0; i < depth; ++i)
			{
				if (i == depth - 1)
				{
					std::cout << line;
					turn_msg += line;
				}
				else
				{
					std::cout << blank_pipe;
					turn_msg += blank_pipe;
				}
			}
			
			std::string printed = p.substr(dir.size());
			if (printed[0] == '/')
				printed = printed.substr(1);

			std::cout << printed << std::endl;
			turn_msg += printed + "\n";
			
			std::string localf = p;
			std::string localdir = get_directory(p);
			localf = localf.substr(localdir.size());
			if (recursive)
			{
				ls(p, turn_msg, recursive, linked,true, depth+1,false,"",shown_dirs);
			}
			if (localf == "/LINKS" || localf == "/LINKS.txt" && !has_shown_linked_entries && linked)
			{
				std::ifstream links;
				links.open(p);
				if (links.is_open())
				{
					has_shown_linked_entries = true;
					bool show_errs = false;
					while (links.good() && !links.eof())
					{
						std::string line;
						std::getline(links, line);
						std::string errorline = line;
						if (starts_with(line, BASE_DIRECTORY_PROXY + "/"))
						{
							line = line.substr(BASE_DIRECTORY_PROXY.size() + 1);
							//ls(line, turn_msg, recursive, linked, !show_errs, depth + 1, true, errorline);
						}
						else
						{
							if (!is_absolute_directory(line))
							{
								line = localdir + "/" + line;
							}
							
							ls(line, turn_msg, recursive, linked, !show_errs, depth+1,true,errorline,shown_dirs);
						}
					}
				}
			}
		}
		if (!override_error)
		{
			turn_msg += "\n";
			std::cout << std::endl;
		}
	}
	catch (const std::exception& E)
	{
		if (!override_error)
		{
			for (int i = 0; i < depth; ++i)
			{
				if (i == depth - 1)
				{
					std::cout << line;
					turn_msg += line;
				}
				else
				{
					std::cout << blank_pipe;
					turn_msg += blank_pipe;
				}
			}
			std::cout << "Invalid directory: " << execution_dir << "/" << dir << std::endl;
			turn_msg += "Invalid directory: " + execution_dir + "/" + dir + "\n";
			std::cout << "Errorline: " << info << std::endl;
		}
		
	}
}

inline void ls(std::string dir, std::string& turn_msg, bool recursive, bool linked)
{
	std::vector<std::string> shown_dirs;
	ls(dir, turn_msg, recursive, linked, false,0,false,"",shown_dirs);
}
 
class creature
{
	int initiative, modifier, hp, max_hp, turn_count, temp_hp, regen, ac=-1;
	
	std::string name, reminder, note;
public:
	bool temp_disable_regen = false;
	std::string turn_start_file = "";
	std::string turn_end_file = "";
	std::list<std::string> flags;
	std::map<std::string, bool> flags_hidden;
	std::list<std::string> aliases;
	std::vector<std::string> initial_flags;
	std::map<std::string, int> variables;
	std::vector<std::string> recharge1; //Recharge 0 just removes it from any recharge list.
	std::vector<std::string> recharge2;
	std::vector<std::string> recharge3;
	std::vector<std::string> recharge4;
	std::vector<std::string> recharge5;
	std::vector<std::string> recharge6;

	bool has_con = false;

	int str = 0, dex = 0, con = 0, intelligence = 0, wis = 0, cha = 0;

	inline void reset_flags()
	{
		flags.clear();
		flags_hidden.clear();
		for (int i = 0; i < initial_flags.size(); ++i)
		{
			add_flag(initial_flags[i], true);
		}

		auto recharge = [&](std::vector<std::string>& recharger)
			{
				for (int i = 0; i < recharger.size(); ++i)
				{
					add_flag(recharger[i], true);
				}
			};

		recharge(recharge1);
		recharge(recharge2);
		recharge(recharge3);
		recharge(recharge4);
		recharge(recharge5);
		recharge(recharge6);
	}

	inline void remove_recharge(const std::string& flag_name)
	{
		auto found = std::find(recharge1.begin(), recharge1.end(), flag_name);
		if (found != recharge1.end())
		{
			recharge1.erase(found);
		}

		found = std::find(recharge2.begin(), recharge2.end(), flag_name);
		if (found != recharge2.end())
		{
			recharge2.erase(found);
		}

		found = std::find(recharge3.begin(), recharge3.end(), flag_name);
		if (found != recharge3.end())
		{
			recharge3.erase(found);
		}

		found = std::find(recharge4.begin(), recharge4.end(), flag_name);
		if (found != recharge4.end())
		{
			recharge4.erase(found);
		}

		found = std::find(recharge5.begin(), recharge5.end(), flag_name);
		if (found != recharge5.end())
		{
			recharge5.erase(found);
		}

		found = std::find(recharge6.begin(), recharge6.end(), flag_name);
		if (found != recharge6.end())
		{
			recharge6.erase(found);
		}
	}

	inline void add_recharge(int frequency, const std::string& flag_name)
	{
		remove_recharge(flag_name);
		switch (frequency)
		{
			case 1: { std::vector<std::string>& recharger = recharge1; if (std::find(recharger.begin(), recharger.end(), flag_name) == recharger.end()) { recharger.push_back(flag_name); } break; }
			case 2: { std::vector<std::string>& recharger = recharge2; if (std::find(recharger.begin(), recharger.end(), flag_name) == recharger.end()) { recharger.push_back(flag_name); } break; }
			case 3: { std::vector<std::string>& recharger = recharge3; if (std::find(recharger.begin(), recharger.end(), flag_name) == recharger.end()) { recharger.push_back(flag_name); } break; }
			case 4: { std::vector<std::string>& recharger = recharge4; if (std::find(recharger.begin(), recharger.end(), flag_name) == recharger.end()) { recharger.push_back(flag_name); } break; }
			case 5: { std::vector<std::string>& recharger = recharge5; if (std::find(recharger.begin(), recharger.end(), flag_name) == recharger.end()) { recharger.push_back(flag_name); } break; }
			case 6: { std::vector<std::string>& recharger = recharge6; if (std::find(recharger.begin(), recharger.end(), flag_name) == recharger.end()) { recharger.push_back(flag_name); } break; }
			default: return;
		}
	}

	inline void poll_recharges()
	{
		auto d6 = [&]() -> int {return 1 + (rand() % 6);};

		auto poll_recharger = [&](std::vector<std::string>& recharger, int odds) {
			for (auto i = recharger.begin(); i != recharger.end(); ++i)
			{
				const std::string& flag_name = *i;
				int roll = d6();
				//std::cout << "RECHARGE ROLL=" << roll << std::endl;
				if (roll >= odds)
				{
					if (!has_flag(flag_name))
					{
						add_flag(flag_name, true);
					}
				}
			}
		};

		poll_recharger(recharge1, 1);
		poll_recharger(recharge2, 2);
		poll_recharger(recharge3, 3);
		poll_recharger(recharge4, 4);
		poll_recharger(recharge5, 5);
		poll_recharger(recharge6, 6);
	}

	bool touched = false;
	bool entered_dex = false;
	inline const std::list<std::string>& get_flags() const
	{
		return flags;
	}

	creature* get_raw_ptr()
	{
		return this;
	}

	bool flag_is_hidden(const std::string& flag_name) const
	{
		if (flags_hidden.count(flag_name) == 0)
		{
			return false;
		}
		else
		{
			return flags_hidden.at(flag_name);
		}
	}

	int get_var(std::string var_name, std::list<creature>& creatures)
	{
		make_lowercase(var_name);
		trim(var_name);
		if (var_name.size() > 2 && var_name[0] == '[' && var_name[var_name.size() - 1] == ']' && var_name != "[ac]" && var_name != "[hp]" && var_name!="[max_hp]")
		{
			var_name = var_name.substr(1);
			var_name.resize(var_name.size() - 1);
			std::string dummy_line = "parse " + var_name;
			bool is_signed = false;
			int num_var = get_number_arg(dummy_line, is_signed, creatures, this);
			var_name = std::to_string(num_var);
		}

		if (var_name == "ac")
		{
			return ac;
		}
		else if (var_name == "hp")
		{
			if (max_hp >= 1)
				return hp;
			else
				return 0;
		}
		else if (var_name == "max_hp")
		{
			if (max_hp < 1)
				return 0;
			else
				return max_hp;
		}
		else if (var_name == "con")
		{
			return con;
		}
		else if (var_name == "int")
		{
			return intelligence;
		}
		else if (var_name == "str")
		{
			return str;
		}
		else if (var_name == "dex")
		{
			return dex;
		}
		else if (var_name == "wis")
			return wis;
		else if (var_name == "cha")
			return cha;
		else if (var_name == "#ac")
		{
			return ac;
		}
		else if (var_name == "#hp")
		{
			if (max_hp >= 1)
				return hp;
			else
				return 0;
		}
		else if (var_name == "#max_hp")
		{
			if (max_hp < 1)
				return 0;
			else
				return max_hp;
		}
		else if (var_name == "#con")
		{
			return con;
		}
		else if (var_name == "#int")
		{
			return intelligence;
		}
		else if (var_name == "#str")
		{
			return str;
		}
		else if (var_name == "#dex")
		{
			return dex;
		}
		else if (var_name == "#wis")
			return wis;
		else if (var_name == "#cha")
			return cha;
		else if (var_name == "#regen" || var_name == "regen")
			return regen;
		else if (variables.count(var_name) == 0)
		{
			if (variables.count("#" + var_name) == 0)
			{
				return 0;
			}
			else
			{
				return variables.at("#" + var_name);
			}
		}
		else
		{
			return variables.at(var_name);
		}
	}

	int get_name_digits()
	{
		int val = 0;
		int index = name.size() - 1;
		auto is_digit = [](char c) -> bool
			{
				if (c == '0')
					return true;
				else if (c == '1')
					return true;
				else if (c == '2')
					return true;
				else if (c == '3')
					return true;
				else if (c == '4')
					return true;
				else if (c == '5')
					return true;
				else if (c == '6')
					return true;
				else if (c == '7')
					return true;
				else if (c == '8')
					return true;
				else if (c == '9')
					return true;
				return false;
			};

		auto char_to_digit = [](char c) -> int
			{
				if (c == '0')
					return 0;
				else if (c == '1')
					return 1;
				else if (c == '2')
					return 2;
				else if (c == '3')
					return 3;
				else if (c == '4')
					return 4;
				else if (c == '5')
					return 5;
				else if (c == '6')
					return 6;
				else if (c == '7')
					return 7;
				else if (c == '8')
					return 8;
				else if (c == '9')
					return 9;
				return 0;
			};
		int power = 1;
		while (index >= 0)
		{
			char c = name[index];
			if (is_digit(c))
			{
				val += power * char_to_digit(c);
				power *= 10;
			}
			--index;
		}
		return val;
	}

	void set_var(std::string var_name, int value)
	{
		make_lowercase(var_name);
		trim(var_name);
		if (var_name == "ac" || var_name == "#ac")
		{
			ac = value;
			if (ac < 0)
				ac = 0;
		}
		else if (var_name == "hp" || var_name == "#hp")
		{
			if (value > 0)
			{
				hp = value;
				if (max_hp < 1)
					max_hp = value;

				if (hp > max_hp)
					hp = max_hp;

				if (hp < 0 && max_hp >= 1)
					hp = 0;
			}
			else
			{
				if (max_hp >= 1)
				{
					hp = 0;
				}
			}
		}
		else if (var_name == "max_hp" || var_name == "#max_hp")
		{
			if (value < 1)
			{
				max_hp = -1;
				hp = -1;
			}
			else
			{
				max_hp = value;
				if (hp > max_hp)
					hp = max_hp;
			}
		}
		else if (var_name == "str" || var_name == "#str")
			str = value;
		else if (var_name == "dex" || var_name == "#dex")
		{
			dex = value;
			entered_dex = true;
		}
		else if (var_name == "con" || var_name == "#con")
		{
			con = value;
			has_con = true;
		}
		else if (var_name == "int" || var_name == "#int")
			intelligence = value;
		else if (var_name == "wis" || var_name == "#wis")
			wis = value;
		else if (var_name == "cha" || var_name == "#cha")
			cha = value;
		else if (var_name == "regen" || var_name == "#regen")
			regen = value;
		else
		{
			if (variables.count(var_name) != 0)
			{
				variables[var_name] = value;
			}
			else
			{
				if (variables.count("#" + var_name) == 0)
				{
					variables.emplace(var_name, value);
				}
				else
				{
					variables["#" + var_name] = value;
				}
			}
		}	
	}

	void remove_var(std::string var_name)
	{
		make_lowercase(var_name);
		if (var_name == "hp" || var_name == "mxa_hp")
		{
			max_hp = -1;
			hp = -1;
		}
		else if (var_name == "ac" || var_name=="#ac")
		{
			ac = 0;
		}
		else if (var_name == "str" || var_name == "#str")
		{
			str = 0;
		}
		else if (var_name == "dex" || var_name == "#dex")
		{
			dex = modifier;
			entered_dex = false;
		}
		else if (var_name == "con" || var_name == "#con")
		{
			has_con = false;
			con = 0;
		}
		else if (var_name == "int" || var_name == "#int")
		{
			intelligence = 0;
		}
		else if (var_name == "wis" || var_name == "#wis")
		{
			wis = 0;
		}
		else if (var_name == "cha" || var_name == "#cha")
		{
			cha = 0;
		}
		else if (var_name == "regen" || var_name == "#regen")
		{
			regen = 0;
			temp_disable_regen = false;
		}
		else if (variables.count(var_name) != 0)
		{
			variables.erase(var_name);
		}
		else if (variables.count("#" + var_name) != 0)
		{
			variables.erase("#" + var_name);
		}
	}

	void hide_var(std::string var_name, std::list<creature>& creatures)
	{
		trim(var_name);
		if (get_lowercase(var_name) == "ac")
		{
			ac = 0;
		}
		else if (has_var(var_name) && var_name.size() > 0 && (var_name[0] != '#'))
		{
			int val = get_var(var_name, creatures);
			remove_var(var_name);
			set_var("#" + var_name, val);
		}
	}

	void show_var(std::string var_name, std::list<creature>& creatures)
	{
		if (has_var(var_name))
		{
			int val = get_var(var_name, creatures);
			remove_var(var_name);
			while(var_name[0] == '#' && var_name.size()>1)
				var_name = var_name.substr(1);
			set_var(var_name, val);
		}

	}

	bool has_var(const std::string& var_name) const
	{
		if (var_name.size() == 0)
			return false;
		else if (variables.count(var_name) != 0)
			return true;
		else if (variables.count("#" + var_name) != 0)
			return true;
		else
			return false;
	}

	inline bool is_concentrating()
	{
		return has_flag("concentrating") || has_flag("concentration") || has_flag("conc") || has_flag("con") || has_flag("c");
	}

	inline bool has_con_bonus()
	{
		return has_con;
	}

	inline int get_con_bonus(std::list<creature>& creatures)
	{
		return con;
	}

	inline int get_str_bonus()
	{
		return str;
	}

	inline int get_dex_bonus()
	{
		if (!entered_dex)
			return modifier;
		else
			return dex;
	}

	inline int get_wis_bonus()
	{
		return wis;
	}

	inline int get_int_bonus()
	{
		return intelligence;
	}

	inline int get_cha_bonus()
	{
		return cha;
	}

	inline void set_reminder(const std::string& new_reminder)
	{
		reminder = new_reminder;
	}

	inline void set_note(const std::string& new_note)
	{
		note = new_note;
	}

	inline const std::string get_reminder(bool display_value) const
	{
		if (display_value)
		{;
			std::string replaced = replace_all(reminder, "\\n", "\n\t\t  ", false);
			return replaced;
		}
		else
		{
			return reminder;
		}
	}

	inline const std::string get_note(bool display_value) const
	{
		if (display_value)
		{
			std::string replaced = replace_all(note, "\\n", "\n\t\t", false);
			return replaced;
		}
		else
		{
			return note;
		}
	}
	
	inline void add_alias(const std::string& new_alias)
	{
		if(std::find(aliases.begin(),aliases.end(), new_alias) == aliases.end())
			aliases.push_back(new_alias);
	}

	inline bool remove_alias(const std::string& al)
	{
		bool has_a = false;
		if (al.size() == 0)
			return false;
		for (auto i = aliases.begin(); i != aliases.end(); ++i)
		{
			if ((*i) == al)
			{
				has_a = true;
				break;
			}
		}
		if (!has_a)
			return false;
		aliases.remove(al);
		if (!has_alias("@all"))
		{
			add_alias("@all");
		}
		return true;
	}

	std::list<std::string> get_all_names() const
	{
		std::list<std::string> all_names = aliases;
		all_names.push_back(get_lowercase(name));
		return all_names;
	}

	inline int get_ac() const
	{
		return ac;
	}

	inline void set_ac(int new_ac)
	{
		ac = new_ac;
		if (ac < 1)
			ac = -1;
	}

	inline std::string get_display_names(int display_level) const
	{
		if (display_level == SHOW_ONE_NAME)
			return name;
		std::string disp = name;
		int count = 0;
		int MAX_ALIASES = ALIASES_SHOWN;
		if (display_level == SHOW_ALL_NAMES)
			MAX_ALIASES = 100;
		int visible_aliases = 0;
		for (auto i = aliases.begin(); i != aliases.end(); ++i)
		{
			//disp += "[" + (*i) + "]";
			if (!((*i)[0] == '@'))
			{
				if (count < MAX_ALIASES)
				{
					disp += "/" + (*i);
					++count;
				}
				++visible_aliases;
			}
		}
		
		if (visible_aliases > MAX_ALIASES)
		{
			disp += ".../";
		}
		return disp;
	}

	inline std::string get_display_names() const
	{
		if (simple_display)
			return get_display_names(SHOW_ONE_NAME);
		else
			return get_display_names(SHOW_SOME_NAMES);
	}

	inline std::string get_flag_list(bool is_my_turn, bool say_if_hidden, bool show_hidden_flags, bool show_hidden_flags_as_parenthises) const
	{
		std::string list;
		for (auto i = flags.begin(); i != flags.end(); ++i)
		{
			std::string base_flag_name = *i;
			std::string flag_name = *i;
			if (say_if_hidden && flag_is_hidden(flag_name))
			{
				if (show_hidden_flags_as_parenthises)
					flag_name = "(" + flag_name + ")";
				else
					flag_name = "#" + flag_name;
			}
			if (!is_my_turn)
			{
				if (!(flag_is_hidden(base_flag_name) && !show_hidden_flags))
				{
					list += flag_name;
					list += ",";
				}
			}
			else if ((*i)[0] != '_')
			{
				if (!(flag_is_hidden(base_flag_name) && !show_hidden_flags))
				{
					list += flag_name;
					list += ",";
				}
			}
		}
		if (list.length() != 0)
		{
			list.resize(list.size() - 1); //Remove characters appended at the end
		}

		return list;
	}

	inline bool has_alias(const std::string& alias) const
	{
		std::string lowercalias = get_lowercase(alias);
		for (auto i = aliases.begin(); i != aliases.end(); ++i)
		{
			if (get_lowercase(*i) == lowercalias)
			{
				return true;
			}
		}
		if (get_lowercase(name) == lowercalias)
			return true;
		return false;
	}

	inline void add_flag(const std::string& flag_name, bool disable_keyword_flags)
	{
		std::string lc = get_lowercase(flag_name);
		if (disable_keyword_flags && (

			lc == "current" 
			|| lc == "executor" 
			|| lc == "all" 

			|| lc == "#current" 
			|| lc == "#executor" 
			|| lc == "#all"
			
			))
			return;
		std::string new_flag = flag_name;
		bool hidden = false;
		if (new_flag.size() >= 1 && new_flag[0] == '#')
		{
			hidden = true;
			new_flag = new_flag.substr(1);
		}
		else if (new_flag.size() >= 2 && (new_flag[0] == '#' && new_flag[1] == '_') || (new_flag[0] == '_' && new_flag[1] == '#'))
		{
			new_flag[0] = ' ';
			new_flag[1] = ' ';
			trim(new_flag);
			new_flag = "_" + new_flag;
			hidden = true;
		}
		if (new_flag.length() != 0 && std::find(flags.begin(), flags.end(), new_flag)==flags.end())
		{
			flags.push_back(new_flag);
			if (hidden)
			{
				if (flags_hidden.count(new_flag) != 0)
				{
					flags_hidden[new_flag] = true;
				}
				else
				{
					flags_hidden.emplace(new_flag, true);
				}
			}
			else
			{
				if (flags_hidden.count(new_flag) != 0)
				{
					flags_hidden[new_flag] = false;
				}
				else
				{
					flags_hidden.emplace(new_flag, false);
				}
			}
			add_alias("@" + get_lowercase(new_flag));
		}
	}

	inline bool has_flag(const std::string& flag)
	{
		for (auto i = flags.begin(); i != flags.end(); ++i)
		{
			if ((*i) == get_lowercase(flag))
			{
				return true;
			}
		}
		return false;
	}

	inline void remove_flag(const std::string& flag)
	{
		std::string lowerc = get_lowercase(flag);
		remove_alias("@" + flag);
		for (auto i = flags.begin(); i != flags.end(); ++i)
		{
			if (get_lowercase(*i) == lowerc)
			{
				flags.erase(i);
				flags_hidden.erase(flag);
				return;
			}
		}
	}

	inline bool has_evasion()
	{
		return has_flag("evasion") || has_flag("avoidance");
	}

	inline void set_initiative(int new_initiative)
	{
		initiative = new_initiative;
	}
	inline int get_initiative() const
	{
		return initiative;
	}
	inline const std::string& get_name() const
	{
		return name;
	}
	inline int get_initiative_modifier() const
	{
		return modifier;
	}
	inline int get_max_hp() const {
		return max_hp;
	}

	inline int get_hp() const{
		return hp;
	}

	inline int get_temp_hp() const {
		return temp_hp;
	}
	creature(const std::string& name, int initiative, int modifier, int max_hp, int hp, int temp_hp, const std::string& flags_list, const std::string& alias_list, int regeneration, 
		int armor_class, const std::list<creature> const* creatures_list, const std::string& start_turn_filename, const std::string& end_turn_filename,
		bool has_con, int str, int dex, int con, int intelligence, int wis, int cha, bool entered_dex) : name(name), initiative(initiative), modifier(modifier), temp_hp(temp_hp),
		hp(hp), max_hp(max_hp), turn_count(-1), regen(regeneration), ac(armor_class), turn_start_file(start_turn_filename), turn_end_file(end_turn_filename),
		has_con(has_con), str(str), dex(dex), con(con), intelligence(intelligence), wis(wis), cha(cha), entered_dex(entered_dex)
	{
		if (hp > max_hp)
			hp = max_hp;

		if (max_hp != -1 && hp < 0)
			hp = 0;

		if (flags_list != "")
		{
			std::string flag = "";
			for (size_t i = 0; i < flags_list.size(); ++i)
			{
				char c = flags_list[i];
				if (c == ' ')
				{
					if (flag != "")
					{
						initial_flags.push_back(flag);
						add_flag(flag, true);
						flag = "";
					}
					break;
				}
				if (c==',' || c=='&')
				{
					initial_flags.push_back(flag);
					add_flag(flag, true);
					flag = "";
				}
				else
				{
					flag += c;
				}
			}
			if (flag != "")
			{
				initial_flags.push_back(flag);
				add_flag(flag, true);
				flag = "";
			}
		}
		trim(this->name);
		if (alias_list != "")
		{
			std::string alias = "";
			for (size_t i = 0; i < alias_list.size(); ++i)
			{
				char c = alias_list[i];
				if (c == ' ')
				{
					if (alias != "")
					{
						trim(alias);
						if(name_is_unique(alias, *creatures_list) && alias!=name)
							aliases.push_back(alias);
						alias = "";
					}
					break;
				}
				if (c == ',' || c == '&')
				{
					trim(alias);
					if (name_is_unique(alias, *creatures_list) && alias != name)
						aliases.push_back(alias);
					alias = "";
				}
				else
				{
					alias += c;
				}
			}
			if (alias != "")
			{
				trim(alias);
				if (name_is_unique(alias, *creatures_list) && alias != name)
					aliases.push_back(alias);
				alias = "";
			}
		}
		aliases.push_back("@all");
		if (!entered_dex)
			dex = modifier;
	}

	creature(const std::string& name, int initiative, int modifier) : name(name), initiative(initiative), modifier(modifier),
		hp(-1), max_hp(-1), turn_count(-1), temp_hp(0), regen(0), ac(-1), dex(modifier)
	{
		aliases.push_back("@all");
	}

	creature(const std::string& name, int initiative) : name(name), initiative(initiative), modifier(0), hp(-1), max_hp(-1), turn_count(-1), temp_hp(0), regen(0), ac(-1)
	{
		aliases.push_back("@all");
	}

	inline const bool operator<(const creature& other) const
	{
		if (initiative == other.initiative)
			if (modifier == other.modifier)
				return name < other.name;
			else
				return modifier > other.modifier;
		return initiative > other.initiative;
	}

	inline int adjust_hp(int amount) {
		if (max_hp != -1)
		{
			if (amount == INT_MAX)
			{
				hp = max_hp;
			}
			else if (amount == -INT_MAX)
			{
				hp = 0;
			}
			else
			{
				if (amount < 0)
				{
					temp_hp += amount;
					if (temp_hp < 0)
					{
						hp += temp_hp;
						temp_hp = 0;
						if (hp < 0)
							hp = 0;
					}
				}
				else
				{
					hp += amount;
					if (hp > max_hp)
						hp = max_hp;
					else if (hp < 0)
						hp = 0;
				}
			}
		}
		return hp;
	}

	inline int set_hp(int new_hp, bool is_signed)
	{
		if (is_signed)
		{
			adjust_hp(new_hp);
			return hp;
		}
		if (max_hp != -1)
		{
			hp = new_hp;
			if (hp > max_hp)
				hp = max_hp;
			else if (hp < 0)
				hp = 0;
		}
		return hp;
	}

	inline void disable_regen_temp()
	{
		temp_disable_regen = true;
	}

	inline bool regen_is_temporarily_disabled()
	{
		return temp_disable_regen;
	}

	inline void set_regen(int regeneration)
	{
		//if (regeneration < 0)
			//regeneration = 0;

		regen = regeneration;
	}

	inline void set_temp_hp(int thp, bool is_signed)
	{
		if (is_signed)
			temp_hp += thp;
		else
			temp_hp = thp;

		if (temp_hp < 0)
			temp_hp = 0;
	}

	inline int get_regen()
	{
		if (temp_disable_regen)
		{
			temp_disable_regen = false;
			return 0;
		}
		else
			return regen;
	}

	inline int get_regen_raw()
	{
		return regen;
	}

	inline void set_max_hp(int new_max_hp, bool is_signed)
	{
		if (is_signed)
		{
			if (max_hp != -1)
			{
				max_hp += new_max_hp;
				if (max_hp < 0)
					max_hp = 0;

				if (hp > max_hp)
					hp = max_hp;
			}
		}
		else
		{
			if (new_max_hp < 1)
				new_max_hp = 1;
			max_hp = new_max_hp;
			if (hp > new_max_hp)
				hp = new_max_hp;
			if (hp == -1)
				hp = max_hp;
		}	
	}

	inline void set_turn_count(int tc)
	{
		turn_count = tc;
	}

	inline int get_turn_count()
	{
		return turn_count;
	}

	inline void set_name(const std::string& new_name)
	{
		name = new_name;
	}
};

std::string get_hp_change_turn_msg(const std::string& name, int old_hp, int new_hp, const std::string& cur_msg, bool is_concentrating, int intended_dmg, int con_bonus, bool found_con_bonus, creature* victim, bool is_adding_string_outside_function);

bool name_is_unique(const std::string& name, const std::list<creature>& creatures)
{
	std::string lowerc = get_lowercase(name); //TODO: Finish adding comprehensive list of commands to this function for filtering.
	if (lowerc == "all" || lowerc == "reset" || lowerc == "round" || lowerc == "quit" || lowerc == "hp"
		|| lowerc == "round" || lowerc == "flag"
		|| lowerc == "move"
		|| lowerc == "mv"
		|| lowerc == "mod"
		|| lowerc == "md"
		|| lowerc == "init"
		|| lowerc == "int"
		|| lowerc == "i"
		|| lowerc == "m"
		|| lowerc == "alias"
		|| lowerc == "as"
		|| lowerc == "al"
		|| lowerc == "full"
		|| lowerc == "rf"
		|| lowerc == "reminder"
			|| lowerc == "quit"
			|| lowerc == "end"
			|| lowerc == "stop"
			|| lowerc == "terminate"
			|| lowerc == "finish"
			|| lowerc == "fr"
			|| lowerc == "f"
			|| lowerc == "add_flag"
			|| lowerc == "rmfg"
			|| lowerc == "addf"
			|| lowerc == "adf"
			|| lowerc == "adflg"
			|| lowerc == "rmflg"
			|| lowerc == "rmf"
			|| lowerc == "frm"
			|| lowerc == "rmflag"
			|| lowerc == "flagrm"
			|| lowerc == "rflag"
			|| lowerc == "flagr"
			|| lowerc == "adfl"
			|| lowerc == "adlf"
			|| lowerc == "temp_hp"
			|| lowerc == "thp"
			|| lowerc == "buffer"
			|| lowerc == "temp_hp"
			|| lowerc == "temphp"
			|| lowerc == "hp"
			|| lowerc == "rm"
			|| lowerc == "remove"
			|| lowerc == "hurt"
			|| lowerc == "dmg"
			|| lowerc == "harm"
			|| lowerc == "damage"
			|| lowerc == "d"
			|| lowerc == "h"
			|| lowerc == "hurt"
			|| lowerc == "hurt all"
			|| lowerc == "dmg all"
			|| lowerc == "damage all"
			|| lowerc == "harm all"
			|| lowerc == "save"
			|| lowerc == "savet"
			|| lowerc == "load"
			|| lowerc == "hurtr"
			|| lowerc == "dmgr"
			|| lowerc == "harmr"
			|| lowerc == "damager"
			|| lowerc == "hurth"
			|| lowerc == "dmgh"
			|| lowerc == "harmh"
			|| lowerc == "damageh"
			|| lowerc == "hurtv"
			|| lowerc == "dmgv"
			|| lowerc == "harmv"
			|| lowerc == "damagev"
			|| lowerc == "hurtv"
			|| lowerc == "dmgv"
			|| lowerc == "harmv"
			|| lowerc == "damagev"
			|| lowerc == "hurtd"
			|| lowerc == "dmgd"
			|| lowerc == "harmd"
			|| lowerc == "damaged"
			|| lowerc == "hurtd"
			|| lowerc == "dmgd"
			|| lowerc == "harmd"
			|| lowerc == "damaged"
			|| lowerc == "heal"
			|| lowerc == "heal all"
			|| lowerc == "health"
			|| lowerc == "max_hp"
			|| lowerc == "max_health"
			|| lowerc == "rename"
			|| lowerc == "reroll"
			|| lowerc == "reset"
			|| lowerc == "kill"
			|| lowerc == "die"
			|| lowerc == "rm"
			|| lowerc == "ko"
			|| lowerc == "trn"
			|| lowerc == "turn"
			|| lowerc == "goto"
			|| lowerc == "go"
			|| lowerc == "visit"
			|| lowerc == "round"
			|| lowerc == "add"
			|| lowerc == "undo"
			|| lowerc == "redo"
			|| lowerc == "u"
			|| lowerc == "r"
			|| lowerc == "leave"
			|| lowerc == "close"
			|| lowerc == "t"
			|| lowerc == "temp"
			|| lowerc == "regen"
			|| lowerc == "disable"
			|| lowerc == "roll"
			|| lowerc == "hp all"
			|| lowerc == "all hp"
			|| lowerc == "health all"
			|| lowerc == "all health"
			|| lowerc == "ac"
			|| lowerc == "clone"
			|| lowerc == "savec"
			|| lowerc == "clean"
			|| lowerc == "cleanup"
			|| lowerc == "keep"
			|| lowerc == "dv"
			|| lowerc == "rv"
			|| lowerc == "tflag"
			|| lowerc == "tfalg"
			|| lowerc == "talfg"
			|| lowerc == "tf"
			|| lowerc == "swap"
			|| lowerc == "sdisplay"
			|| lowerc == "fdisplay"
			|| lowerc == "displays"
			|| lowerc == "displayf"
			|| lowerc == "disp"
			|| lowerc == "sdisp"
			|| lowerc == "dispf"
			|| lowerc == "fdisp"
			|| lowerc == "disps"
			|| lowerc == "tdisp"
			|| lowerc == "dispt"
			|| lowerc == "info"
			|| lowerc == "query"
			|| lowerc == "simpledisp"
			|| lowerc == "simpdisp"
			|| lowerc == "fulldisp"
			|| lowerc == "maxdisp"
			|| lowerc == "mindisp"
			|| lowerc == "dispmin"
			|| lowerc == "pause"
			|| lowerc == "showall"
			|| lowerc == "show"
			|| lowerc == "clear"
			|| lowerc == "cls"
			|| lowerc == "hide"
			|| lowerc == "hideall"
			|| lowerc == "start"
			|| lowerc == "end"
			|| lowerc == "wd"
			|| lowerc == "cd"
			|| lowerc == "cd.."
			|| lowerc == "wd.."
			|| lowerc == "sort"
			|| lowerc == "log"
			|| lowerc == "note"
			|| lowerc == "notes"
			|| lowerc == "recharge0"
			|| lowerc == "recharge1"
			|| lowerc == "recharge2"
			|| lowerc == "recharge3"
			|| lowerc == "recharge4"
			|| lowerc == "recharge5"
			|| lowerc == "recharge6"
			|| lowerc == "recharge1-6"
			|| lowerc == "recharge2-6"
			|| lowerc == "recharge3-6"
			|| lowerc == "recharge4-6"
			|| lowerc == "recharge5-6"
			|| lowerc == "recharge6-6"
			|| lowerc == "if"
			|| lowerc == "hidevar"
			|| lowerc == "showvar"
			|| lowerc == "print"
			|| lowerc == "printnum"
			|| lowerc == "print_num"
			|| lowerc == "dc"
			|| lowerc == "str_save"
			|| lowerc == "dex_save"
			|| lowerc == "con_save"
			|| lowerc == "int_save"
			|| lowerc == "wis_save"
			|| lowerc == "cha_save"
			|| lowerc == "skip"
			|| lowerc == "ls"
			|| lowerc == "repeat"
			|| lowerc == "monster"
		) 
			return false;

	for (int i = 0; i < lowerc.size(); ++i)
	{
		char c = lowerc[i];
		if (
			   c == ','
			|| c == '&'
			|| c == ' '
			|| c == '/'
			|| c == '.'
			|| c == '*'
			|| c == '@'
			|| c == '?'
			|| c == '.'
			|| c == '#'
			|| c == '%'
			|| c == '$'
			|| c == '*'
			|| c == '+'
			|| c == '-'
			|| c == '/'
			|| c == '|'
			|| c == '\\'
			|| c == '{'
			|| c == '}'
			|| c == '|'
			|| c == '['
			|| c == ']'
		)
			return false;
	}
	for (auto i = creatures.begin(); i != creatures.end(); ++i)
	{
		if (i->has_alias(lowerc))
			return false;
	}
	return true;
}

inline void save_state(const std::string& filename, std::list<creature>& creatures, const std::string& turn, int round_num, bool temp_file)
{
	try {
		std::ofstream out;
		if (wd == "")
			out.open(filename);
		else
			out.open(wd + "/" + filename);

		if (!out.is_open())
			throw;

		if (!temp_file)
		{
			out << "reset\n";
			out << "dc " << save_dc << std::endl;
		}

		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			std::string name = i->get_name();
			std::string mod = std::to_string(i->get_initiative_modifier());
			std::string init = std::to_string(i->get_initiative());

			std::string line = name + " " + init;
			if (i->get_initiative_modifier() >= 0)
				line += " +" + mod;
			else
				line += " " + mod;

			line += " str:" + std::to_string(i->get_str_bonus()) + " ";
			if(i->entered_dex)
				line += "dex:" + std::to_string(i->get_dex_bonus()) + " ";
			if(i->has_con_bonus())
				line += "con:" + std::to_string(i->con) + " ";
			line += "int:" + std::to_string(i->get_int_bonus()) + " ";
			line += "wis:" + std::to_string(i->get_wis_bonus()) + " ";
			line += "cha:" + std::to_string(i->get_cha_bonus()) + " ";

			if (i->get_max_hp() != -1)
			{
				std::string max_hp = std::to_string(i->get_max_hp());
				std::string hp = std::to_string(i->get_hp());
				line += " hp:" + hp + "/" + max_hp;
			}
			if (i->get_flags().size() != 0)
			{
				line += " flags:" + i->get_flag_list(false, true,true,false);
			}
			if (i->get_ac() != -1)
				line += " ac:" + std::to_string(i->get_ac());

			if (i->turn_start_file != "")
				line += " start:" + i->turn_start_file;

			if (i->turn_end_file != "")
				line += " end:" + i->turn_end_file;


			line += "\n";

			out << line;

			for (auto alias_iterator = i->aliases.begin(); alias_iterator != i->aliases.end(); ++alias_iterator)
			{
				if ((*alias_iterator)[0]!='@')
					out << "alias " << i->get_name() << " " << (*alias_iterator) << std::endl;
			}

			for (auto vars_iterator = i->variables.begin(); vars_iterator != i->variables.end(); ++vars_iterator)
			{
				out << i->get_name() << ":" << vars_iterator->first << " " << vars_iterator->second << std::endl;
			}

			int regen_amnt = i->get_regen_raw();
			bool is_regen_disabled = i->regen_is_temporarily_disabled();

			if (regen_amnt != 0)
			{
				out << "regen " << i->get_name() << " " << regen_amnt << std::endl;
				if (is_regen_disabled)
					out << "disable " << i->get_name() << std::endl;
			}

			if (i->get_reminder(false).size() != 0)
			{
				out << "reminder " << i->get_name() << " " << i->get_reminder(false) << std::endl;
			}

			if (i->get_note(false).size() != 0)
			{
				out << "note " << i->get_name() << " " << i->get_note(false) << std::endl;
			}

			for (int index = 0; index < i->recharge1.size(); ++index)
			{
				out << "recharge1 " << i->get_name() << " " << i->recharge1[index] << std::endl;
			}

			for (int index = 0; index < i->recharge2.size(); ++index)
			{
				out << "recharge2 " << i->get_name() << " " << i->recharge2[index] << std::endl;
			}

			for (int index = 0; index < i->recharge3.size(); ++index)
			{
				out << "recharge3 " << i->get_name() << " " << i->recharge3[index] << std::endl;
			}

			for (int index = 0; index < i->recharge4.size(); ++index)
			{
				out << "recharge4 " << i->get_name() << " " << i->recharge4[index] << std::endl;
			}

			for (int index = 0; index < i->recharge5.size(); ++index)
			{
				out << "recharge5 " << i->get_name() << " " << i->recharge5[index] << std::endl;
			}

			for (int index = 0; index < i->recharge6.size(); ++index)
			{
				out << "recharge6 " << i->get_name() << " " << i->recharge6[index] << std::endl;
			}

		}

		if (!temp_file)
			out << "round " << std::to_string(round_num) << std::endl;
		if (turn != "")
			out << "turn " << turn;

		if (!temp_file)
		{
			out << std::endl;
			if (simple_display)
			{
				out << "simple display";
			}
			else
			{
				out << "full display";
			}
		}

		out.close();
	}
	catch (const std::exception& E)
	{
		std::cout << "Error saving to file '" << filename << "'" << std::endl;
	}
}

inline bool is_digits(const std::string& input)
{
	for (size_t i = 0; i < input.size(); ++i)
	{
		switch (input[i])
		{
		case '0': break;
		case '1': break;
		case '2': break;
		case '3': break;
		case '4': break;
		case '5': break;
		case '6': break;
		case '7': break;
		case '8': break;
		case '9': break;
		case '+': break;
		case '-': {
			if (i == 0)
				break;
			else
				return false;
		}
		default: return false;
		}
	}
	return true;
}

struct dice
{
	int dice_count = 0;
	int sides = 0;
	int mod = 0;
	bool subtracted = false;
	std::string dmg_type = "";

	dice(int dice_count, int sides, int mod, bool subtracted, const std::string& dmg_type) : dice_count(dice_count), sides(sides), mod(mod), subtracted(subtracted), dmg_type(dmg_type) {}

	int roll()
	{
		int sum = mod;

		for (int i = 0; i < dice_count; ++i)
		{
			int this_roll = 1 + (rand() % sides);
			sum += this_roll;
		}

		return sum;
	}

	int roll_crit()
	{
		int sum = mod;

		for (int i = 0; i < dice_count; ++i)
		{
			sum += 1 + (rand() % sides);
			sum += 1 + (rand() % sides);
		}

		return sum;
	}

	int roll_dice_only()
	{
		int sum = 0;

		for (int i = 0; i < dice_count; ++i)
		{
			int this_roll = 1 + (rand() % sides);
			sum += this_roll;
		}

		return sum;
	}
};

inline int parse_dice(std::string& input)
{
	if (is_digits(input))
		return std::stoi(input);
	std::vector<dice> dmg_dice;
	std::string temp;
	//std::cout << "Preprocessing: " << input << std::endl;
	temp.reserve(input.size());
	size_t start;
	for (start = 0; start < input.size(); ++start)
	{
		char c = input[start];
		if (
			c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9'
			)
			break;
	}
	for (size_t i = start; i < input.size(); ++i)
	{
		if (input[i] != ' ')
			temp += input[i];
	}
	input = temp;
	//std::cout << "Processed: " << input << std::endl;
	int dice_count = 0;
	int sides = 0;
	int mod = 0;
	std::string dmg_type = "";
	bool subtracted = false;
	bool prevnum = false;
	auto push_dice = [&]() -> void
		{
			//std::cout << "Called push_dice, " << dice_count << "d" << sides << " + " << mod << std::endl;
			if (!(dice_count == 0 && sides == 0 && mod == 0))
			{
				//std::cout << "PUSHING DIE: " << dice_count << "d" << sides << " + " << mod << std::endl;
				dmg_dice.emplace_back(dice_count, sides, mod, subtracted, dmg_type);
				dice_count = 0;
				sides = 0;
				mod = 0;
				subtracted = false;
				dmg_type = "";
			}
		};

	int digits = 0;
	auto push_digit = [&](int digit) -> void
		{
			digits *= 10;
			digits += digit;
			prevnum = true;
			//std::cout << "NEW DIGIT: " << digit << std::endl;
		};
	for (size_t i = 0; i < input.size(); ++i)
	{
		char c = input[i];
		//std::cout << c << std::endl;
		switch (c)
		{
		case '+': {
			if (sides != 0 && dice_count != 0 && digits != 0)
			{
				mod = digits;
				digits = 0;
				push_dice();
			}
			else if (sides != 0 && dice_count != 0 && digits == 0)
			{

			}
			if (dice_count != 0)
			{
				sides = digits;
				digits = 0;
			}
			prevnum = false;
			break;
		}

		case '-': {
			if (dice_count != 0)
			{
				sides = digits;
				digits = 0;
			}
			subtracted = true;
			prevnum = false;
			break;
		}

		case 'd': {
			if (prevnum)
			{
				push_dice();
				dice_count = digits;
				digits = 0;
			}
			else
			{
				dmg_type += c;
			}
			prevnum = false;
			break;
		}
		case '0': {
			push_digit(0);
			break;
		}
		case '1': {
			push_digit(1);
			break;
		}
		case '2': {
			push_digit(2);
			break;
		}
		case '3': {
			push_digit(3);
			break;
		}
		case '4': {
			push_digit(4);
			break;
		}
		case '5': {
			push_digit(5);
			break;
		}
		case '6': {
			push_digit(6);
			break;
		}
		case '7': {
			push_digit(7);
			break;
		}
		case '8': {
			push_digit(8);
			break;
		}
		case '9': {
			push_digit(9);
			break;
		}

		default: {
			dmg_type += c;
			prevnum = false;
			break;
		}
		}
	}

	if (digits != 0)
	{
		if (sides == 0)
		{
			sides = digits;
		}
		else
		{
			mod = digits;
			if (subtracted)
			{
				mod = -mod;
			}
		}
	}

	push_dice();

	if (dmg_dice.size() == 0)
		return 0;

	int sum = 0;
	for (int i = 0; i < dmg_dice.size(); ++i)
	{
		sum += dmg_dice[i].roll();
	}
	return sum;
}


inline int get_number_arg(std::string dummy_line, bool& is_signed, std::list<creature>& creatures, creature* executor)
{
	trim(dummy_line);
	is_signed = false;
	size_t first_space = dummy_line.find(" ");
	size_t second_space = dummy_line.find(" ", first_space + 1);
	int value;
	std::string sub;

	if (second_space == std::string::npos)
	{
		sub = dummy_line.substr(first_space);
	}
	else
	{
		sub = dummy_line.substr(second_space, dummy_line.length() - second_space);
	}

	if (sub == " max" || sub == " all" || sub == " full")
	{
		return INT_MAX;
	}

	if (sub == " input")
	{
		std::string user_input = "a";
		while (!is_digits(user_input))
		{
			std::cout << "Please enter a number: ";
			std::getline(std::cin, user_input);
		}
		return std::stoi(user_input);
	}

	std::string trimmed = sub.substr(1);
	if (trimmed.find(":") != std::string::npos || trimmed.find(".") != std::string::npos)
	{
		std::string creature = "";
		int i = 0;
		while (i < trimmed.size() && trimmed[i] != ':' && trimmed[i] != '.')
		{
			creature += trimmed[i];
			++i;
		}
		std::string varname = trimmed.substr(creature.size()+1);
		if (varname[0] == ':')
			varname = varname.substr(1);

		for (auto ci = creatures.begin(); ci != creatures.end(); ++ci)
		{
			if (ci->has_alias(creature) || (creature=="@executor" && (ci->get_raw_ptr()==executor)))
			{
				return ci->get_var(varname, creatures);
			}
		}
		return 0;
	}

	if (sub.size() > 2 && sub[0] == ' ' && (sub[1] == '@') || sub[1]=='#')
	{
		std::string whole = sub.substr(1);
		if (whole.size() > 1 && whole[0] == '@' && whole[1] == '#')
		{
			whole[0] = '#';
			whole[1] = '@';
		}
		if (whole[0] == '#')
		{
			if (whole[1] == '@')
				whole = whole.substr(1);
			else
				whole[0] = '@';
		}
		value = 0;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			if (i->has_alias(whole))
			{
				++value;
			}
		}
		return value;
	}

	size_t trunc;
	for (trunc = 0; trunc < sub.size(); ++trunc)
	{
		char c = sub[trunc];
		if (
			!(
				c == '0'
				|| c == '1'
				|| c == '2'
				|| c == '3'
				|| c == '4'
				|| c == '5'
				|| c == '6'
				|| c == '7'
				|| c == '8'
				|| c == '9'
				|| c == ' '
				|| c == '-'
				|| c == '+'
				|| c == 'd'
				|| c == 'D'
				)
			)
			break;
	}

	if (trunc != sub.size())
	{
		sub.resize(trunc);
	}
	
	trim(sub);
	if(is_digits(sub))
	{
		value = std::stoi(sub);
		if (sub[0] == '+' || value < 0)
			is_signed = true;
	}
	else
	{
		value = parse_dice(sub);
	}
	return value;
};

inline void clone_character(const std::string& name, int count, std::list<creature>& creatures, creature* base)
{
	if (base->touched)
		return;
	std::vector<creature> sorter;
	base->touched = true;
	base->remove_alias("@current");
	for (int i = 0; i < count; ++i)
	{
		creature copy(*base);
		auto get_new_name = [&](const std::string & base_name) -> std::string
		{
				int base_copy_id = i;
				std::string base0_name;
				bool base0 = false;
				if (base_name[base_name.size() - 1] == '0'|| base_name[base_name.size() - 1] == '1')
				{
					base0_name = base_name;
					base0_name.resize(base0_name.size() - 1);
					base0 = true;
				}

				std::string copy_name = "clone";

				bool sorter_contains_name = false;
				for (auto cn = sorter.begin(); cn != sorter.end(); ++cn)
				{
					if (cn->has_alias(copy_name))
					{
						sorter_contains_name = true;
						break;
					}
				}

				while (!name_is_unique(copy_name, creatures) || copy.has_alias(copy_name) || sorter_contains_name)
				{
					if (base0)
					{
						copy_name = base0_name + std::to_string(++base_copy_id);
					}
					else
					{
						copy_name = base_name + std::to_string(++base_copy_id);
					}

					sorter_contains_name = false;
					for (auto cn = sorter.begin(); cn != sorter.end(); ++cn)
					{
						if (cn->has_alias(copy_name))
						{
							sorter_contains_name = true;
							break;
						}
					}
				}
				return copy_name;
		};
		
		
		copy.set_name(get_new_name(copy.get_name()));
		for (auto j = copy.aliases.begin(); j != copy.aliases.end(); ++j)
		{
			if((*j)[0] != '@')
				(*j) = get_new_name(*j);
		}
		copy.set_initiative((rand() % 20) + 1 + copy.get_initiative_modifier());
		copy.touched = true;
		sorter.push_back(copy);
	}

	auto swap_init = [&](creature& a, creature& b) -> void
		{
			int a_init = a.get_initiative();
			a.set_initiative(b.get_initiative());
			b.set_initiative(a_init);
		};

	if (sorter.size() > 1)
	{
		bool swapped = true;
		while (swapped) //bubblesort lmao
		{
			swapped = false;
			for (size_t i = 0; i < sorter.size() - 1; ++i)
			{
				creature& a = sorter[i];
				creature& b = sorter[i + 1];

				if (b.get_initiative() > a.get_initiative())
				{
					swap_init(a, b);
					swapped = true;
				}
			}
		}
	}

	for (size_t i = 0; i < sorter.size(); ++i)
	{
		creatures.push_back(sorter[i]);
	}
	creatures.sort();
}

std::string replace_beginning_if_match(const std::string& base, const std::string& original_beginning, const std::string& new_beginning)
{
	if (starts_with(base, original_beginning))
	{
		return new_beginning + base.substr(original_beginning.size());
	}
	else
	{
		return base;
	}
}

void command_replacement(std::string& dummy_line)
{
	std::string lc = get_lowercase(dummy_line);
	if (lc == "full display mode")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "simple display mode")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "complex display")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "max display")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "display full")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "display max")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "hide")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "hide all")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "hideall")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "min display")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "display min")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "sdisplay")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "displays")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "fdisplay")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "displayf")
	{
		dummy_line = "full display";
		return;
	}


	if (lc == "showall")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "show")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "show all")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "complex disp")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "max disp")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "disp full")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "fulldisp")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "maxdisp")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "disp max")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "min disp")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "disp min")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "mindisp")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "dispmin")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "sdisp")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "disps")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "simpdisp")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "fdisp")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "dispf")
	{
		dummy_line = "full display";
		return;
	}
	if (lc == "simple disp")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "simpledisp")
	{
		dummy_line = "simple display";
		return;
	}
	if (lc == "reset all")
	{
		dummy_line = "reset @all";
		return;
	}
	if (lc == "reset")
	{
		dummy_line = "reset @all";
		return;
	}

	if (starts_with(dummy_line, "monster ") && dummy_line.size()>8)
	{
		std::string monster_name = dummy_line.substr(8);
		std::ifstream temp;
		temp.open("monsters/"+monster_name);
		bool exists = false;
		if (temp.is_open())
		{
			temp.close();
			exists = true;
		}
		if (!exists && !ends_with(dummy_line,".txt"))
		{
			temp.open("monsters/" + monster_name + ".txt");
			if (temp.is_open())
			{
				temp.close();
				exists = true;
				monster_name += ".txt";
			}
		}
		dummy_line = "ld " + BASE_DIRECTORY_PROXY + "/monsters/" + monster_name;
		
		
	}

	
	if (!starts_with(dummy_line, "ld .") && !starts_with(dummy_line,"load ."))
	{
		dummy_line = replace_all(dummy_line, " .", ".", false);
		dummy_line = replace_all(dummy_line, ". ", ".", false);
	}
	dummy_line = replace_first(dummy_line, "falg", "flag", true, false);
	dummy_line = replace_first(dummy_line, "talfg", "tf", true,false);
	dummy_line = replace_first(dummy_line, "tflag", "tf", true,false);
	dummy_line = replace_first(dummy_line, "tfalg", "tf", true,false);
	dummy_line = replace_first(dummy_line, "tlafg", "tf", true,false);
	dummy_line = replace_first(dummy_line, "talfg", "tf", true,false);
	dummy_line = replace_first(dummy_line, "ralias", "ra", true, false);
	dummy_line = replace_first(dummy_line, "aliasr", "ra", true, false);
	dummy_line = replace_first(dummy_line, "ar", "ra", true, false);
	dummy_line = replace_first(dummy_line, "as", "alias", true, false);
	dummy_line = replace_first(dummy_line, "al", "alias", true, false);
	dummy_line = replace_first(dummy_line, "removealias", "ra", true, false);
	dummy_line = replace_first(dummy_line, "remove_alias", "ra", true, false);
	dummy_line = replace_first(dummy_line, "aliasremove", "ra", true, false);
	dummy_line = replace_first(dummy_line, "alias_remove", "ra", true, false);
	dummy_line = replace_first(dummy_line, "notes", "note", true, false);
	dummy_line = replace_first(dummy_line, "rmfg", "rf", true, false);
	dummy_line = replace_first(dummy_line, "rmflg", "rf", true, false);
	dummy_line = replace_first(dummy_line, "rmflag", "rf", true, false);
	dummy_line = replace_first(dummy_line, "flagrm", "rf", true, false);
	dummy_line = replace_first(dummy_line, "rmf", "rf", true, false);
	dummy_line = replace_first(dummy_line, "frm", "rf", true, false);
	dummy_line = replace_first(dummy_line, "adflg", "flag", true, false);
	dummy_line = replace_first(dummy_line, "adfl", "flag", true, false);
	dummy_line = replace_first(dummy_line, "adlf", "flag", true, false);
	dummy_line = replace_first(dummy_line, "flag_add", "add_flag", true, false);
	dummy_line = replace_first(dummy_line, "fr", "rf", true, false);
	dummy_line = replace_first(dummy_line, "hidevar", "hide", true, false);
	dummy_line = replace_first(dummy_line, "showvar", "show", true, false);
	dummy_line = replace_first(dummy_line, "print_num", "printnum", true, false);
	dummy_line = replace_first(dummy_line, "save_dc", "dc", true, false);
	dummy_line = replace_first(dummy_line, "savedc", "dc", true, false);

	dummy_line = replace_first(dummy_line, "buffer", "temp_hp", true, false);
	dummy_line = replace_first(dummy_line, "thp", "temp_hp", true, false);
	dummy_line = replace_first(dummy_line, "temp_hp", "temp_hp", true, false);
	dummy_line = replace_first(dummy_line, "temp", "temp_hp", true, false);
	dummy_line = replace_first(dummy_line, "t", "temp_hp", true, false);

	dummy_line = replace_first(dummy_line, "roll str save", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "roll dex save", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "roll con save", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "roll int save", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "roll wis save", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "roll cha save", "cha_save", true, false);

	dummy_line = replace_first(dummy_line, "roll str", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "roll dex", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "roll con", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "roll int", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "roll wis", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "roll cha", "cha_save", true, false);

	dummy_line = replace_first(dummy_line, "save_str", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "savestr", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "strsave", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_str", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_str_save", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "str_roll", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "rollstr", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "strroll", "str_save", true, false);
	dummy_line = replace_first(dummy_line, "stroll", "str_save", true, false);

	dummy_line = replace_first(dummy_line, "save_con", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "savecon", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "consave", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_con", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_con_save", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "con_roll", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "rollcon", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "conroll", "con_save", true, false);

	dummy_line = replace_first(dummy_line, "save_dex", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "savedex", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "dexsave", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_dex", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_dex_save", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "dex_roll", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "rolldex", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "dexroll", "dex_save", true, false);

	dummy_line = replace_first(dummy_line, "save_int", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "saveint", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "intsave", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_int", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_int_save", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "int_roll", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "rollint", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "introll", "int_save", true, false);

	dummy_line = replace_first(dummy_line, "save_wis", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "savewis", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "wissave", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_wis", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_wis_save", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "wis_roll", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "rollwis", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "wisroll", "wis_save", true, false);

	dummy_line = replace_first(dummy_line, "save_cha", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "savecha", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "chasave", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_cha", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "roll_cha_save", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "cha_roll", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "rollcha", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "charoll", "cha_save", true, false);

	dummy_line = replace_first(dummy_line, "save str", "str_save",true,false);
	dummy_line = replace_first(dummy_line, "str save", "str_save", true, false);

	dummy_line = replace_first(dummy_line, "save dex", "dex_save", true, false);
	dummy_line = replace_first(dummy_line, "dex save", "dex_save", true, false);

	dummy_line = replace_first(dummy_line, "save con", "con_save", true, false);
	dummy_line = replace_first(dummy_line, "con save", "con_save", true, false);

	dummy_line = replace_first(dummy_line, "save int", "int_save", true, false);
	dummy_line = replace_first(dummy_line, "int save", "int_save", true, false);

	dummy_line = replace_first(dummy_line, "save wis", "wis_save", true, false);
	dummy_line = replace_first(dummy_line, "wis save", "wis_save", true, false);

	dummy_line = replace_first(dummy_line, "save cha", "cha_save", true, false);
	dummy_line = replace_first(dummy_line, "cha save", "cha_save", true, false);

	dummy_line = replace_first(dummy_line, "savedc", "dc", true, false);
	dummy_line = replace_first(dummy_line, "save_dc", "dc", true, false);

	dummy_line = replace_first(dummy_line, "rflag", "rf", true, false);
	dummy_line = replace_first(dummy_line, "flagr", "rf", true, false);
	dummy_line = replace_first(dummy_line, "adf", "flag", true, false);
	dummy_line = replace_first(dummy_line, "addf", "flag", true, false);
	dummy_line = replace_first(dummy_line, "add_flag", "flag", true, false);
}

void process_filename(std::string& filename)
{
	if (starts_with(filename, BASE_DIRECTORY_PROXY + "/") && filename.size()>BASE_DIRECTORY_PROXY.size()+1)
	{
		filename = filename.substr(BASE_DIRECTORY_PROXY.size() + 1);
	}
}

std::list<std::list<creature>> creatures_buffer;
std::list<bool> new_round_buffer;
std::list<std::string> turn_msg_buffer;
std::list<index_t> current_turn_buffer;
std::list<size_t> current_round_buffer;
std::list<bool> display_mode_buffer;
std::list<std::string> wd_buffer;
std::list<int> save_dc_buffer;

std::list<std::list<creature>>::iterator creatures_buffer_iterator;
std::list<bool>::iterator new_round_buffer_iterator;
std::list<std::string>::iterator turn_msg_buffer_iterator;
std::list<index_t>::iterator current_turn_buffer_iterator;
std::list<size_t>::iterator current_round_buffer_iterator;
std::list<bool>::iterator display_mode_buffer_iterator;
std::list<std::string>::iterator wd_buffer_iterator;
std::list<int>::iterator save_dc_buffer_iterator;

//Sorts the initiatives of the given creatures after sorting them by name
inline void sort(std::list<creature>& creatures, const std::string& name)
{
	if (creatures.size() < 2)
		return;
	std::vector<creature*> c;
	c.reserve(creatures.size());
	for (auto i = creatures.begin(); i != creatures.end(); ++i)
	{
		if (i->has_alias(name))
			c.push_back(i->get_raw_ptr());
	}

	if (c.size() > 1)
	{
		bool swapped = true;
		while (swapped)
		{
			swapped = false;
			for (int i = 0; i < c.size() - 1; ++i)
			{
				creature* left = c[i];
				creature* right = c[i + 1];
				
				if ((left->get_name_digits() > right->get_name_digits()) || (   (left->get_name_digits()==right->get_name_digits() && left->get_name()>right->get_name()) ))
				{
					c[i] = right;
					c[i + 1] = left;
					swapped = true;
				}
			}
		}

		swapped = true;
		while (swapped)
		{
			swapped = false;
			for (int i = 0; i < c.size() - 1; ++i)
			{
				creature* left = c[i];
				creature* right = c[i + 1];
				if (left->get_initiative() < right->get_initiative())
				{
					int right_initiative = right->get_initiative();
					right->set_initiative(left->get_initiative());
					left->set_initiative(right_initiative);
					swapped = true;
				}
			}
		}

		creatures.sort();
	}
}
const static int STATE_NODO = 0;
const static int STATE_UNDO = 1;
const static int STATE_REDO = 2;
static int get_creature_buffer_manipulation_state = STATE_NODO;

const int VAR_NULL = 0;
const int VAR_SET = 1;
const int VAR_ADD = 2;
const int VAR_SUB = 3;
const int VAR_INCREMENT = 4;
const int VAR_DECREMENT = 5;

const int VAR_GREATER_THAN = 6;
const int VAR_GREATER_THAN_OR_EQUAL = 7;
const int VAR_EQUAL_TO = 8;
const int VAR_LESS_THAN = 9;
const int VAR_LESS_THAN_OR_EQUAL = 10;
const int VAR_NOT_EQUAL = 11;

const int VAR_MULTIPLY = 12;
const int VAR_DIVIDE = 13;


inline std::string resolve_var_name(const std::string& var, std::list<creature>& creatures, creature& current_creature)
{
	if (var.size() > 2 && var[0] == '[' && var[var.size() - 1] == ']')
	{
		std::string og_var = var;
		std::string parse = var;
		parse[0] = ' ';
		parse[var.size() - 1] = ' ';
		trim(parse);
		parse = "parse " + parse;
		bool is_signed = false;
		int val = get_number_arg(parse, is_signed, creatures, &current_creature);
		parse = "#" + std::to_string(val);
		return parse;
	}
	else
	{
		return var;
	}
}

//If the given file does not exist, this function looks for a "LINKS.txt" file in the same directory and searches it for alternative directories to find the file in
//LINKS.txt contains a list of alternate directories to search if a file isn't found in that directory itself.
inline bool search_links(std::string& filename)
{
	replace_all(filename, "\\", "/", false);
	replace_all(filename, "//", "/",false);
	std::string og = filename;
	std::ifstream test;
	test.open(filename);
	if (test.is_open())
	{
		test.close();
		return true;
	}
	else
	{
		std::string dir = get_directory(filename);
		std::ifstream links;
		links.open(dir + "/LINKS.txt");
		if (!links.is_open())
			links.open(dir + "/LINKS");
		if (links.is_open()) //A LINKS file exists - begin searching
		{
			std::vector<std::string> dirs;
			while (links.good() && !links.eof())
			{
				std::string line;
				std::getline(links, line);
				bool is_absolute_dir = is_absolute_directory(line);

				if (starts_with(line, BASE_DIRECTORY_PROXY + "/"))
				{
					line = line.substr(BASE_DIRECTORY_PROXY.size() + 1);

					replace_all(line, "//", "/", false);
					replace_all(line, "\\\\", "/", false);
					replace_all(line, "\\", "/", false);
					if (line[0] == '/' || line[0] == '\\')
						line = line.substr(1);
					if (line[line.size() - 1] == '/' || line[line.size() - 1] == '\\')
						line.resize(line.size() - 1);
				}
				else //Assuming the search is a relative directory
				{
					replace_all(line, "//", "/", false);
					replace_all(line, "\\\\", "/", false);
					replace_all(line, "\\", "/", false);
					if (line[0] == '/' || line[0]=='\\')
						line = line.substr(1);
					if (line[line.size() - 1] == '/' || line[line.size() - 1] == '\\')
						line.resize(line.size() - 1);
					if(!is_absolute_dir)
						line = dir + "/" + line;
				}
				dirs.push_back(line); //Line is "processed" at this point to use consistent formatting, and neither begins nor ends in a slash.
			}
			links.close();
			//Now that I have a complete list of alternative directories to check, I need to get the raw filename and then check each of those directories for it.
			//The first match I find - if any - is to be inserted into the original filename variable.
			//If I don't find a match then no modification is necessary, the original file-opening code will record an error.
			std::string raw_filename = og;
			if(dir!="")
				raw_filename = og.substr(dir.size());
			for (int i = 0; i < dirs.size(); ++i)
			{
				std::string newf = dirs[i] + "/" + raw_filename;
				test.open(newf);
				if (test.is_open())
				{
					test.close();
					filename = newf;
					return true;
				}
			}

			//If it gets to here then no match was found in any linked directories.
			//That means it's time to search recursively linked directories
			for (int i = 0; i < dirs.size(); ++i)
			{
				std::string newf = dirs[i] + "/" + raw_filename;
				if (search_links(newf))
				{
					filename = newf;
					return true;
				}
			}

			return false; //If recursively linked directories don't have it either then it does not exist in this file tree; report that result.
		}
	}
}

//Process command/add a creature, and return whether or not a creature was added.
inline bool get_creature(std::list<creature>& creatures, bool& taking_intiatives, std::string& line, std::ifstream& file, bool takes_commands, bool info_already_in_line, bool may_expect_add_keyword, const std::string& filename, bool& ignore_initial_file_load, std::string directory, bool initial_execution, std::string& turn_msg, bool suppress_display)
{
	auto save_buffer = [&]() -> void
		{
			//std::cout << "SAVING BUFFER\n";
			if (get_creature_buffer_manipulation_state == STATE_NODO && initial_execution)
			{
				creatures_buffer.push_front(creatures);
				new_round_buffer.push_front(false);
				turn_msg_buffer.push_front("turn_msg");
				current_turn_buffer.push_front(0);
				current_round_buffer.push_front(0);
				display_mode_buffer.push_front(simple_display);
				wd_buffer.push_front(wd);
				save_dc_buffer.push_front(save_dc);

				creatures_buffer_iterator = creatures_buffer.begin();
				new_round_buffer_iterator = new_round_buffer.begin();
				turn_msg_buffer_iterator = turn_msg_buffer.begin();
				current_turn_buffer_iterator = current_turn_buffer.begin();
				current_round_buffer_iterator = current_round_buffer.begin();
				display_mode_buffer_iterator = display_mode_buffer.begin();
				wd_buffer_iterator = wd_buffer.begin();
				save_dc_buffer_iterator = save_dc_buffer.begin();
				if (creatures_buffer.size() > MAX_UNDO_STEPS)
				{
					creatures_buffer.pop_back();
					new_round_buffer.pop_back();
					turn_msg_buffer.pop_back();
					current_turn_buffer.pop_back();
					current_round_buffer.pop_back();
					display_mode_buffer.pop_back();
					wd_buffer.pop_back();
					save_dc_buffer.pop_back();
				}
				//std::cout << "Saved buffer\n";
			}
		};

	if (initial_execution)
	{
		if (wd_buffer.size() == 0)
			save_buffer();

		if (get_creature_buffer_manipulation_state == STATE_NODO)
		{
			while (creatures_buffer_iterator != creatures_buffer.begin()) //Once a non redo/undo command is executed, the current buffer - whichever it is - becomes current
			{
				creatures_buffer.pop_front();
				new_round_buffer.pop_front();
				turn_msg_buffer.pop_front();
				current_turn_buffer.pop_front();
				current_round_buffer.pop_front();
				display_mode_buffer.pop_front();
				wd_buffer.pop_front();
				save_dc_buffer.pop_front();
			}
		}
		else
		{
			creatures = *creatures_buffer_iterator;
			simple_display = *display_mode_buffer_iterator;
			wd = *wd_buffer_iterator;
			save_dc = *save_dc_buffer_iterator;

		}
	}
	//takes_commands = true;
	process_filename(directory);
	bool added_creature = false;
	bool using_file = file.is_open() && file.good();
	if(!suppress_display)
		std::cout << std::endl << "________________________________________________" << std::endl;
	std::string lowercase, name, initiative_string, mod_string;
	if (!suppress_display)
	{
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			std::cout << i->get_display_names();
			if (i->get_max_hp() != -1)
				std::cout << " (" << i->get_hp() << "/" << i->get_max_hp() << " hp)";
			if (i->get_flags().size() != 0)
				std::cout << "; [" << i->get_flag_list(false, true, true, true) << "]";
			bool printed_vars = false;
			if (i->variables.size() != 0)
			{
				std::cout << std::endl;
				for (auto vi = i->variables.begin(); vi != i->variables.end(); ++vi)
				{
					std::cout << "\t    " << vi->first << " = " << vi->second << std::endl;
					printed_vars = true;
				}
			}
			if (!printed_vars)
				std::cout << std::endl;
		}
		std::cout << "\nEnter creature name + initiative:" << std::endl;
	}
	if (using_file && !info_already_in_line)
	{
		if (file.eof() || !file.good())
		{
			return false;
		}
		else
		{
			std::getline(file, line);
			if (!suppress_display)
				std::cout << filename << ": \'" << line << "\'" << std::endl;
		}
	}
	else if (!info_already_in_line)
	{
		std::getline(std::cin, line);
	}
	bool used_command = false;


	std::string& original_dummy_line = line;
	std::string dummy_line = line;
	make_lowercase(dummy_line);
	trim(dummy_line);
	command_replacement(dummy_line);
	if (dummy_line == "quit" || dummy_line == "end" || dummy_line == "stop" || dummy_line == "terminate" || dummy_line == "finish" || dummy_line == "leave" || dummy_line == "close")
		exit(0);

	auto dir_fix = [&](std::string& filename)
		{
			if (!starts_with(filename, BASE_DIRECTORY_PROXY + "/"))
			{
				if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
				{
					size_t backi = filename.size() - 1;
					while (filename[backi] != '/' && filename[backi] != '\\')
					{
						--backi;
					}
					if (filename.find(BASE_DIRECTORY_PROXY + "/") != std::string::npos)
					{
						filename = filename.substr(filename.find(BASE_DIRECTORY_PROXY + "/") + (BASE_DIRECTORY_PROXY + "/").size());
					}
					else
					{
						directory = filename;
						directory.resize(backi);
						if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
							wd = directory;
					}
				}
			}
			
		};

	auto cleanup_current = [&]()
		{
			for (auto i = creatures.begin(); i != creatures.end(); ++i)
			{
				i->remove_alias("@current");
			};
		};

	
	if (initial_execution)
	{

		get_creature_buffer_manipulation_state = STATE_NODO; //Default buffer manipulation state for commands

		if (dummy_line == "undo" || dummy_line == "u")
		{
			get_creature_buffer_manipulation_state = STATE_UNDO;
			//Increment buffer iterators
			if (creatures_buffer_iterator != (--creatures_buffer.end()))
			{
				++creatures_buffer_iterator;
				++new_round_buffer_iterator;
				++turn_msg_buffer_iterator;
				++current_turn_buffer_iterator;
				++current_round_buffer_iterator;
				++display_mode_buffer_iterator;
				++wd_buffer_iterator;
				++save_dc_buffer_iterator;
			}
			return false;
		}
		else if (dummy_line == "redo" || dummy_line == "r")
		{
			get_creature_buffer_manipulation_state = STATE_REDO;
			if (creatures_buffer_iterator != creatures_buffer.begin())
			{
				//Decrement buffer iterators
				--creatures_buffer_iterator;
				--new_round_buffer_iterator;
				--turn_msg_buffer_iterator;
				--current_turn_buffer_iterator;
				--current_round_buffer_iterator;
				--display_mode_buffer_iterator;
				--wd_buffer_iterator;
				--save_dc_buffer_iterator;
			}
			return false;
		}
		else if (dummy_line.size() > 5 && dummy_line[0] == 's' && dummy_line[1] == 'o' && dummy_line[2] == 'r' && dummy_line[3] == 't' && dummy_line[4] == ' ')
		{
			std::string arg = dummy_line.substr(4);
			trim(arg);
			sort(creatures, arg);
			used_command = true;
			save_buffer();
			return false;
		}
	}

	if ((dummy_line.size() > 3) && dummy_line[2] == ' ' && dummy_line[1] == 'd' && (dummy_line[0] == 'c' || dummy_line[0] == 'w'))
	{
		used_command = true;
		std::string arg = original_dummy_line.substr(3);
		if (starts_with(arg, BASE_DIRECTORY_PROXY + "/") && directory!="")
		{
			arg = arg.substr(BASE_DIRECTORY_PROXY.size() + 1);
			wd = "";
		}
		wd += "/" + arg;
		wd = replace_all(wd, "//", "/", false);
		wd = replace_all(wd, "\\", "/", false);
		if (wd[wd.size() - 1] == '/' || wd[wd.size() - 1] == '\\')
			wd.resize(wd.size() - 1);
		if (wd[0] == '/')
			wd = wd.substr(1);
		directory = wd;
	}
	else if (dummy_line == "cd")
	{
		used_command = true;
		wd = "";
		directory = "";
	}
	else if (dummy_line == "wd")
	{
		used_command = true;
		if (!suppress_display)
		{
			if (wd == "")
				turn_msg = "Working Directory: " + BASE_DIRECTORY_PROXY + "\n\n";
			else
				turn_msg = "Working Directory: " + BASE_DIRECTORY_PROXY + "/" + wd + "\n\n";
			std::cout << turn_msg;
		}
	}
	else if (dummy_line == "dc")
	{
		try
		{
			turn_msg = "Save DC = " + std::to_string(save_dc) + "\n";
			if (!suppress_display)
				std::cout << turn_msg;
			used_command = true;
		}
		catch (const std::exception& E) {}
	}
	else if (dummy_line.size() >= 2 && dummy_line[0] == '/' && dummy_line[1] == '/')
	{
		used_command = true;
	}
	else if (dummy_line == "wd.." || dummy_line == "cd..")
	{
		used_command = true;
		std::string arg = "..";
		wd += "/" + arg;
		wd = replace_all(wd, "//", "/", false);
		wd = replace_all(wd, "\\", "/", false);
		if (wd[wd.size() - 1] == '/' || wd[wd.size() - 1] == '\\')
			wd.resize(wd.size() - 1);
		if (wd[0] == '/')
			wd = wd.substr(1);
		directory = wd;
	}
	else if (((dummy_line == "wd " + BASE_DIRECTORY_PROXY) || (dummy_line == "cd " + BASE_DIRECTORY_PROXY) || (dummy_line == "cd " + BASE_DIRECTORY_PROXY + "/") || (dummy_line == "wd " + BASE_DIRECTORY_PROXY + "/") || (dummy_line == "cd /" + BASE_DIRECTORY_PROXY + "/") || (dummy_line == "wd / " + BASE_DIRECTORY_PROXY + "/") || (dummy_line == "cd /" + BASE_DIRECTORY_PROXY) || (dummy_line == "wd /" + BASE_DIRECTORY_PROXY)) && directory != "")
	{
		used_command = true;
		wd = "";
		directory = "";
	}
	else if (dummy_line == "ls")
	{
		used_command = true;
		ls(directory, turn_msg,false,false);
	}
	else if (dummy_line == "ls -r" || dummy_line=="ls r" || dummy_line == "ls-r")
	{
		used_command = true;
		ls(directory, turn_msg, true, false);
	}
	else if (dummy_line == "ls -l" || dummy_line == "ls l" || dummy_line=="ls-l")
	{
		used_command = true;
		ls(directory, turn_msg, false, true);
	}
	else if (dummy_line == "ls-lr" || dummy_line == "ls-rl" || dummy_line == "ls -l -r" || dummy_line == "ls -r -l" || dummy_line == "ls -lr" || dummy_line == "ls -rl" || dummy_line == "ls l r" || dummy_line == "ls r l" || dummy_line == "ls lr" || dummy_line == "ls rl")
	{
		used_command = true;
		ls(directory, turn_msg, true, true);
	}
	for (auto i = creatures.begin(); i != creatures.end(); ++i)
		i->touched = false;

		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			if (i->touched)
				continue;
			auto names = i->get_all_names();
			for (auto alias_iterator = names.begin(); alias_iterator != names.end(); ++alias_iterator) //Spaghetti
			{

				std::string lowercase_name = *alias_iterator;

				make_lowercase(lowercase_name);

				if (dummy_line.find(lowercase_name) == std::string::npos)
				{
					continue;
				}
				else if (comp_substring(lowercase_name + " recharge0 ", dummy_line, (lowercase_name + " recharge0 ").length())
					|| comp_substring(lowercase_name + " recharge 0 ", dummy_line, (lowercase_name + " recharge 0 ").length())
					||
					comp_substring("recharge0 " + lowercase_name + " ", dummy_line, ("recharge0 " + lowercase_name + " ").length())
					|| comp_substring("recharge 0 " + lowercase_name + " ", dummy_line, ("recharge 0 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->remove_recharge(flag_name);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge1 ", dummy_line, (lowercase_name + " recharge1 ").length())
					|| comp_substring(lowercase_name + " recharge1-6 ", dummy_line, (lowercase_name + " recharge1-6 ").length())
					||
					comp_substring("recharge1 " + lowercase_name + " ", dummy_line, ("recharge1 " + lowercase_name + " ").length())
					|| comp_substring("recharge1-6 " + lowercase_name + " ", dummy_line, ("recharge1-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 1 ", dummy_line, (lowercase_name + " recharge 1 ").length())
					|| comp_substring(lowercase_name + " recharge 1-6 ", dummy_line, (lowercase_name + " recharge 1-6 ").length())
					||
					comp_substring("recharge 1 " + lowercase_name + " ", dummy_line, ("recharge 1 " + lowercase_name + " ").length())
					|| comp_substring("recharge 1-6 " + lowercase_name + " ", dummy_line, ("recharge 1-6 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->add_recharge(1, flag_name);
					if (!i->has_flag(flag_name))
						i->add_flag(flag_name, true);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge2 ", dummy_line, (lowercase_name + " recharge2 ").length())
					|| comp_substring(lowercase_name + " recharge2-6 ", dummy_line, (lowercase_name + " recharge2-6 ").length())
					||
					comp_substring("recharge2 " + lowercase_name + " ", dummy_line, ("recharge2 " + lowercase_name + " ").length())
					|| comp_substring("recharge2-6 " + lowercase_name + " ", dummy_line, ("recharge2-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 2 ", dummy_line, (lowercase_name + " recharge 2 ").length())
					|| comp_substring(lowercase_name + " recharge 2-6 ", dummy_line, (lowercase_name + " recharge 2-6 ").length())
					||
					comp_substring("recharge 2 " + lowercase_name + " ", dummy_line, ("recharge 2 " + lowercase_name + " ").length())
					|| comp_substring("recharge 2-6 " + lowercase_name + " ", dummy_line, ("recharge 2-6 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->add_recharge(2, flag_name);
					if (!i->has_flag(flag_name))
						i->add_flag(flag_name, true);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge3 ", dummy_line, (lowercase_name + " recharge3 ").length())
					|| comp_substring(lowercase_name + " recharge3-6 ", dummy_line, (lowercase_name + " recharge3-6 ").length())
					||
					comp_substring("recharge3 " + lowercase_name + " ", dummy_line, ("recharge3 " + lowercase_name + " ").length())
					|| comp_substring("recharge3-6 " + lowercase_name + " ", dummy_line, ("recharge3-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 3 ", dummy_line, (lowercase_name + " recharge 3 ").length())
					|| comp_substring(lowercase_name + " recharge 3-6 ", dummy_line, (lowercase_name + " recharge 3-6 ").length())
					||
					comp_substring("recharge 3 " + lowercase_name + " ", dummy_line, ("recharge 3 " + lowercase_name + " ").length())
					|| comp_substring("recharge 3-6 " + lowercase_name + " ", dummy_line, ("recharge 3-6 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->add_recharge(3, flag_name);
					if (!i->has_flag(flag_name))
						i->add_flag(flag_name, true);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge4 ", dummy_line, (lowercase_name + " recharge4 ").length())
					|| comp_substring(lowercase_name + " recharge4-6 ", dummy_line, (lowercase_name + " recharge4-6 ").length())
					||
					comp_substring("recharge4 " + lowercase_name + " ", dummy_line, ("recharge4 " + lowercase_name + " ").length())
					|| comp_substring("recharge4-6 " + lowercase_name + " ", dummy_line, ("recharge4-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 4 ", dummy_line, (lowercase_name + " recharge 4 ").length())
					|| comp_substring(lowercase_name + " recharge 4-6 ", dummy_line, (lowercase_name + " recharge 4-6 ").length())
					||
					comp_substring("recharge 4 " + lowercase_name + " ", dummy_line, ("recharge 4 " + lowercase_name + " ").length())
					|| comp_substring("recharge 4-6 " + lowercase_name + " ", dummy_line, ("recharge 4-6 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->add_recharge(4, flag_name);
					if (!i->has_flag(flag_name))
						i->add_flag(flag_name, true);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge5 ", dummy_line, (lowercase_name + " recharge5 ").length())
					|| comp_substring(lowercase_name + " recharge5-6 ", dummy_line, (lowercase_name + " recharge5-6 ").length())
					||
					comp_substring("recharge5 " + lowercase_name + " ", dummy_line, ("recharge5 " + lowercase_name + " ").length())
					|| comp_substring("recharge5-6 " + lowercase_name + " ", dummy_line, ("recharge5-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 5 ", dummy_line, (lowercase_name + " recharge 5 ").length())
					|| comp_substring(lowercase_name + " recharge 5-6 ", dummy_line, (lowercase_name + " recharge 5-6 ").length())
					||
					comp_substring("recharge 5 " + lowercase_name + " ", dummy_line, ("recharge 5 " + lowercase_name + " ").length())
					|| comp_substring("recharge 5-6 " + lowercase_name + " ", dummy_line, ("recharge 5-6 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->add_recharge(5, flag_name);
					if (!i->has_flag(flag_name))
						i->add_flag(flag_name, true);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge6 ", dummy_line, (lowercase_name + " recharge6 ").length())
					|| comp_substring(lowercase_name + " recharge6-6 ", dummy_line, (lowercase_name + " recharge6-6 ").length())
					||
					comp_substring("recharge6 " + lowercase_name + " ", dummy_line, ("recharge6 " + lowercase_name + " ").length())
					|| comp_substring("recharge6-6 " + lowercase_name + " ", dummy_line, ("recharge6-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 6 ", dummy_line, (lowercase_name + " recharge 6 ").length())
					|| comp_substring(lowercase_name + " recharge 6-6 ", dummy_line, (lowercase_name + " recharge 6-6 ").length())
					||
					comp_substring("recharge 6 " + lowercase_name + " ", dummy_line, ("recharge 6 " + lowercase_name + " ").length())
					|| comp_substring("recharge 6-6 " + lowercase_name + " ", dummy_line, ("recharge 6-6 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->add_recharge(6, flag_name);
					if(!i->has_flag(flag_name))
						i->add_flag(flag_name, true);
					used_command = true;
				}

				else if (comp_substring("clone " + lowercase_name + " ", dummy_line, ("clone " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int clones = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
						i->touched = true;
						i = creatures.begin();
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " clone ", dummy_line, (lowercase_name + " clone ").length()))
				{
					try {
						bool is_signed = false;
						int clones = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
						i->touched = true;
						i = creatures.begin();
					}
					catch (const std::exception& E) {

					}
				}
				else if (dummy_line == ("clone " + lowercase_name))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
					i->touched = true;
					i = creatures.begin();
				}
				else if (comp_substring(lowercase_name + " clone", dummy_line, (lowercase_name + " clone").length()))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
					i->touched = true;
					i = creatures.begin();
				}

				else if (comp_substring(lowercase_name + " tf ", dummy_line, (lowercase_name + " tf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " tf ").length();
						std::string arg = dummy_line.substr(start_length);

						i->add_flag("_" + arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("tf " + lowercase_name + " ", dummy_line, ("tf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("tf " + lowercase_name + " ").length();
						std::string arg = dummy_line.substr(start_length);

						i->add_flag("_" + arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (((lowercase_name + " note") == dummy_line) || (dummy_line == ("note " + lowercase_name)))
				{
					i->set_note("");
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " note ", dummy_line, (lowercase_name + " note ").length()))
				{
					std::string reminder = original_dummy_line.substr((lowercase_name + " note ").length());
					trim(reminder);
					i->set_note(reminder);
					used_command = true;
				}
				else if (comp_substring("note " + lowercase_name + " ", dummy_line, ("note " + lowercase_name + " ").length()))
				{
					std::string reminder = original_dummy_line.substr(("note " + lowercase_name + " ").length());
					trim(reminder);
					i->set_note(reminder);
					used_command = true;
				}

				else if (comp_substring("rv " + lowercase_name + "::", dummy_line, ("rv " + lowercase_name + "::").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + "::").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find("::");
						std::string var = trunc.substr(loc + 2);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + "::", dummy_line, ("rv " + lowercase_name + "::").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + "::").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find("::");
						std::string var = trunc.substr(loc + 2);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rv " + lowercase_name + " ", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find(" ");
						std::string var = trunc.substr(loc);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rv " + lowercase_name + ":", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find(":");
						std::string var = trunc.substr(loc + 1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + ":", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find(":");
						std::string var = trunc.substr(loc + 1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rv " + lowercase_name + ".", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find(".");
						std::string var = trunc.substr(loc+1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + ".", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find(".");
						std::string var = trunc.substr(loc+1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + " ", dummy_line, ("dv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find(" ");
						std::string var = trunc.substr(loc);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " ra ", dummy_line, (lowercase_name + " ra ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " ra ").length();
						std::string arg = dummy_line.substr(start_length);
						if (arg.size() > 0 && arg[0] != '@')
							if (i->remove_alias(arg))
								used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring("ra " + lowercase_name + " ", dummy_line, (lowercase_name + " ra ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " ra ").length();
						std::string arg = dummy_line.substr(start_length);
						if (arg.size() > 0 && arg[0] != '@')
							if (i->remove_alias(arg))
								used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring("swap " + lowercase_name + " ", dummy_line, ("swap " + lowercase_name + " ").length()))
				{
					int len = lowercase_name.size() + 5;
					if (dummy_line.size() > len)
					{
						std::string swap_partner_name = dummy_line.substr(len);
						auto get_creature_from_name = [&](const std::string& sname) -> creature*
							{
								for (auto si = creatures.begin(); si != creatures.end(); ++si)
								{
									if (si->has_alias(sname))
										return si->get_raw_ptr();
								}
								return nullptr;
							};
						trim(swap_partner_name);
						creature* swap_partner = get_creature_from_name(swap_partner_name);
						if (swap_partner)
						{
							used_command = true;
							int my_init = i->get_initiative();
							int their_init = swap_partner->get_initiative();
							i->set_initiative(their_init);
							swap_partner->set_initiative(my_init);
							creatures.sort();
							break;
						}
					}
				}

				else if (comp_substring("if " + lowercase_name + " ", dummy_line, ("if " + lowercase_name + " ").size()))
				{
					int cmd_len = ("if " + lowercase_name + " ").size();
					std::string sub = dummy_line.substr(cmd_len);
					trim(sub);
					std::string og_sub = sub;
					int space = sub.find(" ");
					sub.resize(space);
					if (i->has_flag(sub))
					{
						if (dummy_line.find("{") == std::string::npos)
						{
							sub = og_sub;
							space = sub.find(" ");
							sub = sub.substr(space);
							trim(sub);
							std::string filename = sub;

							if (directory != "")
							{
								filename = directory + "/" + filename;
							}
							else
							{
								if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
								{
									size_t backi = filename.size() - 1;
									while (filename[backi] != '/' && filename[backi] != '\\')
									{
										--backi;
									}
									directory = filename;
									directory.resize(backi);
									if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
										wd = directory;
								}
							}
							std::ifstream new_file;
							process_filename(filename);
							dir_fix(filename);
							search_links(filename);
							new_file.open(filename);
							if (!new_file.is_open())
							{
								std::cout << "Error: Could not open " << filename << std::endl;
								return false;
							}
							else {
								while (new_file.good() && !new_file.eof())
								{
									get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
								}
								ignore_initial_file_load = true;
								new_file.close();
								save_buffer();
								return false;
							}
						}
						else
						{
							sub = og_sub;
							int index = sub.find("{");
							sub = sub.substr(index);
							trim(sub);
							if (sub[sub.size() - 1] == '}')
								sub[sub.size() - 1] = ' ';
							if (sub[0] == '{')
								sub[0] = ' ';
							trim(sub);
							std::ifstream file;
							bool dummy_taking_initiatives = true;
							bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
							used_command = true;
							if (success)
							{
								creatures.sort();
							}
							save_buffer();
							return false;
						}
					}
					else
						used_command = true;
				}

				else if (comp_substring(lowercase_name + " if ", dummy_line, (lowercase_name + " if ").size()))
				{
					int cmd_len = (lowercase_name + " if ").size();
					std::string sub = dummy_line.substr(cmd_len);
					trim(sub);
					std::string og_sub = sub;
					int space = sub.find(" ");
					sub.resize(space);
					if (i->has_flag(sub))
					{
						if (dummy_line.find("{") == std::string::npos)
						{
							sub = og_sub;
							space = sub.find(" ");
							sub = sub.substr(space);
							trim(sub);
							std::string filename = sub;

							if (directory != "")
							{
								filename = directory + "/" + filename;
							}
							else
							{
								if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
								{
									size_t backi = filename.size() - 1;
									while (filename[backi] != '/' && filename[backi] != '\\')
									{
										--backi;
									}
									directory = filename;
									directory.resize(backi);
									if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
										wd = directory;
								}
							}
							std::ifstream new_file;
							process_filename(filename);
							dir_fix(filename);
							search_links(filename);
							new_file.open(filename);
							if (!new_file.is_open())
							{
								std::cout << "Error: Could not open " << filename << std::endl;
								//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
								return false;
							}
							else {
								while (new_file.good() && !new_file.eof())
								{
									get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
								}
								ignore_initial_file_load = true;
								new_file.close();
								save_buffer();
								return false;
							}
						}
						else
						{
							sub = og_sub;
							int index = sub.find("{");
							sub = sub.substr(index);
							trim(sub);
							if (sub[sub.size() - 1] == '}')
								sub[sub.size() - 1] = ' ';
							if (sub[0] == '{')
								sub[0] = ' ';
							trim(sub);
							std::ifstream file;
							bool dummy_taking_initiatives = true;
							bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
							used_command = true;
							if (success)
							{
								creatures.sort();
							}
							save_buffer();
							return false;
						}
					}
					else
						used_command = true;
				}

				else if (comp_substring(lowercase_name + " dv ", dummy_line, (lowercase_name + " dv ").length()))
				{
					try {
						size_t start_length = lowercase_name.length() + 4;
						std::string var = dummy_line.substr(start_length);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " rv ", dummy_line, (lowercase_name + " dv ").length()))
				{
					try {
						size_t start_length = lowercase_name.length() + 4;
						std::string var = dummy_line.substr(start_length);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (
					comp_substring("hide " + lowercase_name, dummy_line, ("hide " + lowercase_name).length())
					)
				{
					try {
						size_t start_length = lowercase_name.length() + 5;
						std::string var = dummy_line.substr(start_length);
						trim(var);
						if (var.size() >= 2 && var[0] == ':')
							var = var.substr(1);
						if (var.size() >= 2 && var[0] == ':')
							var = var.substr(1);

						if (var.size() >= 2 && var[0] == '.')
							var = var.substr(1);
						var = resolve_var_name(var, creatures, *i);
						i->hide_var(var, creatures);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (
					comp_substring("show " + lowercase_name, dummy_line, ("show " + lowercase_name).length())
					)
					{
						try {
							size_t start_length = lowercase_name.length() + 5;
							std::string var = dummy_line.substr(start_length);
							trim(var);
							if (var.size() >= 2 && var[0] == ':')
								var = var.substr(1);
							if (var.size() >= 2 && var[0] == ':')
								var = var.substr(1);

							if (var.size() >= 2 && var[0] == '.')
								var = var.substr(1);
							var = resolve_var_name(var, creatures, *i);
							i->show_var(var, creatures);

							used_command = true;
						}
						catch (const std::exception& E) {

						}
						}
				
				
				else if (comp_substring(lowercase_name + "::", dummy_line, (lowercase_name + "::").length()))
				{
					int last = dummy_line.size() - 1;
					if (dummy_line.size() >= 8 && dummy_line[last] == 'e' && dummy_line[last - 1] == 'd' && dummy_line[last - 2] == 'i' && dummy_line[last - 3] == 'h')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 2);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->hide_var(var, creatures);
						used_command = true;
					}
					else if (dummy_line.size() >= 8 && dummy_line[last] == 'w' && dummy_line[last - 1] == 'o' && dummy_line[last - 2] == 'h' && dummy_line[last - 3] == 's')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 2);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->show_var(var, creatures);
						used_command = true;
					}
					else
					{
						//CURRENT
						try {
							std::string var = dummy_line.substr(lowercase_name.length() + 2);
							//if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
								//var[lowercase_name.length()] = ' ';
							int space = std::string::npos;
							int SET_TYPE = VAR_SET;
							used_command = true;
							std::string stow = "";
							if (var.find("{") != std::string::npos)
							{
								int code_index = var.find("{");
								stow = var.substr(code_index);
								var.resize(code_index);
								var += "TEMP";
							}
							if (space == std::string::npos)
							{
								space = var.find(" != ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("!=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" >= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" <= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" > ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" < ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" == ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("==");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" += ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("+=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" *= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" /= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_DIVIDE;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" -= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_SUB;
								}

							}


							if (space == std::string::npos)
							{
								space = var.find(" = ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("=");
								if (space != std::string::npos)
									var[space] = ' ';
							}


							if (space == std::string::npos)
							{
								space = var.find("--");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_DECREMENT;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("++");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_INCREMENT;
								}
							}

							if (space == std::string::npos)
								space = var.find(" ");

							bool is_signed = false;
							int val = 1;
							if (stow != "")
								var.resize(var.size()-4);
							if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
								val = get_number_arg(var, is_signed, creatures, i->get_raw_ptr());
							if (stow != "")
							{
								var += stow;
							}
							std::string og_var = var;
							
							var.resize(space);
							var = resolve_var_name(var, creatures, *i);
							switch (SET_TYPE)
							{
							case VAR_ADD: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_SUB: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_SET: i->set_var(var, val); break;
							case VAR_MULTIPLY: i->set_var(var, i->get_var(var, creatures) * val); break;
							case VAR_DIVIDE: i->set_var(var, i->get_var(var, creatures) / val); break;
							case VAR_INCREMENT: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_DECREMENT: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_EQUAL_TO: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) == val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);
									
									if (i->get_var(var, creatures) == val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}
							case VAR_GREATER_THAN: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) > val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) > val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_GREATER_THAN_OR_EQUAL: {
								std::string sub = og_var;
								//CURRENT PLACE
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);

									if (i->get_var(var, creatures) >= val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);
									if (i->get_var(var, creatures) >= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_LESS_THAN: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) < val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) < val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}
							case VAR_LESS_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) <= val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) <= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_NOT_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) != val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) != val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}


							default: throw;
							}
							used_command = true;
						}
						catch (const std::exception& E) {
							std::cout << E.what() << std::endl;
						}
					}
					
				}
				else if (comp_substring("--" + lowercase_name + "::", dummy_line, ("--" + lowercase_name + "::").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 4)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 4);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) - 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) - 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("++" + lowercase_name + "::", dummy_line, ("++" + lowercase_name + "::").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 4)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 4);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) + 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}


				else if (comp_substring(lowercase_name + ".", dummy_line, (lowercase_name + ".").length()))
				{
					int last = dummy_line.size() - 1;
					if (dummy_line.size() >= 7 && dummy_line[last] == 'e' && dummy_line[last - 1] == 'd' && dummy_line[last - 2] == 'i' && dummy_line[last - 3] == 'h')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->hide_var(var, creatures);
						used_command = true;
					}
					else if (dummy_line.size() >= 7 && dummy_line[last] == 'w' && dummy_line[last - 1] == 'o' && dummy_line[last - 2] == 'h' && dummy_line[last - 3] == 's')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->show_var(var, creatures);
						used_command = true;
					}
					else
					{
						try {
							used_command = true;
							std::string var = dummy_line.substr(lowercase_name.length() + 1);
							if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
								var[lowercase_name.length()] = ' ';
							int space = std::string::npos;
							int SET_TYPE = VAR_SET;
							std::string stow = "";
							if (var.find("{") != std::string::npos)
							{
								int code_index = var.find("{");
								stow = var.substr(code_index);
								var.resize(code_index);
								var += "TEMP";
							}
							if (space == std::string::npos)
							{
								space = var.find(" != ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("!=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" >= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" <= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" > ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" < ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" == ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("==");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" += ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("+=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" -= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_SUB;
								}

							}


							if (space == std::string::npos)
							{
								space = var.find(" *= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" /= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_DIVIDE;
								}
							}


							if (space == std::string::npos)
							{
								space = var.find(" = ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("=");
								if (space != std::string::npos)
									var[space] = ' ';
							}


							if (space == std::string::npos)
							{
								space = var.find("--");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_DECREMENT;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("++");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_INCREMENT;
								}
							}

							if (space == std::string::npos)
								space = var.find(" ");

							bool is_signed = false;
							int val = 1;
							if (stow != "")
								var.resize(var.size() - 4);
							if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
								val = get_number_arg(var, is_signed, creatures, i->get_raw_ptr());
							if (stow != "")
							{
								var += stow;
							}

							std::string og_var = var; //Right before var is resized
							var.resize(space);
							var = resolve_var_name(var, creatures, *i);
							switch (SET_TYPE)
							{
							case VAR_ADD: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_SUB: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_SET: i->set_var(var, val); break;
							case VAR_INCREMENT: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_DECREMENT: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_MULTIPLY: i->set_var(var, i->get_var(var, creatures) * val); break;
							case VAR_DIVIDE: i->set_var(var, i->get_var(var, creatures) / val); break;

							case VAR_EQUAL_TO: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) == val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) == val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}
							case VAR_GREATER_THAN: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) > val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) > val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_GREATER_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) >= val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) >= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_LESS_THAN: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) < val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) < val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}
							case VAR_LESS_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) <= val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) <= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_NOT_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) != val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) != val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							default: throw;
							}
							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}
					
				}
				else if (comp_substring("--" + lowercase_name + ".", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) - 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) - 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("++" + lowercase_name + ".", dummy_line, ("++" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) + 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + ":", dummy_line, (lowercase_name + ":").length()))
				{
					int last = dummy_line.size() - 1;
					if (dummy_line.size() >= 7 && dummy_line[last] == 'e' && dummy_line[last - 1] == 'd' && dummy_line[last - 2] == 'i' && dummy_line[last - 3] == 'h')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->hide_var(var, creatures);
						used_command = true;
					}
					else if (dummy_line.size() >= 7 && dummy_line[last] == 'w' && dummy_line[last - 1] == 'o' && dummy_line[last - 2] == 'h' && dummy_line[last - 3] == 's')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->show_var(var, creatures);
						used_command = true;
					}
					else
					{
						try {
							used_command = true;
							std::string var = dummy_line.substr(lowercase_name.length() + 1);
							if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
								var[lowercase_name.length()] = ' ';
							int space = std::string::npos;
							
							int SET_TYPE = VAR_SET;
							std::string stow = "";
							if (var.find("{") != std::string::npos)
							{
								int code_index = var.find("{");
								stow = var.substr(code_index);
								var.resize(code_index);
								var += "TEMP";
							}
							if (space == std::string::npos)
							{
								space = var.find(" != ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("!=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" >= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" <= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" > ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" < ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" == ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("==");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" += ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("+=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" -= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_SUB;
								}

							}

							if (space == std::string::npos)
							{
								space = var.find(" *= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" /= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_DIVIDE;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" = ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("=");
								if (space != std::string::npos)
									var[space] = ' ';
							}


							if (space == std::string::npos)
							{
								space = var.find("--");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_DECREMENT;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("++");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_INCREMENT;
								}
							}

							if (space == std::string::npos)
								space = var.find(" ");

							bool is_signed = false;
							int val = 1;
							if (stow != "")
								var.resize(var.size() - 4);
							if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
								val = get_number_arg(var, is_signed, creatures, i->get_raw_ptr());
							if (stow != "")
							{
								var += stow;
							}
							std::string og_var = var;
							var.resize(space);
							var = resolve_var_name(var, creatures, *i);
							switch (SET_TYPE)
							{
							case VAR_ADD: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_SUB: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_SET: i->set_var(var, val); break;
							case VAR_INCREMENT: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_DECREMENT: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_MULTIPLY: i->set_var(var, i->get_var(var, creatures) * val); break;
							case VAR_DIVIDE: i->set_var(var, i->get_var(var, creatures) / val); break;
							case VAR_EQUAL_TO: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) == val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) == val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}
							case VAR_GREATER_THAN: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) > val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) > val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_GREATER_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) >= val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) >= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_LESS_THAN: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) < val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) < val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}
							case VAR_LESS_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) <= val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) <= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							case VAR_NOT_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) != val)
									{
										std::string filename = sub;

										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
												if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
													wd = directory;
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										dir_fix(filename);
										search_links(filename);
										new_file.open(filename);
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
											return false;
										}
										else {
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											ignore_initial_file_load = true;
											new_file.close();
											save_buffer();
											return false;
										}

									}

								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) != val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, directory, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}
								break;
							}

							default: throw;
							}
							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}
				}
				else if (comp_substring("--" + lowercase_name + ":", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) - 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) - 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("++" + lowercase_name + ":", dummy_line, ("++" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) + 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}

			}
		}
	

	if(dummy_line.size() > 5 && dummy_line[0]=='s' && dummy_line[1]=='o' && dummy_line[2] == 'r' && dummy_line[3] == 't' && dummy_line[4] == ' ')
	{
		std::string arg = dummy_line.substr(4);
		trim(arg);
		sort(creatures, arg);
		used_command = true;
	}

	if (takes_commands && !used_command) //In hindsight this is an awful way to parse commands.
	{
		std::string dummy_line = line;
		make_lowercase(dummy_line);
		std::string removal_name = "";
		std::string keep_name = "";
		bool start_over = false;
		size_t len = dummy_line.length();
		size_t l = len - 1;
		if (dummy_line == "simple display")
		{
			simple_display = true;
			used_command = true;
		}
		else if (dummy_line == "full display")
		{
			simple_display = false;
			used_command = true;
		}
		else if (dummy_line == "dispt" || dummy_line == "tdisp" || dummy_line == "disp" || dummy_line == "toggle display" || dummy_line == "toggle disp" || dummy_line == "display toggle" || dummy_line == "disp toggle")
		{
			simple_display = !simple_display;
			used_command = true;
		}
		else if (comp_substring(dummy_line, "rm ", 3))
		{
			removal_name = line.substr(2);
			trim(removal_name);
			make_lowercase(removal_name);
		}
		else if (dummy_line.size() > 3 && comp_substring(dummy_line, "dc ", 3))
		{
			try
			{
				bool is_signed = false;
				save_dc = get_number_arg(dummy_line, is_signed, creatures, nullptr);
				used_command = true;
			}
			catch (const std::exception& E) {}
		}
		else if (dummy_line.size()>6 && comp_substring(dummy_line, "print ", 6))
		{
			std::cout << line.substr(6);
			used_command = true;
			turn_msg += line.substr(6);
			turn_msg += "\n";
			return false;
		}
		else if (dummy_line.size() > 6 && comp_substring(dummy_line, "printnum ", 6))
		{
			bool is_signed = false;
			int num = get_number_arg(dummy_line, is_signed, creatures, nullptr);
			used_command = true;
			std::cout << num << std::endl;
			turn_msg += std::to_string(num);
			turn_msg += "\n";
			return false;
		}
		else if (comp_substring(dummy_line, "remove ", 7))
		{
			removal_name = line.substr(6);
			trim(removal_name);
			make_lowercase(removal_name);
		}
		else if (len>3 && dummy_line[l] == 'm' && dummy_line[l - 1] == 'r' && dummy_line[l-2] == ' ')
		{
			removal_name = dummy_line;
			removal_name.resize(len - 3);
		}
		else if (len > 7 && dummy_line[l] == 'e' && dummy_line[l - 1] == 'v' && dummy_line[l - 2] == 'o' && dummy_line[l - 3] == 'm' && dummy_line[l - 4] == 'e' && dummy_line[l - 5] == 'r' && dummy_line[l - 6] == ' ')
		{
			removal_name = dummy_line;
			removal_name.resize(len - 7);
		}
		if (removal_name != "")
		{
			auto rmc = creatures.begin();
			while (creatures.size() != 0 && rmc != creatures.end())
			{
				if (rmc->has_alias(removal_name))
				{
					rmc->remove_alias("@current");
					creatures.erase(rmc);
					rmc = creatures.begin();
					rmc->remove_alias("@current");
				}
				else
				{
					rmc->remove_alias("@current");
					++rmc;
					rmc->remove_alias("@current");
				}
			}
			save_buffer();
			return false;
		}

		if (comp_substring(dummy_line, "keep ", 5))
		{
			keep_name = line.substr(5);
			trim(keep_name);
			make_lowercase(keep_name);
		}
		if (keep_name != "")
		{
			auto rmc = creatures.begin();
			while (creatures.size() != 0 && rmc != creatures.end())
			{
				if (!(rmc->has_alias(keep_name)))
				{
					rmc->remove_alias("@current");
					creatures.erase(rmc);
					rmc = creatures.begin();
					rmc->remove_alias("@current");
				}
				else
				{
					rmc->remove_alias("@current");
					++rmc;
					rmc->remove_alias("@current");
				}
			}
			save_buffer();
			return false;
		}

		std::string& original_dummy_line = line;
		bool did_erase = false;
		//remove_executor_tags(creatures);
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			if (start_over)
			{
				start_over = false;
				i = creatures.begin();
				//remove_executor_tags(creatures);
			}
			//add_executor_tag(creatures, *i);
			auto names = i->get_all_names();
			for (auto alias_iterator = names.begin(); alias_iterator != names.end(); ++alias_iterator) //Spaghetti
			{
				std::string lowercase_name = *alias_iterator;
				make_lowercase(lowercase_name);
				if (dummy_line.find(lowercase_name) == std::string::npos && dummy_line.find(" all")==std::string::npos && dummy_line.find("save ") == std::string::npos && dummy_line.find("sv ") && dummy_line.find("roll ") == std::string::npos)
				{
					continue;
				}

				if (
					comp_substring("move " + lowercase_name + " ", dummy_line, ("move " + lowercase_name + " ").length()) ||
					comp_substring("mv " + lowercase_name + " ", dummy_line, ("mv " + lowercase_name + " ").length()) ||
					comp_substring("mod " + lowercase_name + " ", dummy_line, ("mod " + lowercase_name + " ").length()) ||
					comp_substring("md " + lowercase_name + " ", dummy_line, ("md " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						if (!i->touched)
						{
							i->set_initiative(val);
							i->touched = true;
							creatures.sort();
							i = creatures.begin();
							start_over = true;
							break;
						}
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("ac " + lowercase_name + " ", dummy_line, ("ac " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int new_ac = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						i->set_ac(new_ac);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " ac ", dummy_line, (lowercase_name + " ac ").length()))
				{
					try {
						bool is_signed = false;
						int new_ac = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						i->set_ac(new_ac);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (((lowercase_name + " reminder") == dummy_line) || (dummy_line == ("reminder " + lowercase_name)))
				{
					i->set_reminder("");
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " reminder ", dummy_line, (lowercase_name + " reminder ").length()))
				{
					std::string reminder = original_dummy_line.substr((lowercase_name + " reminder ").length());
					trim(reminder);
					i->set_reminder(reminder);
					used_command = true;
				}
				else if (comp_substring("reminder " + lowercase_name + " ", dummy_line, ("reminder " + lowercase_name + " ").length()))
				{
					std::string reminder = original_dummy_line.substr(("reminder " + lowercase_name + " ").length());
					trim(reminder);
					i->set_reminder(reminder);
					used_command = true;
				}


				else if (((lowercase_name + " start") == dummy_line) || (dummy_line == ("start " + lowercase_name)))
				{
					i->turn_start_file = "";
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " start ", dummy_line, (lowercase_name + " start ").length()))
				{
					std::string filename = original_dummy_line.substr((lowercase_name + " start ").length());
					trim(filename);
					i->turn_start_file = filename;
					used_command = true;
				}
				else if (dummy_line == lowercase_name + " str_save" || dummy_line == "str_save " + lowercase_name)
				{
					int save = i->get_str_bonus();
					std::string save_name = "str_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_"+save_name+"_success",true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
				}
				else if (dummy_line == lowercase_name + " dex_save" || dummy_line == "dex_save " + lowercase_name)
				{
					int save = i->get_dex_bonus();
					std::string save_name = "dex_save";
					save += 1 + (rand() % 20);
					if (i->has_flag("danger_sense"))
					{
						int reroll = 1 + (rand() % 20) + i->get_dex_bonus();
						if (reroll > save)
							save = reroll;
					}
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
				}
				else if (dummy_line == lowercase_name + " con_save" || dummy_line == "con_save " + lowercase_name)
				{
					int save = i->get_con_bonus(creatures);
					std::string save_name = "con_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
				}
				else if (dummy_line == lowercase_name + " int_save" || dummy_line == "int_save " + lowercase_name)
				{
					int save = i->get_int_bonus();
					std::string save_name = "int_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
				}
				else if (dummy_line == lowercase_name + " wis_save" || dummy_line == "wis_save " + lowercase_name)
				{
					int save = i->get_wis_bonus();
					std::string save_name = "wis_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
					}
				else if (dummy_line == lowercase_name + " cha_save" || dummy_line == "cha_save " + lowercase_name)
				{
					int save = i->get_cha_bonus();
					std::string save_name = "cha_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
				}
				else if (comp_substring("str_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_str_bonus();
						std::string save_name = "str_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						
						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring("dex_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_dex_bonus();
						std::string save_name = "dex_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (i->has_flag("danger_sense"))
						{
							int reroll = 1 + (rand() % 20) + i->get_dex_bonus();
							if (reroll > save)
								save = reroll;
						}
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
							if (i->has_evasion())
								dmg = 0;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
							if (i->has_evasion())
								dmg /= 2;
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring("con_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_con_bonus(creatures);
						std::string save_name = "con_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring("int_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_int_bonus();
						std::string save_name = "int_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring("wis_save " + lowercase_name + " ", dummy_line, ("wis_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_wis_bonus();
						std::string save_name = "wis_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring("cha_save " + lowercase_name + " ", dummy_line, ("cha_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_cha_bonus();
						std::string save_name = "cha_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + " str_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_str_bonus();
						std::string save_name = "str_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + " con_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_con_bonus(creatures);
						std::string save_name = "con_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + " dex_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_dex_bonus();
						std::string save_name = "dex_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (i->has_flag("danger_sense"))
						{
							int reroll = 1 + (rand() % 20) + i->get_dex_bonus();
							if (reroll > save)
								save = reroll;
						}
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
							if (i->has_evasion())
								dmg = 0;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
							if (i->has_evasion())
								dmg /= 2;
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + " int_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_int_bonus();
						std::string save_name = "int_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + " wis_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_wis_bonus();
						std::string save_name = "wis_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + " cha_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_cha_bonus();
						std::string save_name = "cha_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}


				else if (comp_substring("start " + lowercase_name + " ", dummy_line, ("start " + lowercase_name + " ").length()))
				{
					std::string filename = original_dummy_line.substr(("start " + lowercase_name + " ").length());
					trim(filename);
					i->turn_start_file = filename;
					used_command = true;
				}
				else if (((lowercase_name + " end") == dummy_line) || (dummy_line == ("end " + lowercase_name)))
				{
					i->turn_end_file = "";
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " end ", dummy_line, (lowercase_name + " end ").length()))
				{
					std::string filename = original_dummy_line.substr((lowercase_name + " end ").length());
					trim(filename);
					i->turn_end_file = filename;
					used_command = true;
				}
				else if (comp_substring("end " + lowercase_name + " ", dummy_line, ("end " + lowercase_name + " ").length()))
				{
					std::string filename = original_dummy_line.substr(("end " + lowercase_name + " ").length());
					trim(filename);
					i->turn_end_file = filename;
					used_command = true;
				}

				else if (comp_substring("heal " + lowercase_name + " ", dummy_line, ("heal " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " heal ", dummy_line, (lowercase_name + " heal ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						i->adjust_hp(val);
						used_command = true;
					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
				}

				else if (comp_substring("hurt " + lowercase_name + " ", dummy_line, ("hurt " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " hurt ", dummy_line, (lowercase_name + " hurt ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						i->adjust_hp(-val);
						used_command = true;
					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
				}

				else if (comp_substring("rf " + lowercase_name + " ", dummy_line, ("rf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rf " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->remove_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}

				else if (
					comp_substring("alias " + lowercase_name + " ", dummy_line, ("alias " + lowercase_name + " ").length())
					)
				{
					std::string new_alias = dummy_line.substr(("alias " + lowercase_name + " ").length());
					trim(new_alias);
					if (name_is_unique(new_alias, creatures))
					{
						i->add_alias(new_alias);
					}
				}

				else if (comp_substring(lowercase_name + " rf ", dummy_line, (lowercase_name + " rf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rf ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->remove_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("f " + lowercase_name + " ", dummy_line, ("f " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("f " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg, true);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " f ", dummy_line, (lowercase_name + " f ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " f ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg, true);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("flag " + lowercase_name + " ", dummy_line, ("flag " + lowercase_name + " ").length()) )
				{
					try {
						size_t start_length = ("flag " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg, true);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " flag ", dummy_line, (lowercase_name + " flag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flag ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg, true);
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rf " + lowercase_name + " ", dummy_line, ("rf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rf " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " rf ", dummy_line, (lowercase_name + " rf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rf ").length();
						std::string arg =line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("heal all ", dummy_line, 9) || dummy_line == "heal all max" || dummy_line == "heal all all" || dummy_line == "heal all full")
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						unsigned char* tval = reinterpret_cast<unsigned char*>(&(i->touched));
						if (!i->touched)
						{
							i->adjust_hp(val);
							i->touched = true;
						}

						if (i == (--creatures.end()))
						{
							save_buffer();
							return false;
						}

					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
				}

				else if (comp_substring("temp_hp " + lowercase_name + " ", dummy_line, ("temp_hp " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
				
					if (val < 0)
					{
						if (!suppress_display)
							std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring(lowercase_name + " temp_hp ", dummy_line, (lowercase_name + " temp_hp ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

					if (val < 0)
					{
						if (!suppress_display)
							std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}

				else if (comp_substring("regen " + lowercase_name + " ", dummy_line, ("regen " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
					//if (val < 0)
						//val = 0;
					i->set_regen(val);
					used_command = true;
				}

				else if (comp_substring(lowercase_name + " regen ", dummy_line, (lowercase_name + " regen ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
					//if (val < 0)
						//val = 0;
					i->set_regen(val);
					used_command = true;
				}

				else if (comp_substring(lowercase_name + " disable", dummy_line, (lowercase_name + " disable").length()))
				{
					i->disable_regen_temp();
					used_command = true;
				}

				else if (comp_substring("disable " + lowercase_name, dummy_line, ("disable " + lowercase_name).length()))
				{
					i->disable_regen_temp();
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " hp ", dummy_line, (lowercase_name + " hp ").length()) ||
					comp_substring("hp " + lowercase_name + " ", dummy_line, ("hp " + lowercase_name + " ").length())
					)
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
					try {
						size_t slash_index = dummy_line.find("/");
						if (slash_index != std::string::npos)
						{
							try {
								int max_hp = std::stoi(dummy_line.substr(slash_index + 1));
								i->set_max_hp(max_hp, false);
							}
							catch (const std::exception& e)
							{
								std::cout << "Could not parse new Max HP - only changing current HP\n";
							}

						}
						else
						{
							
							i->set_max_hp(val, false);
						}
						i->set_hp(val, is_signed);
						used_command = true;
						//break;
					}
					catch (const std::exception& E) {

					}
				}
			}
		}
		//remove_executor_tags(creatures);
	}
	trim(line);
	if (may_expect_add_keyword && line.size() > 4 && get_lowercase(line.substr(0, 4)) == "add ")
	{
		line = line.substr(4);
	}
	
	if (!used_command && line != "")
	{
		int max_hp = -1;
		int hp = -1;
		int temp_hp = 0;
		
		lowercase = get_lowercase(line);
		if (lowercase == "quit" || lowercase == "end" || lowercase == "stop" || lowercase == "terminate" || lowercase == "finish" || lowercase == "leave" || lowercase == "close")
			exit(0);
		std::string flags;
		index_t flags_index = lowercase.find("flags:");
		if (flags_index != std::string::npos)
		{
			size_t space_after_flags_index = lowercase.find(' ', flags_index);
			size_t length = space_after_flags_index - flags_index;
			flags = line.substr(flags_index+6,length-6);
			if (space_after_flags_index == std::string::npos)
			{
				space_after_flags_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_flags_index + 1, lowercase.length() - space_after_flags_index - 1);
			line =      line.     substr(0, flags_index) + line.substr     (space_after_flags_index + 1, line     .length() - space_after_flags_index - 1);
			trim(lowercase);
			trim(line);
		}

		flags_index = lowercase.find("flag:");
		if ((flags_index != std::string::npos) && (flags.length() == 0))
		{
			size_t space_after_flags_index = lowercase.find(' ', flags_index);
			size_t length = space_after_flags_index - flags_index;
			flags = line.substr(flags_index + 5, length - 5);
			if (space_after_flags_index == std::string::npos)
			{
				space_after_flags_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_flags_index + 1, lowercase.length() - space_after_flags_index - 1);
			line = line.		  substr(0, flags_index) + line.	 substr(space_after_flags_index + 1, line.	   length() - space_after_flags_index - 1);
			trim(lowercase);
			trim(line);
		}

		flags_index = lowercase.find("fl:");
		if ((flags_index != std::string::npos) && (flags.length()==0))
		{
			size_t space_after_flags_index = lowercase.find(' ', flags_index);
			size_t length = space_after_flags_index - flags_index;
			flags = line.substr(flags_index + 3, length - 3);
			if (space_after_flags_index == std::string::npos)
			{
				space_after_flags_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_flags_index + 1, lowercase.length() - space_after_flags_index - 1);
			line =		line.	  substr(0, flags_index) + line.	 substr(space_after_flags_index + 1, line.	   length() - space_after_flags_index - 1);
			trim(lowercase);
			trim(line);
		}

		flags_index = lowercase.find("f:");
		if ( (flags_index != std::string::npos) && (flags.length()==0))
		{
			size_t space_after_flags_index = lowercase.find(' ', flags_index);
			size_t length = space_after_flags_index - flags_index;
			flags = line.substr(flags_index + 2, length - 2);
			if (space_after_flags_index == std::string::npos)
			{
				space_after_flags_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_flags_index + 1, lowercase.length() - space_after_flags_index - 1);
			line =		line.     substr(0, flags_index) + line.     substr(space_after_flags_index + 1, line     .length() - space_after_flags_index - 1);
			trim(lowercase);
			trim(line);
		}

		index_t& regen_index = flags_index;

		regen_index = lowercase.find("regen:");
		int regen_amnt = 0;
		if ((regen_index != std::string::npos) && regen_amnt==0)
		{
			size_t space_after_regen_index = lowercase.find(' ', regen_index);
			size_t length = space_after_regen_index - regen_index;
			std::string regen_string = line.substr(regen_index + 6, length - 6);
			if (space_after_regen_index == std::string::npos)
			{
				space_after_regen_index = lowercase.length() - 1;
			}
			try
			{
				regen_amnt = std::stoi(regen_string);
				//if (regen_amnt < 0)
					//throw;
				lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_regen_index + 1, lowercase.length() - space_after_regen_index - 1);
				line =		line.	  substr(0, flags_index) + line.	 substr(space_after_regen_index + 1, line.	   length() - space_after_regen_index - 1);
				trim(lowercase);
				trim(line);
			}
			catch (const std::exception& E)
			{
				std::cout << "Regeneration amount must be a positive number" << std::endl;
				regen_amnt = 0;
				return false;
			}
		}

		std::string aliases = "";
		index_t alias_index = std::string::npos;
		alias_index = lowercase.find("alias:");
		if ((alias_index != std::string::npos) && (aliases.length() == 0))
		{
			size_t space_after_alias_index = lowercase.find(' ', alias_index);
			size_t length = space_after_alias_index - alias_index;
			if (space_after_alias_index == std::string::npos)
			{
				space_after_alias_index = lowercase.length() - 1;
			}
			aliases = line.substr(alias_index + 6, length - 6);
			lowercase = lowercase.substr(0, alias_index) + lowercase.substr(space_after_alias_index + 1, lowercase.length() - space_after_alias_index - 1);
			line =		line	 .substr(0, alias_index) +		line.substr(space_after_alias_index + 1, line	.  length() - space_after_alias_index - 1);
			trim(lowercase);
			trim(line);
		}
		alias_index = std::string::npos;
		alias_index = lowercase.find("aliases:");
		if ((alias_index != std::string::npos) && (aliases.length() == 0))
		{
			size_t space_after_alias_index = lowercase.find(' ', alias_index);
			if (space_after_alias_index == std::string::npos)
			{
				space_after_alias_index = lowercase.length() - 1;
			}
			size_t length = space_after_alias_index - alias_index;
			aliases = line.substr(alias_index + 8, length - 7);
			
			lowercase = lowercase.substr(0, alias_index) + lowercase.substr(space_after_alias_index + 1, lowercase.length() - space_after_alias_index - 1);
			line =		line.	  substr(0, alias_index) + line		.substr(space_after_alias_index + 1, line	  .length() - space_after_alias_index - 1);
			trim(lowercase);
			trim(line);
		}

		index_t& temp_hp_index = flags_index;
		temp_hp_index = lowercase.find("temp:");
		if (temp_hp_index != std::string::npos && temp_hp==0)
		{
			size_t space_after_index = lowercase.find(' ', temp_hp_index);
			size_t length = space_after_index - temp_hp_index;
			std::string sub = line.substr(temp_hp_index + 5, length - 5);
			try
			{
				temp_hp = std::stoi(sub);
				if (temp_hp < 0)
				{
					temp_hp = 0;
					throw;
				}
			}
			catch (const std::exception& E)
			{
				std::cout << "Temp HP must be a positive number" << std::endl;
				return false;
			}
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
			line =		line.substr		(0, flags_index) + line.	 substr(space_after_index + 1, line		.length() - space_after_index - 1);
			trim(lowercase);
			trim(line);
		}

		temp_hp_index = lowercase.find("temp_hp:");
		if (temp_hp_index != std::string::npos && temp_hp == 0)
		{
			size_t space_after_index = lowercase.find(' ', temp_hp_index);
			size_t length = space_after_index - temp_hp_index;
			std::string sub = line.substr(temp_hp_index + 8, length - 8);
			try
			{
				temp_hp = std::stoi(sub);
				if (temp_hp < 0)
				{
					temp_hp = 0;
					throw;
				}
			}
			catch (const std::exception& E)
			{
				std::cout << "Temp HP must be a positive number" << std::endl;
				return false;
			}
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, temp_hp_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
			line =		line	 .substr(0, temp_hp_index) + line.	   substr(space_after_index + 1, line.	   length() - space_after_index - 1);
			trim(lowercase);
			trim(line);
		}

		temp_hp_index = lowercase.find("thp:");
		if (temp_hp_index != std::string::npos && temp_hp == 0)
		{
			size_t space_after_index = lowercase.find(' ', temp_hp_index);
			size_t length = space_after_index -  temp_hp_index;
			std::string sub = line.substr(temp_hp_index + 4, length - 4);
			try
			{
				temp_hp = std::stoi(sub);
				if (temp_hp < 0)
				{
					temp_hp = 0;
					throw;
				}
			}
			catch (const std::exception& E)
			{
				std::cout << "Temp HP must be a positive number" << std::endl;
				return false;
			}
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, temp_hp_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
			line =		line.	  substr(0, temp_hp_index) + line.	   substr(space_after_index + 1, line. 	 length() - space_after_index - 1);
			trim(lowercase);
			trim(line);
		}
		std::string start_file_name = "";
		size_t start_file_index = lowercase.find("start:");
		if (start_file_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', start_file_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - start_file_index;
			std::string sub = line.substr(start_file_index + 6, length - 6);
			
			start_file_name = sub;

			if (npos)
			{
				lowercase = lowercase.substr(0, start_file_index);
				line = line.substr(0, start_file_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, start_file_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, start_file_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}
			
			
		}

		std::string end_file_name = "";
		size_t end_file_index = lowercase.find("end:");
		if (end_file_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', end_file_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - end_file_index;
			std::string sub = line.substr(end_file_index + 4, length - 4);

			end_file_name = sub;


			if (npos)
			{
				lowercase = lowercase.substr(0, end_file_index);
				line = line.substr(0, end_file_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, end_file_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, end_file_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}

			/*
			lowercase = lowercase.substr(0, end_file_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
			line = line.substr(0, end_file_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
			trim(lowercase);
			trim(line);
			*/
		}
		int str = 0, dex = 0, con = 0, intelligence = 0, wis = 0, cha = 0;
		bool has_con = false;

		size_t str_index = lowercase.find("str:");
		if (str_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', str_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - str_index;
			std::string sub = line.substr(str_index + 4, length - 4);

			try{ str = std::stoi(sub); }
			catch (const std::exception& E) {}


			if (npos)
			{
				lowercase = lowercase.substr(0, str_index);
				line = line.substr(0, str_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, str_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, str_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}
		}

		size_t con_index = lowercase.find("con:");
		if (con_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', con_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - con_index;
			std::string sub = line.substr(con_index + 4, length - 4);
			try { con = std::stoi(sub); has_con = true; }
			catch (const std::exception& E) {}


			if (npos)
			{
				lowercase = lowercase.substr(0, con_index);
				line = line.substr(0, con_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, con_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, con_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}
		}
		bool entered_dex = false;
		size_t ability_index = lowercase.find("dex:");
		if (ability_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', ability_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - ability_index;
			std::string sub = line.substr(ability_index + 4, length - 4);

			try { dex = std::stoi(sub); entered_dex = true; }
			catch (const std::exception& E) {}


			if (npos)
			{
				lowercase = lowercase.substr(0, ability_index);
				line = line.substr(0, ability_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, ability_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, ability_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}
		}

		ability_index = lowercase.find("int:");
		if (ability_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', ability_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - ability_index;
			std::string sub = line.substr(ability_index + 4, length - 4);

			try { intelligence = std::stoi(sub); }
			catch (const std::exception& E) {}


			if (npos)
			{
				lowercase = lowercase.substr(0, ability_index);
				line = line.substr(0, ability_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, ability_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, ability_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}
		}

		ability_index = lowercase.find("wis:");
		if (ability_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', ability_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - ability_index;
			std::string sub = line.substr(ability_index + 4, length - 4);

			try { wis = std::stoi(sub); }
			catch (const std::exception& E) {}


			if (npos)
			{
				lowercase = lowercase.substr(0, ability_index);
				line = line.substr(0, ability_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, ability_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, ability_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}
		}

		ability_index = lowercase.find("cha:");
		if (ability_index != std::string::npos)
		{
			size_t space_after_index = lowercase.find(' ', ability_index);
			bool npos = false;
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length();
				npos = true;
			}
			size_t length = space_after_index - ability_index;
			std::string sub = line.substr(ability_index + 4, length - 4);

			try { cha = std::stoi(sub); }
			catch (const std::exception& E) {}


			if (npos)
			{
				lowercase = lowercase.substr(0, ability_index);
				line = line.substr(0, ability_index);
				trim(lowercase);
				trim(line);
			}
			else
			{
				lowercase = lowercase.substr(0, ability_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
				line = line.substr(0, ability_index) + line.substr(space_after_index + 1, line.length() - space_after_index - 1);
				trim(lowercase);
				trim(line);
			}
		}

		temp_hp_index = lowercase.find("buffer:");
		if (temp_hp_index != std::string::npos && temp_hp == 0)
		{
			size_t space_after_index = lowercase.find(' ', temp_hp_index);
			size_t length = space_after_index - flags_index;
			std::string sub = line.substr(temp_hp_index + 7, length - 7);
			try
			{
				temp_hp = std::stoi(sub);
				if (temp_hp < 0)
				{
					temp_hp = 0;
					throw;
				}
			}
			catch (const std::exception& E)
			{
				std::cout << "Temp HP must be a positive number" << std::endl;
				return false;
			}
			if (space_after_index == std::string::npos)
			{
				space_after_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
			line =		line	 .substr(0, flags_index) + line		.substr(space_after_index + 1, line		.length() - space_after_index - 1);
			trim(lowercase);
			trim(line);
		}

		index_t hp_index = lowercase.find("hp:");
		if (hp_index != std::string::npos)
		{
			size_t space_after_hp_index = lowercase.find(' ', hp_index);
			size_t length = space_after_hp_index - hp_index;
			std::string hp_text = lowercase.substr(hp_index, length);
			
			if (space_after_hp_index == std::string::npos)
			{
				space_after_hp_index = lowercase.length() - 1;
			}
			lowercase = lowercase.substr(0, hp_index) + lowercase.substr(space_after_hp_index + 1, lowercase.length() - space_after_hp_index - 1);
			line =		line.	  substr(0, hp_index) + line	 .substr(space_after_hp_index + 1, line		.length() - space_after_hp_index - 1);
			trim(lowercase);
			trim(line);
			index_t slash_index = hp_text.find('/');
			index_t colon_index = hp_text.find(":");
			
			if (slash_index != std::string::npos)
			{
				std::string base_hp_text = hp_text;
				hp_text = "";
				for (size_t j = 0; j < base_hp_text.size(); ++j)
				{
					char c = base_hp_text[j];
					if (c != ' ')
						hp_text += c;
				}
				try
				{
					std::string max_hp_string = hp_text.substr(slash_index + 1, hp_text.length() - slash_index - 1);
					std::string current_hp_string = hp_text.substr(colon_index + 1, slash_index - colon_index - 1);
					max_hp = parse_dice(max_hp_string);
					hp = parse_dice(current_hp_string);
				}
				catch (const std::exception& E)
				{
					std::cout << "Could not parse HP - not adding creature" << std::endl;
					max_hp = -1;
					hp = -1;
					return false;
				}
			}
			else
			{
				try
				{
					std::string parsed = hp_text.substr(colon_index + 1, hp_text.length() - colon_index - 1);
					max_hp = parse_dice(parsed);
					hp = max_hp;
				}
				catch (const std::exception& E)
				{
					std::cout << "Could not parse HP - not adding creature" << std::endl;
					return false;
				}
			}
		}
		int ac_value = -1;
		index_t ac_index = lowercase.find("ac:");
		if (ac_index != std::string::npos)
		{
			//std::cout << "FOUND AC INDEX AT " << ac_index << std::endl;
			size_t space_after_ac_index = lowercase.find(' ', ac_index);
			size_t length = space_after_ac_index - ac_index;
			std::string ac_text = lowercase.substr(ac_index+3, length-3);
			//std::cout << "PARSED AC TEXT: " << ac_text << std::endl;
			if (space_after_ac_index == std::string::npos)
			{
				space_after_ac_index = lowercase.length() - 1;
			}
			//std::cout << "SPACE AFTER INDEX: " << space_after_ac_index << std::endl;
			lowercase = lowercase.substr(0, ac_index) + lowercase.substr(space_after_ac_index + 1, lowercase.length() - space_after_ac_index - 1);
			trim(lowercase);
			trim(line);
			try
			{
				ac_value = std::stoi(ac_text);
			}
			catch (const std::exception& E)
			{
				std::cout << "Could not parse AC - not adding creature" << std::endl;
				return false;
			}
		}

		trim(lowercase);
		trim(line);
		if (
			takes_commands 
			&& 
			(
			lowercase == "done" || 
			lowercase == "end" || 
			lowercase == "stop" || 
			lowercase == "start" || 
			lowercase == "begin" || 
			lowercase == "finish" || 
			lowercase == "go"
			)
			)
		{
			taking_intiatives = false;
			return false;
		}
		else if (takes_commands && (lowercase == "reset" || lowercase == "clear"))
		{
			creatures.clear();
		}
		else if (takes_commands && (comp_substring("load ", lowercase, 5)))
		{
			std::string filename = dummy_line.substr(5, dummy_line.length() - 5);
			if (directory != "" && !starts_with(filename, BASE_DIRECTORY_PROXY + "/") && !is_absolute_directory(filename))
			{
				filename = directory + "/" + filename;
			}
			else
			{
				if (starts_with(filename, BASE_DIRECTORY_PROXY + "/"))
					filename = filename.substr(BASE_DIRECTORY_PROXY.size() + 1);
				if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
				{
					size_t backi = filename.size() - 1;
					while (filename[backi] != '/' && filename[backi] != '\\')
					{
						--backi;
					}
					directory = filename;
					directory.resize(backi);
					if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
						wd = directory;
				}
			}
			std::ifstream new_file;
			process_filename(filename);
			dir_fix(filename);
			search_links(filename);
			new_file.open(filename);
			if (!new_file.is_open())
			{
				std::cout << "Error: Could not open " << filename << std::endl;
				//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
				return false;
			}
			else {
				while (new_file.good() && !new_file.eof())
				{
					get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
				}
				ignore_initial_file_load = true;
				new_file.close();
				save_buffer();
				return false;
			}
		}
		else if (takes_commands && (comp_substring("ld ", dummy_line, 3)))
		{
			std::string filename = dummy_line.substr(3, dummy_line.length() - 3);
			if (directory != "" && !starts_with(filename,BASE_DIRECTORY_PROXY+"/") && !is_absolute_directory(filename))
			{
				filename = directory + "/" + filename;
			}
			else
			{
				if (starts_with(filename, BASE_DIRECTORY_PROXY + "/"))
					filename = filename.substr(BASE_DIRECTORY_PROXY.size() + 1);
				if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
				{
					size_t backi = filename.size() - 1;
					while (filename[backi] != '/' && filename[backi] != '\\')
					{
						--backi;
					}
					directory = filename;
					directory.resize(backi);
					if (initial_execution && LOAD_CHANGES_WORKING_DIRECTORY)
						wd = directory;
				}
			}
			std::ifstream new_file;
			//std::cout << "OPENING FILE:\"" << filename << "\"\nDIRECTORY:\"" << directory << "\"\n";
			process_filename(filename);
			dir_fix(filename);
			search_links(filename);
			new_file.open(filename);
			if (!new_file.is_open())
			{
				std::cout << "Error: Could not open " << filename << std::endl;
				//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
				return false;
			}
			else {
				while (new_file.good() && !new_file.eof())
				{
					get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
				}
				ignore_initial_file_load = true;
				new_file.close();
				save_buffer();
				return false;
			}
		}
		else if (takes_commands && (comp_substring("save ", lowercase, 5)))
		{
			std::string filename = lowercase.substr(5);
			save_state(filename, creatures, initial_turn, initial_round, false);
			return false;
		}
		else if (takes_commands && (comp_substring("sv ", lowercase, 5)))
		{
			std::string filename = lowercase.substr(3);
			save_state(filename, creatures, initial_turn, initial_round, false);
			return false;
		}
		else if (takes_commands && (comp_substring("savet ", lowercase, 6) || comp_substring("savec ", lowercase, 6)))
		{
			std::string filename = lowercase.substr(6);
			save_state(filename, creatures, initial_turn, initial_round, true);
			return false;
		}
		else if (takes_commands && comp_substring("rename ", lowercase, 7))
		{
			std::string args = line.substr(7);
			size_t delimeter = args.find(' ');
			if (delimeter != std::string::npos)
			{
				std::string original = get_lowercase(args.substr(0, delimeter));
				std::string new_name = args.substr(delimeter + 1);
				if (!name_is_unique(new_name, creatures))
				{
					std::cout << "Names must be unique and cannot be shared with commands!\n";
					return false;
				}
				//std::cout << original << " / " << new_name << std::endl;
				for (auto i = creatures.begin(); i != creatures.end(); ++i)
				{
					creature& c = *i;
					if (get_lowercase(c.get_name()) == original)
					{
						c.set_name(new_name);
						break;
					}
				}
			}
		}
		else if (takes_commands && comp_substring("round ", lowercase, 6))
		{
			std::string args = line.substr(6);
			try
			{
				initial_round = std::stoi(args);
			}
			catch (const std::exception& E)
			{
				std::cout << "Invalid round number" << std::endl;
			}
		}



		else if (takes_commands && comp_substring("turn ", lowercase, 5))
		{
			std::string args = line.substr(5);
			std::string args_lower = get_lowercase(args);
			bool found_character = false;
			for (auto i = creatures.begin(); i != creatures.end(); ++i)
			{
				if (i->has_alias(args_lower))
				{
					found_character = true;
					break;
				}
			}
			if (!found_character)
			{
				std::cout << "Error: could not find character '" << args << "'" << std::endl;
			}
			else
			{
				initial_turn = args_lower;
			}
		}
		else
		{
			trim(lowercase);
			size_t number_of_spaces = std::count(lowercase.begin(), lowercase.end(), ' ');
			if (number_of_spaces == 1)
			{
				index_t space_index = lowercase.find(" ");
				name = line.substr(0, space_index);
				if (!name_is_unique(name, creatures))
				{
					std::cout << "Names must be unique and cannot be shared with commands!" << std::endl;
					return false;
				}
				initiative_string = lowercase.substr(space_index + 1, lowercase.length() - (name.length() + 1));
				try
				{
					if (initiative_string[0] == '+' || initiative_string[0] == '-')
					{
						int modifier = std::stoi(initiative_string);
						int initiative = (rand() % 20) + 1 + modifier;
						if (!entered_dex)
							dex = modifier;
						creatures.emplace_back(name, initiative, modifier, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures, start_file_name, end_file_name, has_con, str, dex, con, intelligence, wis, cha, entered_dex);
						added_creature = true;
						cleanup_current();
					}
					else
					{
						int initiative = std::stoi(initiative_string);
						creatures.emplace_back(name, initiative, 0, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures, start_file_name, end_file_name, has_con, str, dex, con, intelligence, wis, cha, entered_dex);
						added_creature = true;
						cleanup_current();
					}
				}
				catch (const std::exception& E)
				{
					std::cout << "Error: Could not parse input (or ran out of memory - unlikely)" << std::endl;
					if(PRINT_DEBUG)
						std::cout << "\tParsed text: \'" << lowercase << "\'" << std::endl;
					//std::cout << E.what() << std::endl;
				}

			}
			else if (number_of_spaces == 2)
			{
				index_t first_space_index = lowercase.find(" ");
				index_t second_space_index = lowercase.find(" ", first_space_index + 1);
				if (second_space_index == first_space_index + 1)
				{
					std::cout << "Error: Malformed input" << std::endl;
					//std::cout << lowercase << std::endl;
				}
				else
				{
					name = line.substr(0, first_space_index);
					if (!name_is_unique(name, creatures))
					{
						std::cout << "Names must be unique and cannot be shared with commands!" << std::endl;
						return false;
					}
					initiative_string = lowercase.substr(first_space_index + 1, second_space_index - first_space_index - 1);
					mod_string = lowercase.substr(second_space_index + 1, lowercase.length() - (second_space_index + 1));
					try
					{
						int initiative; 
						int modifier;
						if (initiative_string[0] == '+' || initiative_string[0] == '-')
						{
							initiative = std::stoi(mod_string);
							modifier = std::stoi(initiative_string);
						}
						else
						{
							initiative = std::stoi(initiative_string);
							modifier = std::stoi(mod_string);
						}
						if (!entered_dex)
							dex = modifier;
						creatures.emplace_back(name, initiative, modifier, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures, start_file_name, end_file_name, has_con, str, dex, con, intelligence, wis, cha, entered_dex);
						added_creature = true;
						cleanup_current();

					}
					catch (const std::exception& E)
					{
						std::cout << "Error: Could not parse input (or ran out of memory - unlikely)" << std::endl;
						if(PRINT_DEBUG)
							std::cout << "\tParsed text: \'" << lowercase << "\'" << std::endl;
						//std::cout << E.what() << std::endl;
					}
				}
			}
			else if (number_of_spaces == 0)
			{
				index_t end = line.find(' ');
				if (end == std::string::npos)
				{
					end = lowercase.length();
				}
				name = line.substr(0, end);
				if (!name_is_unique(name, creatures))
				{
					std::cout << "Names must be unique and cannot be shared with commands!" << std::endl;
					return false;
				}

				creatures.emplace_back(name, 1 + (rand() % 20), 0, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures, start_file_name, end_file_name, has_con, str, dex, con, intelligence, wis, cha, entered_dex);
				added_creature = true;
				cleanup_current();
			}
			else
			{
				std::cout << "Error: Malformed input (did you forget the space, or add extras?)" << std::endl;
				//std::cout << lowercase << std::endl;
			}
		}
	}

	if (used_command && initial_execution)
	{
		while (creatures_buffer_iterator != creatures_buffer.begin())
		{
			creatures_buffer.pop_front();
			new_round_buffer.pop_front();
			turn_msg_buffer.pop_front();
			current_turn_buffer.pop_front();
			current_round_buffer.pop_front();
			display_mode_buffer.pop_front();
			wd_buffer.pop_front();
			save_dc_buffer.pop_front();
		}
	}
	save_buffer();
	return added_creature;
}








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


long long get_ms()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}



std::string event_log = "";
const static char LOG_HEADER_CHAR = 9;

std::string get_hp_change_turn_msg(const std::string& name, int old_hp, int new_hp, const std::string& cur_msg, bool is_concentrating, int intended_dmg, int con_bonus, bool found_con_bonus, creature* victim, bool is_adding_string_outside_function)
{
	if (old_hp < 0)
		return cur_msg;
	std::string str = cur_msg + name;
	if (is_adding_string_outside_function)
		str = name;

	int diff = old_hp - new_hp;
	if (diff < 0)
		diff = -diff;
	std::string concentration_check_text = "";
	if (intended_dmg > 0 && is_concentrating)
	{
		int dc = intended_dmg / 2;
		if (dc < 10)
			dc = 10;

		int save = 1 + (rand() % 20) + con_bonus;
		concentration_check_text = name + " was forced to make a concentration save (DC " + std::to_string(dc) + ")\n\t";
		concentration_check_text += name + " rolled a " + std::to_string(save);
		
		if (found_con_bonus)
		{
			if (save >= dc)
			{
				concentration_check_text += " (Success)\n";
			}
			else
			{
				concentration_check_text += " (Failure)\n";
				concentration_check_text += "\t" + name + " LOST CONCENTRATION!\n";
				victim->add_flag("_lost_concentration", true);
				victim->remove_flag("c");
				victim->remove_flag("con");
				victim->remove_flag("concentrating");
				victim->remove_flag("concentration");
			}
		}
		else if (is_concentrating)
		{
			concentration_check_text += " (but this is without accounting for CON Save bonus)\n";
		}

	}
	if (new_hp > old_hp)
	{
		str += " recovered " + std::to_string(diff) + " hp.\n";
	}
	else if (new_hp < old_hp)
	{
		str += " took " + std::to_string(diff) + " damage.\n";
	}
	else
	{
		if (concentration_check_text.size() != 0)
			str += concentration_check_text + "\n";
		return cur_msg;
	}
	if (concentration_check_text.size() != 0)
		str += concentration_check_text + "\n";
	return str;
}


//int buffer_index = 0; //0 refers to the head, 1 is previous state, 2 is state before, etc.

std::string print_variables(creature* i, bool override_display_mode)
{
	std::string text = "";
	bool displayed_any = false;
	if (override_display_mode)
	{
		if (i->variables.size() != 0)
		{
			for (auto vi = i->variables.begin(); vi != i->variables.end(); ++vi)
			{
				//std::cout << " << vi->first << " = " << vi->second << std::endl;
				if (displayed_any)
					text += "\n";
				std::string var_name = vi->first;
				if (var_name[0] == '#')
				{
					var_name = var_name.substr(1);
					var_name = "(" + var_name + ")";
				}
				text += "\t    " + var_name + " = " + std::to_string(vi->second);
				displayed_any = true;
			}
		}
	}
	else
	{
		if (i->variables.size() != 0)
		{
			for (auto vi = i->variables.begin(); vi != i->variables.end(); ++vi)
			{
				//std::cout << " << vi->first << " = " << vi->second << std::endl;
				if ((vi->first)[0] != '#' || !simple_display)
				{
					if (displayed_any)
						text += "\n";

					std::string var_name = vi->first;
					if (var_name[0] == '#')
					{
						var_name = var_name.substr(1);
						var_name = "(" + var_name + ")";
					}

					text += "\t    " + var_name + " = " + std::to_string(vi->second);
					displayed_any = true;
				}

			}
		}
	}
	if(displayed_any)
		text += "\n";

	return text;
}

std::string get_info(creature* i, int current_turn, int current_round, bool my_turn)
{
	if (i == nullptr)
		return "";
	std::string turn_msg = "Information for " + i->get_name() + ":\n";
	turn_msg += "\tNames & Aliases: " + i->get_display_names(SHOW_ALL_NAMES) + "\n";
	if(i->get_initiative_modifier()>=0)
		turn_msg += "\tInitiative: " + std::to_string(i->get_initiative()) + " (+" + std::to_string(i->get_initiative_modifier()) + ")\n";
	else
		turn_msg += "\tInitiative: " + std::to_string(i->get_initiative()) + " ("+ std::to_string(i->get_initiative_modifier()) + ")\n";
	turn_msg += "\t\t#" + std::to_string(i->get_turn_count() + 1) + " in turn order\n";
	if (current_turn < i->get_turn_count())
		turn_msg += "\t\tTurns taken: " + std::to_string(current_round - 1) + "\n";
	else if (current_turn == i->get_turn_count())
		turn_msg += "\t\tCurrently taking turn #" + std::to_string(current_round) + "\n";
	else
		turn_msg += "\t\tTurns taken: " + std::to_string(current_round) + "\n";
	if (i->get_ac() != -1)
		turn_msg += "\tArmor Class: " + std::to_string(i->get_ac()) + "\n";
	if (i->get_max_hp() != -1)
	{
		turn_msg += "\tHP: " + std::to_string(i->get_hp()) + " / " + std::to_string(i->get_max_hp()) + "\n";
		if (i->get_temp_hp() != 0)
			turn_msg += "\t\tTemp HP: " + std::to_string(i->get_temp_hp()) + "\n";
	}
	std::string flags = i->get_flag_list(my_turn, true, true, true);
	if (flags != "")
		turn_msg += "\tFlags: " + flags + "\n";
	if (i->get_regen_raw() != 0)
	{
		turn_msg += "\tRegeneration: " + std::to_string(i->get_regen_raw()) + " per round\n";
		turn_msg += "\t\tRegeneration disabled next turn: ";
		if (i->regen_is_temporarily_disabled())
			turn_msg += "true\n";
		else
			turn_msg += "false\n";
	}

	turn_msg += "\nSaving Throws:\n";

	auto disp_save = [&](const std::string& save_name, int val)
		{
			if(val >= 0)
				turn_msg += "\t" + save_name + ": +" + std::to_string(val) + "\n";
			else
				turn_msg += "\t" + save_name + ": " + std::to_string(val) + "\n";
		};

	disp_save("STR", i->get_str_bonus());

	if (i->entered_dex)
	{
		disp_save("DEX", i->get_dex_bonus());
	}
	else
	{
		if (i->get_dex_bonus() < 0)
		{
			turn_msg += "\tDEX: " + std::to_string(i->get_dex_bonus()) + " " + "(note: no value was entered, defaulting to initiative modifier)\n";
		}
		else
		{
			turn_msg += "\tDEX: +" + std::to_string(i->get_dex_bonus()) + " " + "(note: no value was entered, defaulting to initiative modifier)\n";
		}
	}

	if (i->has_con_bonus())
	{
		disp_save("CON", i->con);
	}
	else
	{
		turn_msg += "\tCON: +0 (note: no value was ever entered, will not fully automate concentration saves)\n";
	}

	disp_save("INT", i->get_int_bonus());
	disp_save("WIS", i->get_wis_bonus());
	disp_save("CHA", i->get_cha_bonus());
	turn_msg += "\n";
	auto disp_recharge = [&](std::vector<std::string>& recharger, const std::string& dispname)
		{
			if (recharger.size() != 0)
			{
			turn_msg +="\t" + dispname + ": ";
				for (int j = 0; j < recharger.size(); ++j)
				{
					if (j != 0)
						turn_msg += ", ";
					turn_msg += recharger[j];
				}
			turn_msg += "\n";
			}
		};

	disp_recharge(i->recharge1, "Flag Recharge (1-6)");
	disp_recharge(i->recharge2, "Flag Recharge (2-6)");
	disp_recharge(i->recharge3, "Flag Recharge (3-6)");
	disp_recharge(i->recharge4, "Flag Recharge (4-6)");
	disp_recharge(i->recharge5, "Flag Recharge (5-6)");
	disp_recharge(i->recharge6, "Flag Recharge (6-6)");

	if (i->get_reminder(false) != "")
		turn_msg += "\tReminder: " + (i->get_reminder(true)) + "\n";

	if(i->turn_start_file != "")
		turn_msg += "\tTurn Start File: \"" + i->turn_start_file + "\"\n";
	if (i->turn_end_file != "")
		turn_msg += "\tTurn End File: \"" + i->turn_end_file + "\"\n";

	if (i->variables.size() != 0)
	{
		turn_msg += "\tVariables:\n";
		turn_msg += print_variables(i->get_raw_ptr(), true);
	}
	if (i->get_note(false) != "")
		turn_msg += "\n\tNotes:  " + (i->get_note(true)) + "\n";
	turn_msg += "\n";
	return turn_msg;
}

inline void track_initiatives(std::list<creature>& creatures, std::string& dummy_line, bool ignore_initial_file_load, bool& initial_no_script_run_override)
{
	//std::sort(creatures.begin(), creatures.end());
	bool suppress_display = !DISPLAY_INFO_FROM_LOADED_FILES;
	std::string logfile_name = "log_" + std::to_string(time(NULL)) + ".txt";
	creatures.sort();
	index_t current_turn = 0;
	size_t current_round = initial_round;
	event_log = "Began Round " + std::to_string(current_round) + "\n___________________________________________________________\n";
	bool new_round = true;
	std::string previous_turn_creature_name = "";
	creature* knocked_out_creature = nullptr;
	std::string turn_msg = "";
	auto cleanup_current = [&]()
		{
			for (auto i = creatures.begin(); i != creatures.end(); ++i)
			{
				i->remove_alias("@current");
			}
		};
	if (initial_turn != "")
	{
		int init_turn_setter = 0;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			if (i->has_alias(initial_turn))
			{
				break;
			}
			++init_turn_setter;
		}
		if (init_turn_setter == creatures.size())
		{
			init_turn_setter = 0;
		}
		current_turn = init_turn_setter;
	}
	
	int buffer_manipulation_state = STATE_NODO;
	bool file_load_disable = ignore_initial_file_load;
	
	bool new_turn = true;
	auto save_buffer = [&]() -> void
		{
			//std::cout << "SAVING BUFFER\n";
			if (buffer_manipulation_state == STATE_NODO)
			{
				creatures_buffer.push_front(creatures);
				new_round_buffer.push_front(new_round);
				turn_msg_buffer.push_front(turn_msg);
				current_turn_buffer.push_front(current_turn);
				current_round_buffer.push_front(current_round);
				display_mode_buffer.push_front(simple_display);
				wd_buffer.push_front(wd);
				save_dc_buffer.push_front(save_dc);

				creatures_buffer_iterator = creatures_buffer.begin();
				new_round_buffer_iterator = new_round_buffer.begin();
				turn_msg_buffer_iterator = turn_msg_buffer.begin();
				current_turn_buffer_iterator = current_turn_buffer.begin();
				current_round_buffer_iterator = current_round_buffer.begin();
				display_mode_buffer_iterator = display_mode_buffer.begin();
				wd_buffer_iterator = wd_buffer.begin();
				save_dc_buffer_iterator = save_dc_buffer.begin();
				if (creatures_buffer.size() > MAX_UNDO_STEPS)
				{
					creatures_buffer.pop_back();
					new_round_buffer.pop_back();
					turn_msg_buffer.pop_back();
					current_turn_buffer.pop_back();
					current_round_buffer.pop_back();
					display_mode_buffer.pop_back();
					wd_buffer.pop_back();
					save_dc_buffer.pop_back();
				}
			}
		};
	bool first = true;
	bool new_round2 = false;
	creatures_buffer.clear();
	new_round_buffer.clear();
	turn_msg_buffer.clear();
	current_turn_buffer.clear();
	current_round_buffer.clear();
	display_mode_buffer.clear();
	wd_buffer.clear();
	save_dc_buffer.clear();
	save_buffer(); //To initialize the state buffers so they have a place to begin.
	bool used_repeat_command = false;
	while (true) //Terminated only by an explicit command to do so, which returns the funtion.
	{
		clear();
		//BEGIN UNDO/REDO BUFFER STUFF
		
		
		//Check buffer state and manipulate accordingly
		if (buffer_manipulation_state == STATE_NODO)
		{
			while (creatures_buffer_iterator != creatures_buffer.begin()) //Once a non redo/undo command is executed, the current buffer - whichever it is - becomes current
			{
				creatures_buffer.pop_front();
				new_round_buffer.pop_front();
				turn_msg_buffer.pop_front();
				current_turn_buffer.pop_front();
				current_round_buffer.pop_front();
				display_mode_buffer.pop_front();
				wd_buffer.pop_front();
				save_dc_buffer.pop_front();
			}
		}
		else
		{
			creatures = *creatures_buffer_iterator;
			new_round = *new_round_buffer_iterator;
			turn_msg = *turn_msg_buffer_iterator;
			current_turn = *current_turn_buffer_iterator;
			current_round = *current_round_buffer_iterator;
			simple_display = *display_mode_buffer_iterator;
			wd = *wd_buffer_iterator;
			save_dc = *save_dc_buffer_iterator;

		}

		//END UNDO/REDO BUFFER STUFF
		new_round2 = false;
		if (new_round)
		{
			if (!first)
			{
				std::cout << "Start of a new round." << std::endl;
				first = false;
			}
			
			event_log += "\n\nRound " + std::to_string(current_round) + "\n\n";
			for (auto i = creatures.begin(); i != creatures.end(); ++i)
			{
				event_log += get_info(i->get_raw_ptr(), current_turn, current_round, false);
			}
			new_round = false;
			new_round2 = true;
		}
		std::cout << "Round " << current_round << std::endl << std::endl << std::endl;
		int turn_count = 0; //Used to track the turn counts of each creature
		creature* current_creature = nullptr;
		int regenerated_hp = 0;

		creature* current_creature_2 = nullptr;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			if (current_turn == turn_count)
			{
				current_creature_2 = &(*i);
				current_creature_2->set_turn_count(turn_count);
				break;
			}
			++turn_count;
		}
		turn_count = 0;
		std::cout << std::endl;
		if (turn_msg != "")
		{
			std::cout << turn_msg;
			if (turn_msg[0] != LOG_HEADER_CHAR)
			{
				event_log += turn_msg;
			}
			turn_msg = "";
		}
		else if (SHOW_INFO_EACH_TURN && !used_repeat_command)
		{
			std::cout << get_info(current_creature_2, current_turn, current_round, true) << std::endl;
		}

		std::ofstream log_file;
		if (WRITE_LOGS_TO_FILE)
		{
			log_file.open(logfile_name, std::ofstream::out);
			if (log_file.is_open() && log_file.good())
			{
				log_file << event_log;
			}
			log_file.close();
		}

		if(SHOW_INFO_EACH_TURN)
			std::cout << "__________________________________INITIATIVE DISPLAY__________________________________\n" << std::endl;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			i->touched = false;
			if (current_turn == turn_count)
			{
				std::cout << "  ----> ";
				current_creature = &(*i);
				if ((current_creature->get_name()!=previous_turn_creature_name))
				{
					new_turn = true;
				}

				if (new_turn)
				{
					current_creature->poll_recharges();
					int regen = current_creature->get_regen();
					if (regen != 0 && (current_creature->get_hp() != current_creature->get_max_hp()))
					{
						int old_hp = current_creature->get_hp();
						current_creature->adjust_hp(regen);
						int new_hp = current_creature->get_hp();
						regenerated_hp = new_hp - old_hp;
					}

					if (new_round2)
					{
						event_log += "It is now " + current_creature->get_name() + "\'s turn\n";
					}
					else
					{
						event_log += "It is " + current_creature->get_name() + "\'s turn\n";
					}

					if (!new_round2)
					{
						event_log += get_info(i->get_raw_ptr(), current_turn, current_round, false);
					}
				}
			}
			std::string linedisp = "";

			linedisp += i->get_display_names();
			if(!simple_display)
				linedisp += " [" + std::to_string(i->get_initiative()) + "]";
			if (i->get_ac() != -1)
			{
				linedisp += " <AC " + std::to_string(i->get_ac()) + ">";
			}
			
			if (i->get_max_hp() != -1) {
				if(i->get_temp_hp() == 0)
					linedisp += "; " + std::to_string(i->get_hp()) + " / " + std::to_string(i->get_max_hp()) + " HP";
				else
					linedisp +=  "; " + std::to_string(i->get_hp()) + "[+" + std::to_string(i->get_temp_hp()) + " temp]" + " / " + std::to_string(i->get_max_hp()) + " HP";
			}
			if (i->get_flag_list((current_turn == turn_count && new_turn), true, !simple_display, true)!="")
			{
				linedisp += " | FLAGS: " + i->get_flag_list((current_turn == turn_count && new_turn), true, !simple_display, true);
			}
			if(current_turn==turn_count)
				linedisp += " <---------------------------";

			if (linedisp.size() > CONSOLE_WIDTH)
				linedisp.resize(CONSOLE_WIDTH);
			std::cout << linedisp;
			std::cout << std::endl;
			
			std::cout << print_variables(i->get_raw_ptr(), false);
			i->set_turn_count(turn_count);
			++turn_count;
		}
		turn_msg = "";
		std::cout << std::endl << "It's " << current_creature->get_name() << "\'s turn." << std::endl;
		previous_turn_creature_name = current_creature->get_name();
		if (current_creature->get_reminder(false).size() != 0)
		{
			std::cout << "\tREMINDER: " << current_creature->get_reminder(true) << std::endl;
		}
		if (new_turn || used_repeat_command)
		{
			cleanup_current();
			current_creature->add_alias("@current");
			
			auto flag_marker = current_creature->get_flags().begin();
			while (current_creature->get_flags().size() != 0 && flag_marker != current_creature->get_flags().end())
			{
				if ((*flag_marker)[0] == '_')
				{
					current_creature->flags_hidden.erase(*flag_marker);
					current_creature->flags.erase(flag_marker);
					flag_marker = current_creature->flags.begin();
				}
				else
				{
					++flag_marker;
				}
			}
			auto alias_marker = current_creature->aliases.begin();
			while (current_creature->aliases.size() != 0 && alias_marker != current_creature->aliases.end())
			{
				if ((*alias_marker).size()>1 && (*alias_marker)[0]=='@' && (*alias_marker)[1]=='_')
				{
					current_creature->aliases.erase(alias_marker);
					alias_marker = current_creature->aliases.begin();
				}
				else
				{
					++alias_marker;
				}
			}
			if (regenerated_hp != 0)
				std::cout << current_creature->get_name() << " regenerated " << regenerated_hp << " hit points." << std::endl;
			else
				if (current_creature->get_regen() != 0)
					std::cout << current_creature->get_name() << " usually regenerates " << current_creature->get_regen() << " hp, but didn't heal this round." << std::endl;
		}
		if (current_creature->get_hp() == 0)
		{
			std::cout << "\t" << current_creature->get_name() << " HAS 0 HP!" << std::endl;
		}
		
		//std::cout << "FULLY REPLACED: " << dummy_line << std::endl;
		bool used_command = false;
		if (new_turn || used_repeat_command)
		{
			std::string filename = current_creature->turn_start_file;
			if (filename != "" && ((!file_load_disable) || initial_no_script_run_override || used_repeat_command ))
			{
				used_repeat_command = false;
				if (starts_with(filename, BASE_DIRECTORY_PROXY + "/"))
				{
					filename = filename.substr(BASE_DIRECTORY_PROXY.size()+1);
				}
				std::string dir = get_directory(filename);
				initial_no_script_run_override = false;
				std::ifstream new_file;
				search_links(filename);
				new_file.open(filename);
				std::string line;
				if (!new_file.is_open())
				{
					std::cout << "Error: Could not open " << filename << std::endl;
					//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
				}
				else
				{
					bool taking_initiatives = false;
					while (new_file.good() && !new_file.eof())
					{
						get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, dir, false, turn_msg, suppress_display);
					}
					new_file.close();
					creatures.sort();
					turn_count = current_creature->get_turn_count();
					current_turn = 0;
					for (auto i = creatures.begin(); i != creatures.end(); ++i)
					{
						if (i->get_name() == current_creature->get_name())
						{

							break;
						}
						++current_turn;
					}
				}

				used_command = true;
				new_turn = false;
				buffer_manipulation_state = STATE_NODO;
				save_buffer();
				file_load_disable = true;
				continue;
			}

		}
		dummy_line = "";
		bool complex_character_delay = false;
		if ((current_creature->turn_start_file != "" || current_creature->turn_end_file != "" || current_creature->flags.size() != 0 || current_creature->variables.size() != 0 || current_creature->get_reminder(false).size() != 0) && new_turn && !current_creature->has_flag("simple"))
		{
			complex_character_delay = true;
			
			long long start_time = get_ms();//time(NULL);
			long long current_time = get_ms();//time(NULL);
			
			while (current_time < (start_time + static_cast<long long>(SECONDS_WAITED * 1000.0L)) && dummy_line=="")
			{
				std::cout << "This character seems complex - take the time (at least " << SECONDS_WAITED << " secs) to resolve this turn carefully." << std::endl;
				std::getline(std::cin, dummy_line);
				trim(dummy_line);
				current_time = get_ms();// time(NULL);
			}

		}

		if (!complex_character_delay)
		{
			std::getline(std::cin, dummy_line);
			trim(dummy_line);
		}
		if (dummy_line != "")
			event_log += "\nEntered command: " + dummy_line + "\n";
		else
			event_log += "\nEntered command: next\n";
		std::string original_dummy_line = dummy_line;
		std::string& line = original_dummy_line;
		make_lowercase(dummy_line);
		command_replacement(dummy_line);
		bool did_erase = false;
		bool skip = false;
		int move_turn = -1;
		size_t l = dummy_line.length() - 1;
		//std::string lowercase_current_creature_name = get_lowercase(current_creature->get_name());
		std::string keep_name = "";
		bool skip_command_checks = false;
		used_repeat_command = false;
		if (dummy_line == "repeat")
		{
			used_command = true;
			used_repeat_command = true;
			skip_command_checks = true;
			dummy_line = "pause";
		}
		else
		{
			used_repeat_command = false;
		}
		if (dummy_line == "quit" || dummy_line == "end" || dummy_line == "stop" || dummy_line == "terminate" || dummy_line == "finish" || dummy_line == "leave" || dummy_line == "close")
			return;
		else if (dummy_line == "undo" || dummy_line == "u")
		{
			buffer_manipulation_state = STATE_UNDO;
			//Increment buffer iterators
			if (creatures_buffer_iterator != (--creatures_buffer.end()))
			{
				++creatures_buffer_iterator;
				++new_round_buffer_iterator;
				++turn_msg_buffer_iterator;
				++current_turn_buffer_iterator;
				++current_round_buffer_iterator;
				++display_mode_buffer_iterator;
				++wd_buffer_iterator;
				++save_dc_buffer_iterator;
			}
			continue;
		}
		else if (dummy_line == "redo" || dummy_line == "r")
		{
			buffer_manipulation_state = STATE_REDO;
			if (creatures_buffer_iterator != creatures_buffer.begin())
			{
				//Decrement buffer iterators
				--creatures_buffer_iterator;
				--new_round_buffer_iterator;
				--turn_msg_buffer_iterator;
				--current_turn_buffer_iterator;
				--current_round_buffer_iterator;
				--display_mode_buffer_iterator;
				--wd_buffer_iterator;
				--save_dc_buffer_iterator;
			}
			continue;
		}
		else if (dummy_line == "simple display")
		{
			simple_display = true;
			used_command = true;
			skip_command_checks = true;
		}
		else if (dummy_line == "full display")
		{
			simple_display = false;
			used_command = true;
			skip_command_checks = true;
		}
		else if (dummy_line == "dispt" || dummy_line == "tdisp" || dummy_line == "disp" || dummy_line == "toggle display" || dummy_line == "toggle disp" || dummy_line == "display toggle" || dummy_line == "disp toggle")
		{
			simple_display = !simple_display;
			used_command = true;
			skip_command_checks = true;
		}
		else if ((dummy_line.size() > 3) && dummy_line[2] == ' ' && dummy_line[1] == 'd' && (dummy_line[0] == 'c' || dummy_line[0] == 'w'))
		{
			used_command = true;
			skip_command_checks = true;
			std::string arg = original_dummy_line.substr(3);
			if (starts_with(arg, BASE_DIRECTORY_PROXY+"/") && wd!="")
			{
				arg = arg.substr(BASE_DIRECTORY_PROXY.size()+1);
				wd = "";
			}
			wd += "/" + arg;
			wd = replace_all(wd, "//", "/", false);
			if (wd[wd.size() - 1] == '/' || wd[wd.size() - 1] == '\\')
				wd.resize(wd.size() - 1);
			wd = replace_all(wd, "\\", "/", false);
			if (wd[0] == '/')
				wd = wd.substr(1);
			turn_msg = "Set working directory to \'" + wd + "\'\n\n";
		}
		else if (dummy_line=="dc")
		{
			try
			{
				turn_msg = "Save DC = " + std::to_string(save_dc) + "\n";
				used_command = true;
				skip_command_checks = true;
			}
			catch (const std::exception& E) {}
		}
		else if (dummy_line.size() >= 2 && dummy_line[0] == '/' && dummy_line[1] == '/')
		{
			used_command = true;
			skip_command_checks = true;
		}
		else if (dummy_line.size() > 3 && comp_substring(dummy_line, "dc ", 3))
		{
			try
			{
				bool is_signed = false;
				save_dc = get_number_arg(dummy_line, is_signed, creatures, nullptr);
				used_command = true;
				skip_command_checks = true;
			}
			catch (const std::exception& E) {}
		}
		else if (dummy_line == "wd" || dummy_line == "cd")
		{
			used_command = true;
			skip_command_checks = true;
			if (dummy_line == "cd")
			{
				wd = "";
				turn_msg = "Reset working directory\n\n";
			}
			else
			{
				if (wd == "")
					turn_msg = "Working Directory: " + BASE_DIRECTORY_PROXY + "\n\n";
				else
					turn_msg = "Working Directory: " + BASE_DIRECTORY_PROXY + "/" + wd + "\n\n";
			}
		}
		else if (dummy_line == "cd.." || dummy_line == "wd..")
		{
			used_command = true;
			skip_command_checks = true;
			std::string arg = "..";
			wd += "/" + arg;
			wd = replace_all(wd, "//", "/", false);
			if (wd[wd.size() - 1] == '/' || wd[wd.size() - 1] == '\\')
				wd.resize(wd.size() - 1);
			wd = replace_all(wd, "\\", "/", false);
			if (wd[0] == '/')
				wd = wd.substr(1);
			turn_msg = "Set working directory to \'" + wd + "\'\n\n";
		}
		else if (((dummy_line == "wd "+BASE_DIRECTORY_PROXY) || (dummy_line == "cd "+BASE_DIRECTORY_PROXY) || (dummy_line == "cd " + BASE_DIRECTORY_PROXY+"/") || (dummy_line == "wd " + BASE_DIRECTORY_PROXY+"/") || (dummy_line == "cd /" + BASE_DIRECTORY_PROXY + "/") || (dummy_line == "wd / " + BASE_DIRECTORY_PROXY+"/") || (dummy_line == "cd /" + BASE_DIRECTORY_PROXY) || (dummy_line == "wd /" + BASE_DIRECTORY_PROXY)) && wd != "")
		{
			used_command = true;
			wd = "";
			skip_command_checks = true;
			turn_msg = "Reset working directory\n";
		}
		else if (dummy_line.size() > 5 && dummy_line[0] == 's' && dummy_line[1] == 'o' && dummy_line[2] == 'r' && dummy_line[3] == 't' && dummy_line[4] == ' ')
		{
			std::string arg = dummy_line.substr(4);
			trim(arg);
			sort(creatures, arg);
			used_command = true;
			skip_command_checks = true;
		}
		else if (dummy_line == "ls")
		{
			used_command = true;
			ls(wd, turn_msg, false, false);
			skip_command_checks = true;
		}
		else if (dummy_line == "ls -r" || dummy_line == "ls r" || dummy_line=="ls-r")
		{
			used_command = true;
			ls(wd, turn_msg, true, false);
			skip_command_checks = true;
		}
		else if (dummy_line == "ls -l" || dummy_line == "ls l" || dummy_line=="ls-l")
		{
			used_command = true;
			ls(wd, turn_msg, false, true);
			skip_command_checks = true;
		}
		else if (dummy_line == "ls-lr" || dummy_line=="ls-rl" || dummy_line == "ls -l -r" || dummy_line == "ls -r -l" || dummy_line == "ls -lr" || dummy_line == "ls -rl" || dummy_line == "ls l r" || dummy_line == "ls r l" || dummy_line == "ls lr" || dummy_line == "ls rl")
		{
			used_command = true;
			ls(wd, turn_msg, true, true);
			skip_command_checks = true;
		}
		else
		{
			while (creatures_buffer_iterator != creatures_buffer.begin())
			{
				creatures_buffer.pop_front();
				new_round_buffer.pop_front();
				turn_msg_buffer.pop_front();
				current_turn_buffer.pop_front();
				current_round_buffer.pop_front();
				display_mode_buffer.pop_front();
				wd_buffer.pop_front();
				save_dc_buffer.pop_front();
			}
		}
		
		if (dummy_line == "clean" || dummy_line == "cleanup" || dummy_line == "clear" || dummy_line == "cls")
		{
			auto cleaner = creatures.begin();
			while (creatures.size() != 0 && cleaner != creatures.end())
			{
				if (cleaner->get_hp() == 0)
				{
					creatures.erase(cleaner);
					cleaner = creatures.begin();
				}
				else
				{
					++cleaner;
				}
			}
			used_command = true;
		}
		else if (dummy_line == "skip")
		{
			used_command = true;
			skip_command_checks = true;
			skip = true;
		}
		else if (dummy_line == "pause")
		{
			skip_command_checks = true;
			used_command = true;
		}
		else if ((comp_substring("load ", dummy_line, 5)))
		{
			std::string filename = dummy_line.substr(5, dummy_line.length() - 5);
			std::string directory = wd;
			if (directory != "" && !starts_with(filename, BASE_DIRECTORY_PROXY + "/") && !is_absolute_directory(filename))
			{
				filename = directory + "/" + filename;
			}
			else
			{
				if (starts_with(filename, BASE_DIRECTORY_PROXY + "/"))
					filename = filename.substr(BASE_DIRECTORY_PROXY.size() + 1);
				if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
				{
					size_t backi = filename.size() - 1;
					while (filename[backi] != '/' && filename[backi] != '\\')
					{
						--backi;
					}
					directory = filename;
					directory.resize(backi);
				}
			}
			std::ifstream new_file;
			process_filename(filename);
			search_links(filename);
			new_file.open(filename);
			used_command = true;
			if (!new_file.is_open())
			{
				std::cout << "Error: Could not open " << filename << std::endl;
				//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
			}
			else {
				bool taking_initiatives = false;
				while (new_file.good() && !new_file.eof())
				{
					get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
				}
				new_file.close();
				creatures.sort();
				turn_count = current_creature->get_turn_count();
				current_turn = 0;
				for (auto i = creatures.begin(); i != creatures.end(); ++i)
				{
					if (i->get_name() == current_creature->get_name())
					{

						break;
					}
					++current_turn;
				}
				buffer_manipulation_state = STATE_NODO;
				save_buffer();
				file_load_disable = true;
				continue;
			}
		}
		else if ((comp_substring("ld ", dummy_line, 3)))
		{
			std::string filename = dummy_line.substr(3, dummy_line.length() - 3);
			std::string directory = wd;
			if (directory != "" && !starts_with(filename, BASE_DIRECTORY_PROXY + "/") && !is_absolute_directory(filename))
			{
				filename = directory + "/" + filename;
			}
			else
			{
				if (starts_with(filename, BASE_DIRECTORY_PROXY + "/"))
					filename = filename.substr(BASE_DIRECTORY_PROXY.size() + 1);
				if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
				{
					size_t backi = filename.size() - 1;
					while (filename[backi] != '/' && filename[backi] != '\\')
					{
						--backi;
					}
					directory = filename;
					directory.resize(backi);
				}
			}
			std::ifstream new_file;
			process_filename(filename);
			search_links(filename);
			new_file.open(filename);
			used_command = true;
			if (!new_file.is_open())
			{
				std::cout << "Error: Could not open " << filename << std::endl;
				//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
			}
			else {
				bool taking_initiatives = false;
				while (new_file.good() && !new_file.eof())
				{
					get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
				}
				new_file.close();
				creatures.sort();
				turn_count = current_creature->get_turn_count();
				current_turn = 0;
				for (auto i = creatures.begin(); i != creatures.end(); ++i)
				{
					if (i->get_name() == current_creature->get_name())
					{

						break;
					}
					++current_turn;
				}
				buffer_manipulation_state = STATE_NODO;
				save_buffer();
				file_load_disable = true;
				continue;
			}
		}
		else if ((comp_substring("keep ", dummy_line, 5)))
		{
			keep_name = dummy_line.substr(5);
			trim(keep_name);
		}
		else if (dummy_line.size()>6 && (comp_substring("print  ", dummy_line, 6)))
		{
			std::cout << line.substr(6) << std::endl;
			turn_msg += line.substr(6) + "\n";
			used_command = true;
			skip_command_checks = true;
		}
		else if (dummy_line.size() > 6 && (comp_substring("printnum  ", dummy_line, 6)))
		{
			bool is_signed = false;
			int num = get_number_arg(dummy_line, is_signed, creatures, nullptr);
			std::cout << num << std::endl;
			turn_msg += std::to_string(num) + "\n";
			used_command = true;
			skip_command_checks = true;
		}
		else if (dummy_line.length() > 5 && dummy_line[l] == 'p' && dummy_line[l - 1] == 'e' && dummy_line[l - 2] == 'e' && dummy_line[l - 3] == 'k' && dummy_line[l - 4] == ' ')
		{
			keep_name = dummy_line;
			keep_name.resize(dummy_line.size() - 5);
		}
		else if (dummy_line == "log")
		{
			turn_msg = " EVENT LOG\n";
			turn_msg[0] = LOG_HEADER_CHAR;
			turn_msg += event_log;
			skip_command_checks = true;
			used_command = true;
		}
		
		if (keep_name != "")
		{
			used_command = true;
			skip_command_checks = true;
			auto i = creatures.begin();
			while (creatures.size() > 0 && i != creatures.end())
			{
				if (!i->has_alias(keep_name))
				{
					creatures.erase(i);
					i = creatures.begin();
					did_erase = true;
				}
				else
				{
					++i;
				}
			}
		}
		buffer_manipulation_state = STATE_NODO;
		bool start_over = false;
		for (auto i = creatures.begin(); i != creatures.end() && !skip_command_checks; ++i) {
			if (start_over)
			{
				start_over = false;
				i = creatures.begin();
			}
			auto next = i;
			//auto peek = i;
			//++peek;

			auto kill_creature = [&](const std::string& lowercase_name, bool invert) {
				i = creatures.begin();
				while ((creatures.size()!=0) && (i != creatures.end()))
				{
					if (invert)
					{
						if (  !(i->has_alias(lowercase_name)) )
						{
							i->remove_alias("@current");
							creatures.erase(i);
							i = creatures.begin();
						}
						else
						{
							i->remove_alias("@current");
							i->touched = true;
							++i;
						}
					}
					else
					{
						if (i->has_alias(lowercase_name))
						{
							i->remove_alias("@current");
							creatures.erase(i);
							i = creatures.begin();
							i->remove_alias("@current");
						}
						else
						{
							++i;
						}
					}
				}
				i = creatures.begin();
				next = i;
				i->add_alias("@current");
				++next;
				used_command = true;
				did_erase = true;
			};
			auto all_names = i->get_all_names();

			for (auto alias_iterator = all_names.begin(); alias_iterator != all_names.end(); ++alias_iterator)
			{
				const std::string& lowercase_name = get_lowercase(*alias_iterator);
				if (dummy_line.find(lowercase_name) == std::string::npos && dummy_line.find(" all")==std::string::npos && dummy_line.find("sv ")==std::string::npos && dummy_line.find("save ") == std::string::npos && dummy_line.find("roll ") == std::string::npos && dummy_line.find("round ") == std::string::npos)
				{
					continue;
				}
				if (comp_substring("hurt " + lowercase_name + " ", dummy_line, ("hurt " + lowercase_name + " ").length()) ||
					comp_substring("dmg " + lowercase_name + " ", dummy_line, ("dmg " + lowercase_name + " ").length()) ||
					comp_substring("harm " + lowercase_name + " ", dummy_line, ("harm " + lowercase_name + " ").length()) ||
					comp_substring("damage " + lowercase_name + " ", dummy_line, ("damage " + lowercase_name + " ").length()) ||
					comp_substring("d " + lowercase_name + " ", dummy_line, ("d " + lowercase_name + " ").length()) ||
					comp_substring("h " + lowercase_name + " ", dummy_line, ("h " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						int old_hp = i->get_hp();
						i->adjust_hp(-val);
						int new_hp = i->get_hp();
						if (i->get_hp() == 0) 
						{
							knocked_out_creature = &(*i);
						}
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), val, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),false);
						used_command = true;
					}
					catch (const std::exception& E) {
					
					}
				}
				else if (comp_substring("hurt all ", dummy_line, 9) ||
						 comp_substring("dmg all ", dummy_line, 8) ||
						 comp_substring("harm all ", dummy_line, 9) ||
						 comp_substring("damage all ", dummy_line, 11))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						if (!i->touched)
						{
							i->adjust_hp(-val);
							i->touched = true;
						}
						if (i == (--creatures.end()))
							used_command = true;

					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
				}

				else if (   ((lowercase_name + " reminder") == dummy_line) || (dummy_line == ("reminder " + lowercase_name))   )
				{
					i->set_reminder("");
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " reminder ", dummy_line, (lowercase_name + " reminder ").length()))
				{
					std::string reminder = original_dummy_line.substr((lowercase_name + " reminder ").length());
					trim(reminder);
					i->set_reminder(reminder);
					used_command = true;
				}
				else if (comp_substring("reminder " + lowercase_name + " ", dummy_line, ("reminder " + lowercase_name + " ").length()))
				{
					std::string reminder = original_dummy_line.substr(("reminder " + lowercase_name + " ").length());
					trim(reminder);
					i->set_reminder(reminder);
					used_command = true;
				}

				else if (((lowercase_name + " note") == dummy_line) || (dummy_line == ("note " + lowercase_name)))
				{
					i->set_note("");
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " note ", dummy_line, (lowercase_name + " note ").length()))
				{
					std::string reminder = original_dummy_line.substr((lowercase_name + " note ").length());
					trim(reminder);
					i->set_note(reminder);
					used_command = true;
				}
				else if (comp_substring("note " + lowercase_name + " ", dummy_line, ("note " + lowercase_name + " ").length()))
				{
					std::string reminder = original_dummy_line.substr(("note " + lowercase_name + " ").length());
					trim(reminder);
					i->set_note(reminder);
					used_command = true;
				}

				else if (comp_substring(lowercase_name + " recharge0 ", dummy_line, (lowercase_name + " recharge0 ").length())
					|| comp_substring(lowercase_name + " recharge 0 ", dummy_line, (lowercase_name + " recharge 0 ").length())
					||
					comp_substring("recharge0 " + lowercase_name + " ", dummy_line, ("recharge0 " + lowercase_name + " ").length())
					|| comp_substring("recharge 0 " + lowercase_name + " ", dummy_line, ("recharge 0 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->remove_recharge(flag_name);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge1 ", dummy_line, (lowercase_name + " recharge1 ").length())
					|| comp_substring(lowercase_name + " recharge1-6 ", dummy_line, (lowercase_name + " recharge1-6 ").length())
					||
					comp_substring("recharge1 " + lowercase_name + " ", dummy_line, ("recharge1 " + lowercase_name + " ").length())
					|| comp_substring("recharge1-6 " + lowercase_name + " ", dummy_line, ("recharge1-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 1 ", dummy_line, (lowercase_name + " recharge 1 ").length())
					|| comp_substring(lowercase_name + " recharge 1-6 ", dummy_line, (lowercase_name + " recharge 1-6 ").length())
					||
					comp_substring("recharge 1 " + lowercase_name + " ", dummy_line, ("recharge 1 " + lowercase_name + " ").length())
					|| comp_substring("recharge 1-6 " + lowercase_name + " ", dummy_line, ("recharge 1-6 " + lowercase_name + " ").length())
					)
				{
					std::string flag_name;
					for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
					{
						flag_name += dummy_line[index];
					}
					std::reverse(flag_name.begin(), flag_name.end());
					trim(flag_name);
					i->add_recharge(1, flag_name);
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " recharge2 ", dummy_line, (lowercase_name + " recharge2 ").length())
					|| comp_substring(lowercase_name + " recharge2-6 ", dummy_line, (lowercase_name + " recharge2-6 ").length())
					||
					comp_substring("recharge2 " + lowercase_name + " ", dummy_line, ("recharge2 " + lowercase_name + " ").length())
					|| comp_substring("recharge2-6 " + lowercase_name + " ", dummy_line, ("recharge2-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 2 ", dummy_line, (lowercase_name + " recharge 2 ").length())
					|| comp_substring(lowercase_name + " recharge 2-6 ", dummy_line, (lowercase_name + " recharge 2-6 ").length())
					||
					comp_substring("recharge 2 " + lowercase_name + " ", dummy_line, ("recharge 2 " + lowercase_name + " ").length())
					|| comp_substring("recharge 2-6 " + lowercase_name + " ", dummy_line, ("recharge 2-6 " + lowercase_name + " ").length())
					)
					{
						std::string flag_name;
						for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
						{
							flag_name += dummy_line[index];
						}
						std::reverse(flag_name.begin(), flag_name.end());
						trim(flag_name);
						i->add_recharge(2, flag_name);
						used_command = true;
						}
				else if (comp_substring(lowercase_name + " recharge3 ", dummy_line, (lowercase_name + " recharge3 ").length())
					|| comp_substring(lowercase_name + " recharge3-6 ", dummy_line, (lowercase_name + " recharge3-6 ").length())
					||
					comp_substring("recharge3 " + lowercase_name + " ", dummy_line, ("recharge3 " + lowercase_name + " ").length())
					|| comp_substring("recharge3-6 " + lowercase_name + " ", dummy_line, ("recharge3-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 3 ", dummy_line, (lowercase_name + " recharge 3 ").length())
					|| comp_substring(lowercase_name + " recharge 3-6 ", dummy_line, (lowercase_name + " recharge 3-6 ").length())
					||
					comp_substring("recharge 3 " + lowercase_name + " ", dummy_line, ("recharge 3 " + lowercase_name + " ").length())
					|| comp_substring("recharge 3-6 " + lowercase_name + " ", dummy_line, ("recharge 3-6 " + lowercase_name + " ").length())
					)
					{
						std::string flag_name;
						for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
						{
							flag_name += dummy_line[index];
						}
						std::reverse(flag_name.begin(), flag_name.end());
						trim(flag_name);
						i->add_recharge(3, flag_name);
						used_command = true;
						}
				else if (comp_substring(lowercase_name + " recharge4 ", dummy_line, (lowercase_name + " recharge4 ").length())
					|| comp_substring(lowercase_name + " recharge4-6 ", dummy_line, (lowercase_name + " recharge4-6 ").length())
					||
					comp_substring("recharge4 " + lowercase_name + " ", dummy_line, ("recharge4 " + lowercase_name + " ").length())
					|| comp_substring("recharge4-6 " + lowercase_name + " ", dummy_line, ("recharge4-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 4 ", dummy_line, (lowercase_name + " recharge 4 ").length())
					|| comp_substring(lowercase_name + " recharge 4-6 ", dummy_line, (lowercase_name + " recharge 4-6 ").length())
					||
					comp_substring("recharge 4 " + lowercase_name + " ", dummy_line, ("recharge 4 " + lowercase_name + " ").length())
					|| comp_substring("recharge 4-6 " + lowercase_name + " ", dummy_line, ("recharge 4-6 " + lowercase_name + " ").length())
					)
					{
						std::string flag_name;
						for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
						{
							flag_name += dummy_line[index];
						}
						std::reverse(flag_name.begin(), flag_name.end());
						trim(flag_name);
						i->add_recharge(4, flag_name);
						used_command = true;
						}
				else if (comp_substring(lowercase_name + " recharge5 ", dummy_line, (lowercase_name + " recharge5 ").length())
					|| comp_substring(lowercase_name + " recharge5-6 ", dummy_line, (lowercase_name + " recharge5-6 ").length())
					||
					comp_substring("recharge5 " + lowercase_name + " ", dummy_line, ("recharge5 " + lowercase_name + " ").length())
					|| comp_substring("recharge5-6 " + lowercase_name + " ", dummy_line, ("recharge5-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 5 ", dummy_line, (lowercase_name + " recharge 5 ").length())
					|| comp_substring(lowercase_name + " recharge 5-6 ", dummy_line, (lowercase_name + " recharge 5-6 ").length())
					||
					comp_substring("recharge 5 " + lowercase_name + " ", dummy_line, ("recharge 5 " + lowercase_name + " ").length())
					|| comp_substring("recharge 5-6 " + lowercase_name + " ", dummy_line, ("recharge 5-6 " + lowercase_name + " ").length())
					)
					{
						std::string flag_name;
						for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
						{
							flag_name += dummy_line[index];
						}
						std::reverse(flag_name.begin(), flag_name.end());
						trim(flag_name);
						i->add_recharge(5, flag_name);
						used_command = true;
						}
				else if (comp_substring(lowercase_name + " recharge6 ", dummy_line, (lowercase_name + " recharge6 ").length())
					|| comp_substring(lowercase_name + " recharge6-6 ", dummy_line, (lowercase_name + " recharge6-6 ").length())
					||
					comp_substring("recharge6 " + lowercase_name + " ", dummy_line, ("recharge6 " + lowercase_name + " ").length())
					|| comp_substring("recharge6-6 " + lowercase_name + " ", dummy_line, ("recharge6-6 " + lowercase_name + " ").length())
					||
					comp_substring(lowercase_name + " recharge 6 ", dummy_line, (lowercase_name + " recharge 6 ").length())
					|| comp_substring(lowercase_name + " recharge 6-6 ", dummy_line, (lowercase_name + " recharge 6-6 ").length())
					||
					comp_substring("recharge 6 " + lowercase_name + " ", dummy_line, ("recharge 6 " + lowercase_name + " ").length())
					|| comp_substring("recharge 6-6 " + lowercase_name + " ", dummy_line, ("recharge 6-6 " + lowercase_name + " ").length())
					)
					{
						std::string flag_name;
						for (size_t index = dummy_line.size() - 1; index >= 1 && dummy_line[index] != ' '; --index)
						{
							flag_name += dummy_line[index];
						}
						std::reverse(flag_name.begin(), flag_name.end());
						trim(flag_name);
						i->add_recharge(6, flag_name);
						used_command = true;
						}


				else if (((lowercase_name + " start") == dummy_line) || (dummy_line == ("start " + lowercase_name)))
				{
					i->turn_start_file = "";
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " start ", dummy_line, (lowercase_name + " start ").length()))
				{
					std::string filename = original_dummy_line.substr((lowercase_name + " start ").length());
					trim(filename);
					i->turn_start_file = filename;
					used_command = true;
				}
				else if (comp_substring("start " + lowercase_name + " ", dummy_line, ("start " + lowercase_name + " ").length()))
				{
					std::string filename = original_dummy_line.substr(("start " + lowercase_name + " ").length());
					trim(filename);
					i->turn_start_file = filename;
					used_command = true;
				}
				else if (((lowercase_name + " end") == dummy_line) || (dummy_line == ("end " + lowercase_name)))
				{
					i->turn_end_file = "";
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " end ", dummy_line, (lowercase_name + " end ").length()))
				{
					std::string filename = original_dummy_line.substr((lowercase_name + " end ").length());
					trim(filename);
					i->turn_end_file = filename;
					used_command = true;
				}
				else if (comp_substring("end " + lowercase_name + " ", dummy_line, ("end " + lowercase_name + " ").length()))
				{
					std::string filename = original_dummy_line.substr(("end " + lowercase_name + " ").length());
					trim(filename);
					i->turn_end_file = filename;
					used_command = true;
				}
				else if (comp_substring("ac " + lowercase_name + " ", dummy_line, ("ac " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int new_ac = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						i->set_ac(new_ac);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " ac ", dummy_line, (lowercase_name + " ac ").length()))
				{
					try {
						bool is_signed = false;
						int new_ac = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						i->set_ac(new_ac);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("regen " + lowercase_name + " ", dummy_line, ("regen " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
					//if (val < 0)
						//val = 0;
					i->set_regen(val);
					used_command = true;
					//break;
				}

				else if (   (dummy_line == ("info " + lowercase_name)) || (dummy_line == (lowercase_name + " info"))
					|| (dummy_line == ("query " + lowercase_name)) || (dummy_line == (lowercase_name + " query"))
					)
				{
					turn_msg = get_info(i->get_raw_ptr(), current_turn, current_round,false);
					used_command = true;
					//break;
				}
				else if (comp_substring("swap " + lowercase_name + " ", dummy_line, ("swap " + lowercase_name + " ").length()))
				{
					int len = lowercase_name.size() + 5;
					if (dummy_line.size() > len)
					{
						std::string swap_partner_name = dummy_line.substr(len);
						auto get_creature_from_name = [&](const std::string& sname) -> creature*
							{
								for (auto si = creatures.begin(); si != creatures.end(); ++si)
								{
									if(si->has_alias(sname))
										return si->get_raw_ptr();
								}
								return nullptr;
							};
						trim(swap_partner_name);
						creature* swap_partner = get_creature_from_name(swap_partner_name);
						if (swap_partner)
						{
							used_command = true;
							int my_init = i->get_initiative();
							int their_init = swap_partner->get_initiative();
							i->set_initiative(their_init);
							swap_partner->set_initiative(my_init);
							creatures.sort();
							break;
						}
					}
				}

				else if (comp_substring(lowercase_name + " regen ", dummy_line, (lowercase_name + " regen ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
					//if (val < 0)
						//val = 0;
					i->set_regen(val);
					used_command = true;
					//break;
				}

				else if (comp_substring(lowercase_name + " disable", dummy_line, (lowercase_name + " disable").length()))
				{
					i->disable_regen_temp();
					used_command = true;
					//break;
				}

				else if (comp_substring("disable " + lowercase_name, dummy_line, ("disable " + lowercase_name).length()))
				{
					i->disable_regen_temp();
					used_command = true;
					//break;
				}

				else if (comp_substring("save ", dummy_line, 5))
				{
					std::string filename = dummy_line.substr(5);
					save_state(filename, creatures, current_creature->get_name(), current_round, false);
					used_command = true;
				}
				else if (comp_substring("sv ", dummy_line, 5))
				{
					std::string filename = dummy_line.substr(3);
					save_state(filename, creatures, current_creature->get_name(), current_round, false);
					used_command = true;
				}
				else if (comp_substring("savet ", dummy_line, 6) || comp_substring("savec ", dummy_line, 6))
				{
					std::string filename = dummy_line.substr(6);
					save_state(filename, creatures, current_creature->get_name(), current_round, true);
					used_command = true;
				}
				else if (comp_substring("hurtr " + lowercase_name + " ", dummy_line, ("hurtr " + lowercase_name + " ").length()) ||
					comp_substring("dmgr " + lowercase_name + " ", dummy_line, ("dmgr " + lowercase_name + " ").length()) ||
					comp_substring("harmr " + lowercase_name + " ", dummy_line, ("harmr " + lowercase_name + " ").length()) ||
					comp_substring("damager " + lowercase_name + " ", dummy_line, ("damager " + lowercase_name + " ").length()) ||
					comp_substring("hurth " + lowercase_name + " ", dummy_line, ("hurth " + lowercase_name + " ").length()) ||
					comp_substring("dmgh " + lowercase_name + " ", dummy_line, ("dmgh " + lowercase_name + " ").length()) ||
					comp_substring("harmh " + lowercase_name + " ", dummy_line, ("harmh " + lowercase_name + " ").length()) ||
					comp_substring("damageh " + lowercase_name + " ", dummy_line, ("damageh " + lowercase_name + " ").length())
					)
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						val >>= 1;
						int old_hp = i->get_hp();
						i->adjust_hp(-val);
						int new_hp = i->get_hp();
						if (i->get_hp() == 0)
						{
							knocked_out_creature = &(*i);
						}
						used_command = true;
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), val, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),false);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("hurtd " + lowercase_name + " ", dummy_line, ("hurtd " + lowercase_name + " ").length()) ||
					comp_substring("dmgd " + lowercase_name + " ", dummy_line, ("dmgd " + lowercase_name + " ").length()) ||
					comp_substring("hurtv " + lowercase_name + " ", dummy_line, ("hurtv " + lowercase_name + " ").length()) ||
					comp_substring("dmgv " + lowercase_name + " ", dummy_line, ("dmgv " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						val <<= 1;
						int old_hp = i->get_hp();
						i->adjust_hp(-val);
						int new_hp = i->get_hp();
						if (i->get_hp() == 0)
						{
							knocked_out_creature = &(*i);
						}
						used_command = true;
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), val, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),false);
					}
					catch (const std::exception& E) {

					}
				}
				
				else if (comp_substring("heal " + lowercase_name + " ", dummy_line, ("heal " + lowercase_name + " ").length()) ||
						 comp_substring(lowercase_name + " heal ", dummy_line, (lowercase_name + " heal ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						int old_hp = i->get_hp();
						i->adjust_hp(val);
						int new_hp = i->get_hp();
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), val, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),false);
						used_command = true;
					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
				}
				else if (comp_substring("heal all ", dummy_line, 9) || dummy_line=="heal all max" || dummy_line == "heal all all" || dummy_line == "heal all full")
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						if (!i->touched)
						{
							i->adjust_hp(val);
							i->touched = true;
						}
						
						if(i == (--creatures.end()))
							used_command = true;
					
					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
				}

				else if (comp_substring("clone " + lowercase_name + " ", dummy_line, ("clone " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int clones = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " clone ", dummy_line, (lowercase_name + " clone ").length()))
				{
					try {
						bool is_signed = false;
						int clones = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (dummy_line == ("clone " + lowercase_name) || dummy_line == (lowercase_name + " clone"))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
				}


				else if (comp_substring("flag " + lowercase_name + " ", dummy_line, ("flag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flag " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " flag ", dummy_line, (lowercase_name + " flag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flag ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("f " + lowercase_name + " ", dummy_line, ("f " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("f " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " f ", dummy_line, (lowercase_name + " f ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " f ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " tf ", dummy_line, (lowercase_name + " tf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " tf ").length();
						std::string arg = dummy_line.substr(start_length);

						i->add_flag("_" + arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("tf " + lowercase_name + " ", dummy_line, ("tf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("tf " + lowercase_name + " ").length();
						std::string arg = dummy_line.substr(start_length);

						i->add_flag("_" + arg, true);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}


				else if (comp_substring("rmfl " + lowercase_name + " ", dummy_line, ("rmfl " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmfl " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring("rmlf " + lowercase_name + " ", dummy_line, ("rmlf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmfl " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}


				else if (
					dummy_line.size() > (5+lowercase_name.size())
					&&
					comp_substring("hide " + lowercase_name, dummy_line, ("hide " + lowercase_name).length())
					)
					{
						try {
							size_t start_length = lowercase_name.length() + 5;
							std::string var = dummy_line.substr(start_length);
							trim(var);
							//
							if (var.size() >= 2 && var[0] == ':')
								var = var.substr(1);
							if (var.size() >= 2 && var[0] == ':')
								var = var.substr(1);

							if (var.size() >= 2 && var[0] == '.')
								var = var.substr(1);
							var = resolve_var_name(var, creatures, *i);
							i->hide_var(var, creatures);

							used_command = true;
						}
						catch (const std::exception& E) {

						}
						}

				else if (
					comp_substring("show " + lowercase_name, dummy_line, ("show " + lowercase_name).length())
					)
					{
						try {
							size_t start_length = lowercase_name.length() + 5;
							std::string var = dummy_line.substr(start_length);
							trim(var);
							if (var.size() >= 2 && var[0] == ':')
								var = var.substr(1);
							if (var.size() >= 2 && var[0] == ':')
								var = var.substr(1);

							if (var.size() >= 2 && var[0] == '.')
								var = var.substr(1);

							var = resolve_var_name(var, creatures, *i);
							i->show_var(var, creatures);

							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}


				else if (comp_substring("if " + lowercase_name + " ", dummy_line, ("if " + lowercase_name + " ").size()))
				{
					try {
						int cmd_len = ("if " + lowercase_name + " ").size();
						std::string sub = dummy_line.substr(cmd_len);
						trim(sub);
						std::string og_sub = sub;
						int space = sub.find(" ");
						sub.resize(space);
						if (i->has_flag(sub))
						{
							if (dummy_line.find("{")==std::string::npos)
							{
								sub = og_sub;
								space = sub.find(" ");
								sub = sub.substr(space);
								trim(sub);
								std::string filename = sub;

								std::string directory = wd;
								if (directory != "")
								{
									filename = directory + "/" + filename;
								}
								else
								{
									if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
									{
										size_t backi = filename.size() - 1;
										while (filename[backi] != '/' && filename[backi] != '\\')
										{
											--backi;
										}
										directory = filename;
										directory.resize(backi);
									}
								}
								std::ifstream new_file;
								process_filename(filename);
								search_links(filename);
								new_file.open(filename);
								used_command = true;
								if (!new_file.is_open())
								{
									std::cout << "Error: Could not open " << filename << std::endl;
									//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
								}
								else {
									bool taking_initiatives = false;
									while (new_file.good() && !new_file.eof())
									{
										get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
									}
									new_file.close();
									creatures.sort();
									turn_count = current_creature->get_turn_count();
									current_turn = 0;
									for (auto i = creatures.begin(); i != creatures.end(); ++i)
									{
										if (i->get_name() == current_creature->get_name())
										{

											break;
										}
										++current_turn;
									}
									buffer_manipulation_state = STATE_NODO;
									save_buffer();
									file_load_disable = true;
									continue;
								}
							}
							else
							{
								sub = og_sub;
								int index = sub.find("{");
								sub = sub.substr(index);
								trim(sub);
								if (sub[sub.size() - 1] == '}')
									sub[sub.size() - 1] = ' ';
								if (sub[0] == '{')
									sub[0] = ' ';
								trim(sub);
								std::ifstream file;
								bool dummy_taking_initiatives = true;
								std::string filename = "";
								bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, filename, ignore_initial_file_load, wd, false, turn_msg, suppress_display);
								used_command = true;
								if (success)
								{
									creatures.sort();
								}
								buffer_manipulation_state = STATE_NODO;
								used_command = true;
							}
						}
						else
							used_command = true;
					}
					catch (const std::exception& E)
					{

					}
				}

				else if (comp_substring(lowercase_name + " if ", dummy_line, (lowercase_name + " if ").size()))
				{
					try {
						int cmd_len = (lowercase_name + " if ").size();
						std::string sub = dummy_line.substr(cmd_len);
						trim(sub);
						std::string og_sub = sub;
						int space = sub.find(" ");
						sub.resize(space);
						if (i->has_flag(sub))
						{
							if (dummy_line.find("{") == std::string::npos)
							{
								sub = og_sub;
								space = sub.find(" ");
								sub = sub.substr(space);
								trim(sub);
								std::string filename = sub;

								std::string directory = wd;
								if (directory != "")
								{
									filename = directory + "/" + filename;
								}
								else
								{
									if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
									{
										size_t backi = filename.size() - 1;
										while (filename[backi] != '/' && filename[backi] != '\\')
										{
											--backi;
										}
										directory = filename;
										directory.resize(backi);
									}
								}
								std::ifstream new_file;
								process_filename(filename);
								search_links(filename);
								new_file.open(filename);
								used_command = true;
								if (!new_file.is_open())
								{
									std::cout << "Error: Could not open " << filename << std::endl;
									//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
								}
								else {
									bool taking_initiatives = false;
									while (new_file.good() && !new_file.eof())
									{
										get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
									}
									new_file.close();
									creatures.sort();
									turn_count = current_creature->get_turn_count();
									current_turn = 0;
									for (auto i = creatures.begin(); i != creatures.end(); ++i)
									{
										if (i->get_name() == current_creature->get_name())
										{

											break;
										}
										++current_turn;
									}
									buffer_manipulation_state = STATE_NODO;
									save_buffer();
									file_load_disable = true;
									continue;
								}
							}
							else
							{
								sub = og_sub;
								int index = sub.find("{");
								sub = sub.substr(index);
								trim(sub);
								if (sub[sub.size() - 1] == '}')
									sub[sub.size() - 1] = ' ';
								if (sub[0] == '{')
									sub[0] = ' ';
								trim(sub);
								std::ifstream file;
								bool dummy_taking_initiatives = true;
								std::string filename = "";
								bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, filename, ignore_initial_file_load, wd, false, turn_msg, suppress_display);
								used_command = true;
								if (success)
								{
									creatures.sort();
								}
								buffer_manipulation_state = STATE_NODO;
								//save_buffer();
								
								
									file_load_disable = true;
									// 
								//continue;
								used_command = true;
							}
						}
						else
							used_command = true;
					}
					catch (const std::exception& E)
					{

					}
					}

				else if (comp_substring(lowercase_name + "::", dummy_line, (lowercase_name + "::").length()))
				{
					int last = dummy_line.size() - 1;
					if (dummy_line.size() >= 8 && dummy_line[last] == 'e' && dummy_line[last - 1] == 'd' && dummy_line[last - 2] == 'i' && dummy_line[last - 3] == 'h')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 2);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->hide_var(var, creatures);
						used_command = true;
					}
					else if (dummy_line.size() >= 8 && dummy_line[last] == 'w' && dummy_line[last - 1] == 'o' && dummy_line[last - 2] == 'h' && dummy_line[last - 3] == 's')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 2);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->show_var(var, creatures);
						used_command = true;
					}
					else
					{
						try {
							std::string var = dummy_line.substr(lowercase_name.length() + 2);
							//if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
								//var[lowercase_name.length()] = ' ';
							int space = std::string::npos;
							int SET_TYPE = VAR_SET;
							std::string stow = "";
							if (var.find("{") != std::string::npos)
							{
								int code_index = var.find("{");
								stow = var.substr(code_index);
								var.resize(code_index);
								var += "TEMP";
							}
							if (space == std::string::npos)
							{
								space = var.find(" != ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("!=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}


							if (space == std::string::npos)
							{
								space = var.find(" >= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" <= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" > ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" < ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" == ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("==");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}


							if (space == std::string::npos)
							{
								space = var.find(" += ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("+=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" -= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_SUB;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("-=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_SUB;
								}

							}



							if (space == std::string::npos)
							{
								space = var.find(" *= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" /= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_DIVIDE;
								}
							}


							if (space == std::string::npos)
							{
								space = var.find(" = ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("=");
								if (space != std::string::npos)
									var[space] = ' ';
							}


							if (space == std::string::npos)
							{
								space = var.find("--");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_DECREMENT;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("++");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_INCREMENT;
								}
							}

							if (space == std::string::npos)
								space = var.find(" ");

							bool is_signed = false;
							int val = 1;
							if (stow != "")
								var.resize(var.size() - 4);
							if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
								val = get_number_arg(var, is_signed, creatures, i->get_raw_ptr());
							if (stow != "")
							{
								var += stow;
							}
							std::string og_var = var;
							var.resize(space);
							var = resolve_var_name(var, creatures, *i);
							switch (SET_TYPE)
							{
							case VAR_ADD: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_SUB: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_MULTIPLY: i->set_var(var, i->get_var(var, creatures) * val); break;
							case VAR_DIVIDE: i->set_var(var, i->get_var(var, creatures) / val); break;
							case VAR_SET: i->set_var(var, val); break;
							case VAR_INCREMENT: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_DECREMENT: i->set_var(var, i->get_var(var, creatures) - val); break;

							case VAR_EQUAL_TO: {
								std::string sub = og_var;
								size_t arg_index;
								
								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) == val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) == val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_GREATER_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) >= val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) >= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_GREATER_THAN: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) > val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) > val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_LESS_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) <= val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) <= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_LESS_THAN: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) < val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) < val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}

							case VAR_NOT_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) != val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) != val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}



							default: throw;
							}
							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}

					}
				else if (comp_substring("--" + lowercase_name + "::", dummy_line, ("--" + lowercase_name + "::").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 4)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 4);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) - 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) - 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("++" + lowercase_name + "::", dummy_line, ("++" + lowercase_name + "::").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 4)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 4);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) + 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}


				else if (comp_substring(lowercase_name + ".", dummy_line, (lowercase_name + ".").length()))
				{
					int last = dummy_line.size() - 1;
					if (dummy_line.size() >= 7 && dummy_line[last] == 'e' && dummy_line[last - 1] == 'd' && dummy_line[last - 2] == 'i' && dummy_line[last - 3] == 'h')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->hide_var(var, creatures);
						used_command = true;
					}
					else if (dummy_line.size() >= 7 && dummy_line[last] == 'w' && dummy_line[last - 1] == 'o' && dummy_line[last - 2] == 'h' && dummy_line[last - 3] == 's')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->show_var(var, creatures);
						used_command = true;
					}
					else
					{
						try {
							std::string var = dummy_line.substr(lowercase_name.length() + 1);
							//if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
								//var[lowercase_name.length()] = ' ';
							int space = std::string::npos;
							int SET_TYPE = VAR_SET;
							std::string stow = "";
							if (var.find("{") != std::string::npos)
							{
								int code_index = var.find("{");
								stow = var.substr(code_index);
								var.resize(code_index);
								var += "TEMP";
							}
							if (space == std::string::npos)
							{
								space = var.find(" != ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("!=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}


							if (space == std::string::npos)
							{
								space = var.find(" >= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" <= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" > ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" < ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" == ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("==");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" += ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("+=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" -= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_SUB;
								}

							}

							if (space == std::string::npos)
							{
								space = var.find(" *= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" /= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_DIVIDE;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" = ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("=");
								if (space != std::string::npos)
									var[space] = ' ';
							}


							if (space == std::string::npos)
							{
								space = var.find("--");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_DECREMENT;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("++");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_INCREMENT;
								}
							}

							if (space == std::string::npos)
								space = var.find(" ");

							bool is_signed = false;
							int val = 1;
							if (stow != "")
								var.resize(var.size() - 4);
							if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
								val = get_number_arg(var, is_signed, creatures, i->get_raw_ptr());
							if (stow != "")
							{
								var += stow;
							}
							std::string og_var = var;
							var.resize(space);
							var = resolve_var_name(var, creatures, *i);
							switch (SET_TYPE)
							{
							case VAR_ADD: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_SUB: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_SET: i->set_var(var, val); break;
							case VAR_MULTIPLY: i->set_var(var, i->get_var(var, creatures) * val); break;
							case VAR_DIVIDE: i->set_var(var, i->get_var(var, creatures) / val); break;
							case VAR_INCREMENT: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_DECREMENT: i->set_var(var, i->get_var(var, creatures) - val); break;

							case VAR_EQUAL_TO: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) == val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) == val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_GREATER_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) >= val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) >= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_GREATER_THAN: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) > val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) > val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_LESS_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) <= val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) <= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_LESS_THAN: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) < val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) < val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}

							case VAR_NOT_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) != val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) != val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}

							default: throw;
							}
							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}

					}
				else if (comp_substring("--" + lowercase_name + ".", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) - 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) - 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("++" + lowercase_name + ".", dummy_line, ("++" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) + 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + ":", dummy_line, (lowercase_name + ":").length()))
				{
					int last = dummy_line.size() - 1;
					if (dummy_line.size() >= 7 && dummy_line[last] == 'e' && dummy_line[last - 1] == 'd' && dummy_line[last - 2] == 'i' && dummy_line[last - 3] == 'h')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->hide_var(var, creatures);
						used_command = true;
					}
					else if (dummy_line.size() >= 7 && dummy_line[last] == 'w' && dummy_line[last - 1] == 'o' && dummy_line[last - 2] == 'h' && dummy_line[last - 3] == 's')
					{
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						var.resize(var.size() - 5);
						var = resolve_var_name(var, creatures, *i);
						i->show_var(var, creatures);
						used_command = true;
					}
					else
					{
						try {
							std::string var = dummy_line.substr(lowercase_name.length() + 1);
							if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
								var[lowercase_name.length()] = ' ';
							int space = std::string::npos;
							int SET_TYPE = VAR_SET;
							std::string stow = "";
							if (var.find("{") != std::string::npos)
							{
								int code_index = var.find("{");
								stow = var.substr(code_index);
								var.resize(code_index);
								var += "TEMP";
							}

							if (space == std::string::npos)
							{
								space = var.find(" != ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("!=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_NOT_EQUAL;
								}
							}


							if (space == std::string::npos)
							{
								space = var.find(" >= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_GREATER_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" <= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_LESS_THAN_OR_EQUAL;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" > ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(">");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_GREATER_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" < ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("<");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 1);
									SET_TYPE = VAR_LESS_THAN;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" == ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("==");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_EQUAL_TO;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" *= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("*=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_MULTIPLY;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" /= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("/=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_DIVIDE;
								}
							}
							if (space == std::string::npos)
							{
								space = var.find(" += ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("+=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_ADD;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find(" -= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 4);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-= ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
									SET_TYPE = VAR_SUB;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("-=");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 2);
									SET_TYPE = VAR_SUB;
								}

							}


							if (space == std::string::npos)
							{
								space = var.find(" = ");
								if (space != std::string::npos)
								{
									std::string prefix = var;
									prefix.resize(space);
									var = prefix + " " + var.substr(space + 3);
								}
							}
							if (space == std::string::npos)
							{
								space = var.find("=");
								if (space != std::string::npos)
									var[space] = ' ';
							}


							if (space == std::string::npos)
							{
								space = var.find("--");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_DECREMENT;
								}
							}

							if (space == std::string::npos)
							{
								space = var.find("++");
								if (space != std::string::npos)
								{
									SET_TYPE = VAR_INCREMENT;
								}
							}

							if (space == std::string::npos)
								space = var.find(" ");

							bool is_signed = false;
							int val = 1;
							if (stow != "")
								var.resize(var.size() - 4);
							if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
								val = get_number_arg(var, is_signed, creatures, i->get_raw_ptr());
							if (stow != "")
							{
								var += stow;
							}
							std::string og_var = var;
							var.resize(space);
							switch (SET_TYPE)
							{
							case VAR_ADD: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_SUB: i->set_var(var, i->get_var(var, creatures) - val); break;
							case VAR_SET: i->set_var(var, val); break;
							case VAR_MULTIPLY: i->set_var(var, i->get_var(var, creatures) * val); break;
							case VAR_DIVIDE: i->set_var(var, i->get_var(var, creatures) / val); break;
							case VAR_INCREMENT: i->set_var(var, i->get_var(var, creatures) + val); break;
							case VAR_DECREMENT: i->set_var(var, i->get_var(var, creatures) - val); break;

							case VAR_EQUAL_TO: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) == val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) == val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_GREATER_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) >= val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) >= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_GREATER_THAN: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) > val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) > val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_LESS_THAN_OR_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) <= val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) <= val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							case VAR_LESS_THAN: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) < val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) < val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}

							case VAR_NOT_EQUAL: {
								std::string sub = og_var;
								size_t arg_index;

								if (sub.find("{") == std::string::npos)
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != ' '; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (i->get_var(var, creatures) != val)
									{
										std::string filename = sub;


										std::string directory = wd;
										if (directory != "")
										{
											filename = directory + "/" + filename;
										}
										else
										{
											if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
											{
												size_t backi = filename.size() - 1;
												while (filename[backi] != '/' && filename[backi] != '\\')
												{
													--backi;
												}
												directory = filename;
												directory.resize(backi);
											}
										}
										std::ifstream new_file;
										process_filename(filename);
										search_links(filename);
										new_file.open(filename);
										used_command = true;
										if (!new_file.is_open())
										{
											std::cout << "Error: Could not open " << filename << std::endl;
											//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
										}
										else {
											bool taking_initiatives = false;
											while (new_file.good() && !new_file.eof())
											{
												get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, directory, false, turn_msg, suppress_display);
											}
											new_file.close();
											creatures.sort();
											turn_count = current_creature->get_turn_count();
											current_turn = 0;
											for (auto i = creatures.begin(); i != creatures.end(); ++i)
											{
												if (i->get_name() == current_creature->get_name())
												{

													break;
												}
												++current_turn;
											}
											buffer_manipulation_state = STATE_NODO;
											save_buffer();
											file_load_disable = true;
											continue;
										}

									}
								}
								else
								{
									for (arg_index = sub.size() - 1; arg_index != 0 && sub[arg_index] != '{'; --arg_index)
									{

									}
									std::string num = sub;
									sub = sub.substr(arg_index);
									num.resize(arg_index);
									bool is_signed = false;
									val = get_number_arg(num, is_signed, creatures, i->get_raw_ptr());
									trim(sub);
									if (sub[0] == '{')
										sub[0] = ' ';
									if (sub[sub.size() - 1] == '}')
										sub[sub.size() - 1] = ' ';
									trim(sub);

									if (i->get_var(var, creatures) != val)
									{
										std::ifstream file;
										bool dummy_taking_initiatives = true;
										used_command = true;
										bool success = get_creature(creatures, dummy_taking_initiatives, sub, file, true, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
										if (success)
										{
											creatures.sort();
										}
										else
										{
											//std::cout << "Error\n";
										}
									}
								}

								break;
							}
							default: throw;
							}
							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}
					}
				else if (comp_substring("--" + lowercase_name + ":", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) - 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) - 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("++" + lowercase_name + ":", dummy_line, ("++" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						var = resolve_var_name(var, creatures, *i);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var, creatures) + 1);
						else if (i->variables.count("#" + var) != 0)
							i->set_var("#" + var, i->get_var("#" + var, creatures) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}

				else if (comp_substring("rv " + lowercase_name + "::", dummy_line, ("rv " + lowercase_name + "::").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + "::").length();
						std::string trunc = dummy_line.substr(3);
						//std::cout << "TRUNC=" << trunc << std::endl;
						int loc = trunc.find("::");
						std::string var = trunc.substr(loc + 2);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + "::", dummy_line, ("rv " + lowercase_name + "::").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + "::").length();
						std::string trunc = dummy_line.substr(3);
						//std::cout << "TRUNC=" << trunc << std::endl;
						int loc = trunc.find("::");
						std::string var = trunc.substr(loc + 2);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rv " + lowercase_name + ":", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						//std::cout << "TRUNC=" << trunc << std::endl;
						int loc = trunc.find(":");
						std::string var = trunc.substr(loc + 1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + ":", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						//std::cout << "TRUNC=" << trunc << std::endl;
						int loc = trunc.find(":");
						std::string var = trunc.substr(loc + 1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				
				else if (comp_substring("rv " + lowercase_name + ".", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						//std::cout << "TRUNC=" << trunc << std::endl;
						int loc = trunc.find(".");
						std::string var = trunc.substr(loc+1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + ".", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						//std::cout << "TRUNC=" << trunc << std::endl;
						int loc = trunc.find(".");
						std::string var = trunc.substr(loc+1);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rv " + lowercase_name + " ", dummy_line, ("rv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						std::cout << "TRUNC=" << trunc << std::endl;
						int loc = trunc.find(" ");
						std::string var = trunc.substr(loc);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("dv " + lowercase_name + " ", dummy_line, ("dv " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("dv " + lowercase_name + " ").length();
						std::string trunc = dummy_line.substr(3);
						int loc = trunc.find(" ");
						std::string var = trunc.substr(loc);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " dv ", dummy_line, (lowercase_name + " dv ").length()))
				{
					try {
						size_t start_length = lowercase_name.length() + 4;
						std::string var = dummy_line.substr(start_length);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " rv ", dummy_line, (lowercase_name + " dv ").length()))
				{
					try {
						size_t start_length = lowercase_name.length() + 4;
						std::string var = dummy_line.substr(start_length);
						trim(var);
						var = resolve_var_name(var, creatures, *i);
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " rmfl ", dummy_line, (lowercase_name + " rmfl ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmfl ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " rmlf ", dummy_line, (" rmlf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rlmf ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (dummy_line == lowercase_name + " str_save" || dummy_line == "str_save " + lowercase_name)
				{
					int save = i->get_str_bonus();
					std::string save_name = "str_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
					}
				else if (dummy_line == lowercase_name + " dex_save" || dummy_line == "dex_save " + lowercase_name)
				{
					int save = i->get_dex_bonus();
					std::string save_name = "dex_save";
					save += 1 + (rand() % 20);
					if (i->has_flag("danger_sense"))
					{
						int reroll = 1 + (rand() % 20) + i->get_dex_bonus();
						if (reroll > save)
							save = reroll;
					}
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
					}
				else if (dummy_line == lowercase_name + " con_save" || dummy_line == "con_save " + lowercase_name)
				{
					int save = i->get_con_bonus(creatures);
					std::string save_name = "con_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
					}
				else if (dummy_line == lowercase_name + " int_save" || dummy_line == "int_save " + lowercase_name)
				{
					int save = i->get_int_bonus();
					std::string save_name = "int_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
					}
				else if (dummy_line == lowercase_name + " wis_save" || dummy_line == "wis_save " + lowercase_name)
				{
					int save = i->get_wis_bonus();
					std::string save_name = "wis_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
					}
				else if (dummy_line == lowercase_name + " cha_save" || dummy_line == "cha_save " + lowercase_name)
				{
					int save = i->get_cha_bonus();
					std::string save_name = "cha_save";
					save += 1 + (rand() % 20);
					std::string temp = save_name;
					temp = replace_all(temp, "_", " ", false);
					turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp + " and got a " + std::to_string(save) + " (";
					if (save >= save_dc)
					{
						turn_msg += "success)\n\n";
						i->add_flag("_" + save_name + "_success", true);
					}
					else
					{
						turn_msg += "fail)\n\n";
						i->add_flag("_" + save_name + "_failure", true);
					}
					}
				else if (comp_substring("str_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_str_bonus();
						std::string save_name = "str_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("dex_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_dex_bonus();
						std::string save_name = "dex_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (i->has_flag("danger_sense"))
						{
							int reroll = 1 + (rand() % 20) + i->get_dex_bonus();
							if (reroll > save)
								save = reroll;
						}
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
							if (i->has_evasion())
								dmg = 0;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
							if (i->has_evasion())
								dmg /= 2;
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("con_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_con_bonus(creatures);
						std::string save_name = "con_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("int_save " + lowercase_name + " ", dummy_line, ("str_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_int_bonus();
						std::string save_name = "int_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("wis_save " + lowercase_name + " ", dummy_line, ("wis_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_wis_bonus();
						std::string save_name = "wis_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring("cha_save " + lowercase_name + " ", dummy_line, ("cha_save " + lowercase_name + " ").length()))
				{
					try {
						int save = i->get_cha_bonus();
						std::string save_name = "cha_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + " str_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_str_bonus();
						std::string save_name = "str_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + " con_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_con_bonus(creatures);
						std::string save_name = "con_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + " dex_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_dex_bonus();
						std::string save_name = "dex_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (i->has_flag("danger_sense"))
						{
							int reroll = 1 + (rand() % 20) + i->get_dex_bonus();
							if (reroll > save)
								save = reroll;
						}
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
							if (i->has_evasion())
								dmg = 0;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
							if (i->has_evasion())
								dmg /= 2;
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + " int_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_int_bonus();
						std::string save_name = "int_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + " wis_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_wis_bonus();
						std::string save_name = "wis_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
				else if (comp_substring(lowercase_name + " cha_save ", dummy_line, (lowercase_name + " str_save ").length()))
				{
					try {
						int save = i->get_cha_bonus();
						std::string save_name = "cha_save";

						bool is_signed = false;
						int dmg = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

						bool saved = true;
						save += 1 + (rand() % 20);
						if (save >= save_dc)
						{
							saved = true;
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (success)\n\n";
							save_name += "_success";
							dmg /= 2;
						}
						else
						{
							std::string temp_name = save_name;
							temp_name[3] = ' ';
							turn_msg += i->get_name() + " rolled a DC " + std::to_string(save_dc) + " " + temp_name + " to halve " + std::to_string(dmg) + " damage and got a " + std::to_string(save) + " (fail)\n\n";
							save_name += "_failure";
						}
						int old_hp = i->get_hp();
						i->adjust_hp(-dmg);
						int new_hp = i->get_hp();
						turn_msg += get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), dmg, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),true);
						i->add_flag("_" + save_name, true);
						used_command = true;
					}
					catch (const std::exception& E) {}
					}
	
				else if (
					comp_substring("rf " + lowercase_name + " ", dummy_line, ("rf " + lowercase_name + " ").length())
					||
					comp_substring("fr " + lowercase_name + " ", dummy_line, ("fr " + lowercase_name + " ").length())
					)
				{
					try {
						size_t start_length = ("rf " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (
					comp_substring(lowercase_name + " rf ", dummy_line, (lowercase_name + " rf ").length())
					||
					comp_substring(lowercase_name + " fr ", dummy_line, (lowercase_name + " fr ").length())
					)
				{
					try {
						size_t start_length = (lowercase_name + " rf ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				
				else if (comp_substring(lowercase_name + " ra ", dummy_line, (lowercase_name + " ra ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " ra ").length();
						std::string arg = dummy_line.substr(start_length);
						if (arg.size() > 0 && arg[0] != '@')
							if (i->remove_alias(arg))
								used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("ra " + lowercase_name + " ", dummy_line, (lowercase_name + " ra ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " ra ").length();
						std::string arg = dummy_line.substr(start_length);
						if(arg.size()>0 && arg[0]!='@')
							if (i->remove_alias(arg))
								used_command = true;
					}
					catch (const std::exception& E) {

					}
				}


				else if (comp_substring("temp_hp " + lowercase_name + " ", dummy_line, ("temp_hp " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " temp_hp ", dummy_line, (lowercase_name + " temp_hp ").length()) ||

					comp_substring("temphp " + lowercase_name + " ", dummy_line, ("temphp " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " temphp ", dummy_line, (lowercase_name + " temphp ").length()) ||

					comp_substring(lowercase_name + " buffer ", dummy_line, (lowercase_name + " buffer ").length()) ||
					comp_substring("buffer " + lowercase_name + " ", dummy_line, ("buffer " + lowercase_name + " ").length()) ||

					comp_substring("thp " + lowercase_name + " ", dummy_line, ("thp " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " thp ", dummy_line, (lowercase_name + " thp ").length()) ||

					comp_substring("t " + lowercase_name + " ", dummy_line, ("t " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " t ", dummy_line, (lowercase_name + " t ").length()) ||

					comp_substring("temp " + lowercase_name + " ", dummy_line, ("temp " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " temp ", dummy_line, (lowercase_name + " temp ").length())
					)
					{
						try {
							bool is_signed = false;
							int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());

							i->set_temp_hp(val, is_signed);
							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}


				else if (comp_substring("hp " + lowercase_name + " ", dummy_line, ("hp " + lowercase_name + " ").length()) ||
						 comp_substring(lowercase_name + " hp ", dummy_line, (lowercase_name + " hp ").length()) ||
						 comp_substring("health " + lowercase_name + " ", dummy_line, ("health " + lowercase_name + " ").length()) ||
						 comp_substring(lowercase_name + " health ", dummy_line, (lowercase_name + " health ").length()))
				{
					try {
						bool is_signed = false;
						int old_hp = i->get_hp();
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						try {
							size_t slash_index = dummy_line.find("/");
							if (slash_index != std::string::npos)
							{
								try {
									int max_hp = std::stoi(dummy_line.substr(slash_index + 1));
									i->set_max_hp(max_hp, false);
								}
								catch (const std::exception& e)
								{
									std::cout << "Could not parse new Max HP - only changing current HP\n";
								}

							}
							else if (i->get_max_hp() == -1)
							{
								i->set_max_hp(val, false);
							}
							i->set_hp(val, is_signed);
							int new_hp = i->get_hp();
							turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg, i->is_concentrating(), val, i->get_con_bonus(creatures), i->has_con_bonus(), i->get_raw_ptr(),false);
							used_command = true;
							//break;
						}
						catch (const std::exception& E) {

						}
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("max_hp " + lowercase_name + " ", dummy_line, ("max_hp " + lowercase_name + " ").length()) ||
						 comp_substring(lowercase_name + " max_hp ", dummy_line, (lowercase_name + " max_hp ").length()) ||
						 comp_substring("max_health " + lowercase_name + " ", dummy_line, ("max_health " + lowercase_name + " ").length()) ||
						 comp_substring(lowercase_name + " max_health ", dummy_line, (lowercase_name + " max_health ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
						i->set_max_hp(val, is_signed);
						if (i->get_hp() == -1)
							i->set_hp(val, false);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("rename " + lowercase_name + " ", dummy_line, ("rename " + lowercase_name + " ").length()) ||
						 comp_substring(lowercase_name + " rename ", dummy_line, (lowercase_name + " rename ").length()))
				{
					int len = ("rename " + lowercase_name + " ").length();
					std::string new_name = original_dummy_line.substr(len);
					i->set_name(new_name);
					used_command = true;
				}
				else if ((dummy_line == "reroll all" || dummy_line == "reroll @all") && !i->touched)
				{
					int roll = 1 + (rand() % 20);
					i->set_initiative(roll + (i->get_initiative_modifier()));
					i->touched = true;
					if (i == (--creatures.end()))
					{
						used_command = true;
						creatures.sort();
					}
					}
				else if (
					comp_substring("reroll " + lowercase_name, dummy_line, ("reroll " + lowercase_name).length()) ||
					comp_substring(lowercase_name + " reroll ", dummy_line, (lowercase_name + " reroll ").length()))
				{
					try {
						if (!i->touched)
						{
							int roll = 1 + (rand() % 20);
							i->set_initiative(roll + (i->get_initiative_modifier()));
							creatures.sort();
							used_command = true;
							i->touched = true;
						}
					}
					catch (const std::exception& E) {

					}
				}
				else if (dummy_line == "reset")
				{
					int roll = 1 + (rand() % 20);
					i->set_initiative(roll + (i->get_initiative_modifier()));
					i->set_hp(i->get_max_hp(), false);
					i->reset_flags();
					i->variables.clear();
					i->temp_disable_regen = false;
					current_round = 1;
					if (i == (--creatures.end()))
					{
						turn_count = 0;
						move_turn = creatures.begin()->get_turn_count();
						new_turn = true;
						used_command = true;
						file_load_disable = false;
						creatures.sort();
					}
				}
				else if (dummy_line == "kill " + lowercase_name || 
					dummy_line == "die " + lowercase_name || 
					dummy_line == "remove " + lowercase_name || 
					dummy_line == "rm " + lowercase_name
					||

					dummy_line == lowercase_name + "kill" ||
					dummy_line == lowercase_name + "die" ||
					dummy_line == lowercase_name + "remove" ||
					dummy_line == lowercase_name + "rm"
					)
				{
					kill_creature(lowercase_name, false);
					if (creatures.size() == 0)
						return;
				}
				else if (dummy_line == "ko " + lowercase_name)
				{
					i->set_hp(0, false);
					used_command = true;
				}
				else if (dummy_line == lowercase_name + " ko")
				{
					i->set_hp(0, false);
					used_command = true;
				}
				else if (
					(dummy_line == "reset " + lowercase_name || dummy_line == lowercase_name + " reset")
					)
				{
						i->reset_flags();
						i->temp_disable_regen = false;
						i->variables.clear();
					i->set_hp(i->get_max_hp(), false);
					used_command = true;
				}
				else if (
					(dummy_line == "trn " + lowercase_name) || 
					(dummy_line == "turn " + lowercase_name) ||
					(dummy_line == "goto " + lowercase_name) ||
					(dummy_line == "go " + lowercase_name) ||
					(dummy_line == "visit " + lowercase_name) ||
					(dummy_line == lowercase_name)
					)
				{
					move_turn = i->get_turn_count();
					i->add_alias("@current");
					used_command = true;
					new_turn = true;
					file_load_disable = false;
				}
				else if (comp_substring("round ", dummy_line, 6))
				{
					try {
						std::string sub = dummy_line.substr(6, dummy_line.length() - 6);
						int val = std::stoi(sub);
						if (val < 1) {
							val = 1;
						}
						current_round = val;
						used_command = true;
					}
					catch (const std::exception& E) {
					
					}
				}
				else if (dummy_line == "i " + lowercase_name || dummy_line == lowercase_name+" i")
				{
					turn_msg = get_info(i->get_raw_ptr(), current_turn, current_round, false);
					used_command = true;
				}
				else if (
					comp_substring("move " + lowercase_name + " ", dummy_line, ("move " + lowercase_name + " ").length()) ||
					comp_substring("mv " + lowercase_name + " ", dummy_line, ("mv " + lowercase_name + " ").length()) ||
					comp_substring("mod " + lowercase_name + " ", dummy_line, ("mod " + lowercase_name + " ").length()) ||
					comp_substring("md " + lowercase_name + " ", dummy_line, ("md " + lowercase_name + " ").length()) ||
					comp_substring("init " + lowercase_name + " ", dummy_line, ("init " + lowercase_name + " ").length()) ||
					comp_substring("initiative " + lowercase_name + " ", dummy_line, ("initiative " + lowercase_name + " ").length()) ||
					comp_substring("int " + lowercase_name + " ", dummy_line, ("int " + lowercase_name + " ").length()) ||
					comp_substring("i " + lowercase_name + " ", dummy_line, ("i " + lowercase_name + " ").length()) ||
					comp_substring("m " + lowercase_name + " ", dummy_line, ("m " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " mv ", dummy_line, (lowercase_name + " mv ").length()) ||
					comp_substring(lowercase_name + " md ", dummy_line, (lowercase_name + " md ").length()) ||
					comp_substring(lowercase_name + " mod ", dummy_line, (lowercase_name + " mod ").length()) ||
					comp_substring(lowercase_name + " i ", dummy_line, (lowercase_name + " i ").length()) ||
					comp_substring(lowercase_name + " init ", dummy_line, (lowercase_name + " init ").length()) ||
					comp_substring(lowercase_name + " initiative ", dummy_line, (lowercase_name + " initiative ").length()) ||
					comp_substring(lowercase_name + " ", dummy_line, lowercase_name.length() + 1))
					{
						try {
							bool is_signed;
							if (!i->touched)
							{
								int val = get_number_arg(dummy_line, is_signed, creatures, i->get_raw_ptr());
								i->set_initiative(val);
								i->touched = true;
								creatures.sort();
								i = creatures.begin();
								start_over = true;
								break;
							}
							
							used_command = true;
						}
						catch (const std::exception& E) {
							//std::cout << E.what() << std::endl;
						}
					}

				else if (comp_substring("roll ", dummy_line, 5))
				{
					bool is_signed;
					std::string base_dice_pattern = dummy_line.substr(5);
					std::string dice_pattern;
					for (size_t j = 0; j < base_dice_pattern.size(); ++j)
					{
						char c = base_dice_pattern[j];
						if (c != ' ')
							dice_pattern += c;
					}
					make_lowercase(dice_pattern);
					turn_msg = "Rolled " + dice_pattern + ", got ";
					int val = get_number_arg("roll " + dice_pattern, is_signed, creatures, i->get_raw_ptr());
					turn_msg += std::to_string(val);
					turn_msg += "\n\n";
					used_command = true;
					break;
				}
				
				else if (
					comp_substring("alias " + lowercase_name + " ", dummy_line, ("alias " + lowercase_name + " ").length())
					)
					{
						std::string new_alias = dummy_line.substr(("alias " + lowercase_name + " ").length());
						trim(new_alias);
						if (name_is_unique(new_alias, creatures))
						{
							i->add_alias(new_alias);
							used_command = true;
							break;
						}
					}

				//if (did_erase || used_command)
					//break;
			}

			//if (did_erase || used_command)
				//break;
		}
		if (!used_command && dummy_line.substr(0,4)=="add " && dummy_line.length()>4)
		{
			//dummy_line = dummy_line.substr(4);
			//original_dummy_line = original_dummy_line.substr(4);
			std::ifstream file;
			bool dummy_taking_initiatives = true;
			used_command = true;
			bool success = get_creature(creatures, dummy_taking_initiatives, original_dummy_line, file, false, true, true, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
			if (success)
			{
				creatures.sort();
			}
			else
			{
				std::cout << "Error\n";
			}
		}
		else if (!used_command && dummy_line == "add")
		{
			std::ifstream file;
			bool dummy_taking_initiatives = true;
			used_command = true;
			bool success = get_creature(creatures, dummy_taking_initiatives, dummy_line, file, false, false, false, "", ignore_initial_file_load, wd, false, turn_msg, suppress_display);
			if (success)
			{
				creatures.sort();
			}
			else
			{
				std::cout << "Error\n";
			}
		}

		if (!used_command)
		{
			if (dummy_line == "")
			{
				std::string filename = current_creature->turn_end_file;
				if (filename != "")
				{
					std::ifstream new_file;
					process_filename(filename);
					search_links(filename);
					new_file.open(filename);
					std::string line;
					if (!new_file.is_open())
					{
						std::cout << "Error: Could not open " << filename << std::endl;
						//std::cerr << "\tError details: " << std::strerror(errno) << std::endl;
					}
					else
					{
						bool taking_initiatives = false;
						while (new_file.good() && !new_file.eof())
						{
							get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename, ignore_initial_file_load, wd, false, turn_msg, suppress_display);
						}
						new_file.close();
						creatures.sort();
						turn_count = current_creature->get_turn_count();
						current_turn = 0;
						for (auto i = creatures.begin(); i != creatures.end(); ++i)
						{
							if (i->get_name() == current_creature->get_name())
							{

								break;
							}
							++current_turn;
						}
					}
				}
				current_creature->remove_alias("@current");
				++current_turn;
				new_turn = true;
				file_load_disable = false;

			}
			else
			{
				turn_msg = "Error: Could not execute command (did you make a typo?)\n\n";
			}	
		}
		else
		{
			new_turn = false;
		}

		if (skip)
		{
			move_turn = current_turn + 1;
			current_creature->remove_alias("@current");
			skip = false;
		}

		if (move_turn != -1)
		{
			if (move_turn != current_turn)
			{
				current_creature->remove_alias("@current");
				new_turn = true;
				file_load_disable = false;
			}
			current_turn = move_turn;
		}

		if (current_turn >= creatures.size())
		{
			current_turn = 0;
			++current_round;
			new_round = true;
		}

		if (knocked_out_creature != nullptr)
		{
			turn_msg = knocked_out_creature->get_name() + " WAS KNOCKED OUT!\n\n" + turn_msg;
			knocked_out_creature = nullptr;
		}
		save_buffer(); //For undoing/redoing purposes
	}
}

int main(int argc, char** args)
{
	if (argc > 2)
		std::cout << "Invalid console arguments. Continuing as normal." << std::endl;
	execution_dir = args[0];
	execution_dir = get_directory(execution_dir);
	for (int i = 0; i < execution_dir.size(); ++i)
	{
		if (execution_dir[i] == '\\')
			execution_dir[i] = '/';
	}
	std::ifstream redirect;
	redirect.open(execution_dir + "/REDIRECT.txt");
	if (!redirect.is_open())
		redirect.open(execution_dir + "/REDIRECT");
	if (redirect.is_open())
	{
		std::string line;
		std::getline(redirect, line);
		execution_dir = line;
		for (int i = 0; i < execution_dir.size(); ++i)
		{
			if (execution_dir[i] == '\\')
				execution_dir[i] = '/';
		}
		redirect.close();
	}
	std::cout << "Execution directory: " << execution_dir << std::endl;
	srand(time(NULL)); //Ready the RNG
	std::list<creature> creatures; //Create a list to hold the creature data in

	//Display help instructions
	std::cout << "Program started, begin character entries below. See \"instrunctions.txt\" for help." << std::endl;

	const static bool PROMPT_FILE_LOAD = false;
	
	std::string line; //A place to store input from the keyboard
	bool taking_intiatives = true; //Boolean to track whether or not user is done entering initiatives yet or not
	std::ifstream file;
	std::string filename = "";
	if (argc == 2) //First arg is the directory the program is executed at.
	{
		filename = args[1];
		file.open(filename);

		if (!file.is_open())
		{
			std::cout << "Error: Could not open " << filename << std::endl;
			std::cout << "Proceeding as normal" << std::endl;
		}
	}
	else
	{
		if (PROMPT_FILE_LOAD)
		{
			std::cout << "Load encounter from file? (Y/N)" << std::endl;
			while (line != "y" && line != "n")
			{
				std::getline(std::cin, line);
				trim(line);
				make_lowercase(line);
			}

			if (line == "y")
			{
				std::cout << "Enter file:" << std::endl;
				std::getline(std::cin, line);
				filename = line;
				file.open(filename);
				while (!file.is_open())
				{
					std::cout << "Error: Could not open " << line << std::endl;
					std::cout << "Enter file:" << std::endl;
					std::getline(std::cin, line);
					file.open(line);
				}
			}
		}
	}
	bool ignore_initial_file_load = false;
	std::string dummy_turn_msg = "";
	while (taking_intiatives) //Allow user to enter initiatives
	{
		get_creature(creatures, taking_intiatives, line, file, true, false, true, filename, ignore_initial_file_load, wd, true, dummy_turn_msg, false);
	}
	bool initial_no_script_run_override = true;
	//If it gets here then the user has entered 'stop' or 'done' or 'end', so it's ready to move to tracking mode
	if (creatures.size() == 0)
	{
		std::cout << "Detected 0 creatures." << std::endl;
	}
	else
	{
		track_initiatives(creatures, line, ignore_initial_file_load, initial_no_script_run_override);
	}
	std::cout << "Combat has ended. Press \'enter\' to terminate program." << std::endl;
	std::getline(std::cin, line);
	return 0;
}
