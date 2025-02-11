/*
This may just be some of the worst code I've ever written. 
Originally it was intended to track initiatives and nothing else, but over time it's become a tool that handles more and more, far beyond the original scope of what
it was intended to do.
At first it started small - just hit point tracking. It was only a couple commands, no need to write a sophisticated and robust system to manage them right? Just
a quick'n'dirty hack to add one small feature.
But after one week I realized I needed just a few more features to make it more useful, so I added just a few more commands to rearrange initiatives and that's it.
That was probably the point at which I should have started overhauling the code, but I figured it was just a one-time thing and then I'd be done.
But time and time again, I kept adding "just a little more", adding hack after little hack, growing like a tumor.
Now it's spiraled out of control, and to rewrite it is more work than just adding a little bit to the festering pile. So this is how it remains.

Anyhow: SPAGHETTI WARNING

*/
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <random>
#include <fstream>
#include <climits>
#include <thread>
#include <chrono>
#include <time.h>
#include <map>

const static bool PRINT_DEBUG = false;
typedef size_t index_t;
static int initial_round = 1; //Global variables are a bad practice
static std::string initial_turn = ""; //Stores name of character whose turn will start
void trim(std::string& str);
bool simple_display = false; //Controls whether or not simple display mode is enabled.
//Makes an existing string lowercase in-place

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

const static int SHOW_ONE_NAME = 0;
const static int SHOW_SOME_NAMES = 1;
const static int SHOW_ALL_NAMES = 2;
class creature
{
	int initiative, modifier, hp, max_hp, turn_count, temp_hp, regen, ac=-1;
	bool temp_disable_regen = false;
	std::string name, reminder;
public:
	std::list<std::string> flags;
	std::map<std::string, bool> flags_hidden;
	std::list<std::string> aliases;
	std::map<std::string, int> variables;
	bool touched = false;
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

	int get_var(std::string var_name)
	{
		make_lowercase(var_name);
		if (variables.count(var_name) == 0)
		{
			return 0;
		}
		else
		{
			return variables[var_name];
		}
	}

	void set_var(std::string var_name, int value)
	{
		make_lowercase(var_name);
		if (variables.count(var_name) != 0)
		{
			variables[var_name] = value;
		}
		else
		{
			variables.emplace(var_name, value);
		}
	}

	void remove_var(std::string var_name)
	{
		make_lowercase(var_name);
		if (variables.count(var_name) != 0)
		{
			variables.erase(var_name);
		}
	}

	inline void set_reminder(const std::string& new_reminder)
	{
		reminder = new_reminder;
	}

	inline const std::string& get_reminder() const
	{
		return reminder;
	}
	
	inline void add_alias(const std::string& new_alias)
	{
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
		int MAX_ALIASES = 2;
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

	inline void add_flag(const std::string& flag_name)
	{
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
	creature(const std::string& name, int initiative, int modifier, int max_hp, int hp, int temp_hp, const std::string& flags_list, const std::string& alias_list, int regeneration, int armor_class, const std::list<creature> const* creatures_list) : name(name), initiative(initiative), modifier(modifier), temp_hp(temp_hp),
		hp(hp), max_hp(max_hp), turn_count(-1), regen(regeneration), ac(armor_class)
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
						add_flag(flag);
						flag = "";
					}
					break;
				}
				if (c==',' || c=='&')
				{
					add_flag(flag);
					flag = "";
				}
				else
				{
					flag += c;
				}
			}
			if (flag != "")
			{
				add_flag(flag);
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
	}

	creature(const std::string& name, int initiative, int modifier) : name(name), initiative(initiative), modifier(modifier),
		hp(-1), max_hp(-1), turn_count(-1), temp_hp(0), regen(0), ac(-1)
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

inline bool comp_substring(const std::string& first, const std::string& second, size_t chars_to_compare) {
	for (index_t i = 0; i < chars_to_compare; ++i) {
		if (first[i] != second[i])
			return false;
	}
	return true;
}

inline void trim(std::string& str)
{
	size_t leading_spaces, trailing_spaces;
	for(leading_spaces = 0; leading_spaces < str.length() && str[leading_spaces] == ' '; ++leading_spaces){}
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
		out.open(filename);
		if (!out.is_open())
			throw;

		if(!temp_file)
			out << "reset\n";

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

			if (i->get_reminder().size() != 0)
			{
				out << "reminder " << i->get_name() << " " << i->get_reminder() << std::endl;
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
		case '-': break;
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

inline int get_number_arg(const std::string& dummy_line, bool& is_signed)
{
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

	//std::cout << "PARSED SUBSTRING: " << sub << std::endl;
	
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
	base->touched = true;
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

				while (!name_is_unique(copy_name, creatures) || copy.has_alias(copy_name))
				{
					if (base0)
					{
						copy_name = base0_name + std::to_string(++base_copy_id);
					}
					else
					{
						copy_name = base_name + std::to_string(++base_copy_id);
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
		creatures.push_back(copy);
	}
	creatures.sort();
}


void command_replacement(std::string& dummy_line)
{
	if (dummy_line == "complex display")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "max display")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "display full")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "display max")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "min display")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "display min")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "sdisplay")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "displays")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "fdisplay")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "displayf")
	{
		dummy_line = "full display";
		return;
	}



	if (dummy_line == "complex disp")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "max disp")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "disp full")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "fulldisp")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "maxdisp")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "disp max")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "min disp")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "disp min")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "mindisp")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "dispmin")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "sdisp")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "disps")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "simpdisp")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "fdisp")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "dispf")
	{
		dummy_line = "full display";
		return;
	}
	if (dummy_line == "simple disp")
	{
		dummy_line = "simple display";
		return;
	}
	if (dummy_line == "simpledisp")
	{
		dummy_line = "simple display";
		return;
	}

	dummy_line = replace_all(dummy_line, " .", ".", false);
	dummy_line = replace_all(dummy_line, ". ", ".", false);
	dummy_line = replace_all(dummy_line, "talfg", "tf", true,false);
	dummy_line = replace_all(dummy_line, "tflag", "tf", true,false);
	dummy_line = replace_all(dummy_line, "tfalg", "tf", true,false);
	dummy_line = replace_all(dummy_line, "tlafg", "tf", true,false);
	dummy_line = replace_all(dummy_line, "talfg", "tf", true,false);
	dummy_line = replace_all(dummy_line, "ralias", "ra", true, false);
	dummy_line = replace_all(dummy_line, "aliasr", "ra", true, false);
	dummy_line = replace_all(dummy_line, "ar", "ra", true, false);
	dummy_line = replace_all(dummy_line, "removealias", "ra", true, false);
	dummy_line = replace_all(dummy_line, "remove_alias", "ra", true, false);
	dummy_line = replace_all(dummy_line, "aliasremove", "ra", true, false);
	dummy_line = replace_all(dummy_line, "alias_remove", "ra", true, false);
}

