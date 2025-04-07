This part of the Roguelike project implements a C++ parser for monster description files used in a Roguelike game. Each monster is defined in a text file located 
at $HOME/.rlg327/monster_desc.txt, using a simple custom format that includes fields like name, description, symbol, color, speed, hitpoints, 
damage, abilities, and rarity. The parser reads and validates each monster entry between BEGIN MONSTER and END tags, ignoring any bad or 
incomplete entries. Parsed monsters are printed in the terminal


For TA:
    It works 