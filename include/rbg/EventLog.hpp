#pragma once
#include <string>
#include <vector>
namespace rbg {
struct Event { double time{}; std::string category, message; };
class EventLog {
public:
    void add(double time, std::string category, std::string message);
    const std::vector<Event>& events() const { return events_; }
    void clear() { events_.clear(); }
private: std::vector<Event> events_;
};
}
