#include "database.h"
#include <algorithm>
#include <tuple>
#include <fstream>
#include <iostream>
#include "../json/json.hpp"

void Database::insert_if_unique(std::vector<std::string>& list, std::vector<bool>& end, const std::string& name) {
    if (std::find_if(list.begin(), list.end(), [name](const std::string& item){ return item==name; })==list.end()) {
        list.push_back(name);
        end.push_back(false);
    }
    else
        m_log << "name: "<<name<<" not unique\n"<<std::flush;
}

bool Database::is_unique(std::vector<std::string>& list, const std::string& name) {
    return std::find_if(list.begin(), list.end(), [name](const std::string& item){ return item == name; }) == list.end();
}

std::tuple<std::vector<std::string>, std::vector<uint32_t>, std::vector<bool>> Database::db_select(std::vector<std::string> selector)
{
    std::vector<std::string> ret;
    std::vector<uint32_t> idList;
    std::vector<bool> end;
    for(auto& i : movie_db) {
        uint32_t cat_cnt{0};
        bool found{false};
        if (i.category.size() >= selector.size()) {
            found = true;
            for (auto &cat : i.category) {
                if (cat_cnt < selector.size()) {
                    if (std::get<1>(cat) != selector[cat_cnt]) {
                        found = false;
                        break;
                    }
                    cat_cnt++;
                }
            }
        }
        if (found) {
            if (selector.size() != i.category.size()) {
                if (is_unique(ret, std::get<1>(i.category[cat_cnt]))) {
                    ret.push_back(std::get<1>(i.category[cat_cnt]));
                    end.push_back(false);
                    idList.push_back(i.id);
                }
            }
            else {
                ret.push_back(i.name);
                end.push_back(true);
                idList.push_back(i.id);
            }
        }
    }
    return std::make_tuple(ret,idList,end);
}

bool Database::insertJson(const std::string &filepath) {

    static constexpr const char *m_JsonNameTag{"name"};
    static constexpr const char *m_JsonDescTag{"desc"};
    static constexpr const char *m_JsonUrlTag{"url"};
    static constexpr const char *m_JsonImageTag{"img"};
    static constexpr const char *m_JsonCategoryTag{"categories"};
    static constexpr const char *m_JsonTagTag{"tag"};
    static constexpr const char *m_JsonPlayerTag{"player"};

    std::string fullName(filepath+"/database.json");

    std::ifstream ifs(fullName.c_str());
    if (!ifs.is_open()) {
        std::cerr << "could not open database <"<<fullName<<">\n";
        return false;
    }
    nlohmann::json j;
    try {
        ifs >> j;
    } catch (std::exception& exception) {
        m_log << "cannot parse database <"<<fullName<<">"<<std::endl<<std::flush;
        m_log << " -> " << exception.what()<<std::endl<<std::flush;
        return false;
    }

    int32_t startId {int32_t(movie_db.size())};

    for (const nlohmann::json &elem : j) {
        Entry entry;
        try {
            entry.name = elem.at(m_JsonNameTag);
            entry.description = elem.at(m_JsonDescTag);
            entry.url = elem.at(m_JsonUrlTag);
            entry.bg_url = elem.at(m_JsonImageTag);
            entry.basePath = filepath;
            if (elem.find(m_JsonTagTag) != elem.end())
                entry.tag = elem.at(m_JsonTagTag);
            if (elem.find(m_JsonPlayerTag) != elem.end())
                entry.player = elem.at(m_JsonPlayerTag);
            const nlohmann::json categories = elem.at(m_JsonCategoryTag);
            for (const auto &cat : categories) {
                entry.category.push_back(std::make_tuple<std::string, std::string>("", cat));
            }
        } catch (...) {
            continue;
        }

        // if no dublicate insert element
        auto duplicate = std::find_if(movie_db.begin(), movie_db.end(), [entry](const Entry e){ return e.url == entry.url && e.basePath == entry.basePath;});
        if (duplicate==movie_db.end())
            movie_db.push_back(entry);
    }
    for (auto i : movie_db) {
        m_log << "add <"<<i.name<<"> "<<i.basePath<<"\n" << std::flush;
    }

    std::sort(movie_db.begin(), movie_db.end(), [](const Entry& entry1, const Entry& entry2) { return entry1.url < entry2.url; });

    int32_t idCounter{0};
    for(auto& entry : movie_db)
        entry.id = idCounter++;

    return startId != idCounter;
}

bool Database::removePartial(const std::string &filepath) {
    movie_db.erase(
            std::remove_if(
                    movie_db.begin(),
                    movie_db.end(),
                    [&](Entry &elem) -> bool {
                        m_log << "is "<<elem.basePath<<" == "<<filepath<<"\n"<<std::flush;
                        if (elem.basePath == filepath)
                            m_log << "remove "<<elem.name<<std::endl;
                        return elem.basePath.find(filepath) != std::string::npos;
                    }), movie_db.end()
    );
    m_log << "sub: movies size: " << movie_db.size()<<"\n"<<std::flush;
    for (auto i : movie_db) {
        m_log << "sub <"<<i.name<<"> "<<i.basePath<<"\n";
    }
}

bool Database::write(const std::string &filename) {
    nlohmann::json j;
    for(const auto i : movie_db) {
        static constexpr const char *m_JsonNameTag{"name"};
        static constexpr const char *m_JsonDescTag{"desc"};
        static constexpr const char *m_JsonUrlTag{"url"};
        static constexpr const char *m_JsonImageTag{"img"};
        static constexpr const char *m_JsonCategoryTag{"categories"};
        static constexpr const char *m_JsonTagTag{"tag"};
        static constexpr const char *m_JsonPlayerTag{"player"};

        nlohmann::json elem;
        elem[m_JsonNameTag] = i.name;
        elem[m_JsonDescTag] = i.description;
        elem[m_JsonUrlTag] = i.url;
        elem[m_JsonImageTag] = i.bg_url;
        elem[m_JsonTagTag] = i.tag;
        elem[m_JsonPlayerTag] = i.player;

        nlohmann::json categories;
        std::vector<std::string> catList;
        for (const auto& j : i.category) {
            catList.push_back(std::get<1>(j));
        }
        elem[m_JsonCategoryTag] = catList;

        j.push_back(elem);
    }

    std::ofstream ofs(filename.c_str());
    if (!ofs.is_open()) {
        std::cerr << "could not open database <"<<filename<<"\n";
        return false;
    }

    ofs << j.dump(2);
}

int32_t Database::getIDbyName(const std::string &name) {
    auto it = std::find_if(movie_db.begin(), movie_db.end(), [name](const Entry& entry){ return entry.name == name; });
    if ( it != movie_db.end())
        return it->id;
    return -1;
}

bool Database::replace_name(int32_t id, const std::string &name) {
    if (id < movie_db.size()) {
        movie_db.at(id).name = name;
        return true;
    }
    return false;
}

bool Database::replace_description(int32_t id, const std::string &desc) {
    if (id < movie_db.size()) {
        movie_db.at(id).description = desc;
        return true;
    }
    return false;
}

bool Database::clean_categories(int32_t id) {
    movie_db.at(id).category.clear();
}

bool Database::add_player(int32_t id, const std::string &playName) {
    movie_db.at(id).player = playName;
}

bool Database::add_categorie(int32_t id, const std::string &catName) {
    movie_db.at(id).category.push_back(std::make_tuple("",catName));
}

int32_t Database::size() const {
    return (int32_t) movie_db.size();
}
