#ifndef PROJECT_DATABASE_H
#define PROJECT_DATABASE_H

#include <vector>
#include <string>
#include <cinttypes>
#include <algorithm>
#include "../json/json.hpp"
#include <boost/optional.hpp>

class Database {

    struct Entry {
        int32_t id;
        std::string name;
        std::string description;
        std::string url;
        std::string bg_url;
        std::vector<std::tuple<std::string, std::string>> category; // category type-name / category-name in sort-order
        std::string basePath;
        std::string tag;
        std::string player;
    };

    std::vector<Entry> movie_db;
    std::ofstream& m_log;

protected:

    void insert_if_unique(std::vector<std::string>& list, std::vector<bool>& end, const std::string& name);

    bool is_unique(std::vector<std::string>& list, const std::string& name);

public:
    explicit Database(std::ofstream& log) : m_log(log){}

    std::string getFullUrl(uint32_t id) const { return movie_db[id].basePath + "/" + movie_db[id].url; }
    std::string getUrl(uint32_t id) const { return movie_db[id].url; }
    std::string getDescription(uint32_t id) const { return movie_db[id].description; }
    std::string getImageUrl(uint32_t id) const { return movie_db[id].bg_url; }
    std::string getBasePath(uint32_t id) const { return movie_db[id].basePath; }
    std::string getName(uint32_t id) const { return movie_db[id].name; }
    std::string getTag(uint32_t id) const { return movie_db[id].tag; }
    std::string getplayer(uint32_t id) const { return movie_db[id].player; }

    int32_t getIDbyName(const std::string& name);
    bool replace_name(int32_t id, const std::string& name);
    bool replace_description(int32_t id, const std::string& desc);
    bool replace_basePath(int32_t id, const std::string& basePath);
    bool clean_categories(int32_t id);
    bool add_player(int32_t id, const std::string& playName);
    bool add_categorie(int32_t id, const std::string& catName);
    int32_t size() const;

    boost::optional<uint32_t> findUrl(const std::string& url) {
        auto it = std::find_if(movie_db.begin(), movie_db.end(), [url](const Entry& entry) { return entry.url == url; });
        if (it==movie_db.end())
            return boost::none;
        else
            return it->id;
    }

    int32_t createNewEntry(const std::string& name, const std::string& desc, const std::string& url, const std::string& ,
                                      const std::string& imgUrl, const std::vector<std::tuple<std::string, std::string>>& categories,
                                      const std::string& tag, const std::string& player) {

        Entry entry;

        entry.id = static_cast<int32_t>(movie_db.size());
        entry.name = name;
        entry.description = desc;
        entry.category = categories;
        entry.url = url;
        entry.tag = tag;
        entry.bg_url = imgUrl;
        entry.player = player;

        movie_db.push_back(entry);

        return entry.id;
    }
    
    std::tuple<std::vector<std::string>, std::vector<uint32_t>, std::vector<bool>> db_select(std::vector<std::string> selector);
    bool insertJson(const std::string &filepath);
    bool removePartial(const std::string &filepath);
    bool empty() { return movie_db.empty(); }

    bool write(const std::string& filename);
};

#endif //PROJECT_DATABASE_H
