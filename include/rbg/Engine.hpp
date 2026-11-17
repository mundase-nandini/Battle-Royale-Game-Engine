#pragma once
#include "Match.hpp"
#include <memory>
namespace rbg {
class Engine {
public:
    bool initialize();
    Match& createMatch(std::size_t capacity=20);
    Match* match() { return match_.get(); }
    void shutdown();
private: std::unique_ptr<Match> match_; bool initialized_{false};
};
}
