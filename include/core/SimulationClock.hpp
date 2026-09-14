#pragma once

#include <string>

namespace conesim {

class SimulationClock {
private:
    static double clockTime;
    static SimulationClock* clock;

    SimulationClock();

public:
    static SimulationClock* getInstance();

    static double getTime();
    static int getIntTime();

    void advance(double time);
    void setTime(double time);

    std::string toString() const;

    static void reset();
};

}