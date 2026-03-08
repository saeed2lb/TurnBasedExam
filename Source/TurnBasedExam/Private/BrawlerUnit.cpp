#include "BrawlerUnit.h"

ABrawlerUnit::ABrawlerUnit()
{
    MaxHealth = 40;
    MovementRange = 6;
    AttackType = EAttackType::Melee;
    AttackRange = 1;
    MinDamage = 1;
    MaxDamage = 6;
}