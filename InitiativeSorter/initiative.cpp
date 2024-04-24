#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <random>
#include <fstream>

typedef size_t index_t;
class creature
{
	int initiative, modifier, hp, max_hp;
	std::string name;
public:
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

	inline int get_max_hp() const {
		return max_hp;
	}

	inline int get_hp() const{
		return hp;
	}
	creature(const std::string& name, int initiative, int modifier, int max_hp, int hp) : name(name), initiative(initiative), modifier(modifier),
		hp(hp), max_hp(max_hp)
	{
		if (hp > max_hp)
			hp = max_hp;
	}
	creature(const std::string& name, int initiative, int modifier) : name(name), initiative(initiative), modifier(modifier),
		hp(-1), max_hp(-1)
	{}
	creature(const std::string& name, int initiative) : name(name), initiative(initiative), modifier(0), hp(-1), max_hp(-1)
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
			hp += amount;
			if (hp > max_hp)
				hp = max_hp;
			else if (hp < 0)
				hp = 0;
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

	inline void set_max_hp(int new_max_hp)
	{
		if (new_max_hp < 1)
			new_max_hp = 1;
		max_hp = new_max_hp;
		if (hp > new_max_hp)
			hp = new_max_hp;
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
}

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

//Lazy but cross-platform way to "clear" the screen
inline void clear()
{
	for (int i = 0; i < 100; ++i)
		std::cout << std::endl;
}

inline bool name_is_unique(const std::string& name, const std::list<creature>& creatures)
{
	for (auto i = creatures.begin(); i != creatures.end(); ++i)
	{
		if (get_lowercase(name) == get_lowercase(i->get_name()))
			return false;
	}
	return true;
}

inline void get_creature(std::list<creature>& creatures, bool& taking_intiatives, std::string& line, std::ifstream& file)
{
	bool using_file = file.is_open() && file.good();
	std::string lowercase, name, initiative_string, mod_string;
	std::cout << "Enter creature name + initiative:" << std::endl;
	if (using_file)
	{
		if (file.eof() || !file.good())
		{
			return;
		}
		else
		{
			std::getline(file, line);
			std::cout << line << std::endl;
		}
	}
	else
	{
		std::getline(std::cin, line);
	}
	bool used_command = false;

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
		else if (comp_substring("hp " + lowercase_name + " ", dummy_line, ("hp " + lowercase_name + " ").length()))
		{
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

				int val = get_number_arg();
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
			for (auto i = creatures.begin(); i != creatures.end(); ++i)
			{
				creature& c = *i;
				if (get_lowercase(c.get_name()) == lowercase_name)
				{
					creatures.erase(i);
					break;
				}
			}
			used_command = true;
		}
	}
	
	if (!used_command && line != "")
	{
		int max_hp = -1;
		int hp = -1;
		trim(line);
		lowercase = get_lowercase(line);
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
					return;
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
					return;
				}
			}
		}
		trim(lowercase);
		if (lowercase == "done" || 
			lowercase == "end" || 
			lowercase == "stop" || 
			lowercase == "start" || 
			lowercase == "begin" || 
			lowercase == "finish" || 
			lowercase == "go")
		{
			taking_intiatives = false;
			return;
		}
		else if (lowercase == "back" || lowercase == "undo" || lowercase == "reverse" || lowercase == "cancel")
		{
			creatures.pop_back();
		}
		else if (lowercase == "reset" || lowercase == "clear")
		{
			creatures.clear();
		}
		else if (comp_substring("load ", lowercase, 5))
		{
			std::string filename = line.substr(5, line.length() - 5);
			std::ifstream new_file;
			new_file.open(filename);
			if (!new_file.is_open())
			{
				std::cout << "Error: Could not open " << filename << std::endl;
				return;
			}
			else {
				while (new_file.good() && !new_file.eof())
				{
					get_creature(creatures, taking_intiatives, line, new_file);
				}
				file.close();
				return;
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
					return;
				}
				initiative_string = lowercase.substr(space_index + 1, lowercase.length() - (name.length() + 1));
				try
				{
					if (initiative_string[0] == '+' || initiative_string[0] == '-')
					{
						int modifier = std::stoi(initiative_string);
						int initiative = (rand() % 20) + 1 + modifier;
						creatures.emplace_back(name, initiative, modifier, max_hp, hp);
					}
					else
					{
						int initiative = std::stoi(initiative_string);
						creatures.emplace_back(name, initiative, 0, max_hp, hp);
					}
				}
				catch (const std::exception& E)
				{
					std::cout << "Error: Could not parse input (or ran out of memory - unlikely)" << std::endl;
				}

			}
			else if (number_of_spaces == 2)
			{
				index_t first_space_index = lowercase.find(" ");
				index_t second_space_index = lowercase.find(" ", first_space_index + 1);
				if (second_space_index == first_space_index + 1)
				{
					std::cout << "Error: Malformed input" << std::endl;
				}
				else
				{
					name = line.substr(0, first_space_index);
					if (!name_is_unique(name, creatures))
					{
						std::cout << "Names must be unique!" << std::endl;
						return;
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
						creatures.emplace_back(name, initiative, modifier, max_hp, hp);

					}
					catch (const std::exception& E)
					{
						std::cout << "Error: Could not parse input (or ran out of memory - unlikely)" << std::endl;
					}
				}
			}
			else
			{
				std::cout << "Error: Malformed input (did you forget the space, or add extras?)" << std::endl;
			}
		}
	}
}

