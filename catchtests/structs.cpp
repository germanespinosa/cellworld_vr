#include<catch.h>
#include <cell_world.h>
#include <cell_world_tools.h>
#include <cell_world_vr.h>

using namespace std;
using namespace json_cpp;
using namespace cell_world;
# define M_PI           3.14159265358979323846

TEST_CASE("location"){
    std::string world_name("hexa_10_05_vr");
    World world(Json_create<World>(Web_resource::from("world").key(world_name).get()));
    Cell_group cells(world.create_cell_group());
    Map map(cells);
    Location start_vertex{2149,790};
    Location end_vertex{427,1775};

    cout << endl << endl;
    cout << "start_vertex: " << start_vertex << endl;
    cout << "end_vertex: " << end_vertex << endl;
    cout << "mid: " << (end_vertex+start_vertex) * .5 << endl ;
    double distance = end_vertex.dist(start_vertex);
    cout << "total distance : " << distance << endl;
    cout << "x change R: " << (end_vertex - start_vertex) * (1.0/48.0);
    cout << endl << endl;

    double intercell_distance = distance / (21.0 + 1.0/3.0);
    double col_distance = intercell_distance / 2;
    double row_distance = sqrt( intercell_distance * intercell_distance - intercell_distance * intercell_distance / 4 );
    double side  = row_distance / 1.5;
    double padding = intercell_distance / 6;
    cout << "inter cell distance: " << intercell_distance << endl;
    cout << "side: " << side << " " << side/intercell_distance << endl;
    cout << "padding: "<<  padding  << " " << padding / side << " " << padding / intercell_distance << " " << distance/padding << endl;
    cout << endl << endl;

    Location v = end_vertex - start_vertex;
    double theta = atan2(v.y, v.x);
    Location xVector = Location{cos(theta),sin(theta)};
    Location xChange = xVector * col_distance;
    cout << "xChange: " << xChange << endl;
    double thetay = theta - M_PI / 2.0;
    Location yVector = Location{cos(thetay),sin(thetay)};
    Location yChange = yVector * row_distance;
    cout << "yChange: " << yChange << endl;
    cout << endl << endl;

    Location origin = (end_vertex + start_vertex) * .5;
    cout << "end_vertex: " << end_vertex << " == " << start_vertex + Location{cos(theta),sin(theta)} * distance
            << " == " << start_vertex + Location{cos(theta),sin(theta)} * (2 * padding + 21 * intercell_distance) << endl;
    cout << "origin: " << origin << " == " << start_vertex + Location{cos(theta),sin(theta)} * (10.5 * intercell_distance + padding) << endl;
    cout << endl << endl;


    Location start_cell_location = start_vertex +  Location{cos(theta),sin(theta)} * (padding + col_distance);
    cout << "start_cell_location: " << start_cell_location << endl;

    for (int x = -20;x <= 20; x++)
        for (int y = -10;y <= 10; y++){
            if (abs(x) + abs(y) <= 20 && (abs(x) + abs(y)) % 2 == 0){
                Coordinates coord (x,y);
                Location loc = origin + xChange * (double)x + yChange * (double)y;
                world.cells[map[coord].id].location = loc;
                cout << loc.x << "," << loc.y << "," << "#" << coord.x << "#"  << coord.y << endl;
            }
        }

    cout << "origin calculated: " << map[Coordinates{0,0}].location << endl;
    cout << "vertex distance: " << map[Coordinates{-20,0}].location.dist(map[Coordinates{20,0}].location) << endl;
    cout << "vertex distance: " << map[Coordinates{-10,-10}].location.dist(map[Coordinates{10,10}].location) << endl;
    cout << "vertex distance: " << map[Coordinates{-10,10}].location.dist(map[Coordinates{10,-10}].location) << endl;

    cout << endl << endl;
    cout << "(-20,  0): " << map[Coordinates{20,0}].location << "( 20,  0): " << map[Coordinates{20,0}].location << endl;
    cout << "(-10,-10): " << map[Coordinates{-10,-10}].location << "(-10,-10): " << map[Coordinates{10,10}].location << endl;
    cout << "(-10, 10): " << map[Coordinates{-10,10}].location << "( 10,-10): " << map[Coordinates{ 10,-10}].location << endl;

    cout << endl << endl;
    cout << "vertex1: " << map[Coordinates{-20,0}].location << endl;
    cout << "vertex2: " << map[Coordinates{-10,10}].location << endl;
    cout << "vertex3: " << map[Coordinates{10,10}].location << endl;
    cout << "vertex4: " << map[Coordinates{20,0}].location << endl;
    cout << "vertex5: " << map[Coordinates{10,-10}].location << endl;
    cout << "vertex6: " << map[Coordinates{-10,-10}].location<< endl;

    cout << endl << endl;

    Location vertex[6] = { start_vertex ,
                           Location {start_vertex.x,end_vertex.y},
                           ((start_vertex + end_vertex) *.5 ) + Location{0, end_vertex.y - start_vertex.y},
                           end_vertex,
                           Location {end_vertex.x,start_vertex.y},
                           ((start_vertex + end_vertex) *.5 ) + Location{0, start_vertex.y - end_vertex.y }};

    cout << "corner1: " << vertex[0] << endl;
    cout << "corner2: " << vertex[1] << endl;
    cout << "corner3: " << vertex[2] << endl;
    cout << "corner4: " << vertex[3] << endl;
    cout << "corner5: " << vertex[4] << endl;
    cout << "corner6: " << vertex[5] << endl;

    cout << endl << endl;

    cout << "center: " << (vertex[0]+vertex[3]) *.5 << endl;

    cout << endl << endl;


    cout << "dist: " << vertex[2].x - vertex[0].x << endl;


}

const std::string format_time(string format) {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[256];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), format.c_str(), &tstruct);
    return buf;
}

TEST_CASE("filename") {
    cout << format_time("%Y%m%d%H%M%S.log") << endl;
}