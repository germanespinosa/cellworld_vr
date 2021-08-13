#pragma once
#include <cell_world.h>
#include <easy_tcp.h>
#include <fstream>
#include <cell_world_vr/state.h>

namespace cell_world::vr {
    struct Vr_service : easy_tcp::Service {
        Cell get_cell(Location l);
        void on_connect() override;
        void create_new_log_file(const std::string &);
        void on_incoming_data(const std::string &plugin_data) override;
        void on_disconnect() override;

        void update_agents_coordinates();
        State state;
        Cell predator_destination;
        std::ofstream log_file;

        static std::string format_time(const std::string &format);
        static void set_destination_folder(const std::string &);
        static void set_experiment(const std::string &);
        static void set_world(const std::string &);
        static void set_speed(double);

    private:
        Cell destination = Cell::ghost_cell();
        std::string participant_id;
        std::string file_name;
        unsigned int record_count = 0;
    };
}