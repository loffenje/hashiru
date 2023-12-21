#include "pugixml.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using u8 = uint8_t;
using u32 = uint32_t;
using f32 = float;

using TermFreq = std::unordered_map<std::string, u32>;
using Index = std::unordered_map<std::string, TermFreq>;

#include "utils.h"
#include "httplib.h"
#include "lexer.h"

namespace fs = std::filesystem;

void parse_tree(const pugi::xml_node &node, std::string &content) {
    if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata) {
        content.append(node.value());
        content.append(" ");
    }

    for (pugi::xml_node child : node.children()) {
        parse_tree(child, content);
    }
}

bool read_file_into_string(const char *filename, std::string &content) {
    pugi::xml_document doc;
    if (!doc.load_file(filename)) {
        fprintf(stderr, "Failed to read %s\n", filename);
        return false;
    }

    for (pugi::xml_node child : doc.children()) {
        parse_tree(child, content);
    }

    return true;
}

void show_usage() {
    fprintf(stderr, "Usage: \n");
    fprintf(stderr, "- 'index' <dir>\n");
    fprintf(stderr, "- 'serve' <addr> <index>\n");
    exit(1);
}

void run_index_command(const char *dir) {
    std::string content;
    std::filesystem::path root {dir};

    Index index;

    for (const auto &dir_entry : fs::recursive_directory_iterator(root)) {
        std::string content;
        std::string path = dir_entry.path().string();

        printf("Indexing: %s\n", path.c_str());
        if (!read_file_into_string(path.c_str(), content)) {
            continue;
        }

        TermFreq term_freq;

        Lexer lexer(content);
        while (!lexer.is_end()) {
            auto token = lexer.next_token();

            auto it = term_freq.find(token);
            if (it != term_freq.end()) {
                it->second++;
            } else {
                term_freq.insert({token, 1});
            }
        }

        index.insert({path, term_freq});
    }

    export_index_to_vars("index.vars", index);
}

void serve_static_file(const char *filename, const char *content_type, httplib::Response &res) {
    std::ifstream f(filename);
    if (f.is_open()) {
        std::stringstream buf;
        buf << f.rdbuf();

        res.set_content(buf.str(), content_type);
        res.status = 200;
    } else {
        res.set_content("Can't serve a file", "text/plain");
        res.status = 401;
    }
}

f32 tf(const std::string &t, const TermFreq &d) {
    f32 result = 0.0f;
    f32 a = 0.0f;

    auto it = d.find(t);
    if (it != d.end()) a = it->second;

    f32 sum = 0;
    for (const auto &[_, val] : d)
        sum += val;

    if (sum != 0) result = a / sum;

    return result;
}

f32 idf(const std::string &t, const Index &d) {
    u32 N = d.size();
    u32 sum = 0;

    for (const auto &[_, tf_map] : d) {
        if (tf_map.contains(t)) sum++;
    }

    N = std::max(N, 1u);
    sum = std::max(sum, 1u);

    f32 result = log10(static_cast<f32>(N) / static_cast<f32>(sum));

    return result;
}

void set_router(httplib::Server &server, const Index &index) {
    server.set_error_handler([](const auto &req, auto &res) {
        auto fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, res.status);
        res.set_content(buf, "text/html");
    });

    server.Post("/api/search", [&](const httplib::Request &req, httplib::Response &res,
                                   const httplib::ContentReader &content_reader) {
        std::string body;
        content_reader([&](const char *data, size_t len) {
            body.append(data, len);
            return true;
        });

        std::vector<std::pair<std::string, f32>> data;
        for (const auto &[path, d] : index) {
            Lexer lexer(body);
            f32 rank = 0.0f;
            while (!lexer.is_end()) {
                auto token = lexer.next_token();
                f32 tf_idf = tf(token, d) * idf(token, index);

                rank += tf_idf;
            }

            data.push_back({path, rank});
        }

        std::sort(data.begin(), data.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

        data = take(data, 20);

        std::string json = to_json_buf(data);
        res.set_content(json, "application/json");
    });

    server.Get("/index.js", [](const httplib::Request &, httplib::Response &res) {
        serve_static_file("assets/index.js", "application/javascript", res);
    });

    server.Get("/", [](const httplib::Request &, httplib::Response &res) {
        serve_static_file("assets/index.html", "text/html; charset=utf-8", res);
    });

    server.Get("/index.html", [](const httplib::Request &, httplib::Response &res) {
        serve_static_file("assets/index.html", "text/html; charset=utf-8", res);
    });
}

void run_server_command(const char *addr, const char *indexfile) {
    std::string in = addr;
    auto tokens = split(in, ':');

    std::string host = "0.0.0.0";
    int port = 8888;

    if (!tokens.empty()) {
        if (is_number(tokens[1]) && !tokens[1].empty()) port = std::stoi(tokens[1]);
        else
            fprintf(stderr, "Invalid port provided, using 8888 instead\n");

        host = tokens[0];
    }

    httplib::Server server;

    Index index;
    import_vars_to_index(indexfile, index);

    set_router(server, index);

    if (!server.listen(host, port)) {
        fprintf(stderr, "Can't start server at %s:%d\n", host.c_str(), port);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        show_usage();
    }

    std::string command = argv[1];

    if (command == "index") run_index_command(argv[2]);
    else if (command == "serve") {
        if (argc < 4) show_usage();

        run_server_command(argv[2], argv[3]);
    } else if (command == "check_vars") {
        Index index;
        import_vars_to_index(argv[2], index);
        export_index_to_vars("test_index.vars", index);
    } else
        show_usage();

    return 0;
}
