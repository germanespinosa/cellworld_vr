#include <iostream>
#include <cell_world_vr.h>
#include <easy_tcp.h>

using namespace std;
using namespace json_cpp;
using namespace cell_world;
using namespace cell_world::vr;
using namespace easy_tcp;

int main(int argc, char *argv[])
{
    if (argc != 4) {
        cout << "Wrong parameters." << endl;
        cout << "Usage: ./cellworld_server [Port] [World_name] [speed]" << endl;
        exit(1);
    }
    string world_name (argv[2]);
    Vr_service::set_world(world_name);
    int port = atoi(argv[1]);
    Vr_service::set_speed(atof(argv[3]));

    // start server on port 65123
    Server<Vr_service> server ;
    if (server.start(port)) {
        std::cout << "Server setup succeeded on port " << port << std::endl;
    } else {
        std::cout << "Server setup failed " << std::endl;
        return EXIT_FAILURE;
    }
    while(1);
    return 0;
}
