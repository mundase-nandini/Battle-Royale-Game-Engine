#pragma once
namespace rbg {
class SafeZone {
public:
    float radius() const { return radius_; }
    int phase() const { return phase_; }
    float damagePerSecond() const { return damagePerSecond_; }
    void reset();
    bool shrink();
private: float radius_{100}, damagePerSecond_{5}; int phase_{0};
};
}
