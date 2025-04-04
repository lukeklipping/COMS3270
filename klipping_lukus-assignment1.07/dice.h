#include <random>

class dice
{
private:
    int base;
    int number;
    int sides;

public:
    dice() : base(0), number(0), sides(0) {}
    dice(int base, int number, int sides) : base(base), number(number), sides(sides) {}

    void set_dice(int base, int number, int sides)
    {
        this->base = base;
        this->number = number;
        this->sides = sides;
    }
    int get_base() { return base; }
    int get_number() { return number; }
    int get_sides() { return sides; }
};