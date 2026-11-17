#include "rbg/Match.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
namespace rbg {
Match::Match(std::size_t cap):capacity_(cap) {
    weapons_={{1,"Royal Assault Rifle","ASSAULT",24,58,0.85f,"Balanced automatic rifle"},
              {2,"Longshot Sniper","SNIPER",62,100,1.8f,"High damage precision weapon"},
              {3,"Titan LMG","HEAVY",18,48,0.55f,"Sustained heavy fire"},
              {4,"Violet Repeater","ARCANE",32,66,1.1f,"Charged energy repeater"},
              {5,"Dagger Pair","MELEE",45,8,1.0f,"Close-range specialist"}};
    log_.add(0,"ENGINE","Match created. Capacity: "+std::to_string(capacity_));
}
Player* Match::addPlayer(const std::string& name, PlayerClass type){
    if(players_.size()>=capacity_||name.empty())return nullptr;
    Player p; p.id=nextId_++; p.name=name; p.position={float((p.id*17)%85+8),float((p.id*29)%85+8),float((p.id*3)%7)};
    // Each specialist arrives with a deliberate, class-owned loadout and stat profile.
    switch(type) {
      case PlayerClass::Assault: p.weaponId=1; p.maxHealth=110; p.health=110; p.maxShield=90; p.shield=90; p.armor=22; p.moveSpeed=10.5f; break;
      case PlayerClass::Recon: p.weaponId=2; p.maxHealth=85; p.health=85; p.maxShield=95; p.shield=95; p.armor=10; p.moveSpeed=14.0f; break;
      case PlayerClass::Sentinel: p.weaponId=3; p.maxHealth=145; p.health=145; p.maxShield=110; p.shield=110; p.armor=42; p.moveSpeed=7.0f; break;
      case PlayerClass::Medic: p.weaponId=4; p.maxHealth=100; p.health=100; p.maxShield=140; p.shield=140; p.armor=16; p.moveSpeed=9.0f; break;
    }
    p.playerClass=type;
    players_.push_back(p); log_.add(elapsed_,"PLAYER",""+p.name+" entered the battlefield (ID "+std::to_string(p.id)+").");
    return &players_.back();
}
bool Match::removePlayer(PlayerId id){
    auto it=std::find_if(players_.begin(),players_.end(),[&](const Player&p){return p.id==id;});
    if(it==players_.end())return false;
    log_.add(elapsed_,"PLAYER",it->name+" removed from roster."); players_.erase(it); return true;
}
Player* Match::find(PlayerId id){for(auto& p:players_)if(p.id==id)return &p;return nullptr;}
const Weapon* Match::weapon(int id)const{for(const auto&w:weapons_)if(w.id==id)return &w;return nullptr;}
bool Match::assignWeapon(PlayerId id,int wid){auto*p=find(id);if(!p||!weapon(wid))return false;p->weaponId=wid;log_.add(elapsed_,"LOADOUT",p->name+" equipped "+weapon(wid)->name+".");return true;}
int Match::aliveCount()const{return static_cast<int>(std::count_if(players_.begin(),players_.end(),[](const Player&p){return p.alive;}));}
void Match::start(){if(state_==MatchState::Ready||state_==MatchState::Paused){state_=MatchState::Running;log_.add(elapsed_,"MATCH","Simulation started.");}}
void Match::pause(){if(state_==MatchState::Running){state_=MatchState::Paused;log_.add(elapsed_,"MATCH","Simulation paused.");}}
void Match::resume(){start();}
void Match::reset(){for(auto&p:players_){p.health=p.maxHealth;p.shield=p.maxShield;p.alive=true;p.eliminations=0;p.abilityCharge=0;p.abilityCooldown=0;p.fireCooldown=0;p.damageDealt=0;p.damageTaken=0;p.trail.clear();}elapsed_=0;combatAccumulator_=zoneAccumulator_=0;zone_.reset();state_=MatchState::Ready;log_.clear();log_.add(0,"MATCH","Match reset.");}
void Match::tick(float dt){
    if(state_!=MatchState::Running||dt<=0)return;
    dt=std::min(dt,0.08f); elapsed_+=dt; combatAccumulator_+=dt; zoneAccumulator_+=dt;
    // Autonomous combat movement: seek targets, kite at weapon range, and leave visible trails.
    for(auto& p:players_) if(p.alive){
        Player* target=nullptr; float best=1e9f;
        for(auto& q:players_) if(q.alive && q.id!=p.id){
            float dx=q.position.x-p.position.x,dy=q.position.y-p.position.y,d=std::sqrt(dx*dx+dy*dy);
            if(d<best){best=d;target=&q;}
        }
        if(target){
            float dx=target->position.x-p.position.x,dy=target->position.y-p.position.y;
            float d=std::max(0.001f,std::sqrt(dx*dx+dy*dy));
            const Weapon* w=weapon(p.weaponId); float desired=w? w->range*0.68f:24.0f;
            float direction=(d>desired?1.0f:-0.28f);
            if(p.playerClass==PlayerClass::Sentinel) direction=(d>desired*.8f?0.75f:-0.08f);
            if(p.playerClass==PlayerClass::Recon) direction=(d>desired?1.25f:-0.65f);
            p.velocity.x=(dx/d)*p.moveSpeed*direction;
            p.velocity.y=(dy/d)*p.moveSpeed*direction;
            // A gentle lateral strafe keeps encounters from looking like straight-line pursuit.
            float strafe=(p.id%2?1.0f:-1.0f)*p.moveSpeed*.24f;
            p.position.x+=(p.velocity.x-(dy/d)*strafe)*dt;
            p.position.y+=(p.velocity.y+(dx/d)*strafe)*dt;
        }
        p.position.x=std::clamp(p.position.x,3.0f,97.0f);p.position.y=std::clamp(p.position.y,3.0f,97.0f);
        p.abilityCooldown=std::max(0.0f,p.abilityCooldown-dt);p.fireCooldown=std::max(0.0f,p.fireCooldown-dt);p.abilityCharge=std::min(100.0f,p.abilityCharge+dt*3.0f);
        if(p.abilityCharge>=100 && p.abilityCooldown<=0){
            if(p.playerClass==PlayerClass::Medic){p.shield=std::min(p.maxShield,p.shield+18.0f);log_.add(elapsed_,"ABILITY",p.name+" triggered NOVA PULSE — shields restored.");}
            else if(p.playerClass==PlayerClass::Sentinel){p.armor=std::min(60.0f,p.armor+4.0f);log_.add(elapsed_,"ABILITY",p.name+" activated FORTIFY.");}
            else {p.position.x=std::clamp(p.position.x+(p.velocity.x*.35f),3.0f,97.0f);p.position.y=std::clamp(p.position.y+(p.velocity.y*.35f),3.0f,97.0f);log_.add(elapsed_,"ABILITY",p.name+" activated "+abilityName(p.playerClass)+".");}
            p.abilityCharge=0;p.abilityCooldown=12.0f;
        }
        if(p.trail.empty()||std::hypot(p.position.x-p.trail.back().x,p.position.y-p.trail.back().y)>1.2f){
            p.trail.push_back(p.position);if(p.trail.size()>24)p.trail.erase(p.trail.begin());
        }
    }
    if(autoCombat_&&combatAccumulator_>=0.18f){combatAccumulator_=0;combatTick();}
    if(zoneAccumulator_>=18.0f){zoneAccumulator_=0;if(zone_.shrink())log_.add(elapsed_,"ZONE","Safe zone contracted to radius "+std::to_string(zone_.radius())+".");}
    for(auto&p:players_)if(p.alive){float dx=p.position.x-50,dy=p.position.y-50;if(std::sqrt(dx*dx+dy*dy)>zone_.radius()){p.health-=zone_.damagePerSecond()*dt;if(p.health<=0){p.health=0;p.alive=false;log_.add(elapsed_,"ELIMINATION",p.name+" was eliminated by the storm.");}}}
    if(players_.size()>0&&aliveCount()<=1&&state_==MatchState::Running){state_=MatchState::Finished;log_.add(elapsed_,"MATCH","Match finished.");}
}
void Match::combatTick(){
    for(auto& attacker:players_) {
        if(!attacker.alive || attacker.fireCooldown>0) continue;
        const Weapon* w=weapon(attacker.weaponId); if(!w) continue;
        Player* target=nullptr; float best=1.0e9f;
        for(auto& candidate:players_) if(candidate.alive && candidate.id!=attacker.id) {
            float d=std::hypot(candidate.position.x-attacker.position.x,candidate.position.y-attacker.position.y);
            // Recon prioritises the weakest visible target; other classes select nearest.
            float score=d;
            if(attacker.playerClass==PlayerClass::Recon) score=d+candidate.health*0.10f;
            if(score<best){best=score;target=&candidate;}
        }
        if(!target) continue;
        float distance=std::hypot(target->position.x-attacker.position.x,target->position.y-attacker.position.y);
        if(distance>w->range) continue;
        attacker.fireCooldown=w->fireRate;
        float raw=w->damage;
        if(attacker.playerClass==PlayerClass::Assault && attacker.abilityCooldown>8.0f) raw*=1.15f;
        float damage=std::max(1.0f,raw-target->armor*0.12f);
        float shieldHit=std::min(target->shield,damage); target->shield-=shieldHit; damage-=shieldHit;
        float hpBefore=target->health; target->health=std::max(0.0f,target->health-damage);
        float inflicted=shieldHit+(hpBefore-target->health); attacker.damageDealt+=inflicted; target->damageTaken+=inflicted;
        std::ostringstream msg; msg<<attacker.name<<" ["<<className(attacker.playerClass)<<"] hit "<<target->name<<" with "<<w->name<<" for "<<inflicted<<" damage";
        log_.add(elapsed_,"COMBAT",msg.str());
        if(target->health<=0){target->alive=false;++attacker.eliminations;log_.add(elapsed_,"ELIMINATION",attacker.name+" eliminated "+target->name+" with "+w->name+".");}
    }
}

}
