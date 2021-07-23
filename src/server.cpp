#include <iostream>
#include <signal.h>
#include <cell_world_vr.h>
#include <cell_world_vr/net/tcp_server.h>


using namespace std;
using namespace json_cpp;
using namespace cell_world;
using namespace cell_world::vr;
using namespace cell_world::vr::net;

// declare the server
TcpServer server;

// declare a server observer which will receive incoming messages.
// the server supports multiple observers
server_observer_t observer;


// observer callback. will be called for every new message received by clients
// with the requested IP address
void onIncomingMsg(const Client & client, const char * msg, size_t size) {
    std::string msgStr = msg;
    Message server_cmd;
    Message client_cmd;
    msgStr >> server_cmd;
    if (server_cmd.command == "update_game_state"){
        cout << server_cmd.content << endl;
        State_vector game_state_vector;
        server_cmd.content >> game_state_vector;
        client_cmd.command = "update_predator_destination";
        State game_state(game_state_vector);
        client_cmd.content << game_state.prey.location.to_location();
        std::cout << "New game state: " << game_state << std::endl;
    }

    std::string replyMsg;
    if (Chance::coin_toss(.5)){
        Message set_speed;
        set_speed.command = "update_predator_speed";
        set_speed.content = "3";
        replyMsg << set_speed;
    } else {
        replyMsg << client_cmd;
    }
    server.sendToAllClients(replyMsg.c_str(), replyMsg.length());
    cout << "sent to game : " << replyMsg << endl;
//    }
}

// observer callback. will be called when client disconnects
void onClientDisconnected(const Client & client) {
    std::cout << "Client: " << client.getIp() << " disconnected: " << client.getInfoMessage() << std::endl;
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout << "Wrong parameter." << endl;
        cout << "Usage: ./tcp_server [Port]" << endl;
        exit(1);
    }
    int port = atoi(argv[1]);
    // start server on port 65123
    pipe_ret_t startRet = server.start(port);
    if (startRet.success) {
        std::cout << "Server setup succeeded on port " << port << std::endl;
    } else {
        std::cout << "Server setup failed: " << startRet.msg << std::endl;
        return EXIT_FAILURE;
    }

    // configure and register observer
    observer.incoming_packet_func = onIncomingMsg;
    observer.disconnected_func = onClientDisconnected;
    server.subscribe(observer);

    // receive clients
    while(1) {
        Client client = server.acceptClient(0);
        if (client.isConnected()) {
            std::cout << "Got client with IP: " << client.getIp() << std::endl;
            server.printClients();
        } else {
            std::cout << "Accepting client failed: " << client.getInfoMessage() << std::endl;
        }
        sleep(1);
    }

    return 0;
}
