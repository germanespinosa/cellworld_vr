#pragma once

#include <cell_world_vr.h>

namespace cell_world::vr {
    struct World_conversion {
        World_conversion (const Location &src_start, const Location &src_end, const Location &dst_start, const Location &dst_end);
        Location convert(Location);
        double source_rotation;
        double source_side;
        Location source_center;
        double destination_rotation;
        double destination_side;
        Location destination_center;
        double rotation;
        double side_ratio;
    };
}