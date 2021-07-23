#pragma once

#include <json_cpp.h>
#include <cell_world.h>

namespace cell_world::vr {
    struct Rotation3 : json_cpp::Json_object {
        Json_object_members(
                Add_member(roll);
                Add_member(pitch);
                Add_member(yaw);
        )
        double roll,pitch,yaw;
    };
}