#include "SniperUnit.h"

ASniperUnit::ASniperUnit()
{
    MaxHealth = 20;
    MovementRange = 4;
    AttackType = EAttackType::Ranged;
    AttackRange = 10;
    MinDamage = 4;
    MaxDamage = 8;
}