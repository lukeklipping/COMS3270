#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

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
  char symbol;
  int color;
  std::string desc;
  int speed;
  int damage;
  int HP;
  std::vector<int> abilities;

public:
  // default constructor
  monsterInfo() : name(), symbol(), color(), desc(), speed(), damage(), HP(), abilities() {}

  // parameterized constructor
  monsterInfo(const std::string &name, char symbol, int color, const std::string &desc, int speed, int damage, int HP, const std::vector<int> &abilities)
      : name(name), symbol(symbol), color(color), desc(desc), speed(speed), damage(damage), HP(HP), abilities(abilities)
  {
  }

  // Setters
  void set_monster(std::string name, char symbol, int color, std::string desc, int speed, int damage, int HP, std::vector<int> abilities)
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
  int get_color() { return color; }
  std::string get_name() { return name; }
  char get_symbol() { return symbol; }
  std::string get_desc() { return desc; }
  int get_speed() { return speed; }
  int get_damage() { return damage; }
  int get_HP() { return HP; }
  std::vector<int> get_abilities() { return abilities; }
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
    d = "";
    while (std::getline(rf, desc))
    {
      desc = trim(desc);
      if (desc == ".")
      {
        break; // end of description
      }
      if (!d.empty())
      {
        d += " ";
      }
      d += desc;
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

    if (plus_position == std::string::npos && d_position == std::string::npos || plus_position > d_position)
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
    std::string dam_string = trim(line.substr(5));
    size_t plus_position = dam_string.find("+");
    size_t d_position = dam_string.find("d");

    if (plus_position == std::string::npos && d_position == std::string::npos || plus_position > d_position)
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

  return false;
}

bool parse_ability()
{
  /*
    0 = smart
    1 = tele
    2 = tunnel
    3 = erratic
    4 = pass
  */
}

// only print out values of parsed monster
int parse(std::ifstream &readF)
{
  bool metadata, monster1 = false;
  std::string line = "";
  getline(readF, line);
  line = trim(line);
  dice *dye;

  std::string color, name, symbol, desc, speed, damage, HP, abilities, rrty = "";

  // begin first line
  if (trim(line) != "RLG327 MONSTER DESCRIPTION 1")
  {
    std::cout << "Error: Invalid file format" << std::endl;
    return -1;
  }
  else
  {
    metadata = true;
  }
  // while not end of file, read
  while (!readF.eof())
  {
    std::string attr;
    readF >> attr; // read word one by one
    attr = trim(attr);

    std::getline(readF, line);
    line = trim(line);
    if (attr + line == "BEGIN MONSTER")
    {
      if (monster1)
      {
        std::cout << std::endl;
      }
      else
      {
        monster1 = true;
      }
      continue;
    }

    // name
    if (attr == "NAME" && parse_name(name, line))
    {
      std::cout << "Name: " << name << std::endl;
      // skips to next iteration of loop
      continue;
    }
    else
    {
      std::cout << "Error: Invalid name format" << std::endl;
      return -1;
    }

    // symbol
    if (attr == "SYMB" && parse_symbol(symbol, line))
    {
      std::cout << "Symbol: " << symbol << std::endl;
      continue;
    }
    else
    {
      std::cout << "Error: Invalid symbol format" << std::endl;
      return -1;
    }

    // color
    if (attr == "COLOR" && parse_color(color, line))
    {
      std::cout << "Color: " << color << std::endl;
      continue;
    }
    else
    {
      std::cout << "Error: Invalid color format" << std::endl;
      return -1;
    }
    // description
    if (attr == "DESC" && parse_desc(desc, readF, line))
    {
      std::cout << "Desc: " << std::endl
                << desc << std::endl;
      continue;
    }
    else
    {
      std::cout << "Error: Invalid description format" << std::endl;
      return -1;
    }
    // speed
    if (attr == "SPEED" && parse_speed(speed, line, *dye))
    {
      std::cout << "Speed: " << speed << std::endl;
      continue;
    }
    else
    {
      std::cout << "Error: Invalid speed format" << std::endl;
      return -1;
    }

    // damage
    if (attr == "DAM" && parse_damage(damage, line, *dye))
    {
      std::cout << "Damage: " << damage << std::endl;
      continue;
    }
    else
    {
      std::cout << "Error: Invalid damage format" << std::endl;
      return -1;
    }

    // HP
    if (attr == "HP" && parse_HP())
    {
      std::cout << "HP: " << HP << std::endl;
      continue;
    }
    else
    {
      std::cout << "Error: Invalid HP format" << std::endl;
      return -1;
    }
    // RRTY
    if (attr == "RRTY" && parse_RRTY())
    {
      std::cout << "RRTY: " << rrty << std::endl;
      continue;
    }
    else
    {
      std::cout << "Error: Invalid RRTY format" << std::endl;
      return -1;
    }

    // ABIL
    if (attr == "ABIL" && parse_ability() {
      std::cout << "Abilities: " << abilities << std::endl;
      continue; } else {
      std::cout << "Error: Invalid ability format" << std::endl;
      return -1; })
      // END
      // print out all information parsed
      if (attr == "END")
      {
        std::cout << name << std::endl;
        std::cout << desc << std::endl;
        std::cout << color << std::endl;
        std::cout << speed << std::endl;
        std::cout << abilities << std::endl;
        std::cout << HP << std::endl;
        std::cout << damage << std::endl;
        std::cout << symbol << std::endl;
        std::cout << rrty << std::endl;
      }
  }
}

// parse the monster file
int main(int argc, char *argv[])
{
  // creat _desc.txt
  std::string file;
  std::ifstream readF;
  file = getenv("HOME");
  file += "/.rlg327/monster_desc.txt";

  // open filehow
  readF.open(file.c_str());

  parse(readF);

  readF.close();
  return 0;
}
