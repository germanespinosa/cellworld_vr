#include <cell_world_vr/vr_service.h>
#include <cell_world_vr/message.h>
#include <cell_world_tools.h>

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
        paths(world.create_paths(Json_create<Path_builder>(Web_resource::from("paths").key(world.name).key("astar").get()))){}
        World world;
        Cell_group cells;
        Map map;
        Graph graph;
        Paths paths;
    };

    Data *data = nullptr;
    double speed = 0;

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

    void Vr_service::update_agents_coordinates(){
        state.predator.cell = get_cell(state.predator.location.to_location());
        state.prey.cell =  get_cell(state.prey.location.to_location());
        auto move = data->paths.get_move(state.predator.cell,state.prey.cell);
        auto destination_coordinates = state.predator.cell.coordinates + move;
        predator_destination = data->map[destination_coordinates];
    }

    void Vr_service::on_connect()  {
        cout << "new client connected "<< endl;
    }

    const std::string format_time(string format) {
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[256];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), format.c_str(), &tstruct);
        return buf;
    }

    void Vr_service::create_new_log_file(){
        string file_name = format_time("%Y%m%d%H%M%S.log");
        log_file.open (file_name);
    }

    void Vr_service::on_incoming_data(const std::string &plugin_data)  {
        cout << "Request:" << plugin_data << endl;
        Message request;
        Message response;
        plugin_data >> request ;
        if (request.command == "start_episode") {
            response.command = "set_speed";
            response.content << Json_object_wrapper(speed);
            send_data(response.to_json());
            create_new_log_file();
            log_file << "[";
        }
        if (request.command == "get_spawn_cell") {
            response.command = "set_spawn_cell";
            response.content << Json_object_wrapper(data->cells.free_cells().random_cell().id);
            send_data(response.to_json());
        }
        if (request.command == "end_episode") {
            log_file << "]";
            log_file.close();
        }
        if (request.command == "set_game_state"){
            State_vector game_state_vector;
            request.content >> game_state_vector;
            state.update(game_state_vector);
            update_agents_coordinates();
            log_file << state << "," << endl;
            if (state.predator.cell == state.prey.cell) {
                response.command = "set_prey_caught";
                response.content = "";
                send_data(response.to_json());
            } else {
                response.command = "set_destination_cell";
                response.content << Json_object_wrapper(predator_destination.id);
                send_data(response.to_json());
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
        if (!response.command.empty())
            cout << "Response:" << response.to_json() << endl << endl;
    }

    void Vr_service::on_disconnect()  {
        cout << "client disconnected "<< endl;
    }

    void Vr_service::set_world(const string &world_name) {
        data = new Data(world_name);
    }

    void Vr_service::set_speed(double new_speed) {
        speed = new_speed;
    }
}