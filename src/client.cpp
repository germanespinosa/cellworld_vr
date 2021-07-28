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
    msg.command = "get_cell";

    for (int i=0; i<331; i++) {
        msg.content << Json_wrap_object(i);
        string m;
        m << msg;
        cout << m <<  endl;
        connection.send_data(m.c_str(),m.size());
        usleep(10000);
        while (!connection.receive_data());
        cout << connection.buffer << endl;
    }

    return 0;
}
