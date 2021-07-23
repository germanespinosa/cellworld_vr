#include <cell_world_vr/state.h>

using namespace json_cpp;

namespace cell_world::vr {
    State::State(const State_vector &state_vector) {
        episode = (unsigned int) state_vector[0];
        time_stamp = state_vector[1];
        prey.location.x = state_vector[2];
        prey.location.y = state_vector[3];
        prey.location.z = state_vector[4];
        prey.rotation.roll = state_vector[5];
        prey.rotation.pitch = state_vector[6];
        prey.rotation.yaw = state_vector[7];
        predator.location.x = state_vector[8];
        predator.location.y = state_vector[9];
        predator.location.z = state_vector[10];
        predator.rotation.roll = state_vector[11];
        predator.rotation.pitch = state_vector[12];
        predator.rotation.yaw = state_vector[13];
    }
}