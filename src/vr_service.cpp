#include <cell_world_vr/vr_service.h>
#include <cell_world_vr/message.h>
#include <cell_world_tools.h>
#include <filesystem>

using namespace json_cpp;
using namespace std;

namespace cell_world::vr {

    struct Occlusions : Json_object {
        Json_object_members(
                Add_member(OcclusionIds);
                );
        Json_vector<unsigned int> OcclusionIds;
    };

    struct Data{
        Data (const std::string &world_name):
        world(Json_create<World>(Web_resource::from("world").key(world_name).get())),
        cells(world.create_cell_group()),
        map(cells),
        graph(world.create_graph()),
        visibility(world.create_graph(Json_create<Graph_builder>(Web_resource::from("graph").key(world_name).key("visibility").get()))),
        invert_visibility(Visibility::invert(visibility)),
        paths(world.create_paths(Json_create<Path_builder>(Web_resource::from("paths").key(world.name).key("astar").get()))){
        }
        World world;
        Cell_group cells;
        Map map;
        Graph graph;
        Graph visibility;
        Graph invert_visibility;
        Paths paths;
    };

    Data *data = nullptr;
    double speed = 0;
    string destination_folder;
    string experiment_name;
    bool show_visibility = false;
    double view_angle = M_PI;
    unsigned int min_ghost_distance = 5;
    Cell_group spawn_locations;

    Cell Vr_service::get_cell(Location l){
        double min_dis=l.dist(data->cells[0].location);
        unsigned int min_id = 0;
        for (auto &cellr:data->cells){
            auto &cell = cellr.get();
            auto d = l.dist(cell.location);
            if (d < min_dis) {
                min_dis = d;
                min_id = cell.id;
            }
        }
        return data->cells[min_id];
    }

    bool Vr_service::update_spawn_locations (){
        auto start = data->map[{-20,0}];
        spawn_locations.clear();
        for (auto &cell : data->cells){
            if (!cell.get().occluded && data->paths.get_steps(start,cell) >= (int)min_ghost_distance) {
                spawn_locations.add(cell);
            }
        }
        if (spawn_locations.empty()) return false;
        return true;
    }

    bool Vr_service::update_state(const State_vector &game_state_vector){
        auto prey_cell = state.prey.cell;
        auto predator_cell = state.predator.cell;
        state.update(game_state_vector);
        state.predator.cell = get_cell(state.predator.location.to_location());
        state.prey.cell =  get_cell(state.prey.location.to_location());
        Visibility_cone cone(data->visibility, view_angle / 2);
        visibility_cone = cone.visible_cells(state.predator.cell,Visibility_cone::to_radians(-state.predator.rotation.yaw - 90));
        predator_instruction.contact = visibility_cone.contains(state.prey.cell);
        if (predator_instruction.contact) {
            predator_instruction.destination = state.prey.cell.id;
        } else {
            if ((prey_cell != predator_cell) && (predator_instruction.destination == Cell::ghost_cell().id || predator_instruction.destination == state.predator.cell.id)) {
                auto inverted_visibility = data->cells.free_cells() - visibility_cone;
                predator_instruction.destination = inverted_visibility.random_cell().id;
            }
        }
        auto move = data->paths.get_move(state.predator.cell,data->cells[predator_instruction.destination]);
        auto destination_coordinates = state.predator.cell.coordinates + move;
        predator_instruction.next_step = data->map[destination_coordinates].id;
        return prey_cell != state.prey.cell || predator_cell != state.predator.cell;
    }

    void Vr_service::on_connect()  {
        cout << "new client connected "<< endl;
    }

