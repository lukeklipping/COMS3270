#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "dungeon.h"
#include "pc.h"
#include "npc.h"
#include "move.h"
#include "utils.h"
#include "io.h"
#include "dice.h"

const char *victory =
    "\n                                       o\n"
    "                                      $\"\"$o\n"
    "                                     $\"  $$\n"
    "                                      $$$$\n"
    "                                      o \"$o\n"
    "                                     o\"  \"$\n"
    "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
    "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
    "\"oo   o o o o\n"
    "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
    "   \"o$$\"    o$$\n"
    "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
    "     o\"\"\n"
    "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
    "   o\"\n"
    "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
    "\"$$$  $\n"
    "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
    "\"\"      \"\"\" \"\n"
    "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
    "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
    "\"$$$$\n"
    "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
    "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
    "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
    "              $\"                                                 \"$\n"
    "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
    "$\"$\"$\"$\"$\"$\"$\"$\n"
    "                                   You win!\n\n";

const char *tombstone =
    "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
    "               /     /         \\             __\n"
    "              /     /           \\            ||\n"
    "             /____ /   Rest in   \\           ||\n"
    "            |     |    Pieces     |          ||\n"
    "            |     |               |          ||\n"
    "            |     |   A. Luser    |          ||\n"
    "            |     |               |          ||\n"
    "            |     |     * *   * * |         _||_\n"
    "            |     |     *\\/* *\\/* |        | TT |\n"
    "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
    "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
    "            |     |         \\/..\"\"\"\"\"...\"\"\""
    "\\ || /.\"\"\".......\"\"\"\"...\n"
    "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
    "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
    "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
    "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
    "            You're dead.  Better luck in the next life.\n\n\n";

void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n"
          "          [-n|--nummon <count>]\n",
          name);

  exit(-1);
}

class monsterInfo
{
private:
  std::string name;
  std::string symbol;
  std::string color;
  std::string desc;
  std::string speed;
  std::string damage;
  std::string HP;
  std::vector<int> abilities;

public:
  // default constructor
  monsterInfo() : name(), symbol(), color(), desc(), speed(), damage(), HP(), abilities() {}

  // parameterized constructor
  monsterInfo(std::string name, std::string symbol, std::string color, std::string desc, std::string speed, std::string damage, std::string HP, std::vector<int> abilities)
      : name(name), symbol(symbol), color(color), desc(desc), speed(speed), damage(damage), HP(HP), abilities(abilities) {}

  // Setters
  void set_monster(std::string name, std::string symbol, std::string color, std::string desc, std::string speed, std::string damage, std::string HP, std::vector<int> abilities)
  {
    this->name = name;
    this->symbol = symbol;
    this->color = color;
    this->desc = desc;
    this->speed = speed;
    this->damage = damage;
    this->HP = HP;
    this->abilities = abilities;
  }

  // Getters
  /*int get_color() { return color; }
  std::string get_name() { return name; }
  char get_symbol() { return symbol; }
  std::string get_desc() { return desc; }
  int get_speed() { return speed; }
  int get_damage() { return damage; }
  int get_HP() { return HP; }*/
  std::vector<int> get_abilities() { return abilities; }

  friend std::ostream &operator<<(std::ostream &os, const monsterInfo &m)
  {
    os << "Abilities: ";
    for (size_t i = 0; i < m.abilities.size(); ++i)
    {
      os << m.abilities[i];
      if (i != m.abilities.size() - 1)
        os << ", ";
    }
    return os;
  }
};

/* trims beginning and trailing whitespace */
std::string trim(const std::string &line)
{
  std::string::const_iterator iter = line.begin();
  while (iter != line.end() && isspace(*iter))
  { // skip leading whitespace
    ++iter;
  }
  std::string::const_reverse_iterator riter = line.rbegin();
  while (riter.base() != iter && isspace(*riter))
  { // skip trailing whitespace
    ++riter;
  }
  return std::string(iter, riter.base());
}

bool parse_name(std::string &n, const std::string &line)
{
  if (line.rfind("NAME", 0) == 0)
  {
    n = trim(line.substr(4));
    return true;
  }
  return false;
}

bool parse_symbol(std::string &s, const std::string &line)
{
  if (line.rfind("SYMB", 0) == 0)
  {
    s = trim(line.substr(4))[0];
    return true;
  }
  return false;
}

bool parse_color(std::string &c, const std::string &line)
{
  if (line.rfind("COLOR", 0) == 0)
  {
    c = trim(line.substr(5));
    return true;
  }
  return false;
}

