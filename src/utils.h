#ifndef UTILS_H
#define UTILS_H

bool is_number(const std::string &s) {
    for (char ch : s) {
        if (!isdigit(ch)) return false;
    }

    return true;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, delim)) {
        result.push_back(token);
    }

    return result;
}

void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](u8 ch) { return !isspace(ch); }));
}

void abort(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

using TF_Pair = std::pair<std::string, f32>;

std::vector<TF_Pair> take(const std::vector<TF_Pair> &data, u32 size) {
    std::vector<TF_Pair> result;
    result.reserve(size);

    for (u32 i = 0; i < size; i++) {
        result.emplace_back(data[i]);
    }

    return result;
}

std::string to_json_buf(const std::vector<TF_Pair> &data) {
    std::string result;

    std::string item;
    result.append("[");
    for (const auto &[k, v] : data) {
        item.append("[\"");
        item.append(k);
        item.append("\",");
        item.append(std::to_string(v));
        item.append("],");
    }

    if (!item.empty()) item.pop_back();

    result.append(std::move(item));
    result.append("]");

    return result;
}

///////////// VARS(simple format for storing data)  ///////////////

void import_vars_to_index(const char *file, Index &index) {
    std::ifstream f(file);
    if (f.is_open()) {
        std::string line;
        std::string path;
        TermFreq term_freq;
        bool parsing_tf = false;
        while (std::getline(f, line)) {
            ltrim(line);

            // special case for closing ']'
            if (line.find("]") != std::string::npos && line.find(":") == std::string::npos) {
                index.insert({path, term_freq});
                term_freq.clear();
                parsing_tf = false;
                continue;
            }

            std::vector<std::string> tokens;

            // special case when ':' is the term
            if (line.starts_with(':')) {
                line = line.substr(1);
                tokens = split(line, ':');
                tokens[0] = ':';
            } else if (line.find(":") == std::string::npos) {
                abort(line.c_str());
            } else {
                tokens = split(line, ':');
            }

            if (tokens.size() != 2) abort(line.c_str());

            if (tokens[1] == "[") {
                if (parsing_tf) abort(line.c_str());

                path = tokens[0];
                parsing_tf = true;
                continue;
            }

            if (is_number(tokens[1]) && !tokens[1].empty()) term_freq.insert({tokens[0], std::stoi(tokens[1])});
            else
                abort(line.c_str());
        }
    }
}

void export_index_to_vars(const char *file, const Index &index) {
    std::ofstream f(file);

    for (const auto &[path, map] : index) {
        f << path << ":[\n";

        for (const auto &[token, n] : map) {
            f << " " << token << ":" << n << "\n";
        }

        f << "]\n";
    }
}

#endif
