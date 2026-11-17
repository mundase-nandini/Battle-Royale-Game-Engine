#pragma once
#include <string>
namespace rbg {
struct Weapon {
    int id{}; std::string name; std::string className; float damage{}; float range{};
    float fireRate{}; const char* description{};
};
}
