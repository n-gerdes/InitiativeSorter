#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <random>
#include <fstream>
#include <climits>

typedef size_t index_t;
static int initial_round = 1; //Global variables are a bad practice but this is a tiny program
static std::string initial_turn = ""; //Stores name of character whose turn will start

//Makes an existing string lowercase in place
inline void make_lowercase(std::string& str)
{
	for (index_t i = 0; i < str.length(); ++i)
		str[i] = std::tolower(str[i]);
}

//Returns a copy of a string that's made to be lowercase, while preserving the original
inline std::string get_lowercase(std::string str) //Pass by value creates a copy that I can make lowercase before returning
{
	make_lowercase(str);
	return str;
}

class creature
{
	int initiative, modifier, hp, max_hp, turn_count, temp_hp;
	std::string name;
	std::list<std::string> flags;
public:
	inline const std::list<std::string>& get_flags() const
	{
		return flags;
	}

	inline std::string get_flag_list() const
	{
		std::string list;
		for (auto i = flags.begin(); i != flags.end(); ++i)
		{
			list += (*i);
			list += ",";
		}
		if (list.length() != 0)
		{
			list.resize(list.size() - 1); //Remove characters appended at the end
		}

		return list;
	}

	inline void add_flag(const std::string& new_flag)
	{
		if (new_flag.length() != 0 && std::find(flags.begin(), flags.end(), new_flag)==flags.end())
		{
			flags.push_back(new_flag);
		}
	}

