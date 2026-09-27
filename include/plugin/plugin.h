#pragma once
#include "types.h"

struct Plugin{
    Plugin(const fa_plugin& plugin):id(plugin.id), version(plugin.version){}
    std::string id;
    std::string version;
};