//Process command/add a creature, and return whether or not a creature was added.
inline bool get_creature(std::list<creature>& creatures, bool& taking_intiatives, std::string& line, std::ifstream& file, bool takes_commands, bool info_already_in_line, bool may_expect_add_keyword, const std::string& filename)
{
	//takes_commands = true;
	bool added_creature = false;
	bool using_file = file.is_open() && file.good();
	std::cout << std::endl << "________________________________________________" << std::endl;
	std::string lowercase, name, initiative_string, mod_string;
	for (auto i = creatures.begin(); i != creatures.end(); ++i)
	{
		std::cout << i->get_display_names();
		if (i->get_max_hp() != -1)
			std::cout << " (" << i->get_hp() << "/" << i->get_max_hp() << " hp)";
		if (i->get_flags().size() != 0)
			std::cout << "; [" << i->get_flag_list(false, true,true,true) << "]";
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
		if(!printed_vars)
			std::cout << std::endl;
	}
	std::cout << "\nEnter creature name + initiative:" << std::endl;
	if (using_file && !info_already_in_line)
	{
		if (file.eof() || !file.good())
		{
			return false;
		}
		else
		{
			std::getline(file, line);
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

				else if (comp_substring("clone " + lowercase_name + " ", dummy_line, ("clone " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int clones = get_number_arg(dummy_line, is_signed);
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
						i->touched = true;
						i = creatures.begin();
						//return false;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " clone ", dummy_line, (lowercase_name + " clone ").length()))
				{
					try {
						bool is_signed = false;
						int clones = get_number_arg(dummy_line, is_signed);
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
						i->touched = true;
						//return false;
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
					//return false;
				}
				else if (comp_substring(lowercase_name + " clone", dummy_line, (lowercase_name + " clone").length()))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
					i->touched = true;
					i = creatures.begin();
					//return false;
				}

				else if (comp_substring(lowercase_name + " tf ", dummy_line, (lowercase_name + " tf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " tf ").length();
						std::string arg = dummy_line.substr(start_length);

						i->add_flag("_" + arg);
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

						i->add_flag("_" + arg);
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

				else if (comp_substring(lowercase_name + " dv ", dummy_line, (lowercase_name + " dv ").length()))
				{
					try {
						size_t start_length = lowercase_name.length() + 4;
						std::string var = dummy_line.substr(start_length);
						trim(var);
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
						i->remove_var(var);

						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + ".", dummy_line, (lowercase_name + ".").length()))
				{
					try {
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
							var[lowercase_name.length()] = ' ';
						int space = std::string::npos;
						const int VAR_SET = 1;
						const int VAR_ADD = 2;
						const int VAR_SUB = 3;
						const int VAR_INCREMENT = 4;
						const int VAR_DECREMENT = 5;
						int SET_TYPE = VAR_SET;

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
						if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
							val = get_number_arg(var, is_signed);

						var.resize(space);
						switch (SET_TYPE)
						{
						case VAR_ADD: i->set_var(var, i->get_var(var) + val); break;
						case VAR_SUB: i->set_var(var, i->get_var(var) - val); break;
						case VAR_SET: i->set_var(var, val); break;
						case VAR_INCREMENT: i->set_var(var, i->get_var(var) + val); break;
						case VAR_DECREMENT: i->set_var(var, i->get_var(var) - val); break;
						default: throw;
						}
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("--" + lowercase_name + ".", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) - 1);
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
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + ":", dummy_line, (lowercase_name + ":").length()))
				{
					try {
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
							var[lowercase_name.length()] = ' ';
						int space = std::string::npos;
						const int VAR_SET = 1;
						const int VAR_ADD = 2;
						const int VAR_SUB = 3;
						const int VAR_INCREMENT = 4;
						const int VAR_DECREMENT = 5;
						int SET_TYPE = VAR_SET;

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
						if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
							val = get_number_arg(var, is_signed);

						var.resize(space);
						switch (SET_TYPE)
						{
						case VAR_ADD: i->set_var(var, i->get_var(var) + val); break;
						case VAR_SUB: i->set_var(var, i->get_var(var) - val); break;
						case VAR_SET: i->set_var(var, val); break;
						case VAR_INCREMENT: i->set_var(var, i->get_var(var) + val); break;
						case VAR_DECREMENT: i->set_var(var, i->get_var(var) - val); break;
						default: throw;
						}
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("--" + lowercase_name + ":", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) - 1);
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
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}

			}
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
					creatures.erase(rmc);
					rmc = creatures.begin();
				}
				else
				{
					++rmc;
				}
			}
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
					creatures.erase(rmc);
					rmc = creatures.begin();
				}
				else
				{
					++rmc;
				}
			}
			return false;
		}

		std::string& original_dummy_line = line;
		bool did_erase = false;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			if (start_over)
			{
				start_over = false;
				i = creatures.begin();
			}
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
						int val = get_number_arg(dummy_line, is_signed);
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
						int new_ac = get_number_arg(dummy_line, is_signed);
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
						int new_ac = get_number_arg(dummy_line, is_signed);
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

				else if (comp_substring("heal " + lowercase_name + " ", dummy_line, ("heal " + lowercase_name + " ").length()) ||
					comp_substring(lowercase_name + " heal ", dummy_line, (lowercase_name + " heal ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed);
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
						int val = get_number_arg(dummy_line, is_signed);
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
				else if
					(comp_substring("al " + lowercase_name + " ", dummy_line, ("al " + lowercase_name + " ").length())
						)
				{
					std::string new_alias = dummy_line.substr(("al " + lowercase_name + " ").length());
					trim(new_alias);
					if (name_is_unique(new_alias, creatures))
					{
						i->add_alias(new_alias);
					}
				}
				else if
					(comp_substring("as " + lowercase_name + " ", dummy_line, ("as " + lowercase_name + " ").length())
						)
				{
					std::string new_alias = dummy_line.substr(("al " + lowercase_name + " ").length() );
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
				else if (comp_substring("fr " + lowercase_name + " ", dummy_line, ("fr " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("fr " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->remove_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " fr ", dummy_line, (lowercase_name + " fr ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " fr ").length();
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

						i->add_flag(arg);
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

						i->add_flag(arg);
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

						i->add_flag(arg);
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

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("add_flag " + lowercase_name + " ", dummy_line, ("add_flag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("add_flag " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " add_flag ", dummy_line, (lowercase_name + " add_flag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " add_flag ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("flag_add " + lowercase_name + " ", dummy_line, ("flag_add " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flag_add " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " flag_add ", dummy_line, (lowercase_name + " flag_add ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flag_add ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rmfg " + lowercase_name + " ", dummy_line, ("rmfg " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmfg " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " rmfg ", dummy_line, (lowercase_name + " rmfg ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmfg ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}


				else if (comp_substring("addf " + lowercase_name + " ", dummy_line, ("addf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("addf " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " addf ", dummy_line, (lowercase_name + " addf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " addf ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("adf " + lowercase_name + " ", dummy_line, ("adf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adf " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " adf ", dummy_line, (lowercase_name + " adf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adf ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("adflg " + lowercase_name + " ", dummy_line, ("adflg " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adflg " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " adflg ", dummy_line, (lowercase_name + " adflg ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adflg ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("rmflg " + lowercase_name + " ", dummy_line, ("rmflg " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmflg " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " rmflg ", dummy_line, (lowercase_name + " rmflg ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmflg ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}


				else if (comp_substring("rmf " + lowercase_name + " ", dummy_line, ("rmf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmf " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("frm " + lowercase_name + " ", dummy_line, ("frm " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("frm " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " rmf ", dummy_line, (lowercase_name + " rmf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmf ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " frm ", dummy_line, (lowercase_name + " frm ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " frm ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("rmflag " + lowercase_name + " ", dummy_line, ("rmflag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmflag " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("flagrm " + lowercase_name + " ", dummy_line, ("flagrm " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flagrm " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " rmflag ", dummy_line, (lowercase_name + " rmflag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmflag ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " flagrm ", dummy_line, (lowercase_name + " flagrm ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flagrm ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
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
				else if (comp_substring("fr " + lowercase_name + " ", dummy_line, ("fr " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("fr " + lowercase_name + " ").length();
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
				else if (comp_substring(lowercase_name + " fr ", dummy_line, (lowercase_name + " fr ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " fr ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("rflag " + lowercase_name + " ", dummy_line, ("rflag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rflag " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				
				}

				else if (comp_substring("flagr " + lowercase_name + " ", dummy_line, ("flagr " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flagr " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " rflag ", dummy_line, (lowercase_name + " rflag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rflag ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " flagr ", dummy_line, (lowercase_name + " flagr ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flagr ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("adfl " + lowercase_name + " ", dummy_line, ("adfl " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adfl " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("adlf " + lowercase_name + " ", dummy_line, ("adlf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adlf " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " adfl ", dummy_line, (lowercase_name + " adfl ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adfl ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring(lowercase_name + " adlf ", dummy_line, (lowercase_name + " adlf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adlf ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("heal all ", dummy_line, 9) || dummy_line == "heal all max" || dummy_line == "heal all all" || dummy_line == "heal all full")
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed);
						unsigned char* tval = reinterpret_cast<unsigned char*>(&(i->touched));
						if (!i->touched)
						{
							i->adjust_hp(val);
							i->touched = true;
						}

						if (i == (--creatures.end()))
							return false;

					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
				}

				else if (comp_substring("temp_hp " + lowercase_name + " ", dummy_line, ("temp_hp " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);
				
					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring("thp " + lowercase_name + " ", dummy_line, ("thp " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring("t " + lowercase_name + " ", dummy_line, ("t " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
					}
				else if (comp_substring("buffer " + lowercase_name + " ", dummy_line, ("buffer " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
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
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring(lowercase_name + " temp ", dummy_line, (lowercase_name + " temp ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring(lowercase_name + " thp ", dummy_line, (lowercase_name + " thp ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring(lowercase_name + " t ", dummy_line, (lowercase_name + " t ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}

				}
				else if (comp_substring(lowercase_name + " buffer ", dummy_line, (lowercase_name + " buffer ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}

				}
				else if (comp_substring(lowercase_name + " temphp ", dummy_line, (lowercase_name + " temphp ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring("temphp " + lowercase_name + " ", dummy_line, ("temphp " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}
				}
				else if (comp_substring("temp " + lowercase_name + " ", dummy_line, ("temp " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);

					if (val < 0)
					{
						std::cout << "Temp HP must be a non-negative number." << std::endl;
					}
					else
					{
						i->set_temp_hp(val, is_signed);
						used_command = true;
					}

				}


				else if (comp_substring("max_hp all ", dummy_line, 7) ||
					comp_substring("all max_hp ", dummy_line, 7) ||
					comp_substring("max_health all ", dummy_line, 11) ||
					comp_substring("all max_health ", dummy_line, 11))
					{
						try {
							if (i->touched == false)
							{
								bool is_signed = false;
								int val = get_number_arg(dummy_line, is_signed);
								i->set_max_hp(val, is_signed);
								i->touched = true;
							}

							if (i == (--creatures.end()))
								used_command = true;

						}
						catch (const std::exception& E) {
							//std::cout << E.what() << std::endl;
						}
						}

				else if (comp_substring("regen " + lowercase_name + " ", dummy_line, ("regen " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);
					//if (val < 0)
						//val = 0;
					i->set_regen(val);
					used_command = true;
				}

				else if (comp_substring(lowercase_name + " regen ", dummy_line, (lowercase_name + " regen ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);
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
					int val = get_number_arg(dummy_line, is_signed);
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
			trim(lowercase);
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
			lowercase = line.	  substr(0, alias_index) + line		.substr(space_after_alias_index + 1, line	  .length() - space_after_alias_index - 1);
			trim(lowercase);
			trim(line);
		}

		index_t& temp_hp_index = flags_index;
		temp_hp_index = lowercase.find("temp:");
		if (temp_hp_index != std::string::npos && temp_hp==0)
		{
			size_t space_after_index = lowercase.find(' ', temp_hp_index);
			size_t length = space_after_index - flags_index;
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
			size_t length = space_after_index - flags_index;
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
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
			line =		line	 .substr(0, flags_index) + line.	 substr(space_after_index + 1, line.	 length() - space_after_index - 1);
			trim(lowercase);
			trim(line);
		}

		temp_hp_index = lowercase.find("thp:");
		if (temp_hp_index != std::string::npos && temp_hp == 0)
		{
			size_t space_after_index = lowercase.find(' ', temp_hp_index);
			size_t length = space_after_index - flags_index;
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
			lowercase = lowercase.substr(0, flags_index) + lowercase.substr(space_after_index + 1, lowercase.length() - space_after_index - 1);
			line =		line.	  substr(0, flags_index) + line.	 substr(space_after_index + 1, line.	 length() - space_after_index - 1);
			trim(lowercase);
			trim(line);
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
			std::string filename = line.substr(5, line.length() - 5);
			std::ifstream new_file;
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
					get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename);
				}
				new_file.close();
				return false;
			}
		}
		else if (takes_commands && (comp_substring("ld ", lowercase, 3)))
		{
			std::string filename = line.substr(3, line.length() - 3);
			std::ifstream new_file;
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
					get_creature(creatures, taking_intiatives, line, new_file, true, false, true, filename);
				}
				new_file.close();
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
						creatures.emplace_back(name, initiative, modifier, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures);
						added_creature = true;
					}
					else
					{
						int initiative = std::stoi(initiative_string);
						creatures.emplace_back(name, initiative, 0, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures);
						added_creature = true;
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
						creatures.emplace_back(name, initiative, modifier, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures);
						added_creature = true;

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
				creatures.emplace_back(name, 1 + (rand() % 20), 0, max_hp, hp, temp_hp, flags, aliases, regen_amnt, ac_value, &creatures);
				added_creature = true;
			}
			else
			{
				std::cout << "Error: Malformed input (did you forget the space, or add extras?)" << std::endl;
				//std::cout << lowercase << std::endl;
			}
		}
	}

	return added_creature;
}








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////








const static int MAX_UNDO_STEPS = 20;
std::list<std::list<creature>> creatures_buffer;
std::list<bool> new_round_buffer;
std::list<std::string> turn_msg_buffer;
std::list<index_t> current_turn_buffer;
std::list<size_t> current_round_buffer;
std::list<bool> display_mode_buffer;

std::string get_hp_change_turn_msg(const std::string& name, int old_hp, int new_hp, const std::string& cur_msg)
{
	std::string str = cur_msg + name;
	int diff = old_hp - new_hp;
	if (diff < 0)
		diff = -diff;
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
		return cur_msg;
	}
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
				text += "\t    " + vi->first + " = " + std::to_string(vi->second);
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
					text += "\t    " + vi->first + " = " + std::to_string(vi->second);
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
	if(i->get_initiative()>=0)
		turn_msg += "\tInitiative: " + std::to_string(i->get_initiative()) + " (+" + std::to_string(i->get_initiative_modifier()) + ")\n";
	else
		turn_msg += "\tInitiative: " + std::to_string(i->get_initiative()) + " (-" + std::to_string(i->get_initiative_modifier()) + ")\n";
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
	if (i->get_reminder() != "")
		turn_msg += "\tReminder: \"" + i->get_reminder() + "\"\n";
	if (i->variables.size() != 0)
	{
		turn_msg += "\tVariables:\n";
		turn_msg += print_variables(i->get_raw_ptr(), true);
	}
	turn_msg += "\n";
	return turn_msg;
}

inline void track_initiatives(std::list<creature>& creatures, std::string& dummy_line)
{
	//std::sort(creatures.begin(), creatures.end());
	creatures.sort();
	index_t current_turn = 0;
	size_t current_round = initial_round;
	bool new_round = false;
	std::string previous_turn_creature_name = "";
	creature* knocked_out_creature = nullptr;
	std::string turn_msg = "";
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
	const static int STATE_NODO = 0;
	const static int STATE_UNDO = 1;
	const static int STATE_REDO = 2;
	int buffer_manipulation_state = STATE_NODO;
	
	std::list<std::list<creature>>::iterator creatures_buffer_iterator;
	std::list<bool>::iterator new_round_buffer_iterator;
	std::list<std::string>::iterator turn_msg_buffer_iterator;
	std::list<index_t>::iterator current_turn_buffer_iterator;
	std::list<size_t>::iterator current_round_buffer_iterator;
	std::list<bool>::iterator display_mode_buffer_iterator;
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

				creatures_buffer_iterator = creatures_buffer.begin();
				new_round_buffer_iterator = new_round_buffer.begin();
				turn_msg_buffer_iterator = turn_msg_buffer.begin();
				current_turn_buffer_iterator = current_turn_buffer.begin();
				current_round_buffer_iterator = current_round_buffer.begin();
				display_mode_buffer_iterator = display_mode_buffer.begin();
				if (creatures_buffer.size() > MAX_UNDO_STEPS)
				{
					creatures_buffer.pop_back();
					new_round_buffer.pop_back();
					turn_msg_buffer.pop_back();
					current_turn_buffer.pop_back();
					current_round_buffer.pop_back();
					display_mode_buffer.pop_back();
				}
			}
		};

	save_buffer(); //To initialize the state buffers so they have a place to begin.
	while (true) //Terminated only by an explicit command to do so, which returns the funtion.
	{
		clear();
		//BEGIN UNDO/REDO BUFFER STUFF
		
		
		//First save the current state of the program to the buffer.
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
		}

		//Then check if it should switch to a previous buffer-state
		

		//END UNDO/REDO BUFFER STUFF
		if (new_round)
		{
			std::cout << "Start of a new round." << std::endl;
			new_round = false;
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
			turn_msg = "";
		}
		else
		{
			std::cout << get_info(current_creature_2, current_turn, current_round, true) << std::endl;
		}
		std::cout << "-------------------------INITIATIVE DISPLAY-------------------------\n" << std::endl;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			i->touched = false;
			if (current_turn == turn_count)
			{
				std::cout << "  ---> ";
				current_creature = &(*i);
				if (current_creature->get_name() != previous_turn_creature_name)
				{
					new_turn = true;
				}

				if (new_turn)
				{
					int regen = current_creature->get_regen();
					if (regen != 0 && (current_creature->get_hp() != current_creature->get_max_hp()))
					{
						int old_hp = current_creature->get_hp();
						current_creature->adjust_hp(regen);
						int new_hp = current_creature->get_hp();
						regenerated_hp = new_hp - old_hp;
					}
				}
			}
			std::cout << i->get_display_names();
			if(!simple_display)
				std::cout << " [" << i->get_initiative() << "]";
			if (i->get_ac() != -1)
			{
				std::cout << " <AC " << i->get_ac() << ">";
			}
			
			if (i->get_max_hp() != -1) {
				if(i->get_temp_hp() == 0)
					std::cout << "; " << i->get_hp() << " / " << i->get_max_hp() << " HP";
				else
					std::cout << "; " << i->get_hp() << "[+" << i->get_temp_hp() << " temp]" << " / " << i->get_max_hp() << " HP";
			}
			if (i->get_flag_list((current_turn == turn_count && new_turn), true, !simple_display, true)!="")
			{
				std::cout << " | FLAGS: " << i->get_flag_list((current_turn == turn_count && new_turn), true, !simple_display, true);
			}
			if(current_turn==turn_count)
				std::cout << " <-----------";
			std::cout << std::endl;

			std::cout << print_variables(i->get_raw_ptr(), false);
			i->set_turn_count(turn_count);
			++turn_count;
		}
		turn_msg = "";
		std::cout << std::endl << "It's " << current_creature->get_name() << "\'s turn." << std::endl;
		previous_turn_creature_name = current_creature->get_name();
		if (new_turn)
		{
			if (current_creature->get_reminder().size() != 0)
			{
				std::cout << "\t" << current_creature->get_reminder() << std::endl;
			}
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
		std::getline(std::cin, dummy_line);
		trim(dummy_line);
		std::string original_dummy_line = dummy_line;
		std::string& line = original_dummy_line;
		make_lowercase(dummy_line);
		command_replacement(dummy_line);
		//std::cout << "FULLY REPLACED: " << dummy_line << std::endl;
		bool used_command = false;
		bool did_erase = false;
		int move_turn = -1;
		size_t l = dummy_line.length() - 1;
		//std::string lowercase_current_creature_name = get_lowercase(current_creature->get_name());
		std::string keep_name = "";
		bool skip_command_checks = false;
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
			}
		}
		
		if (dummy_line == "clean" || dummy_line == "cleanup")
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
		else if (dummy_line == "pause")
		{
			skip_command_checks = true;
			used_command = true;
		}
		else if ((comp_substring("load ", dummy_line, 5)))
		{
			std::string filename = line.substr(5, line.length() - 5);
			std::ifstream new_file;
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
					get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename);
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
				continue;
			}
		}
		else if ((comp_substring("ld ", dummy_line, 3)))
		{
			std::string filename = line.substr(3, line.length() - 3);
			std::ifstream new_file;
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
					get_creature(creatures, taking_initiatives, line, new_file, true, false, true, filename);
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
				continue;
			}
		}
		else if ((comp_substring("keep ", dummy_line, 5)))
		{
			keep_name = dummy_line.substr(5);
			trim(keep_name);
		}
		else if (dummy_line.length() > 5 && dummy_line[l] == 'p' && dummy_line[l - 1] == 'e' && dummy_line[l - 2] == 'e' && dummy_line[l - 3] == 'k' && dummy_line[l - 4] == ' ')
		{
			keep_name = dummy_line;
			keep_name.resize(dummy_line.size() - 5);
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
							creatures.erase(i);
							i = creatures.begin();
						}
						else
						{
							i->touched = true;
							++i;
						}
					}
					else
					{
						if (i->has_alias(lowercase_name))
						{
							creatures.erase(i);
							i = creatures.begin();
						}
						else
						{
							++i;
						}
					}
				}
				i = creatures.begin();
				next = i;
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
						int val = get_number_arg(dummy_line, is_signed);
						int old_hp = i->get_hp();
						i->adjust_hp(-val);
						int new_hp = i->get_hp();
						if (i->get_hp() == 0) 
						{
							knocked_out_creature = &(*i);
						}
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg);
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
						int val = get_number_arg(dummy_line, is_signed);
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

				else if (comp_substring("clone " + lowercase_name + " ", dummy_line, ("clone " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int clones = get_number_arg(dummy_line, is_signed);
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
						int clones = get_number_arg(dummy_line, is_signed);
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (dummy_line == ("clone " + lowercase_name))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " clone", dummy_line, (lowercase_name + " clone").length()))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
				}

				else if (comp_substring("ac " + lowercase_name + " ", dummy_line, ("ac " + lowercase_name + " ").length()))
				{
					try {
						bool is_signed = false;
						int new_ac = get_number_arg(dummy_line, is_signed);
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
						int new_ac = get_number_arg(dummy_line, is_signed);
						i->set_ac(new_ac);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("regen " + lowercase_name + " ", dummy_line, ("regen " + lowercase_name + " ").length()))
				{
					bool is_signed = false;
					int val = get_number_arg(dummy_line, is_signed);
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
					int val = get_number_arg(dummy_line, is_signed);
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
						int val = get_number_arg(dummy_line, is_signed);
						val >>= 1;
						int old_hp = i->get_hp();
						i->adjust_hp(-val);
						int new_hp = i->get_hp();
						if (i->get_hp() == 0)
						{
							knocked_out_creature = &(*i);
						}
						used_command = true;
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg);
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
						int val = get_number_arg(dummy_line, is_signed);
						val <<= 1;
						int old_hp = i->get_hp();
						i->adjust_hp(-val);
						int new_hp = i->get_hp();
						if (i->get_hp() == 0)
						{
							knocked_out_creature = &(*i);
						}
						used_command = true;
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg);
					}
					catch (const std::exception& E) {

					}
				}
				
				else if (comp_substring("heal " + lowercase_name + " ", dummy_line, ("heal " + lowercase_name + " ").length()) ||
						 comp_substring(lowercase_name + " heal ", dummy_line, (lowercase_name + " heal ").length()))
				{
					try {
						bool is_signed = false;
						int val = get_number_arg(dummy_line, is_signed);
						int old_hp = i->get_hp();
						i->adjust_hp(val);
						int new_hp = i->get_hp();
						turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg);
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
						int val = get_number_arg(dummy_line, is_signed);
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
						int clones = get_number_arg(dummy_line, is_signed);
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
						int clones = get_number_arg(dummy_line, is_signed);
						clone_character(lowercase_name, clones, creatures, i->get_raw_ptr());
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("clone " + lowercase_name, dummy_line, ("clone " + lowercase_name).length()))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
				}
				else if (comp_substring(lowercase_name + " clone", dummy_line, (lowercase_name + " clone").length()))
				{
					clone_character(lowercase_name, 1, creatures, i->get_raw_ptr());
					used_command = true;
				}


				else if (comp_substring("flag " + lowercase_name + " ", dummy_line, ("flag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flag " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
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

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("adflg " + lowercase_name + " ", dummy_line, ("adflg " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adflg " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " adflg ", dummy_line, (lowercase_name + " adflg ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adflg ").length();
						std::string arg = line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring("rmflg " + lowercase_name + " ", dummy_line, ("rmflg " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmflg " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring(lowercase_name + " rmflg ", dummy_line, (lowercase_name + " rmflg ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmflg ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring("rmfg " + lowercase_name + " ", dummy_line, ("rmfg " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmfg " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring(lowercase_name + " rmfg ", dummy_line, (lowercase_name + " rmfg ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmfg ").length();
						std::string arg = line.substr(start_length);

						i->remove_flag(arg);
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

						i->add_flag(arg);
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

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}

				else if (comp_substring("add_flag " + lowercase_name + " ", dummy_line, ("add_flag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("add_flag " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " add_flag ", dummy_line, (lowercase_name + " add_flag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " add_flag ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring("flag_add " + lowercase_name + " ", dummy_line, ("flag_add " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flag_add " + lowercase_name + " ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " flag_add ", dummy_line, (lowercase_name + " flag_add ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flag_add ").length();
						std::string arg = line.substr(start_length);
						used_command = true;

						i->add_flag(arg);
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring("adfl " + lowercase_name + " ", dummy_line, ("adfl " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adfl " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring("adlf " + lowercase_name + " ", dummy_line, ("adlf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adlf " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring(lowercase_name + " adfl ", dummy_line, (lowercase_name + " adfl ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adfl ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring(lowercase_name + " adlf ", dummy_line, (lowercase_name + " adlf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adlf ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}


				else if (comp_substring("addf " + lowercase_name + " ", dummy_line, ("addf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("addf " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " addf ", dummy_line, (lowercase_name + " addf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " addf ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}

				else if (comp_substring("adf " + lowercase_name + " ", dummy_line, ("adf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("adf " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->add_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " adf ", dummy_line, (lowercase_name + " adf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " adf ").length();
						std::string arg = dummy_line.substr(start_length);

						i->add_flag(arg);
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

						i->add_flag("_" + arg);
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

						i->add_flag("_" + arg);
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

				else if (comp_substring(lowercase_name + ".", dummy_line, (lowercase_name + ".").length()))
				{
					try {
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						if (var.size()>lowercase_name.length() && var[lowercase_name.length()] == '=')
							var[lowercase_name.length()] = ' ';
						int space = std::string::npos;
						const int VAR_SET = 1;
						const int VAR_ADD = 2;
						const int VAR_SUB = 3;
						const int VAR_INCREMENT = 4;
						const int VAR_DECREMENT = 5;
						int SET_TYPE = VAR_SET;

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
							if(space != std::string::npos)
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
						if((SET_TYPE!=VAR_INCREMENT) && (SET_TYPE!=VAR_DECREMENT))
							val = get_number_arg(var, is_signed);
						
						var.resize(space);
						switch (SET_TYPE)
						{
							case VAR_ADD: i->set_var(var, i->get_var(var) + val); break;
							case VAR_SUB: i->set_var(var, i->get_var(var)-val); break;
							case VAR_SET: i->set_var(var, val); break;
							case VAR_INCREMENT: i->set_var(var, i->get_var(var) + val); break;
							case VAR_DECREMENT: i->set_var(var, i->get_var(var) - val); break;
							default: throw;
						}
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("--" + lowercase_name + ".", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) - 1);
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
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
				}
				else if (comp_substring(lowercase_name + ":", dummy_line, (lowercase_name + ":").length()))
				{
					try {
						std::string var = dummy_line.substr(lowercase_name.length() + 1);
						if (var.size() > lowercase_name.length() && var[lowercase_name.length()] == '=')
							var[lowercase_name.length()] = ' ';
						int space = std::string::npos;
						const int VAR_SET = 1;
						const int VAR_ADD = 2;
						const int VAR_SUB = 3;
						const int VAR_INCREMENT = 4;
						const int VAR_DECREMENT = 5;
						int SET_TYPE = VAR_SET;

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
						if ((SET_TYPE != VAR_INCREMENT) && (SET_TYPE != VAR_DECREMENT))
							val = get_number_arg(var, is_signed);

						var.resize(space);
						switch (SET_TYPE)
						{
						case VAR_ADD: i->set_var(var, i->get_var(var) + val); break;
						case VAR_SUB: i->set_var(var, i->get_var(var) - val); break;
						case VAR_SET: i->set_var(var, val); break;
						case VAR_INCREMENT: i->set_var(var, i->get_var(var) + val); break;
						case VAR_DECREMENT: i->set_var(var, i->get_var(var) - val); break;
						default: throw;
						}
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring("--" + lowercase_name + ":", dummy_line, ("--" + lowercase_name + ".").length()))
				{
					try {
						if (dummy_line.length() <= lowercase_name.length() + 3)
							throw;
						std::string var = dummy_line.substr(lowercase_name.length() + 3);
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) - 1);
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
						if (i->variables.count(var) != 0)
							i->set_var(var, i->get_var(var) + 1);
						used_command = true;
					}
					catch (const std::exception& E) {}
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

				else if (comp_substring("rmf " + lowercase_name + " ", dummy_line, ("rmf " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmf " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring("frm " + lowercase_name + " ", dummy_line, ("frm " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("frm " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " rmf ", dummy_line, (lowercase_name + " rmf ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmf ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " frm ", dummy_line, (lowercase_name + " frm ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " frm ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring("rmflag " + lowercase_name + " ", dummy_line, ("rmflag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rmflag " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring("flagrm " + lowercase_name + " ", dummy_line, ("flagrm " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flagrm " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " rmflag ", dummy_line, (lowercase_name + " rmflag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rmflag ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}
				else if (comp_substring(lowercase_name + " flagrm ", dummy_line, (lowercase_name + " flagrm ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flagrm ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
					}


				else if (comp_substring("rf " + lowercase_name + " ", dummy_line, ("rf " + lowercase_name + " ").length()))
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
				else if (comp_substring("fr " + lowercase_name + " ", dummy_line, ("fr " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("fr " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

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
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " fr ", dummy_line, (lowercase_name + " fr ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " fr ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("rflag " + lowercase_name + " ", dummy_line, ("rflag " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("rflag " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring("flagr " + lowercase_name + " ", dummy_line, ("flagr " + lowercase_name + " ").length()))
				{
					try {
						size_t start_length = ("flagr " + lowercase_name + " ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " rflag ", dummy_line, (lowercase_name + " rflag ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " rflag ").length();
						std::string arg = original_dummy_line.substr(start_length);

						i->remove_flag(arg);
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (comp_substring(lowercase_name + " flagr ", dummy_line, (lowercase_name + " flagr ").length()))
				{
					try {
						size_t start_length = (lowercase_name + " flagr ").length();
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
							int val = get_number_arg(dummy_line, is_signed);

							i->set_temp_hp(val, is_signed);
							used_command = true;
						}
						catch (const std::exception& E) {

						}
					}


				else if (comp_substring("hp all ", dummy_line, 7) ||
					comp_substring("all hp ", dummy_line, 7) ||
					comp_substring("health all ", dummy_line, 11) ||
					comp_substring("all health ", dummy_line, 11))
					{
						try {
							if (i->touched == false)
							{
								bool is_signed = false;
								int val = get_number_arg(dummy_line, is_signed);
								if (i->get_max_hp() == -1)
								{
									i->set_max_hp(val, false);
								}
								i->set_hp(val, is_signed);
								if (i->get_hp() == 0)
								{
									knocked_out_creature = &(*i);
								}
								i->touched = true;
							}
							
							if (i == (--creatures.end()))
								used_command = true;

						}
						catch (const std::exception& E) {
							//std::cout << E.what() << std::endl;
						}
					}

				else if (comp_substring("max_hp all ", dummy_line, 7) ||
					comp_substring("all max_hp ", dummy_line, 7) ||
					comp_substring("max_health all ", dummy_line, 11) ||
					comp_substring("all max_health ", dummy_line, 11))
					{
						try {
							if (i->touched == false)
							{
								bool is_signed = false;
								int val = get_number_arg(dummy_line, is_signed);
								i->set_max_hp(val, is_signed);
								i->touched = true;
							}

							if (i == (--creatures.end()))
								used_command = true;

						}
						catch (const std::exception& E) {
							//std::cout << E.what() << std::endl;
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
						int val = get_number_arg(dummy_line, is_signed);
						//std::cout << "PARSED HP: " << val << std::endl;
						try {
							size_t slash_index = dummy_line.find("/");
							if (slash_index != std::string::npos)
							{
								try {
									int max_hp = std::stoi(dummy_line.substr(slash_index + 1));
									i->set_max_hp(max_hp, false);
									//std::cout << "PARSED MAX HP " << max_hp << std::endl;
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
							turn_msg = get_hp_change_turn_msg(i->get_name(), old_hp, new_hp, turn_msg);
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
						int val = get_number_arg(dummy_line, is_signed);
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
				else if (
					comp_substring("reroll " + lowercase_name, dummy_line, ("reroll " + lowercase_name).length()) ||
					comp_substring(lowercase_name + " reroll ", dummy_line, (lowercase_name + " reroll ").length()))
				{
					try {
						int roll = 1 + (rand() % 20);
						i->set_initiative( roll+(i->get_initiative_modifier()) );
						creatures.sort();
						used_command = true;
					}
					catch (const std::exception& E) {

					}
				}
				else if (dummy_line == "reroll all")
				{
					int roll = 1 + (rand() % 20);
					i->set_initiative(roll + (i->get_initiative_modifier()));
					if (i == (--creatures.end()))
					{
						used_command = true;
						creatures.sort();
					}
				}
				else if (dummy_line == "reset")
				{
					int roll = 1 + (rand() % 20);
					i->set_initiative(roll + (i->get_initiative_modifier()));
					i->set_hp(i->get_max_hp(), false);
					current_round = 1;
					if (i == (--creatures.end()))
					{
						turn_count = 0;
						move_turn = creatures.begin()->get_turn_count();
						used_command = true;
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
					used_command = true;
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
								int val = get_number_arg(dummy_line, is_signed);
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
					int val = get_number_arg("roll " + dice_pattern, is_signed);
					turn_msg += std::to_string(val);
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
					else if
						(comp_substring("al " + lowercase_name + " ", dummy_line, ("al " + lowercase_name + " ").length())
						)
						{
							std::string new_alias = dummy_line.substr(("al " + lowercase_name + " ").length());
							trim(new_alias);
							if (name_is_unique(new_alias, creatures))
							{
								i->add_alias(new_alias);
								used_command = true;
								break;
							}
					}
					else if
						(comp_substring("as " + lowercase_name + " ", dummy_line, ("as " + lowercase_name + " ").length())
						)
						{
							std::string new_alias = dummy_line.substr(("as " + lowercase_name + " ").length());
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
			bool success = get_creature(creatures, dummy_taking_initiatives, original_dummy_line, file, false, true, true, "");
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
			bool success = get_creature(creatures, dummy_taking_initiatives, dummy_line, file, false, false, false, "");
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
			++current_turn;
			new_turn = true;
		}
		else
		{
			new_turn = false;
		}

		if (move_turn != -1)
		{
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
	
	srand(time(NULL)); //Ready the RNG
	std::list<creature> creatures; //Create a list to hold the creature data in

	//Display help instructions
	std::cout << "When prompted, add creatures by giving its name, initiative, and modifier. The name must be one word." << std::endl;
	std::cout << "The initiative and modifier must be separated by a space (i.e., \"Rogue 17 +6\")" << std::endl;
	std::cout << "When you're done entering creatures, type \'done\' to finish (not case sensitive)" << std::endl;
	std::cout << "When you're tracking initiatives, pressing the \'Enter\' key will advance the initiative counter." << std::endl;
	std::cout << std::endl;
	std::cout << "You can also make it roll initiatives for you by telling it a modifier instead of a roll" << std::endl;
	std::cout << "(i.e., \"Barbarian +3\")" << std::endl;
	std::cout << std::endl;
	std::cout << "Add HP:[Hit Points] or HP:[Current HP]/[Max HP] to enable optional HP tracking." << std::endl;
	std::cout << "Creatures with HP tracking enabled can be hurt or healed with their corresponding commands (\'hurt\' & \'heal\')" << std::endl;
	std::cout << std::endl;
	std::cout << "You can also set a creature's HP with the \'hp\' command." << std::endl;
	std::cout << std::endl;
	std::cout << "You must manually kill creatures with 0 HP via the \'kill\' command." << std::endl;
	std::cout << std::endl << std::endl;
	std::cout << "Flags can be specified when adding creatures with the 'flags:' modifier. Flags are comma-separated and do not permit\nspaces." << std::endl;
	std::cout << "Flags can also be added or removed with 'flag', 'rmflag', and other variations." << std::endl;
	std::cout << "Can also track temp hp, as well as automatic regeneration with \'temp\' and \'regen\'" << std::endl;
	std::cout << "The \'disable\' command temporarily disables a creature\'s regeneration for one round" << std::endl;
	std::cout << std::endl << "Use \'roll [dice pattern]\' to tell the program to roll dice and tell you the output." << std::endl;
	std::cout << "A die pattern (with no spaces) can also be used in most numerical inputs to roll dice instead." << std::endl;
	std::cout << "\tIf the command applies to multiple creatures, it typically rolls each one separately." << std::endl;
	std::cout << std::endl << std::endl;
	std::cout << "Save the state of the encounter with \'save [filename]\', and load it again later with \'load [filename]\'" << std::endl;
	std::cout << "\t\'savec\' or \'savet\' saves the creatures, but does not remove existing characters or set the round number when\n\tloaded." << std::endl;
	std::cout << std::endl;
	std::cout << "Use \'clone\' to clone a character and add them to the tracker. Can also specify how many clones to make." << std::endl << std::endl;
	std::cout << "\'flag\' can be used to add flags to a character. \'rf\' can be used to remove them." << std::endl;
	std::cout << "Starting a flag with an underscore (\'_\') makes it a temporary flag that gets deleted at the start of the character's\nnext turn.\n";
	std::cout << "When referencing characters, use either their name or '@flag' to reference all characters with a given flag.\n@all references all characters.";
	std::cout << std::endl << std::endl;
	std::cout << "\'clean\' or \'cleanup\' removes every character with 0 hp from the turn order.";
	std::cout << std::endl << std::endl;
	std::cout << "\'rm\' can be used to remove addressed creature(s). \'keep\' removes all except the addressed creature(s)." << std::endl;
	std::cout << "Can use \'swap\' to swap two character's initiative values." << std::endl;
	std::cout << "Can use \'simple display\' or \'full display\' to change how much information is shown about each creature on the display." << std::endl;
	std::cout << "Call \'info\' on a creature to display all information about it, even in simple display mode." << std::endl;
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
	while (taking_intiatives) //Allow user to enter initiatives
	{
		get_creature(creatures, taking_intiatives, line, file, true, false, true, filename);
	}

	//If it gets here then the user has entered 'stop' or 'done' or 'end', so it's ready to move to tracking mode
	if (creatures.size() == 0)
	{
		std::cout << "Detected 0 creatures." << std::endl;
	}
	else
	{
		track_initiatives(creatures, line);
	}
	std::cout << "Combat has ended. Press \'enter\' to terminate program." << std::endl;
	std::getline(std::cin, line);
	return 0;
}
