#include<catch.h>
#include <cell_world.h>
#include <cell_world_tools.h>
#include <cell_world_vr.h>

using namespace std;
using namespace json_cpp;
using namespace cell_world;


struct Data{
    Data (const std::string &world_name):
            world(Json_create<World>(Web_resource::from("world").key(world_name).get())),
            cells(world.create_cell_group()),
            map(cells),
            graph(world.create_graph()),
            paths(world.create_paths(
                    Json_create<Path_builder>(
                            Web_resource::from("paths").key(world.name).key("astar").get()
                    )
            )){

    }
    World world;
    Cell_group cells;
    Map map;
    Graph graph;
    Paths paths;
} ;

TEST_CASE("location"){
    Data data("hexa_10_05_vr");
    cout << " dist " << data.map[Coordinates{-20,0}].location.dist(data.map[Coordinates{-18,0}].location) << endl;
}