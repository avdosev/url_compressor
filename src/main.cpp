#include <iostream>
#include <fstream>
#include <optional>
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <map>
#include <array>
#include <queue>

namespace fs = std::filesystem;

struct hash_path {
    std::size_t operator()(const fs::path &path) const {
        return hash_value(path);
    }
};


size_t raw_line_counter(std::istream& istream) {
    size_t res = 0;
    constexpr size_t buff_size = 1024*64;
    std::vector<char> buff(buff_size);
    while (!istream.eof()) {
        istream.read(buff.data(), buff.size());
        auto const len_read = istream.gcount();
        for (size_t i = 0; i < len_read; i++) {
            res += buff[i] == '\n';
        }
    }
    return res;
}


template <class elapsedFnc, class ...args_t>
void check_elapsed(elapsedFnc fnc, args_t... args) {
    auto begin = std::chrono::steady_clock::now();

    fnc(args...);

    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "the time: " << elapsed_ms.count() << " ms" << std::endl;
}


class file_stream_map {
    public:
        file_stream_map() = default;
        file_stream_map(const file_stream_map&) = delete;

        std::ostream& try_get_or_insert(fs::path path, std::string_view filename) {
            auto write_file_it = files.find(path);
            if (write_file_it == files.end()) {
                fs::create_directories(path);
                auto full_file_path = path;
                full_file_path /= filename;

                std::ofstream f{full_file_path};

                write_file_it = files.insert({path, move(f)}).first;
            }

            return write_file_it->second;
        }

        ~file_stream_map() {
            for (auto& [dir, file] : files) {
                file.close();
            }
        }

    private:
        using path_to_stream_map = std::unordered_map<fs::path, std::ofstream, hash_path>;
        path_to_stream_map files;
};


void compression_url(std::string_view line, file_stream_map& files_map) {
    static const char* filename = "urls.txt";
    constexpr auto len_http = std::size("http://") - 1;

    if (line.empty()) return;

    std::string_view line_without_http(line);
    line_without_http.remove_prefix(len_http);

    auto i = line_without_http.find_last_of('/');


    auto sections = line_without_http.substr(0, i);
    auto image_url = line_without_http.substr(i+1);


    fs::path path("./urls");
    path /= sections;

    auto& write_file = files_map.try_get_or_insert(path, filename);

    write_file << image_url << std::endl;
}


void compress_file(fs::path filename) {
    using std::ifstream;
    ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "file " << filename << " not open" << std::endl;
        return;
    }

    file_stream_map files;
    std::string line;
    line.reserve(350);
    while (std::getline(file, line)) {
        compression_url(line, files);
    }
}


int main() {

    std::ios_base::sync_with_stdio(false);

    fs::path filename = "./data/url_all.txt";

    check_elapsed([&](){
        compress_file(filename);
    });

    return 0;
}

