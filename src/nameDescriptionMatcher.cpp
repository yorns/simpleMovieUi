#include <string>
#include <iostream>
#include "../json/json.hpp"
#include "database.h"
#include <regex>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[])
{

    std::ofstream log("desc.log");
    Database database(log);

    if (argc != 4) {
        std::cerr << "usage "<<argv[0]<<" <database.json> <descriptive_file> <basefilename>\n";
    }

    std::string fullName{argv[1]};
    std::string descFile{argv[2]};
    std::string basefilename{argv[3]};

    if (!database.insertJson(fullName)) {
        std::cerr << "could not open database <"<<fullName<<"\n";
        return -1;
    }

    std::cerr << "read discriptive file\n";

    std::ifstream dfs(descFile.c_str());
    if (!dfs.is_open()) {
        std::cerr << "could not open descriptive file <"<<descFile<<"\n";
        return -1;
    }

    std::string name;
    std::string season;
    std::string episode;
    std::string description;

    for( std::string line; std::getline( dfs, line ); )
    {
        const std::regex pattern1{"(.*) \\(Staffel ([0-9]+), Folge ([0-9]+)"};
        std::smatch match1{};

        if (std::regex_search(line, match1, pattern1)) {
            name = match1[1].str();
            season = match1[2].str();
            episode = match1[3].str();
            std::cerr << "Staffel: "<<season<<" Folge: "<<episode<<" <"<<name<<"> \n";
        }

        const std::regex pattern2{"(.*) \\(Text: "};
        std::smatch match2{};

        if (std::regex_search(line, match2, pattern2)) {
            description = match2[1].str();
            std::cerr << "Description: "<<description << "\n";
        }

        if (!(name.empty() || season.empty() || episode.empty() || description.empty())) {

            // calculate name
            std::string filename = basefilename+"_S"+season+"_";
            if (episode.length() == 1)
                filename += "0";
            filename += episode;

            int32_t id = database.getIDbyName(filename);

            if (id >= 0){
                database.replace_name(id, name);
                database.replace_description(id, description);
            }

            name.clear();
            season.clear();
            episode.clear();
            description.clear();

        }

        database.write("database_rew.json");
    }

}
