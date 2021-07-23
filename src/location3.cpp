#include <cell_world_vr/location3.h>

cell_world::Location cell_world::vr::Location3::to_location() {
    return {x,y};
}
