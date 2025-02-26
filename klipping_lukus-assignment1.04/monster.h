#ifndef MONSTER_H
#define MONSTER_H

#define MON_INTELLIGENT 0x00000001
#define MON_TELEPATHY 0x00000002
#define MON_TUNNEL 0x00000004
#define MON_ERRATIC 0x00000008
#define MONS_BIT04 0x00000010
#define MONS_BIT05 0x00000020
#define MONS_BIT06 0x00000040
#define MONS_BIT07 0x00000080
#define MONS_BIT08 0x00000100
#define MONS_BIT09 0x00000200
#define MONS_BIT10 0x00000400
#define MONS_BIT11 0x00000800
#define MONS_BIT12 0x00001000
#define MONS_BIT13 0x00002000
#define MONS_BIT14 0x00004000
#define MONS_BIT15 0x00008000
#define MONS_BIT16 0x00010000
#define MONS_BIT17 0x00020000
#define MONS_BIT18 0x00040000
#define MONS_BIT19 0x00080000
#define MONS_BIT20 0x00100000
#define MONS_BIT21 0x00200000
#define MONS_BIT22 0x00400000
#define MONS_BIT23 0x00800000
#define MONS_BIT24 0x01000000
#define MONS_BIT25 0x02000000
#define MONS_BIT26 0x04000000
#define MONS_BIT27 0x08000000
#define MONS_BIT28 0x10000000
#define MONS_BIT29 0x20000000
#define MONS_BIT30 0x40000000
#define MONS_BIT31 0x80000000

// higher num = lower speed
#define MONSTER_MAX_SPEED 5
#define MONSTER_MIN_SPEED 20

typedef struct dungeon dungeon_t;
typedef struct character character_t;

typedef struct
{
    int characteristic;
} monster_t;

void monsters_generate(dungeon_t *d, heap_t *h);
void monsters_delete(monster_t *m);
void monsters_next(dungeon_t *d, character_t *c, pair_t new_pos);
int monsters_number(dungeon_t *d);
int32_t monster_comp(const void *c1, const void *c2);

#endif