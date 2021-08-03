#include <iostream>
#include <cell_world_tools.h>
#include <cell_world_vr.h>
#include <easy_tcp.h>
#include <time.h>

using namespace std;
using namespace json_cpp;
using namespace cell_world;
using namespace cell_world::vr;
using namespace easy_tcp;


struct Data{
    Data (const std::string &world_name):
    world(Json_create<World>(Web_resource::from("world").key(world_name).get())),
    cells(world.create_cell_group()),
    map(cells),
    graph(world.create_graph()),
    pb(Json_create<Path_builder>(Web_resource::from("paths").key(world.name).key("astar").get())),
    paths(world.create_paths(pb)){}
    World world;
    Cell_group cells;
    Map map;
    Graph graph;
    Path_builder pb;
    Paths paths;
} ;

const std::string format_time(string format) {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[256];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), format.c_str(), &tstruct);
    return buf;
}

Data *static_data;
double speed;

struct Experiment_service : Service {
    Experiment_service() : data(*static_data){
    }

    Cell get_cell(Location l){
        double min_dis=l.dist(data.cells[0].location);
        unsigned int min_id = 0;
        for (auto &cellr:data.cells){
            auto &cell = cellr.get();
            auto d = l.dist(cell.location);
            if (d < min_dis) {
                min_dis = d;
                min_id = cell.id;
            }
        }
        return data.cells[min_id];
    }

    void update_agents_coordinates(){
        state.predator.cell = get_cell(state.predator.location.to_location());
        state.prey.cell =  get_cell(state.prey.location.to_location());
        auto move = data.paths.get_move(state.predator.cell,state.prey.cell);
        auto destination_coordinates = state.predator.cell.coordinates + move;
        predator_destination = data.map[destination_coordinates];
    }

    void on_connect() override {
        cout << "new client connected "<< endl;
    }

    void create_new_log_file(){
        string file_name = format_time("%Y%m%d%H%M%S.log");
        log_file.open (file_name);
    }

    void on_incoming_data(const std::string &plugin_data) override {
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
            response.content << Json_object_wrapper(data.cells.free_cells().random_cell().id);
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
            response.command = "set_destination_cell";
            response.content << Json_object_wrapper(predator_destination.id);
            send_data(response.to_json());
        }
        if (request.command == "get_cell"){
            unsigned int cell_id;
            cell_id = stoi(request.content);
            response.command = "set_cell";
            response.content << data.world[cell_id];
            send_data(response.to_json());
        }
        if (!response.command.empty())
        cout << "Response:" << response.to_json() << endl << endl;
    }

    void on_disconnect() override {
        cout << "client disconnected "<< endl;
    }

    Data &data;
    State state;
    Cell predator_destination;
    ofstream log_file;
};

int main(int argc, char *argv[])
{
    if (argc != 4) {
        cout << "Wrong parameters." << endl;
        cout << "Usage: ./cellworld_server [Port] [World_name]" << endl;
        exit(1);
    }
    string world_name (argv[2]);
    static_data = new Data(world_name);

    int port = atoi(argv[1]);

    speed = atof(argv[3]);

    // start server on port 65123
    Server<Experiment_service> server ;
    if (server.start(port)) {
        std::cout << "Server setup succeeded on port " << port << std::endl;
    } else {
        std::cout << "Server setup failed " << std::endl;
        return EXIT_FAILURE;
    }
    while(1) {

    }
    delete(static_data);
    return 0;
}
