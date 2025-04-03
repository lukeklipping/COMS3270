#include <random>

class Dice
{
public:
    Dice(int sides) : sides(sides), generator(std::random_device()()) {}

    int roll()
    {
        std::uniform_int_distribution<int> distribution(1, sides);
        return distribution(generator);
    }

private:
    int sides;
    std::mt19937 generator;
};