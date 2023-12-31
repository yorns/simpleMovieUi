#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <functional>
#include <regex>
#include <dirent.h>
#include <sys/types.h>
#include <nlohmann/json.hpp>
#include "database.h"

#define UNUSED(x) [&x]{}()

std::string readfile(const std::string &fileName)
{
    std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return std::string(bytes.data(), fileSize);
}

std::string readcmdl()
{
    std::string in(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
    return in;
}

std::tuple<std::string::size_type, std::string::size_type>
get_start_of(const std::string& data, const std::string& whatToFind, std::string tag,
             std::string::size_type startPos = 0,
             std::string::size_type endPos = std::string::npos)
{

    UNUSED(endPos);
    
    if (data[0] != '<') {
        std::cerr << "data must begin with a '<"<<tag<<"' \n";
        return std::make_tuple(std::string::npos,0);
    }

    std::string::size_type start = data.find(whatToFind, startPos);

    if (start == std::string::npos) {
        std::cerr << "no new start for \'"<<whatToFind<<"\' found\n";
        return std::make_tuple(std::string::npos,0);
    }

    uint32_t tagcount(1);
    std::string::size_type cnt(start+1);

//    std::cerr << data.substr(start, 200)<<"\n";

    while(tagcount) {
        std::string::size_type pos = data.find('<', cnt);
        if (pos == std::string::npos) {
            std::cerr << "no closing tag found\n";
            return std::make_tuple(std::string::npos,0);
        }
        if (data.substr(pos,tag.length()+2) == "</"+tag) {
            tagcount--;
            cnt=pos+tag.length()+2;
            continue;
        }
        if (data.substr(pos,tag.length()+1) == "<"+tag) {
            tagcount++;
            cnt=pos+tag.length()+1;
            continue;
        }
        cnt=pos+1;
    }

    std::string::size_type end = data.find('>', cnt);

    if (end == std::string::npos) {
        std::cerr << "no end tag found\n";
        return std::make_tuple(std::string::npos,0);
    }

    return std::make_tuple(start, end+1);
}


std::string getTitel(const std::string& line) {
    std::smatch match{};
    const std::regex pattern{">[0-9]+\\. ([^<]*)"};
    if (std::regex_search(line, match, pattern)) {
        return match[1].str();
    }

    return line;
}

std::string getSeriesAndNo(const std::string& line) {
    std::smatch match{};
    const std::regex pattern{"Staffel ([0-9]+), Folge ([0-9]+)"};
    if (std::regex_search(line, match, pattern)) {
        return match[1].str()+ " " + match[2].str();
    }

    return line;
}

std::string getDesc(const std::string& line) {
    std::string::size_type start, end;
    std::tie(start, end) = get_start_of(line, "<p", "p");

    if (start == std::string::npos) {
        std::cerr << "was not able to find \"<p\" in \""<<line<<"\n";
        return std::string();
    }

    std::string l = std::regex_replace(line.substr(start,end-start), std::regex("<[/]*p>"), "");

//  std::cerr << "<<<<<<<<<<<<<<<\n"<<l<<"\n<<<<<<<<<<<<<<<<<<<\n";

    if (l.find("<span class=\"text-quelle\">") != std::string::npos) {
        std::smatch match{};
        const std::regex pattern{"([^<]*).*\\((.*)\\)"};
        if (std::regex_search(l, match, pattern)) {
            return match[1].str()+ " " + match[2].str();
        }
    }
    else {
        return l;
    }

    return line.substr(start, end-start);
}

std::vector<std::tuple<std::string, uint32_t, uint32_t, std::string>> getFileList(std::string& path) {

    std::vector<std::tuple<std::string, uint32_t, uint32_t, std::string>> fileList;

    if (path.at(path.length()-1) == '/')
        path.pop_back();

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            std::string tmp { ent->d_name };
            std::smatch match{};
            const std::regex pattern{"([^_]+)_S([0-9]+)_([0-9]+)\\.(.+)$"};
            if (std::regex_search(tmp, match, pattern)) {
                std::string fullName = match[0].str();
                std::string basename = match[1].str();
                uint32_t series_no = static_cast<uint32_t>(std::stoi(match[2].str()));
                uint32_t file_no  = static_cast<uint32_t>(std::stoi(match[3].str()));
                fileList.push_back(std::make_tuple(basename, series_no, file_no, path+"/"+fullName));
            }
            else {
                std::cerr << "no match found for <"<<tmp<<">\n";
            }
        }
        closedir (dir);
    }

    return fileList;
}

enum class Identifier {
    Titel,
    SeriesAndNo,
    Desc
};

struct Entry {
    std::string titel;
    std::string desc;
    uint32_t series_no;
    uint32_t file_no;
};

