#include <string>
#include <iostream>
#include "json/json.hpp"
#include <regex>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[])
{

    if (argc != 3) {
        std::cerr << "usage "<<argv[0]<<" <database.json> <descriptive_file>\n";
    }

    static constexpr const char *m_JsonNameTag{"name"};
    static constexpr const char *m_JsonDescTag{"desc"};
    static constexpr const char *m_JsonUrlTag{"url"};
    static constexpr const char *m_JsonImageTag{"img"};
    static constexpr const char *m_JsonCategoryTag{"categories"};

    std::string fullName{argv[1]};
    std::string descFile{argv[2]};

    std::ifstream ifs(fullName.c_str());
    if (!ifs.is_open()) {
        std::cerr << "could not open database <"<<fullName<<"\n";
        return -1;
    }

    std::ifstream dfs(descFile.c_str());
    if (!dfs.is_open()) {
        std::cerr << "could not open descriptive file <"<<descFile<<"\n";
        return -1;
    }

    nlohmann::json j;
    ifs >> j;

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

    }

}
