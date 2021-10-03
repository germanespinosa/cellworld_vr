#include <iostream>
#include <cell_world_vr/vr_service.h>
#include <cell_world_tools.h>
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
        cout << "Usage: ./set_view_angle [View_angle]" << endl;
        exit(1);
    }
    string view_angle (argv[1]);
    cout << "setting ghost view angle to: " << view_angle << "... " << flush;

    Connection connection = Connection::connect_remote("127.0.0.1", Vr_service::port());
    Message message;
    message.command = "set_view_angle";
    message.content = view_angle;
    string msg_string;
    msg_string << message;
    connection.send_data(msg_string.c_str(), msg_string.size() + 1);
    while (!connection.receive_data());
    string result_str(connection.buffer);
    Message result;
    result_str >> result;
    cout << result.content << endl;
    return 0;
}
