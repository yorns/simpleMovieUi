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

//#include <filesystem>
//namespace fs = std::filesystem;

std::string readfile(const std::string &fileName)
{
    std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return std::string(bytes.data(), fileSize);
}

std::tuple<std::string::size_type, std::string::size_type>
get_start_of(const std::string& data, const std::string whatToFind, std::string tag,
             std::string::size_type startPos = 0,
             std::string::size_type endPos = std::string::npos)
{
    if (data[0] != '<') {
        std::cerr << "data must begin with a '<"<<tag<<"' \n";
        return std::make_tuple(std::string::npos,0);
    }

    std::string::size_type start = data.find(whatToFind, startPos);

    if (start == std::string::npos) {
        std::cerr << "no tag found\n";
        return std::make_tuple(std::string::npos,0);
    }

    uint32_t tagcount(1);
    std::string::size_type cnt(start+1);

    while(tagcount) {
        std::string::size_type pos = data.find("<", cnt);
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

std::vector<std::string> getFileList(const std::string& path, const std::string& base) {

    std::vector<std::string> fileList;

    DIR *dir;
    struct dirent *ent;
    std::cerr << "get File list from dir "<<path<<"\n";
    if ((dir = opendir (path.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            std::string tmp { ent->d_name };
            if (tmp.substr(0,base.length()) == base) {
                fileList.push_back(tmp);
                std::cerr << " - " << tmp <<"\n";
            }
        }
        closedir (dir);
    }

    /*
        std::vector<std::string> fileList;
        for (const auto & p : fs::directory_iterator(path)) {
           std::cerr << " - " << p.string() << "\n";
           if (p.string().substring(0,base.length()) == base)
              fileList.push_back(p.string())
        }
    */
    return fileList;
}

int main(int argc, char* argv[])
{

    if (argc != 3) {
        std::cerr << "usage: "<<argv[0] <<" <filename> <database>\n";
        return -1;
    }

    nlohmann::json database;

    std::cerr << "try to find area for base file "<<argv[2]<<" in file <"<<argv[1]<<">\n";

    std::string file = readfile(argv[1]);

    std::vector<std::string> fileList = getFileList(".",argv[2]);

    std::string::size_type start(0), end(std::string::npos);
    std::string::size_type part_start, part_end;
    std::string::size_type episode_start, episode_end;

    while (start != std::string::npos) {

        std::tie(part_start, part_end) = get_start_of(file, "<div class=\"episode-output \"", "div", start, end);

        enum class Identifier {
            Titel,
            SeriesAndNo,
            Desc
        };

        std::vector<std::tuple<std::string, std::function<std::string(const std::string&)>,Identifier>>
        infoList{
            std::make_tuple("<div class=\"episode-output-titel \"", getTitel, Identifier::Titel),
            std::make_tuple("<div class=\"episode-output-instaffel\"", getSeriesAndNo, Identifier::SeriesAndNo),
            std::make_tuple("<div class=\"episode-output-inhalt\"", getDesc, Identifier::Desc)
        };

        if (part_start != std::string::npos) {
            std::string titel, desc;
            uint32_t series_no, file_no;
            std::string output;
            for (auto i : infoList ) {
                std::tie(episode_start, episode_end) = get_start_of(file, std::get<0>(i), "div", part_start, part_end-part_start);

                if (episode_start != std::string::npos) {
                    output = std::get<1>(i)(file.substr(episode_start, episode_end-episode_start));
//          std::cerr << "found:\n"<<output<<"\n";
                    switch (std::get<2>(i)) {
                    case Identifier::Titel: {
                        titel = output;
                        break;
                    }
                    case Identifier::SeriesAndNo: {
                        std::stringstream str;
                        str << output;
                        str >> series_no >> file_no;
                        break;
                    }
                    case Identifier::Desc: {
                        desc = output;
                    }
                    }
                }
                start = part_end;
            }
            std::cerr << "found "<<argv[2]<<"_S"<<series_no<<"_"<<file_no<<"  -> "<<titel<<"\n"<<desc<<"\n";
        }
        else {
            start = std::string::npos;
        }
    }


}

