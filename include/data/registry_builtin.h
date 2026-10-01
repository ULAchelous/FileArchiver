#pragma once
#include "registry.h"

struct Context;

namespace reg{

    void fa_register_builtin_types(Context& ctx);
    void fa_register_builtin_repo_templates(Context& ctx);
}
