#pragma once
#include <cell_world.h>
#include <easy_tcp.h>
#include <fstream>
#include <cell_world_vr/state.h>

namespace cell_world::vr {
    struct Vr_service : easy_tcp::Service {
        struct Predator_instruction : json_cpp::Json_object {
            Json_object_members(
                    Add_member(destination);
                    Add_member(next_step);
                    Add_member(contact);
                    );
            unsigned int destination = Cell::ghost_cell().id;
            unsigned int next_step = Cell::ghost_cell().id;
            bool contact;
        };

        Cell get_cell(Location l);
        void on_connect() override;
        void create_new_log_file(const std::string &);
        void close_log_file();

        void on_incoming_data(const std::string &plugin_data) override;
        void on_disconnect() override;

        bool update_state(const State_vector &);
        State state;
        std::ofstream log_file;

        static std::string format_time(const std::string &format);
        static bool set_destination_folder(const std::string &);
        static bool set_experiment(const std::string &);
        static bool new_experiment();
        static bool set_world(const std::string &);
        static bool set_speed(double);
        static bool set_view_angle(double);
        static bool update_spawn_locations();
        static bool set_ghost_min_distance(unsigned int);

    private:
        Predator_instruction predator_instruction;
        std::string participant_id;
        std::string file_name;
        Cell_group visibility_cone;
        unsigned int record_count = 0;
    };
}