#include "rbg/EventLog.hpp"
namespace rbg {
void EventLog::add(double t, std::string c, std::string m) {
    events_.push_back({t, std::move(c), std::move(m)});
    if(events_.size()>1000) events_.erase(events_.begin(), events_.begin()+200);
}
}
