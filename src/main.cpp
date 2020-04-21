#include <iostream>
#include <fstream>
#include <optional>
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <map>


struct hash_path {
    std::size_t operator()(const std::filesystem::path &path) const {
        return hash_value(path);
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    using namespace std::string_literals;
    namespace fs = std::filesystem;

    fs::path name = "./data/url_all.txt";
    fs::path data_name = "./urls.txt";
    using std::ifstream;
    ifstream file;
    file.open(name);
    if (!file.is_open()) {
        std::cout << "file " << name << " not open" << std::endl;
        return -1;
    }

    auto begin = std::chrono::steady_clock::now();

    auto len_http = ("http://"s).size();

    std::unordered_map<std::filesystem::path, std::ofstream, hash_path> files;
    //count_lines = rawgen_count(name)
    //print('lines in file:', count_lines)
    //percent, prev_percent = 0, 0
    std::string line;
    line.reserve(350);
    while (std::getline(file, line)) {
        if (line.empty()) break;
        std::string_view line_without_http(line);
        line_without_http.remove_prefix(len_http);

        auto i = line_without_http.find_last_of('/');


        auto sections = line_without_http.substr(0, i);
        auto image_url = line_without_http.substr(i+1);


        fs::path path("./urls/");
        path += sections;

        auto write_file_it = files.find(path);
        if (write_file_it == files.end()) {
            fs::create_directories(path);
            auto filename = path;
            filename += data_name;

            std::ofstream f{filename};


            write_file_it = files.insert({path, move(f)}).first;
        }

        auto& write_file = write_file_it->second;

        write_file << image_url << std::endl;


//        percent, prev_percent = i * 100 // count_lines, percent
//
//        if percent != prev_percent:
//            print(f'{percent}%')
    }


    std::cout << "closing file descriptors..." << std::endl;

    for (auto& [dir, file] : files) {
        file.close();
    }


    auto end = std::chrono::steady_clock::now();

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::seconds>(end - begin);
    std::cout << "the time: " << elapsed_ms.count() << " s" << std::endl;

    return 0;
}

