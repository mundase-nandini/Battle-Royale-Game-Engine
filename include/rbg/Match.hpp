#pragma once
#include "Types.hpp"
#include "Player.hpp"
#include "Weapon.hpp"
#include "Zone.hpp"
#include "EventLog.hpp"
#include <vector>
namespace rbg {
class Match {
public:
    explicit Match(std::size_t capacity=20);
    Player* addPlayer(const std::string& name, PlayerClass type=PlayerClass::Assault);
    bool removePlayer(PlayerId id);
    Player* find(PlayerId id);
    const std::vector<Player>& players() const { return players_; }
    const std::vector<Weapon>& weapons() const { return weapons_; }
    const Weapon* weapon(int id) const;
    bool assignWeapon(PlayerId player, int weaponId);
    void start(); void pause(); void resume(); void reset(); void tick(float dt);
    void setAutoCombat(bool value) { autoCombat_=value; }
    bool autoCombat() const { return autoCombat_; }
    MatchState state() const { return state_; }
    double elapsed() const { return elapsed_; }
    std::size_t capacity() const { return capacity_; }
    int aliveCount() const;
    SafeZone& zone() { return zone_; }
    const SafeZone& zone() const { return zone_; }
    EventLog& log() { return log_; }
    const EventLog& log() const { return log_; }
private:
    void combatTick();
    std::vector<Player> players_; std::vector<Weapon> weapons_; std::size_t capacity_;
    PlayerId nextId_{1}; MatchState state_{MatchState::Ready}; double elapsed_{0};
    float combatAccumulator_{0}, zoneAccumulator_{0}; bool autoCombat_{true};
    SafeZone zone_; EventLog log_; std::size_t targetCursor_{0};
};
}
