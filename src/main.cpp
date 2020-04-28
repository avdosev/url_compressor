#include <iostream>
#include <fstream>
#include <optional>
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <map>
#include <array>
#include <queue>
#include <cstddef>

namespace fs = std::filesystem;


class line_reader {
    public:
        line_reader(std::istream& stream, size_t buff_size) : buff(buff_size), istream(stream) {
            current_index = buff.size();
        }

        void readline(std::string& res) {
            if (current_index >= buff.size()) {
                istream.read(buff.data(), buff.size());
                current_index = 0;
            }

            auto const len_read = istream.gcount();
            size_t i;
            for (i = current_index; i < len_read; i++) {
                if (buff[i] == '\n') break;
            }

            auto end_string = buff.cbegin() + i;
            std::copy(buff.cbegin()+current_index, end_string, std::back_inserter(res));

            current_index = i+1;

            if (buff[i] != '\n') {
                this->readline(res);
            }
        }

        bool eof() const {
            return istream.eof() && current_index >= istream.gcount();
        }
    private:
        ptrdiff_t current_index;
        std::vector<char> buff;
        std::istream& istream;
};


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
        struct hash_path {
            std::size_t operator()(const fs::path &path) const {
                return hash_value(path);
            }
        };
        using path_to_stream_map = std::unordered_map<fs::path, std::ofstream, hash_path>;
        path_to_stream_map files;
};


void compression_url(std::string_view line, file_stream_map& files_map) {
    const char* filename = "urls.txt";
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
    line_reader file_reader(file, 1024*64);
    std::string line;
    line.reserve(300);
    while (!file_reader.eof()) {
        file_reader.readline(line);
        compression_url(line, files);
        line.clear();
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

