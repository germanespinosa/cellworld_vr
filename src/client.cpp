#include <cell_world_vr.h>
#include <easy_tcp.h>
#include <iostream>
#include <unistd.h>

using namespace easy_tcp;
using namespace std;
using namespace cell_world::vr;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout << "Wrong parameter." << endl;
        cout << "Usage: ./tcp_client [Port]" << endl;
        exit(1);
    }
    int port = atoi(argv[1]);
    auto connection = Connection::connect_remote("127.0.0.1",port);

    Message msg;
    msg.command = "update_experiment_state";
    msg.content = "[1,2,3,4,5,6,7,8,9,0,1,2,3]";
    while(1) {
        string m;
        m << msg;
        cout << m <<  endl;
        connection.send_data(m.c_str(),m.size());
        usleep(10000);
    }

    return 0;
}