bool parse_desc(std::string &d, std::ifstream &rf, const std::string &line)
{
  if (line.rfind("DESC", 0) == 0)
  {
    std::string desc;
    // d = "";
    while (std::getline(rf, desc))
    {
      // desc = trim(desc);
      if (desc == ".")
      {
        break; // end of description
      }
      if (!d.empty())
      {
        d += " ";
      }
      // if (d.length() > 77)
      d += desc + "\n";
    }
    return true;
  }
  return false;
}

bool parse_speed(std::string &s, const std::string &line, dice &d)
{
  int base, number, sides;
  if (line.rfind("SPEED", 0) == 0)
  {
    std::string speed_string = trim(line.substr(5));
    size_t plus_position = speed_string.find("+");
    size_t d_position = speed_string.find("d");

    if ((plus_position == std::string::npos && d_position == std::string::npos) || plus_position > d_position)
    {
      std::cout << "Error: Invalid speed format" << std::endl;
      return false;
    }

    // try
    try
    {
      base = std::stoi(speed_string.substr(0, plus_position));
      number = std::stoi(speed_string.substr(plus_position + 1, d_position - plus_position - 1));
      sides = std::stoi(speed_string.substr(d_position + 1));

      d = dice(base, number, sides);
    }
    catch (const std::invalid_argument &e)
    {
      std::cout << "Error: Invalid speed format" << std::endl;
      return false;
    }
    s = speed_string;
    return true;
  }
  return false;
}

bool parse_damage(std::string &d, const std::string &line, dice &die)
{
  int base, number, sides;
  if (line.rfind("DAM", 0) == 0)
  {
    std::string dam_string = trim(line.substr(3));
    size_t plus_position = dam_string.find("+");
    size_t d_position = dam_string.find("d");

    if ((plus_position == std::string::npos && d_position == std::string::npos) || plus_position > d_position)
    {
      std::cout << "Error: Invalid speed format" << std::endl;
      return false;
    }

    // try
    try
    {
      base = std::stoi(dam_string.substr(0, plus_position));
      number = std::stoi(dam_string.substr(plus_position + 1, d_position - plus_position - 1));
      sides = std::stoi(dam_string.substr(d_position + 1));

      die.set_dice(base, number, sides);
    }
    catch (const std::invalid_argument &e)
    {
      std::cout << "Error: Invalid speed format" << std::endl;
      return false;
    }
    d = dam_string;
    return true;
  }
  return false;
}

bool parse_abilities(std::vector<int> &a, const std::string &line)
{
  /*
    0 = smart
    1 = tele
    2 = tunnel
    3 = erratic
    4 = pass
    5 = uniq
  */
  static const std::unordered_map<std::string, int> ability_map = {
      {"SMART", 0},
      {"TELE", 1}, // like a java hashmap
      {"TUNNEL", 2},
      {"ERRATIC", 3},
      {"PASS", 4},
      {"UNIQ", 5},
      {"PICKUP", 6},
      {"DESTROY", 7},
      {"BOSS", 8}};

  if (line.rfind("ABIL", 0) == 0)
  {
    std::string ability_string = trim(line.substr(5));
    std::istringstream iss(ability_string);
    std::string token;
    while (iss >> token)
    {
      token = trim(token);
      auto it = ability_map.find(token);
      if (it != ability_map.end())
      {
        a.push_back(it->second);
      }
      else
      {
        std::cout << "Error: Invalid ability format" << std::endl;
        return false;
      }
    }
    return true;
  }
  return false;
}

bool parse_RRTY(std::string &r, const std::string &line, dice &d)
{
  if (line.rfind("RRTY", 0) == 0)
  {
    r = trim(line.substr(4))[0];
    return true;
  }
  return false;
}

bool parse_HP(std::string &h, const std::string &line, dice &d)
{
  int base, number, sides;
  if (line.rfind("HP", 0) == 0)
  {
    std::string HP_string = trim(line.substr(2));
    size_t plus_position = HP_string.find("+");
    size_t d_position = HP_string.find("d");

    if ((plus_position == std::string::npos && d_position == std::string::npos) || plus_position > d_position)
    {
      std::cout << "Error: Invalid HP format" << std::endl;
      return false;
    }

    // try
    try
    {
      base = std::stoi(HP_string.substr(0, plus_position));
      number = std::stoi(HP_string.substr(plus_position + 1, d_position - plus_position - 1));
      sides = std::stoi(HP_string.substr(d_position + 1));

      d.set_dice(base, number, sides);
    }
    catch (const std::invalid_argument &e)
    {
      std::cout << "Error: Invalid speed format" << std::endl;
      return false;
    }
    h = HP_string;
    return true;
  }
  return false;
}

