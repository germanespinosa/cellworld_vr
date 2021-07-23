#pragma once

namespace cell_world::vr::net {

    struct pipe_ret_t {
        bool success;
        std::string msg;

        pipe_ret_t() {
            success = false;
            msg = "";
        }
    };

}