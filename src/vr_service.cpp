#include <cell_world_vr/vr_service.h>
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
        visibility(world.create_graph(Json_create<Graph_builder>(Web_resource::from("graph").key(world_name).key("visibility").get()))),
        paths(world.create_paths(Json_create<Path_builder>(Web_resource::from("paths").key(world.name).key("astar").get()))){
        }
        World world;
        Cell_group cells;
        Map map;
        Graph visibility;
        Paths paths;
    };

    Data *data = new Data("hexa_10_00_vr");
    double speed = 100;
    string destination_folder;
    string experiment_name;
    bool show_visibility = false;
    double view_angle = 145 * 2 * M_PI / 360;
    unsigned int min_ghost_distance = 5;
    Cell_group spawn_locations;
    bool incognito_mode = false;
    string participant_name;
    vector<string> participant_names;
    vector<int> participant_ids;
    Vr_service *receiving_updates;

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
        cout << spawn_locations << endl;
        if (spawn_locations.empty()) return false;
        return true;
    }

    bool Vr_service::update_state(const State_vector &game_state_vector){
        auto prey_cell = state.prey.cell;
        auto predator_cell = state.predator.cell;
        state.update(game_state_vector);
        state.predator.cell = get_cell(state.predator.location.to_location());
        state.prey.cell =  get_cell(state.prey.location.to_location());
        Visibility_cone cone(data->visibility, view_angle);
        visibility_cone = cone.visible_cells(state.predator.cell,Visibility_cone::to_radians(-state.predator.rotation.yaw - 90));
        if (!incognito_mode) {
            predator_instruction.contact = visibility_cone.contains(state.prey.cell);
            if (predator_instruction.contact) {
                predator_instruction.destination = state.prey.cell.id;
            } else {
                if ((prey_cell != predator_cell) && (predator_instruction.destination == Cell::ghost_cell().id || predator_instruction.destination == state.predator.cell.id)) {
                    auto inverted_visibility = data->cells.free_cells() - visibility_cone;
                    predator_instruction.destination = inverted_visibility.random_cell().id;
                }
            }
            if (prey_cell != predator_cell) {
                auto move = data->paths.get_move(state.predator.cell,data->cells[predator_instruction.destination]);
                auto destination_coordinates = state.predator.cell.coordinates + move;
                predator_instruction.next_step = data->map[destination_coordinates].id;
            } else {
                predator_instruction.next_step = predator_cell.id;
            }
        } else { //incognito mode
            predator_instruction.contact = false; // ghost can't see the player
            if (predator_instruction.destination == Cell::ghost_cell().id || predator_instruction.destination == state.predator.cell.id) {
                auto inverted_visibility = data->cells.free_cells() - visibility_cone;
                predator_instruction.destination = inverted_visibility.random_cell().id;
            }
            auto move = data->paths.get_move(state.predator.cell,data->cells[predator_instruction.destination]);
            auto destination_coordinates = state.predator.cell.coordinates + move;
            predator_instruction.next_step = data->map[destination_coordinates].id;
        }
        return !incognito_mode && (prey_cell != state.prey.cell || predator_cell != state.predator.cell);
    }

    void Vr_service::on_connect()  {
    }

    std::string Vr_service::format_time(const string &format) {
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[256];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), format.c_str(), &tstruct);
        return buf;
    }

    string get_participant(int id){
        if (!participant_name.empty()){
            string participant = participant_name;
            participant_name = "";
            for (unsigned int i=0;i<participant_ids.size();i++){
                if (participant_ids[i]==id) {
                    participant_names[i] = participant;
                    return participant;
                }
            }
            participant_ids.push_back(id);
            participant_names.push_back(participant);
            return participant;
        } else {
            for (unsigned int i=0;i<participant_ids.size();i++){
                if (participant_ids[i]==id) {
                     return participant_names[i];
                }
            }
            participant_ids.push_back(id);
            string participant = json_cpp::Json_object_wrapper<int>(id).to_json();
            participant_names.push_back(participant);
            return participant;
        }
    }

    void Vr_service::create_new_log_file(int participant_id){
        string participant = get_participant(participant_id);

        string folder = destination_folder + "/"+ experiment_name + "/" + data->world.name + "/P" + participant;
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
            predator_instruction.destination = Cell::ghost_cell().id;
            predator_instruction.next_step = Cell::ghost_cell().id;
            predator_instruction.contact = false;
            response.command = "set_speed";
            response.content << Json_object_wrapper(speed);
            send_data(response.to_json());
            create_new_log_file(atoi(request.content.c_str()));
            record_count = 0;
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

        // register update receiver
        if (request.command == "get_updates"){
            receiving_updates = this;
            cout << "Registering for updates" << endl;
            set_spawn_cell();
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

        if (request.command == "set_incognito_mode"){
            response.command = "result";
            if (set_incognito_mode(atoi(request.content.c_str()) != 0)){
                response.content = "success";
            } else {
                response.content = "fail";
            }
            send_data(response.to_json());
        }

        if (request.command == "show_visibility"){
            response.command = "result";
            if (set_show_visibility(true)){
                response.content = "success";
            } else {
                response.content = "fail";
            }
            send_data(response.to_json());
        }

        if (request.command == "hide_visibility"){
            response.command = "result";
            if (set_show_visibility(false)){
                response.content = "success";
            } else {
                response.content = "fail";
            }
            send_data(response.to_json());
            send_update({"show_visibility",""});
            send_update({"hide_visibility",""});
            response.content = "";

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
            if (set_view_angle(atof(request.content.c_str()) * M_PI * 2 / 360)) {
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

        if (request.command == "set_participant_name"){
            response.command = "result";
            if (set_participant_name(request.content)) {
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
        if (this == receiving_updates) {
            receiving_updates = nullptr;
            cout << "Deregistering for updates" << endl;
        }
        cout << "client disconnected "<< endl;
    }

    bool Vr_service::set_world(const string &world_name) {
        try {
            auto new_data = new Data(world_name);
            auto old_data = data;
            data = new_data;
            free(old_data);
        } catch (...) {
            return false;
        }
        Occlusions occlusions;
        auto occluded_cells = data->cells.occluded_cells();
        for (const Cell &c:occluded_cells){
            occlusions.OcclusionIds.push_back(c.id);
        }
        return send_update({"set_occlusions",occlusions.to_json()}) && update_spawn_locations() && set_spawn_cell();
    }

    bool Vr_service::set_speed(double new_speed) {
        if (new_speed <= 0) return false;
        speed = new_speed;
        return send_update({"set_speed", std::to_string(speed)});
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
        return update_spawn_locations() && set_spawn_cell();
    }

    bool Vr_service::set_incognito_mode(bool new_incognito_mode) {
        incognito_mode = new_incognito_mode;
        return true;
    }

    bool Vr_service::set_show_visibility(bool new_show_visibility) {
        show_visibility = new_show_visibility;
        send_update({show_visibility?"show_visibility":"hide_visibility",""});
        return true;
    }

    bool Vr_service::set_participant_name(const string &new_participant_name) {
        if (new_participant_name.empty()) return false;
        participant_name = new_participant_name;
        return true;
    }

    bool Vr_service::send_update(const Message &update) {
        if (receiving_updates) {
            cout << "Updating client: " << update << endl;
            return receiving_updates->send_data(update.to_json());
        }
        return false;
    }

    int Vr_service::port() {
        string port_str (std::getenv("CELLWORLD_VR_PORT")?std::getenv("CELLWORLD_VR_PORT"):"4000");
        return atoi(port_str.c_str());
    }

    bool Vr_service::set_spawn_cell() {
        unsigned int spawn_cell_id;
        if (!spawn_locations.empty())
            spawn_cell_id = spawn_locations.random_cell().id;
        else
            spawn_cell_id = data->cells.free_cells().random_cell().id;
        return send_update({"set_spawn_cell", std::to_string(spawn_cell_id)});
    }

}