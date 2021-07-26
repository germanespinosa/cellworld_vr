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
                );
        Location3 location;
        Rotation3 rotation;
    };

    using State_vector = json_cpp::Json_vector<double>;

    struct State : json_cpp::Json_object {
        State(){};
        State (const State_vector &);
        Json_object_members(
                Add_member(episode);
                Add_member(time_stamp);
                Add_member(predator);
                Add_member(prey);
        );
        unsigned int episode;
        double time_stamp;
        Agent_state predator;
        Agent_state prey;
    };
}