#include <string>
#include <iostream>
#include "json/json.hpp"
#include "database.h"
#include <regex>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[])
{

    std::ofstream log("desc.log");
    Database database(log);

    if (argc != 3) {
        std::cerr << "usage "<<argv[0]<<" <series name> <database.json>\n";
    }

    std::string seriesName(argv[1]);
    std::string fullName{argv[2]};

    if (!database.insertJson(fullName)) {
        std::cerr << "could not open database <"<<fullName<<"\n";
        return -1;
    }

    const std::regex pattern1{".*_S([0-9]+)_([0-9]+)\\."};
    for(int32_t i{0}; i<database.size(); ++i) {

        std::smatch match1{};
        std::string name = database.getUrl(i);
        if (std::regex_search(name, match1, pattern1)) {
            database.clean_categories(i);
            database.add_categorie(i, "serien");
            database.add_categorie(i, seriesName);
            database.add_categorie(i, "Staffel " + match1[1].str());
        }
    }

    database.write("database_rew.json");

}
