#pragma once

#include <json_cpp.h>
#include <cell_world.h>

namespace cell_world::vr {
    struct Location3 : json_cpp::Json_object {
        Json_object_members(
                Add_member(x);
                Add_member(y);
                Add_member(z);
        )
        double x, y, z;
        Location to_location();
    };
}