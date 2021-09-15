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
    if (argc != 2) {
        cout << "Wrong parameters." << endl;
        cout << "Usage: ./cellworld_server [destination_folder]" << endl;
        exit(1);
    }
    int port = Vr_service::port();
    string destination_folder (argv[1]);
    Vr_service::set_destination_folder(destination_folder);
    Vr_service::new_experiment();

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
