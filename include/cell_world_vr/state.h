#pragma once

#include <json_cpp.h>
#include <cell_world.h>
#include <cell_world_vr/location3.h>
#include <cell_world_vr/rotation3.h>

namespace cell_world::vr {
    struct Agent_state : json_cpp::Json_object {
        Json_object_members(
                Add_member(location);
                Add_member(rotation);
                Add_member(cell);
                );
        Location3 location;
        Rotation3 rotation;
        Cell cell;
    };

    using State_vector = json_cpp::Json_vector<double>;

    struct State : json_cpp::Json_object {
        State(){};
        void update (const State_vector &);
        Json_object_members(
                Add_member(time_stamp);
                Add_member(predator);
                Add_member(prey);
        );
        double time_stamp;
        Agent_state predator;
        Agent_state prey;
    };
}