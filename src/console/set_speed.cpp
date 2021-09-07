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
        cout << "Usage: ./set_speed [Speed]" << endl;
        exit(1);
    }
    string speed (argv[1]);
    cout << "setting ghost speed to: " << speed << "... " << flush;

    Connection connection = Connection::connect_remote("127.0.0.1", 4000);
    Message message;
    message.command = "set_speed";
    message.content = speed;
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
