#pragma once

#include <json_cpp.h>
#include <cell_world.h>
#include <cell_world_vr/state.h>
#include <cell_world_tools.h>

namespace cell_world::vr {
    struct Vr_episode : json_cpp::Json_object {
        Vr_episode();
        Json_object_members(
                Add_member(start_time);
                Add_member(steps);
                );
        json_cpp::Json_date start_time;
        json_cpp::Json_vector<State> steps;
    };
}