// only print out values of parsed monster
int parse(std::ifstream &readF)
{
  // bool metadata, monster1 = false;
  std::string line;
  // begin first line
  if (!std::getline(readF, line) || trim(line) != "RLG327 MONSTER DESCRIPTION 1")
  {
    std::cout << "Error: Invalid file format at 407" << std::endl;
    return -1;
  }

  dice dye;

  monsterInfo monster_info;              // object of monsterInfo
  std::vector<monsterInfo> monster_list; // list of monsters

  std::string color, name, symbol, desc, speed, damage, HP, rrty;
  std::vector<int> abilities;
  bool in_monster = false;

  // while not end of file, read
  while (std::getline(readF, line))
  {
    line = trim(line);
    if (line.empty())
    {
      continue;
    }

    if (line == "BEGIN MONSTER")
    {
      // std::cout << "Begin monster" << std::endl;
      in_monster = true;
      color = name = symbol = desc = speed = damage = HP = rrty = "";
      abilities.clear();
      continue;
    }
    if (!in_monster)
    {
      std::cout << "Error: Invalid begin format" << std::endl;
      return -1;
    }

    // END
    // print out all information parsed
    if (line == "END")
    {
      std::cout << "Name: " << name << std::endl;
      std::cout << "desc: " << desc << std::endl;
      std::cout << "color: " << color << std::endl;
      std::cout << "speed: " << speed << std::endl;
      std::cout << "Abilities: ";
      for (size_t i = 0; i < abilities.size(); ++i)
      {
        std::cout << abilities[i];
        if (i != abilities.size() - 1)
          std::cout << ", ";
      }
      std::cout << std::endl;
      std::cout << "HP: " << HP << std::endl;
      std::cout << "damage: " << damage << std::endl;
      std::cout << "symbol: " << symbol << std::endl;
      std::cout << "rrty: " << rrty << std::endl
                << std::endl;

      monster_info.set_monster(name, symbol, color, desc, speed, damage, HP, abilities);
      monster_list.push_back(monster_info);
      in_monster = false;
      continue;
    }

    std::string attr;
    std::istringstream iss(line);
    iss >> attr;
    attr = trim(attr);
    std::getline(iss, line);
    line = trim(line);

    // name
    if (attr == "NAME" && parse_name(name, attr + " " + line))
    {
      // std::cout << "Name: " << name << std::endl;
      //  skips to next iteration of loop
      continue;
    }

    // symbol
    if (attr == "SYMB" && parse_symbol(symbol, attr + " " + line))
    {
      // std::cout << "Symbol: " << symbol << std::endl;
      continue;
    }

    // color
    if (attr == "COLOR" && parse_color(color, attr + " " + line))
    {
      // std::cout << "Color: " << color << std::endl;
      continue;
    }

    // description
    if (attr == "DESC" && parse_desc(desc, readF, attr + " " + line))
    {
      /*std::cout << "Desc: " << std::endl
                << desc << std::endl;*/
      continue;
    }

    // speed
    if (attr == "SPEED" && parse_speed(speed, attr + " " + line, dye))
    {
      // std::cout << "Speed: " << speed << std::endl;
      continue;
    }

    // damage
    if (attr == "DAM" && parse_damage(damage, attr + " " + line, dye))
    {
      // std::cout << "Damage: " << damage << std::endl;
      continue;
    }
    // HP
    if (attr == "HP" && parse_HP(HP, attr + " " + line, dye))
    {
      // std::cout << "HP: " << HP << std::endl;
      continue;
    }

    // RRTY
    if (attr == "RRTY" && parse_RRTY(rrty, attr + " " + line, dye))
    {
      // std::cout << "RRTY: " << rrty << std::endl;
      continue;
    }

    // ABIL
    if (attr == "ABIL" && parse_abilities(abilities, attr + " " + line))
    {
      /*std::cout << "Abilities: ";
      for (size_t i = 0; i < abilities.size(); ++i)
      {
        std::cout << abilities[i];
        if (i != abilities.size() - 1)
          std::cout << ", ";
      }
      std::cout << std::endl;*/
      continue;
    }

    std::cout << "Error: Unknown attribute: " << attr << std::endl;
    return -1;
  }
  return 0;
}

// parse the monster file
int main(int argc, char *argv[])
{
  // creat _desc.txt
  std::string file;
  std::ifstream readF;
  file = getenv("HOME");
  file += std::string("/") + ".rlg327" + "/" + "monster_desc.txt";

  // open filehow
  readF.open(file.c_str());

  int read = parse(readF);
  if (read == -1)
  {
    std::cout << "Error: Invalid file format at line 593" << std::endl;
  }
  std::cout << "End of file, parsing correct" << std::endl;
  readF.close();
  return 0;
}
