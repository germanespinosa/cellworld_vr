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
    if (argc != 1) {
        cout << "Wrong parameters." << endl;
        cout << "Usage: ./new_experiment" << endl;
        exit(1);
    }
    cout << "creating new experiment... " << flush;

    Connection connection = Connection::connect_remote("127.0.0.1", Vr_service::port());
    Message message;
    message.command = "new_experiment";
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
