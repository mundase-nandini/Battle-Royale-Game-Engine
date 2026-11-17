#include "rbg/Zone.hpp"
namespace rbg {
void SafeZone::reset(){radius_=100;phase_=0;damagePerSecond_=5;}
bool SafeZone::shrink(){
    if(radius_<=18) return false;
    radius_*=0.78f; ++phase_; damagePerSecond_+=2.0f; return true;
}
}
