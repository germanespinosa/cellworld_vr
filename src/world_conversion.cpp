#include <cell_world_vr/world_conversion.h>


using namespace std;

namespace cell_world::vr {
    World_conversion::World_conversion(const Location &src_start, const Location &src_end, const Location &dst_start, const Location &dst_end) {
        source_center = (src_start + src_end) *.5;
        source_side = src_start.dist(source_center);
        source_rotation = src_start.atan(src_end);
        destination_center = (dst_start + dst_end) *.5;
        destination_side = dst_start.dist(destination_center);
        destination_rotation = dst_start.atan(dst_end);
        rotation = -source_rotation + destination_rotation;
        side_ratio = destination_side / source_side;
    }

    Location World_conversion::convert(Location src_loc) {
        auto dst_dis = src_loc.dist(source_center) * side_ratio;
        auto theta = src_loc.atan(source_center) + rotation;
        auto dst_loc = Location(sin(theta), cos(theta) * .5 / .433) * dst_dis + destination_center;
        return dst_loc;
    }

}