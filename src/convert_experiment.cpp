#include <iostream>
#include <cell_world_vr.h>
#include <easy_tcp.h>
#include <filesystem>

using namespace std;
using namespace json_cpp;
using namespace cell_world;
using namespace cell_world::vr;
using namespace easy_tcp;
World_conversion wc ({2149,790}, {427,1775}, {1,.5}, {0,.5});


Experiment convert_folder(const string &path, const string &experiment_name, const string &world_name, const string &subject_name, const string &destination){
    Experiment experiment;
    experiment.duration = 0;
    experiment.name = experiment_name;
    experiment.world_name = world_name;
    experiment.subject_name = subject_name;
    cout << "Subject " << subject_name << "trajectories" << endl;
    for (const auto & entry : filesystem::directory_iterator(path)) {
        cout << "Reading " << entry << "... ";
        std::ifstream t(entry.path());
        std::stringstream buffer;
        buffer << t.rdbuf();
        string json = buffer.str();
        if (json[json.size() - 3] == ',')  json[json.size() - 3] = ' ';
        json_cpp::Json_vector<State> steps;
        try {
            json >> steps;
            cout << "success" << endl;
        } catch (...){
            cout << "fail" << endl;
            continue;
        }
        auto &episode = experiment.episodes.emplace_back();

        episode.start_time = Json_date::now();
        unsigned int i = 0;
        for (auto &a:steps) {
            Episode_data_point prey_edp;
            prey_edp.location = wc.convert(a.prey.location.to_location());
            prey_edp.coordinates = a.prey.cell.coordinates;
            prey_edp.agent_name = "human";
            prey_edp.time_stamp = a.time_stamp;
            prey_edp.frame = i;
            episode.trajectories.push_back(prey_edp);

            Episode_data_point predator_edp;
            predator_edp.location = wc.convert(a.predator.location.to_location());
            predator_edp.coordinates = a.predator.cell.coordinates;
            predator_edp.agent_name = "ghost";
            predator_edp.time_stamp = a.time_stamp;
            predator_edp.frame = i++;
            episode.trajectories.push_back(predator_edp);
        }
    }
    experiment.save( destination + "/" + experiment_name + "_" + subject_name + "_" + world_name + "_" +  ".json");
    return experiment;
}

void convert_world(string path, string experiment_name, string world_name, string destination){
    for (const auto & entry : filesystem::directory_iterator(path.c_str())) {
        if (entry.is_directory()) {
            string subject_folder = entry.path().string();
            string subject_name (subject_folder.c_str() + subject_folder.find_last_of('/') + 1);
            convert_folder(subject_folder, experiment_name, world_name, subject_name, destination);
        }
    }
}

void convert_experiment_folder(string path, string experiment_name, string destination){
    for (const auto & entry : filesystem::directory_iterator(path.c_str())) {
        if (entry.is_directory()) {
            string world_folder = entry.path().string();
            string world_name (world_folder.c_str() + world_folder.find_last_of('/') + 1);
            convert_world(world_folder, experiment_name, world_name,destination);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cout << "Wrong parameters." << endl;
        cout << "Usage: ./cellworld_convert [source_folder] [destination_folder]" << endl;
        exit(1);
    }
    std::string source_path(argv[1]);
    std::string destination_path(argv[2]);
    for (const auto & entry : filesystem::directory_iterator(source_path.c_str())) {
        if (entry.is_directory()) {
            string experiment_folder = entry.path().string();
            string experiment_name (experiment_folder.c_str() + experiment_folder.find_last_of('/') + 1);
            cout << "entry: "<< experiment_name << endl;
            convert_experiment_folder(experiment_folder, experiment_name, destination_path);
        }
    }

}