void createSeriesDatabaseEntries(const std::string& file, std::vector<Entry>& entryList)
//, const std::string& baseName,
//                   const std::string& display_baseName )
{

    std::string::size_type start{0};
    std::string::size_type end{std::string::npos};
    std::string::size_type part_start, part_end;
    std::string::size_type episode_start, episode_end;


    while (1) {

        std::tie(part_start, part_end) = get_start_of(file, "<div class=\"episode-output \"", "div", start, end);

        if (part_start == std::string::npos)
            break;

        std::vector<std::tuple<std::string, std::function<std::string(const std::string&)>,Identifier>>
                infoList{
                std::make_tuple("<div class=\"episode-output-titel \"", getTitel, Identifier::Titel),
                std::make_tuple("<div class=\"episode-output-instaffel\"", getSeriesAndNo, Identifier::SeriesAndNo),
                std::make_tuple("<div class=\"episode-output-inhalt\"", getDesc, Identifier::Desc)
        };

        if (part_start != std::string::npos) {
            Entry entry;
            std::string output;
            for (auto i : infoList ) {
                std::tie(episode_start, episode_end) = get_start_of(file, std::get<0>(i), "div", part_start, part_end-part_start);

                if (episode_start != std::string::npos) {
                    output = std::get<1>(i)(file.substr(episode_start, episode_end-episode_start));
                    switch (std::get<2>(i)) {
                        case Identifier::Titel: {
                            entry.titel = output;
                            break;
                        }
                        case Identifier::SeriesAndNo: {
                            std::stringstream str;
                            str << output;
                            str >> entry.series_no >> entry.file_no;
                            break;
                        }
                        case Identifier::Desc: {
                            entry.desc = output;
                        }
                    }
                }
                start = part_end;
            }
            if (!entry.titel.empty() && ! entry.desc.empty() && entry.series_no>0 && entry.file_no >0) {
                std::cerr << "include ("<<entry.series_no<<" "<<entry.file_no<<") "<<entry.titel<<"\n";
                entryList.push_back(entry);
            }
        }
        else {
            start = std::string::npos;
        }
    }

}

int main(int argc, char* argv[])
{

    if (argc != 5 && argc != 2) {
        std::cerr << "usage: "<<argv[0] <<" [<series name>] <html-filename> [<series-path> <database>]\n";
        return -1;
    }

    bool test { argc == 2 };

    std::string seriesName{ test ? "" :argv[1]};
    std::string htmlFilename{ test ? argv[1] : argv[2]};
    std::string seriesPath{ test ? "" : argv[3]};
    std::string databaseName{ test ? "" : argv[4]};

    std::string file;
    if (htmlFilename == "-") {
        std::cerr << "loading html from standard in\n";
        file = readcmdl();
    }
    else {
        std::cerr << "loading html file <"<<htmlFilename<<">\n";
        file = readfile(htmlFilename);
    }

    std::vector<Entry> entryList;
    createSeriesDatabaseEntries(file, entryList);

    for(auto it : entryList) {
        std::cerr << " -> Series no "<<it.series_no << " file no "<< it.file_no <<" : " << it.titel << "\n";
    }

    if (test) {
        std::cerr << "End test run\n";
        return 0;
    }

    std::cerr << "load files from given path <"<<seriesPath<<">\n";
    std::vector<std::tuple<std::string, uint32_t, uint32_t, std::string>> fileList = getFileList(seriesPath);

    std::ofstream log("desc.log");
    Database database(log);

    std::ifstream ifs(databaseName);
    if (ifs.good()) {
        ifs.close();
        database.insertJson(databaseName);
    }

    std::cerr << "\n";

    for(auto fList_it : fileList) {
        std::string url{std::get<3>(fList_it)};
        std::uint32_t series_no {std::get<1>(fList_it)};
        std::uint32_t file_no {std::get<2>(fList_it)};
        std::string baseName {std::get<0>(fList_it)};

        std::cerr << "try to add <"<<baseName<<"> series no "<< series_no<< " file no "<<file_no<<"\n";

        // now lets see if we have something matching in our html analyse file
        auto entry_it = std::find_if(entryList.begin(), entryList.end(),
                                     [series_no, file_no](const Entry& entry)
                                     { return entry.series_no == series_no && entry.file_no == file_no; });

        if (entry_it == entryList.end()) {
            std::cerr << " could not find anything for <"<<baseName<<"/"<<url<<"\n";
            continue;
        }

        boost::optional<uint32_t> val;
        if (entry_it != entryList.end() && (val = database.findUrl(url))) {
            uint32_t id = *val;
            // found an entry for the file in the database
            database.replace_name(id, entry_it->titel);
            database.replace_description(id, entry_it->desc);

            database.clean_categories(id);
            database.add_categorie(id, "serien");
            database.add_categorie(id, seriesName);
            database.add_categorie(id, "Staffel "+std::to_string(series_no));

        }
        else {
            std::vector<std::tuple<std::string, std::string>> categories;
            categories.push_back(std::make_tuple("","serien"));
            categories.push_back(std::make_tuple("",seriesName));
            categories.push_back(std::make_tuple("", "Staffel "+std::to_string(series_no)));

            database.createNewEntry(entry_it->titel,entry_it->desc, url, seriesPath, "", categories, "" ,"" );

        }
    }

    database.write(databaseName);

    return 0;
}