inline void track_initiatives(std::list<creature>& creatures, std::string& dummy_line)
{
	//std::sort(creatures.begin(), creatures.end());
	creatures.sort();
	index_t current_turn = 0;
	size_t current_round = 1;
	bool new_round = false;
	creature* knocked_out_creature = nullptr;
	while (true) //Terminated only by an explicit command to do so, which returns the funtion.
	{
		clear();
		if (knocked_out_creature != nullptr)
		{
			std::cout << knocked_out_creature->get_name() << " WAS KNOCKED OUT!" << std::endl << std::endl;
			knocked_out_creature = nullptr;
		}
		if (new_round)
		{
			std::cout << "Start of a new round." << std::endl;
			new_round = false;
		}
		std::cout << "Round " << current_round << std::endl << std::endl << std::endl;
		int turn_count = 0;
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
				std::cout << "; " << i->get_hp() << " / " << i->get_max_hp() << " HP";
			}
			std::cout << std::endl;
			++turn_count;
		}
		std::cout << std::endl;
		std::cout << "It's " << current_creature->get_name() << "\'s turn." << std::endl;
		std::getline(std::cin, dummy_line);
		trim(dummy_line);
		make_lowercase(dummy_line);
		if (dummy_line == "quit" || dummy_line == "end" || dummy_line == "stop" || dummy_line == "terminate" || dummy_line == "finish")
			return;

		bool used_command = false;
		bool did_erase = false;
		std::string lowercase_current_creature_name = get_lowercase(current_creature->get_name());
		for (auto i = creatures.begin(); i != creatures.end(); ++i) {
			const std::string& lowercase_name = get_lowercase(i->get_name());
			auto next = i;
			auto peek = i;
			++peek;

			auto get_number_arg = [&]() -> int {
				size_t first_space = dummy_line.find(" ");
				size_t second_space = dummy_line.find(" ", first_space + 1);

				std::string sub = dummy_line.substr(second_space, dummy_line.length() - second_space);
				int value = std::stoi(sub);
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
				comp_substring("damage " + lowercase_name + " ", dummy_line, ("damage " + lowercase_name + " ").length()))
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
			else if (comp_substring("heal " + lowercase_name + " ", dummy_line, ("heal " + lowercase_name + " ").length()))
			{
				try {
					int val = get_number_arg();
					i->adjust_hp(val);
					used_command = true;
				}
				catch (const std::exception& E) {

				}
			}
			else if (comp_substring("hp " + lowercase_name + " ", dummy_line, ("hp " + lowercase_name + " ").length()))
			{
				try {
					int val = get_number_arg();
					i->set_hp(val);
					if (val == 0)
					{
						knocked_out_creature = &(*i);
					}
					used_command = true;
				}
				catch (const std::exception& E) {

				}
			}
			else if (
				comp_substring("move " + lowercase_name + " ", dummy_line, ("move " + lowercase_name + " ").length()) ||
				comp_substring("mv " + lowercase_name + " ", dummy_line, ("mv " + lowercase_name + " ").length()) || 
				comp_substring("mod " + lowercase_name + " ", dummy_line, ("mod " + lowercase_name + " ").length()) ||
				comp_substring("md " + lowercase_name + " ", dummy_line, ("md " + lowercase_name + " ").length())   )
			{
				try {
					int val = get_number_arg();
					i->set_initiative(val);
					creatures.sort();
					used_command = true;
				}
				catch (const std::exception& E) {

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
			else if (dummy_line == "add")
			{
				dummy_line = "";
				std::ifstream file;
				bool dummy_taking_initiatives = true;
				get_creature(creatures, dummy_taking_initiatives, dummy_line, file);
				creatures.sort();
				used_command = true;
			}

			if (did_erase || used_command)
				break;
		}

		if(!used_command)
			++current_turn;

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
	std::cout << "When you're tracking initiatives, pressing the \'Enter'\ key will advance the initiative counter." << std::endl;
	std::cout << std::endl;
	std::cout << "You can also make it roll initiatives for you by telling it a modifier instead of a roll" << std::endl;
	std::cout << "(i.e., \"Barbarian +3\")" << std::endl;
	std::cout << std::endl;
	std::cout << "Add HP:[Hit Points] or HP:[Current HP]/[Max HP] to enable optional HP tracking." << std::endl;
	std::cout << "Creatures with HP tracking enabled can be hurt or healed with their corresponding commands (\'hurt\' & \'heal\')" << std::endl;
	std::cout << std::endl;
	std::cout << "You can also set a creature's HP with the \'hp\' command." << std::endl;
	std::cout << std::endl;
	std::cout << "You must manually kill creatures with 0 HP via the \'kill'\ command." << std::endl;
	std::cout << std::endl << std::endl;

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
		get_creature(creatures, taking_intiatives, line, file);
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
	std::cout << "Combat has ended. Press \'\enter\' to terminate program." << std::endl;
	std::getline(std::cin, line);
	return 0;
}