//
// Created by joern on 03.04.18.
//

#ifndef PROJECT_DATABASE_H
#define PROJECT_DATABASE_H

#include <vector>
#include <string>
#include <cinttypes>

class Database {

    struct Entry {
        int id;
        std::string name;
        std::string description;
        std::string url;
        std::string bg_url;
        std::vector<std::tuple<std::string, std::string>> category; // category type-name / category-name in sort-order
        std::string basePath;
    };

    std::vector<Entry> movie_db;

protected:

    void insert_if_unique(std::vector<std::string>& list, const std::string& name);

public:

    std::string getFullUrl(uint32_t id) const { return movie_db[id].basePath + movie_db[id].url; }
    std::string getUrl(uint32_t id) const { return movie_db[id].url; }
    std::string getDescription(uint32_t id) const { return movie_db[id].description; }
    std::string getImageUrl(uint32_t id) const { return movie_db[id].bg_url; }
    std::string getBasePath(uint32_t id) const { return movie_db[id].basePath; }
    std::string getName(uint32_t id) const { return movie_db[id].name; }

    std::tuple<std::vector<std::string>, std::vector<uint32_t>, bool> db_select(std::vector<std::string> selector);
    bool readjson(const std::string& filepath);

};


#endif //PROJECT_DATABASE_H