    std::string Vr_service::format_time(const string &format) {
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[256];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), format.c_str(), &tstruct);
        return buf;
    }

    void Vr_service::create_new_log_file(const std::string &participant){
        participant_id = participant;
        string folder = destination_folder + "/"+ experiment_name + "/" + data->world.name + "/P" + participant_id;
        filesystem::create_directory(destination_folder);
        filesystem::create_directory(destination_folder + "/"+ experiment_name);
        filesystem::create_directory(destination_folder + "/"+ experiment_name + "/" + data->world.name);
        filesystem::create_directory(folder);
        file_name = folder + "/" + format_time("%Y%m%d%H%M%S.log");
        log_file.open (file_name);
        log_file << "[";
    }

    void Vr_service::close_log_file() {
        if (log_file.is_open()) {
            log_file << "]";
            log_file.close();
        }
    }

    void Vr_service::on_incoming_data(const std::string &plugin_data)  {
        cout << "Request:" << plugin_data << endl;
        Message request;
        Message response;
        try {
            plugin_data >> request;
        } catch (...) {
            return; // ignores malformed messages
        }
        if (request.command == "start_episode") {
            response.command = "set_speed";
            response.content << Json_object_wrapper(speed);
            send_data(response.to_json());
            create_new_log_file(request.content);
            record_count = 0;
        }
        if (request.command == "get_spawn_cell") {
            response.command = "set_spawn_cell";

            int spawn_cell_id;

            if (!spawn_locations.empty())
                spawn_cell_id = spawn_locations.random_cell().id;
            else
                spawn_cell_id = data->cells.free_cells().random_cell().id;

            cout << "new spawn cell " << spawn_cell_id << endl ;
            response.content << Json_object_wrapper(spawn_cell_id);
            send_data(response.to_json());
        }
        if (request.command == "set_game_state"){
            State_vector game_state_vector;
            request.content >> game_state_vector;
            bool changes = update_state(game_state_vector);
            if (record_count++) log_file << ",";
            log_file << state << endl;
            if (changes) {
                if (state.predator.cell == state.prey.cell) {
                    response.command = "set_prey_caught";
                    response.content = "";
                    send_data(response.to_json());
                } else {
                    response.command = "set_destination_cell";
                    cout << predator_instruction << endl;
                    response.content << predator_instruction;
                    send_data(response.to_json());
                }
            }
        }
        if (request.command == "get_occlusions"){
            Occlusions occlusions;
            auto occluded_cells = data->cells.occluded_cells();
            for (const Cell &c:occluded_cells){
                occlusions.OcclusionIds.push_back(c.id);
            }
            response.command = "set_occlusions";
            response.content << occlusions;
            send_data(response.to_json());
        }
        if (request.command == "get_cell"){
            unsigned int cell_id;
            cell_id = stoi(request.content);
            response.command = "set_cell";
            response.content << data->world[cell_id];
            send_data(response.to_json());
        }
        if (request.command == "get_visibility"){
            response.command = "set_visibility";
            response.content << visibility_cone;
            send_data(response.to_json());
        }
        if (request.command == "get_show_visibility"){
            if (show_visibility)
                response.command = "show_visibility";
            else
                response.command = "hide_visibility";
            response.content = "";
            send_data(response.to_json());
        }
        if (request.command == "get_world_name"){
            response.command = "set_world_name";
            response.content = data->world.name;
            send_data(response.to_json());
        }

        // console
        if (request.command == "set_min_ghost_distance"){
            response.command = "result";
            if (set_ghost_min_distance(atoi(request.content.c_str()))){
                response.content = "success";
            } else {
                response.content = "failed";
            }
            send_data(response.to_json());
        }
        if (request.command == "new_experiment"){
            response.command = "result";
            if (new_experiment()) {
                response.content = "success: " + experiment_name;
            }else{
                response.content = "fail";
            }
            send_data(response.to_json());
        }
        if (request.command == "show_visibility"){
            show_visibility = true;
            response.command = "result";
            response.content = "success";
            send_data(response.to_json());
        }
        if (request.command == "hide_visibility"){
            show_visibility = false;
            response.command = "result";
            response.content = "success";
            send_data(response.to_json());
        }
        if (request.command == "set_world"){
            response.command = "result";
            if (set_world(request.content)) {
                response.content = "success";
            }else{
                response.content = "fail";
            }
            send_data(response.to_json());
        }
        if (request.command == "set_view_angle"){
            response.command = "result";
            if (set_view_angle(Visibility_cone::to_radians(atof(request.content.c_str())))) {
                response.content = "success";
            } else {
                response.content = "fail";
            }
            send_data(response.to_json());
        }
        if (request.command == "set_speed"){
            response.command = "result";
            if (set_speed(atof(request.content.c_str()))) {
                response.content = "success";
            } else {
                response.content = "fail";
            }
            send_data(response.to_json());
        }
        if (!response.command.empty())  cout << "Response:" << response.to_json() << endl << endl;
    }

    void Vr_service::on_disconnect()  {
        close_log_file();
        cout << "client disconnected "<< endl;
    }

    bool Vr_service::set_world(const string &world_name) {
        try {
            data = new Data(world_name);
        } catch (...) {
            return false;
        }
        return update_spawn_locations();
    }

    bool Vr_service::set_speed(double new_speed) {
        if (new_speed <= 0) return false;
        speed = new_speed;
        return true;
    }

    bool Vr_service::set_view_angle(double new_view_angle) {
        if (view_angle <= 0) return false;
        view_angle = new_view_angle;
        return true;
    }


    bool  Vr_service::set_destination_folder(const std::string &folder) {
        destination_folder = folder;
        return true;
    }

    bool Vr_service::set_experiment(const string &experiment) {
        if (experiment.empty()) return false;
        experiment_name = experiment;
        return true;
    }

    bool Vr_service::new_experiment() {
        return set_experiment(format_time("%Y%m%d_%H%M"));
    }

    bool Vr_service::set_ghost_min_distance(unsigned int value) {
        min_ghost_distance = value;
        return update_spawn_locations();
    }

}