	inline void remove_flag(const std::string& flag)
	{
		std::string lowerc = get_lowercase(flag);
		for (auto i = flags.begin(); i != flags.end(); ++i)
		{
			if (get_lowercase(*i) == lowerc)
			{
				flags.erase(i);
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

	creature(const std::string& name, int initiative, int modifier, int max_hp, int hp, int temp_hp, const std::string& flags_list) : name(name), initiative(initiative), modifier(modifier), temp_hp(temp_hp),
		hp(hp), max_hp(max_hp), turn_count(-1)
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
			add_flag(flag);
		}
	}
	creature(const std::string& name, int initiative, int modifier) : name(name), initiative(initiative), modifier(modifier),
		hp(-1), max_hp(-1), turn_count(-1), temp_hp(0)
	{}
	creature(const std::string& name, int initiative) : name(name), initiative(initiative), modifier(0), hp(-1), max_hp(-1), turn_count(-1), temp_hp(0)
	{}

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

	inline int set_hp(int new_hp)
	{
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

	inline void set_temp_hp(int thp)
	{
		temp_hp = thp;
	}

	inline void set_max_hp(int new_max_hp)
	{
		if (new_max_hp < 1)
			new_max_hp = 1;
		max_hp = new_max_hp;
		if (hp > new_max_hp)
			hp = new_max_hp;
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

inline bool name_is_unique(const std::string& name, const std::list<creature>& creatures)
{
	std::string lowerc = get_lowercase(name);
	if (lowerc == "all" || lowerc == "reset" || lowerc == "round" || lowerc == "quit" || lowerc == "hp")
		return false;
	for (auto i = creatures.begin(); i != creatures.end(); ++i)
	{
		if (lowerc == get_lowercase(i->get_name()))
			return false;
	}
	return true;
}

inline void save_state(const std::string& filename, std::list<creature>& creatures, const std::string& turn, int round_num)
{
	try {
		std::ofstream out;
		out.open(filename);
		if (!out.is_open())
			throw;
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
				line += " flags:" + i->get_flag_list();
			}
			line += "\n";

			out << line;
		}

		out << "round " << std::to_string(round_num) << std::endl;
		if (turn != "")
			out << "turn " << turn << std::endl;

		out.close();
	}
	catch (const std::exception& E)
	{
		std::cout << "Error saving to file '" << filename << "'" << std::endl;
	}
}

//Process command/add a creature, and return whether or not a creature was added.
inline bool get_creature(std::list<creature>& creatures, bool& taking_intiatives, std::string& line, std::ifstream& file, bool takes_commands, bool info_already_in_line, bool may_expect_add_keyword)
{
	bool added_creature = false;
	bool using_file = file.is_open() && file.good();
	std::cout << std::endl << "________________________________________________" << std::endl;
	std::string lowercase, name, initiative_string, mod_string;
	for (auto i = creatures.begin(); i != creatures.end(); ++i)
	{
		std::cout << i->get_name();
		if (i->get_max_hp() != -1)
			std::cout << " (" << i->get_max_hp() << " hp)";
		if (i->get_flags().size() != 0)
			std::cout << "; [" << i->get_flag_list() << "]";
		
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
			std::cout << line << std::endl;
		}
	}
	else if (!info_already_in_line)
	{
		std::getline(std::cin, line);
	}
	bool used_command = false;

	if (takes_commands) //In hindsight this is an awful way to parse commands.
	{
		std::string& original_dummy_line = line;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			std::string lowercase_name = (*i).get_name();
			std::string dummy_line = line;
			make_lowercase(dummy_line);
			make_lowercase(lowercase_name);

			auto get_number_arg = [&]() -> int {
				size_t first_space = dummy_line.find(" ");
				size_t second_space = dummy_line.find(" ", first_space + 1);

				std::string sub = dummy_line.substr(second_space, dummy_line.length() - second_space);
				int value = std::stoi(sub);
				return value;
			};

			if (
				comp_substring("move " + lowercase_name + " ", dummy_line, ("move " + lowercase_name + " ").length()) ||
				comp_substring("mv " + lowercase_name + " ", dummy_line, ("mv " + lowercase_name + " ").length()) ||
				comp_substring("mod " + lowercase_name + " ", dummy_line, ("mod " + lowercase_name + " ").length()) ||
				comp_substring("md " + lowercase_name + " ", dummy_line, ("md " + lowercase_name + " ").length()))
			{
				try {
					int val = get_number_arg();
					i->set_initiative(val);
					creatures.sort();
					used_command = true;
					break;
				}
				catch (const std::exception& E) {

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


			else if (comp_substring("temp_hp " + lowercase_name + " ", dummy_line, ("temp_hp " + lowercase_name + " ").length()))
			{
				int val = get_number_arg();
				
				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
			}
			else if (comp_substring("thp " + lowercase_name + " ", dummy_line, ("thp " + lowercase_name + " ").length()))
			{
				int val = get_number_arg();

				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
			}
			else if (comp_substring("buffer " + lowercase_name + " ", dummy_line, ("buffer " + lowercase_name + " ").length()))
			{
				int val = get_number_arg();

				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
			}
			else if (comp_substring(lowercase_name + " temp_hp ", dummy_line, (lowercase_name + " temp_hp ").length()))
			{
				int val = get_number_arg();

				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
			}
			else if (comp_substring(lowercase_name + " thp ", dummy_line, (lowercase_name + " thp ").length()))
			{
				int val = get_number_arg();

				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
			}
			else if (comp_substring(lowercase_name + " buffer ", dummy_line, (lowercase_name + " buffer ").length()))
			{
				int val = get_number_arg();

				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
			}
			else if (comp_substring(lowercase_name + " temphp ", dummy_line, (lowercase_name + " temphp ").length()))
			{
				int val = get_number_arg();

				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
			}
			else if (comp_substring("temphp " + lowercase_name + " ", dummy_line, ("temphp " + lowercase_name + " ").length()))
			{
				int val = get_number_arg();

				if (val < 0)
				{
					std::cout << "Temp HP must be a non-negative number." << std::endl;
				}
				else
				{
					i->set_temp_hp(val);
				}
				}

			else if (comp_substring("hp " + lowercase_name + " ", dummy_line, ("hp " + lowercase_name + " ").length()))
			{
				int val = get_number_arg();
				try {
					size_t slash_index = dummy_line.find("/");
					if (slash_index != std::string::npos)
					{
						try {
							int max_hp = std::stoi(dummy_line.substr(slash_index + 1));
							i->set_max_hp(max_hp);
						}
						catch (const std::exception& e)
						{
							std::cout << "Could not parse new Max HP - only changing current HP\n";
						}

					}
					else
					{
						i->set_max_hp(val);
					}
					i->set_hp(val);
					used_command = true;
					break;
				}
				catch (const std::exception& E) {

				}
			}
			else if (comp_substring(lowercase_name + " hp ", dummy_line, (lowercase_name + " hp ").length()))
			{
				int val = get_number_arg();
				try {
					size_t slash_index = dummy_line.find("/");
					if (slash_index != std::string::npos)
					{
						try {
							int max_hp = std::stoi(dummy_line.substr(slash_index + 1));
							i->set_max_hp(max_hp);
						}
						catch (const std::exception& e)
						{
							std::cout << "Could not parse new Max HP - only changing current HP\n";
						}

					}
					else
					{
						i->set_max_hp(val);
					}
					i->set_hp(val);
					used_command = true;
					break;
				}
				catch (const std::exception& E) {

				}
			}
			else if (
				dummy_line == "remove " + lowercase_name ||
				dummy_line == "rm " + lowercase_name)
			{
				bool removed_all = false;
				if (get_lowercase(initial_turn) == lowercase_name)
					initial_turn = "";
				for (auto rmi = creatures.begin(); rmi != creatures.end(); ++rmi)
				{
					creature& c = *rmi;
					if (get_lowercase(c.get_name()) == lowercase_name)
					{
						bool erased_first = false;
						if (i == creatures.begin())
							erased_first = true;
						if (!erased_first)
							--i;
						creatures.erase(rmi);
						if (erased_first)
							i = creatures.begin();
						if (creatures.size() == 0)
							removed_all = true;
						break;
					}
				}
				used_command = true;
				if (removed_all)
				{
					break;
				}
			}
		}
	}
	
	if (may_expect_add_keyword && line.size() > 4 && get_lowercase(line.substr(0, 4)) == "add ")
	{
		line = line.substr(4);
	}
	
	if (!used_command && line != "")
	{
		int max_hp = -1;
		int hp = -1;
		int temp_hp = 0;
		trim(line);
		lowercase = get_lowercase(line);
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
			trim(lowercase);
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
			trim(lowercase);
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
				//std::cout << "////" << sub << "/////\n";
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
			trim(lowercase);
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
			trim(lowercase);
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
			trim(lowercase);
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
			trim(lowercase);
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
			trim(lowercase);
			index_t slash_index = hp_text.find('/');
			index_t colon_index = hp_text.find(":");
			
			if (slash_index != std::string::npos)
			{
				try
				{
					std::string max_hp_string = hp_text.substr(slash_index + 1, hp_text.length() - slash_index - 1);
					std::string current_hp_string = hp_text.substr(colon_index + 1, slash_index - colon_index - 1);
					max_hp = std::stoi(max_hp_string);
					hp = std::stoi(current_hp_string);
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
					max_hp = std::stoi(hp_text.substr(colon_index + 1, hp_text.length() - colon_index - 1));
					hp = max_hp;
				}
				catch (const std::exception& E)
				{
					std::cout << "Could not parse HP - not adding creature" << std::endl;
					return false;
				}
			}
		}
		trim(lowercase);
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
		else if (takes_commands && (lowercase == "back" || lowercase == "undo" || lowercase == "reverse" || lowercase == "cancel"))
		{
			creatures.pop_back();
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
				return false;
			}
			else {
				while (new_file.good() && !new_file.eof())
				{
					get_creature(creatures, taking_intiatives, line, new_file, true, false, true);
				}
				new_file.close();
				return false;
			}
		}
		else if (takes_commands && (comp_substring("save ", lowercase, 5)))
		{
			std::string filename = lowercase.substr(5);
			save_state(filename, creatures, initial_turn, initial_round);
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
				std::cout << original << " / " << new_name << std::endl;
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
				if (get_lowercase(i->get_name()) == args_lower)
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
			size_t number_of_spaces = std::count(lowercase.begin(), lowercase.end(), ' ');
			if (number_of_spaces == 1)
			{
				index_t space_index = lowercase.find(" ");
				name = line.substr(0, space_index);
				if (!name_is_unique(name, creatures))
				{
					std::cout << "Names must be unique!" << std::endl;
					return false;
				}
				initiative_string = lowercase.substr(space_index + 1, lowercase.length() - (name.length() + 1));
				try
				{
					if (initiative_string[0] == '+' || initiative_string[0] == '-')
					{
						int modifier = std::stoi(initiative_string);
						int initiative = (rand() % 20) + 1 + modifier;
						creatures.emplace_back(name, initiative, modifier, max_hp, hp, temp_hp, flags);
						added_creature = true;
					}
					else
					{
						int initiative = std::stoi(initiative_string);
						creatures.emplace_back(name, initiative, 0, max_hp, hp, temp_hp, flags);
						added_creature = true;
					}
				}
				catch (const std::exception& E)
				{
					std::cout << "Error: Could not parse input (or ran out of memory - unlikely)" << std::endl;
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
						std::cout << "Names must be unique!" << std::endl;
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
						creatures.emplace_back(name, initiative, modifier, max_hp, hp, temp_hp, flags);
						added_creature = true;

					}
					catch (const std::exception& E)
					{
						std::cout << "Error: Could not parse input (or ran out of memory - unlikely)" << std::endl;
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
				creatures.emplace_back(name, 1 + (rand() % 20), 0, max_hp, hp, temp_hp, flags);
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

inline void track_initiatives(std::list<creature>& creatures, std::string& dummy_line)
{
	//std::sort(creatures.begin(), creatures.end());
	creatures.sort();
	index_t current_turn = 0;
	size_t current_round = initial_round;
	bool new_round = false;
	creature* knocked_out_creature = nullptr;
	std::string turn_msg = "";
	if (initial_turn != "")
	{
		int init_turn_setter = 0;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			std::string lowerc = get_lowercase(i->get_name());
			if (lowerc == initial_turn)
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
	while (true) //Terminated only by an explicit command to do so, which returns the funtion.
	{
		clear();
		if (knocked_out_creature != nullptr)
		{
			std::cout << knocked_out_creature->get_name() << " WAS KNOCKED OUT!" << std::endl << std::endl;
			knocked_out_creature = nullptr;
		}
		if (turn_msg != "")
		{
			std::cout << turn_msg << std::endl;
		}
		turn_msg = "";
		if (new_round)
		{
			std::cout << "Start of a new round." << std::endl;
			new_round = false;
		}
		std::cout << "Round " << current_round << std::endl << std::endl << std::endl;
		int turn_count = 0; //Used to track the turn counts of each creature
		creature* current_creature = nullptr;
		for (auto i = creatures.begin(); i != creatures.end(); ++i)
		{
			if (current_turn == turn_count)
			{
				std::cout << "  ---> ";
				current_creature = &(*i);
			}
			std::cout << i->get_name() << " [" << i->get_initiative() << "]";
			if (i->get_max_hp() != -1) {
				if(i->get_temp_hp() == 0)
					std::cout << "; " << i->get_hp() << " / " << i->get_max_hp() << " HP";
				else
					std::cout << "; " << i->get_hp() << "[+" << i->get_temp_hp() << " temp]" << " / " << i->get_max_hp() << " HP";
			}
			if (i->get_flags().size() != 0)
			{
				std::cout << " | FLAGS: " << i->get_flag_list();
			}
			std::cout << std::endl;
			i->set_turn_count(turn_count);
			++turn_count;
		}
		std::cout << std::endl;
		std::cout << "It's " << current_creature->get_name() << "\'s turn." << std::endl;
		if (current_creature->get_hp() == 0)
		{
			std::cout << "\t" << current_creature->get_name() << " HAS 0 HP!" << std::endl;
		}
		std::getline(std::cin, dummy_line);
		trim(dummy_line);
		std::string original_dummy_line = dummy_line;
		std::string& line = original_dummy_line;
		make_lowercase(dummy_line);
		if (dummy_line == "quit" || dummy_line == "end" || dummy_line == "stop" || dummy_line == "terminate" || dummy_line == "finish")
			return;

		bool used_command = false;
		bool did_erase = false;
		int move_turn = -1;
		std::string lowercase_current_creature_name = get_lowercase(current_creature->get_name());
		for (auto i = creatures.begin(); i != creatures.end(); ++i) {
			const std::string& lowercase_name = get_lowercase(i->get_name());
			auto next = i;
			auto peek = i;
			++peek;

			auto get_number_arg = [&]() -> int {
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

				if (sub == " max" || sub == " all")
				{
					value = INT_MAX;
				}
				else
				{
					value = std::stoi(sub);
				}

				return value;
			};

			auto kill_creature = [&]() {
				if (lowercase_current_creature_name == lowercase_name) {
					++next;
				}

				creatures.erase(i);
				i = next;
				used_command = true;
				did_erase = true;
			};

			if (comp_substring("hurt " + lowercase_name + " ", dummy_line, ("hurt " + lowercase_name + " ").length()) ||
				comp_substring("dmg " + lowercase_name + " ", dummy_line, ("dmg " + lowercase_name + " ").length()) ||
				comp_substring("harm " + lowercase_name + " ", dummy_line, ("harm " + lowercase_name + " ").length()) ||
				comp_substring("damage " + lowercase_name + " ", dummy_line, ("damage " + lowercase_name + " ").length()) ||
				comp_substring("d " + lowercase_name + " ", dummy_line, ("d " + lowercase_name + " ").length()) ||
				comp_substring("h " + lowercase_name + " ", dummy_line, ("h " + lowercase_name + " ").length()))
			{
				try {
					int val = get_number_arg();
					i->adjust_hp(-val);
					if (i->get_hp() == 0) 
					{
						knocked_out_creature = &(*i);
					}
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
					int val = get_number_arg();
					i->adjust_hp(-val);
					if (i == (--creatures.end()))
						used_command = true;

				}
				catch (const std::exception& E) {
					//std::cout << E.what() << std::endl;
				}
			}
			else if (comp_substring("save ", dummy_line, 5))
			{
				std::string filename = dummy_line.substr(5);
				save_state(filename, creatures, current_creature->get_name(), current_round);
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
					int val = get_number_arg();
					val >>= 1;
					i->adjust_hp(-val);
					if (i->get_hp() == 0)
					{
						knocked_out_creature = &(*i);
					}
					used_command = true;
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
					int val = get_number_arg();
					val <<= 1;
					i->adjust_hp(-val);
					if (i->get_hp() == 0)
					{
						knocked_out_creature = &(*i);
					}
					used_command = true;
				}
				catch (const std::exception& E) {

				}
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
				}
				else {
					bool taking_initiatives = false;
					while (new_file.good() && !new_file.eof())
					{
						get_creature(creatures, taking_initiatives, line, new_file, true, false, true);
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
			else if (comp_substring("heal " + lowercase_name + " ", dummy_line, ("heal " + lowercase_name + " ").length()) ||
					 comp_substring(lowercase_name + " heal ", dummy_line, (lowercase_name + " heal ").length()))
			{
				try {
					int val = get_number_arg();
					i->adjust_hp(val);
					used_command = true;
				}
				catch (const std::exception& E) {
					//std::cout << E.what() << std::endl;
				}
			}
			else if (comp_substring("heal all ", dummy_line, 9))
			{
				try {
					int val = get_number_arg();
					i->adjust_hp(val);
					if(i == (--creatures.end()))
						used_command = true;
					
				}
				catch (const std::exception& E) {
					//std::cout << E.what() << std::endl;
				}
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
					std::string arg = original_dummy_line.substr(start_length);

					i->add_flag(arg);
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


			else if (comp_substring("temp_hp " + lowercase_name + " ", dummy_line, ("temp_hp " + lowercase_name + " ").length()) ||
				comp_substring(lowercase_name + " temp_hp ", dummy_line, (lowercase_name + " temp_hp ").length()) ||

				comp_substring("temphp " + lowercase_name + " ", dummy_line, ("temphp " + lowercase_name + " ").length()) ||
				comp_substring(lowercase_name + " temphp ", dummy_line, (lowercase_name + " temphp ").length()) ||

				comp_substring(lowercase_name + " buffer ", dummy_line, (lowercase_name + " buffer ").length()) ||
				comp_substring("buffer " + lowercase_name + " ", dummy_line, ("buffer " + lowercase_name + " ").length()) ||

				comp_substring("thp " + lowercase_name + " ", dummy_line, ("thp " + lowercase_name + " ").length()) ||
				comp_substring(lowercase_name + " thp ", dummy_line, (lowercase_name + " thp ").length()))
				{
					try {
						int val = get_number_arg();
						if (val < 0)
							throw;

						i->set_temp_hp(val);
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
					int val = get_number_arg();
					if (i->get_max_hp() == -1)
					{
						i->set_max_hp(val);
					}
					i->set_hp(val);
					if (i->get_hp() == 0)
					{
						knocked_out_creature = &(*i);
					}
					used_command = true;
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
					int val = get_number_arg();
					i->set_max_hp(val);
					if (i->get_hp() == -1)
						i->set_hp(val);
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
				i->set_hp(i->get_max_hp());
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
				dummy_line == "rm " + lowercase_name)
			{
				kill_creature();
				if (creatures.size() == 0)
					return;
			}
			else if (dummy_line == "ko " + lowercase_name)
			{
				i->set_hp(0);
				used_command = true;
			}
			else if (
				(dummy_line == "reset " + lowercase_name)
				)
			{
				i->set_hp(i->get_max_hp());
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
						int val = get_number_arg();
						i->set_initiative(val);
						creatures.sort();
						used_command = true;
					}
					catch (const std::exception& E) {
						//std::cout << E.what() << std::endl;
					}
			}

			if (did_erase || used_command)
				break;
		}

		if (!used_command && dummy_line.substr(0,4)=="add " && dummy_line.length()>4)
		{
			//dummy_line = dummy_line.substr(4);
			//original_dummy_line = original_dummy_line.substr(4);
			std::ifstream file;
			bool dummy_taking_initiatives = true;
			used_command = true;
			bool success = get_creature(creatures, dummy_taking_initiatives, original_dummy_line, file, false, true, true);
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
			bool success = get_creature(creatures, dummy_taking_initiatives, dummy_line, file, false, false, false);
			if (success)
			{
				creatures.sort();
			}
			else
			{
				std::cout << "Error\n";
			}
		}

		if(!used_command)
			++current_turn;

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
	}
}

int main(int argc, char** args)
{
	if (argc > 2)
		std::cout << "Invalid console arguments. Continuing as normal." << std::endl;
	
	srand(time(NULL)); //Ready the RNG
	std::list<creature> creatures; //Create a vector to hold the creature data in

	//Display help instructions
	std::cout << "This program sorts creatures by initiative and can track turns and rounds\n" << std::endl;
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
	std::cout << "Flags can be specified when adding creatures with the 'flags:' modifier. Flags are comma-separated and do not permit spaces." << std::endl;
	std::cout << "Flags can also be added or removed with 'flag', 'rmflag', and other variations." << std::endl;

	const static bool PROMPT_FILE_LOAD = false;
	
	std::string line; //A place to store input from the keyboard
	bool taking_intiatives = true; //Boolean to track whether or not user is done entering initiatives yet or not
	std::ifstream file;
	if (argc == 2) //First arg is the directory the program is executed at.
	{
		std::string filename = args[1];
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
				file.open(line);
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
		get_creature(creatures, taking_intiatives, line, file, true, false, true);
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