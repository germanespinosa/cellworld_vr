#include <cell_world_vr/state.h>

using namespace json_cpp;

namespace cell_world::vr {
    void State::update(const State_vector &state_vector) {
        time_stamp = state_vector[0];
        prey.location.x = state_vector[1];
        prey.location.y = state_vector[2];
        prey.location.z = state_vector[3];
        prey.rotation.roll = state_vector[4];
        prey.rotation.pitch = state_vector[5];
        prey.rotation.yaw = state_vector[6];
        predator.location.x = state_vector[7];
        predator.location.y = state_vector[8];
        predator.location.z = state_vector[9];
        predator.rotation.roll = state_vector[10];
        predator.rotation.pitch = state_vector[11];
        predator.rotation.yaw = state_vector[12];
    }
}