#pragma once

#include <json_cpp.h>

namespace cell_world::vr {
    struct Message : json_cpp::Json_object {
        Json_object_members(
                Add_member(command);
                Add_member(content);
                );
        std::string command;
        std::string content;
    };
}