#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "database.h"
#include <regex>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[])
{

    std::ofstream log("desc.log");
    Database database(log);

    if (argc != 3) {
        std::cerr << "usage "<<argv[0]<<" <series name> <database_path>\n";
    }

    std::string seriesName(argv[1]);
    std::string fullName{argv[2]};

    if (!database.insertJson(fullName)) {
        std::cerr << "could not open database <"<<fullName<<"\n";
        return -1;
    }

    std::cerr << "Serie: "<<seriesName<<"_([0-9]+).\n";

    const std::regex pattern1{seriesName+"_([0-9]+)\\."};
    for(int32_t i{0}; i<database.size(); ++i) {
        std::smatch match1{};
        std::string name = database.getUrl(i);
        std::cout << "testing <"<<name<<"> ";
        if (std::regex_search(name, match1, pattern1)) {
            std::cout << "match found <"<<seriesName<<">\n";
            database.add_player(i, " deinterlace");
        }
        else {
            std::cout << "no match found\n";
        }
    }

    database.write("database_rew.json");

}
