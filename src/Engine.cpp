#include "rbg/Engine.hpp"
namespace rbg {
bool Engine::initialize(){initialized_=true;return true;}
Match& Engine::createMatch(std::size_t cap){if(!initialized_)initialize();match_=std::make_unique<Match>(cap);return *match_;}
void Engine::shutdown(){match_.reset();initialized_=false;}
}
