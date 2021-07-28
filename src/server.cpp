#include <strstream>
#include <iostream>
#include <cell_world_tools.h>
#include <cell_world_vr.h>
#include <easy_tcp.h>

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
    paths(world.create_paths(pb)){

    }
    World world;
    Cell_group cells;
    Map map;
    Graph graph;
    Path_builder pb;
    Paths paths;
} ;

struct Experiment_service : Service {
    Experiment_service() : data("hexa_10_05_vr"){

    }

    Location get_destination() {
        if (data.map[predator].occluded)
            return data.map[prey].location;
        Move move = data.paths.get_move(data.map[predator], data.map[prey]);
        return data.map[predator + move].location;
    }

    void update_agents_coordinates(){
        predator = data.map[state.predator.location.to_location()].coordinates;
        prey = data.map[state.prey.location.to_location()].coordinates;
    }

    void on_connect() override {
        cout << "new client connected "<< endl;
    }

    void on_incoming_data(const std::string &plugin_data) override {
        cout << plugin_data<< endl;
        Message request;
        plugin_data >> request ;
        if (request.command == "update_game_state"){
            cout << request.content << endl;
            State_vector game_state_vector;
            request.content >> game_state_vector;
            state = State(game_state_vector);
            update_agents_coordinates();
            Message response;
            response.command = "update_predator_destination";
            State game_state(game_state_vector);
            response.content << get_destination();
            send_data(response.to_json());
        }
        if (request.command == "get_cell"){
            unsigned int cell_id;
            cell_id = stoi(request.content);
            Message response;
            response.command = "set_cell";
            response.content << data.world[cell_id];
            send_data(response.to_json());
        }
    }

    void on_disconnect() override {
        cout << "client disconnected "<< endl;
    }

    Data data;
    State state;
    Coordinates prey;
    Coordinates predator;
};

//
//// observer callback. will be called for every new message received by clients
//// with the requested IP address
//void onIncomingMsg(const Client & client, const char * msg, size_t size) {
//    std::string msgStr = msg;
//    Message server_cmd;
//    Message client_cmd;
//    msgStr >> server_cmd;
//    if (server_cmd.command == "update_game_state"){
//        cout << server_cmd.content << endl;
//        State_vector game_state_vector;
//        server_cmd.content >> game_state_vector;
//        client_cmd.command = "update_predator_destination";
//        State game_state(game_state_vector);
//        client_cmd.content << game_state.prey.location.to_location();
//        std::cout << "New game state: " << game_state << std::endl;
//    }
//
//    std::string replyMsg;
//    if (Chance::coin_toss(.5)){
//        Message set_speed;
//        set_speed.command = "update_predator_speed";
//        set_speed.content = "3";
//        replyMsg << set_speed;
//    } else {
//        replyMsg << client_cmd;
//    }
//    server.sendToAllClients(replyMsg.c_str(), replyMsg.length());
//    cout << "sent to game : " << replyMsg << endl;
////    }
//}
//
//// observer callback. will be called when client disconnects
//void onClientDisconnected(const Client & client) {
//    std::cout << "Client: " << client.getIp() << " disconnected: " << client.getInfoMessage() << std::endl;
//}
//

int main(int argc, char *argv[])
{
//    auto &wr = Web_resource::from("paths").key("hexa_10_05_vr").key("astar").get();
//
//    string s;
//    stringstream ss (s);
//    ss << wr;
//    cout << s;

    Data data("hexa_10_05_vr");
    if (argc != 2) {
        cout << "Wrong parameter." << endl;
        cout << "Usage: ./tcp_server [Port]" << endl;
        exit(1);
    }
    int port = atoi(argv[1]);
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

    return 0;
}
