#pragma once
#include "Types.hpp"
#include <string>
#include <vector>
namespace rbg {
struct Player {
 PlayerId id{}; std::string name; PlayerClass playerClass{PlayerClass::Assault};
 int weaponId{1}; Vec3 position{}, velocity{};
 float health{100}, maxHealth{100}, shield{100}, maxShield{100}, armor{20};
 float moveSpeed{8.0f}, abilityCooldown{0}, abilityCharge{0}, fireCooldown{0};
 float damageDealt{0}, damageTaken{0};
 bool alive{true}; int eliminations{0};
 std::vector<Vec3> trail;
};
}
