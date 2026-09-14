#pragma once

#include <vector>

class Host;

namespace conesim {

class UpdateListener {
public:
    virtual ~UpdateListener();

    virtual void updated(const std::vector<Host*>& hosts) = 0;
};

}