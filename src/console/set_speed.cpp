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
    if (argc != 2 && argc != 3) {
        cout << "Wrong parameters." << endl;
        cout << "Usage: ./set_speed [speed_change] [participant_id]" << endl;
        exit(1);
    }
    Speed_change speed_change;
    speed_change.change = atof(argv[1]);
    speed_change.participant_id = Not_found;
    cout << "changing participant";
    if (argc == 3 ) {
        speed_change.participant_id = atoi(argv[2]);
        cout << " " << speed_change.participant_id;
    } else {
        cout << "s";
    }
    cout << " speed by " << speed_change.change << "... " << flush;

    Connection connection = Connection::connect_remote("127.0.0.1", Vr_service::port());
    Message message;
    message.command = "set_participant_name";
    message.content = speed_change.to_json();
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
