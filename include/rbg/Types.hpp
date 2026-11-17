#pragma once
#include <cstdint>
#include <string>
namespace rbg {
using PlayerId = std::uint32_t;
enum class MatchState { Ready, Running, Paused, Finished };
enum class PlayerClass { Assault=0, Recon=1, Sentinel=2, Medic=3 };
inline const char* className(PlayerClass c) {
 switch(c){case PlayerClass::Assault:return "VANGUARD";case PlayerClass::Recon:return "PHANTOM";case PlayerClass::Sentinel:return "TITAN";case PlayerClass::Medic:return "ARCWEAVER";} return "UNKNOWN";
}
inline const char* abilityName(PlayerClass c) {
 switch(c){case PlayerClass::Assault:return "OVERDRIVE";case PlayerClass::Recon:return "PHASE DASH";case PlayerClass::Sentinel:return "FORTIFY";case PlayerClass::Medic:return "NOVA PULSE";} return "UNKNOWN";
}
struct Vec3 { float x{0}, y{0}, z{0}; };
inline const char* toString(MatchState s) {
 switch(s) { case MatchState::Ready:return "READY"; case MatchState::Running:return "RUNNING"; case MatchState::Paused:return "PAUSED"; case MatchState::Finished:return "FINISHED"; }
 return "UNKNOWN";
}
}
