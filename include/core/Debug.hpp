#pragma once

#include <iostream>
#include <string>

namespace conesim {

class Debug {
private:
    static std::ostream* out;
    static int debugLevel;
    static long long timingStart;
    static std::string timingCause;
    
public:
    static void setDebugLevel(int level);
    static void setPrintStream(std::ostream& outStrm);

    static void p(const std::string& txt);
    static void p(const std::string& txt, int level);
    static void p(const std::string& txt, int level, bool timestamp);

    static void pt(const std::string& txt, int level);
    static void pt(const std::string& txt);

    static void startTiming(const std::string& cause);
    static void doneTiming();
};

}