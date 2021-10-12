#pragma once
#include <cell_world.h>
#include <easy_tcp.h>
#include <fstream>
#include <cell_world_vr/state.h>
#include <cell_world_tools/message.h>


namespace cell_world::vr {

    enum Mode{
        in_experiment,
        in_training
    };

    struct Occlusions : json_cpp::Json_object {
        Json_object_members(
                Add_member(OcclusionIds);
        );
        json_cpp::Json_vector<unsigned int> OcclusionIds;
    };

    struct Training : json_cpp::Json_object{
        Training() = default;
        Json_object_members(
                Add_member(initial_speed);
                Add_member(initial_speed_adjustment);
                Add_member(correction);
        );
        double initial_speed = 300;
        double initial_speed_adjustment = 100;
        double correction = .75;
    };

    struct Participant : json_cpp::Json_object{
        Participant() = default;
        Participant(int id) : id(id){

        }
        Json_object_members(
                Add_member(name);
                Add_member(id);
                Add_member(success);
                Add_member(fail);
                Add_member(speed);
                Add_member(adjustment);
        );
        std::string name;
        int id{};
        int success{};
        int fail{};
        double speed{};
        bool last{};
        double adjustment{};
    };

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
        void create_new_log_file(int);
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

        static bool set_training(const std::string &);

        static bool set_view_angle(double);

        static bool update_spawn_locations();
        static bool set_ghost_min_distance(unsigned int);

        static bool set_show_visibility(bool);
        static bool set_incognito_mode(bool);
        static bool set_participant_name(const std::string &);
        static bool start_training();
        static bool stop_training();

        static bool set_spawn_cell();
        static bool send_update(const Message &);
        static int port();
    private:
        Predator_instruction predator_instruction;
        std::string file_name;
        Cell_group visibility_cone;
        unsigned int record_count = 0;
        bool active = false;
    };
}