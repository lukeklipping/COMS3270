New methods
    generate_PC(), count_up_stairs(), count_down_stairs(), read_dungeon_map(), 
    read_rooms(), read_stairs(), save_stairs(), load_dungeon(), and save_dungeon(), calculate_dungeon_size()

    Most of these are helper methods for load and save, to make their bodies of code shorter.

    My code opens all of the saved_dungeons, with "./dungeon --load saved_dungeons/welldone.rlg327"
    when it is in the same directory as the program. My code also saves and loads when "./dungeon 
    --save" and "./dungeon --load" from the rlg directory under dungeon.rlg327. I can save a file under the 
    name to the rlg directory, but cannot load it directly from that directory. Although it will load it as
    "./dungeon --load test.rlg327" from its own directory.
