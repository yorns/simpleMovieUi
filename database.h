//
// Created by joern on 03.04.18.
//

#ifndef PROJECT_DATABASE_H
#define PROJECT_DATABASE_H

#include <vector>
#include <string>
#include <cinttypes>
#include <algorithm>
#include "json/json.hpp"

class Database {

    struct Entry {
        int32_t id;
        std::string name;
        std::string description;
        std::string url;
        std::string bg_url;
        std::vector<std::tuple<std::string, std::string>> category; // category type-name / category-name in sort-order
        std::string basePath;
    };

    std::vector<Entry> movie_db;
    std::ofstream& m_log;

protected:

    void insert_if_unique(std::vector<std::string>& list, const std::string& name);

public:
    Database(std::ofstream& log) : m_log(log){}

    std::string getFullUrl(uint32_t id) const { return movie_db[id].basePath + "/" + movie_db[id].url; }
    std::string getUrl(uint32_t id) const { return movie_db[id].url; }
    std::string getDescription(uint32_t id) const { return movie_db[id].description; }
    std::string getImageUrl(uint32_t id) const { return movie_db[id].bg_url; }
    std::string getBasePath(uint32_t id) const { return movie_db[id].basePath; }
    std::string getName(uint32_t id) const { return movie_db[id].name; }

    int32_t getIDbyName(const std::string& name) {
        auto it = std::find_if(movie_db.begin(), movie_db.end(), [name](const Entry& entry){ return entry.name == name; });
        if ( it != movie_db.end())
            return it->id;
        return -1;
    }

    bool replace_name(int32_t id, const std::string& name) {
        if (id < movie_db.size()) {
            movie_db.at(id).name = name;
            return true;
        }
        return false;
    }

    bool replace_description(int32_t id, const std::string& desc) {
        if (id < movie_db.size()) {
            movie_db.at(id).description = desc;
            return true;
        }
        return false;
    }

    bool clean_categories(int32_t id) {
        movie_db.at(id).category.clear();
    }

    bool add_categorie(int32_t id, const std::string& catName) {
        movie_db.at(id).category.push_back(std::make_tuple("",catName));
    }

    int32_t size() const {
        return (int32_t) movie_db.size();
    }

    std::tuple<std::vector<std::string>, std::vector<uint32_t>, bool> db_select(std::vector<std::string> selector);
    bool insertJson(const std::string &filepath);
    bool removePartial(const std::string &filepath);
    bool empty() { return movie_db.empty(); }

    bool write(const std::string& filename);
};


#endif //PROJECT_DATABASE_H
