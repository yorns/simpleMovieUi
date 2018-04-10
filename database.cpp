#include "database.h"
#include <algorithm>
#include <tuple>
#include <fstream>
#include <iostream>
#include "json/json.hpp"

void Database::insert_if_unique(std::vector<std::string>& list, const std::string& name) {
    if (std::find_if(list.begin(), list.end(), [name](const std::string& item){ return item==name; })==list.end())
        list.push_back(name);
}

std::tuple<std::vector<std::string>, std::vector<uint32_t>, bool> Database::db_select(std::vector<std::string> selector)
{
    std::vector<std::string> ret;
    std::vector<uint32_t> idList;
    bool end{false};
    for(auto& i : movie_db) {
        uint32_t cat_cnt{0};
        bool found{true};
        for (auto& cat : i.category) {
            if (cat_cnt < selector.size()) {
                if (std::get<1>(cat) != selector[cat_cnt]) {
                    found=false;
                    break;
                }
                cat_cnt++;
            }
        }
        if (found) {
            idList.push_back(i.id);
            if (selector.size() != i.category.size()) {
                insert_if_unique(ret, std::get<1>(i.category[cat_cnt]));
            }
            else {
                ret.push_back(i.name);
                end = true;
            }
        }
    }
    return std::make_tuple(ret,idList,end);
}

bool Database::readjson(const std::string& filepath) {

    static constexpr const char *m_JsonNameTag{"name"};
    static constexpr const char *m_JsonDescTag{"desc"};
    static constexpr const char *m_JsonUrlTag{"url"};
    static constexpr const char *m_JsonImageTag{"img"};
    static constexpr const char *m_JsonCategoryTag{"categories"};

    std::string fullName(filepath+"/database.json");

    std::ifstream ifs(fullName.c_str());
    if (!ifs.is_open()) {
        std::cerr << "could not open database <"<<fullName<<"\n";
        return false;
    }
    nlohmann::json j;
    ifs >> j;

    static int idCounter{0};

    int startId = idCounter;

    for (const auto &elem : j) {
        Entry entry;
        try {
            entry.name = elem.at(m_JsonNameTag);
            entry.description = elem.at(m_JsonDescTag);
            entry.url = elem.at(m_JsonUrlTag);
            entry.bg_url = elem.at(m_JsonImageTag);
            entry.basePath = filepath;
            const nlohmann::json categories = elem.at(m_JsonCategoryTag);
            for (const auto &cat : categories) {
                entry.category.push_back(std::make_tuple<std::string, std::string>("", cat));
            }
        } catch (...) {
            continue;
        }
        entry.id = idCounter++;
        movie_db.push_back(entry);
    }
    return startId != idCounter;
}
