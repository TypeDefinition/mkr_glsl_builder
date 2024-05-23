#pragma once

#include <cstdint>
#include <string>
#include <regex>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <stack>
#include <vector>
#include <queue>

namespace mkr {
class glsl_builder {
 private:
    std::unordered_map<std::string /* Name */, std::string /* Content */> srcs_;
    std::unordered_map<std::string, std::unordered_set<std::string>> in_edges_, out_edges_;
    std::unordered_map<std::string, size_t> in_degrees_, out_degrees_;
    std::stack<std::string> sorted_;
    std::unordered_map<std::string, bool> pragma_once_;

    static bool has_pragma_once(const std::string &_content) {
        static const std::regex spec(R"(^[\s]*#pragma[\s]+once[\s\r\n]*)", std::regex::ECMAScript);
        std::smatch match;
        return std::regex_search(_content, match, spec);
    }

    static std::string remove_pragma_once(const std::string &_content) {
        static const std::regex spec(R"(^[\s]*#pragma[\s]+once[\s\r\n]*)", std::regex::ECMAScript | std::regex ::multiline);
        return std::regex_replace(_content, spec, "");
    }

    static std::string extract_name(const std::string &_incl) {
        static const std::regex spec(R"(<[a-zA-z0-9_.]+>)", std::regex::ECMAScript);
        std::smatch match;
        std::regex_search(_incl, match, spec);
        return match.str().substr(1, match.str().length() - 2);
    }

    static std::unordered_set<std::string> find_includes(const std::string &_src) {
        static const std::regex spec(R"(^[\s]*#include[\s]+<[a-zA-z0-9_.]+>[\s\r\n]*)", std::regex::ECMAScript | std::regex::multiline);
        std::unordered_set<std::string> out;
        std::smatch match;
        std::string::const_iterator iter(_src.cbegin());
        while (iter != _src.end()) {
            std::regex_search(iter, _src.cend(), match, spec);
            if (!match.empty()) {
                std::string name = extract_name(match.str());
                out.insert(name);
            }
            iter = match.suffix().first;
        }
        return out;
    }

    void find_pragma_once() {
        pragma_once_.clear();

        for (auto &iter : srcs_) {
            const auto &name = iter.first;
            const auto &content = iter.second;
            pragma_once_[name] = has_pragma_once(content);
        }
    }

    void find_edges() {
        out_edges_.clear();
        in_edges_.clear();

        // Get out-edges.
        for (auto &iter : srcs_) {
            const auto &name = iter.first;
            const auto &content = iter.second;
            out_edges_[name] = find_includes(iter.second);

            // Check that the edges are valid.
            for (const auto &to : out_edges_[name]) {
                if (!srcs_.contains(to)) {
                    std::string err_msg = "glsl_builder - Cannot include missing source " + to + ".";
                    throw std::runtime_error(err_msg);
                }
            }
        }

        // Get in-edges.
        for (auto &iter : out_edges_) {
            const auto &from = iter.first;
            for (const auto &to : iter.second) {
                in_edges_[to].insert(from);
            }
        }
    }

    void find_degrees() {
        out_degrees_.clear();
        in_degrees_.clear();

        // Get out-degrees & in-degrees.
        for (auto &iter : srcs_) {
            const auto &name = iter.first;
            out_degrees_[name] = out_edges_[name].size();
            in_degrees_[name] = in_edges_[name].size();
        }
    }

    void toposort() {
        // Clear sorted stack.
        sorted_ = std::stack<std::string>{};

        std::queue<std::string> topo_queue;
        for (const auto &iter : in_degrees_) {
            if (iter.second == 0) {
                topo_queue.push(iter.first);
            }
        }

        if (topo_queue.size() != 1) {
            throw std::runtime_error("glsl_builder - There must be exactly 1 file which is not included by any other file.");
        }

        while (!topo_queue.empty()) {
            const auto &from = topo_queue.front();
            topo_queue.pop();
            sorted_.push(from);
            for (const auto &to : out_edges_[from]) {
                if (--in_degrees_[to] == 0) {
                    topo_queue.push(to);
                }
            }
        }

        for (auto &iter : in_degrees_) {
            if (iter.second != 0) {
                throw std::runtime_error("glsl_builder - Cyclic dependency detected.");
            }
        }
    }

 public:
    glsl_builder() = default;

    ~glsl_builder() = default;

    void add(const std::string &_name, const std::string &_source) {
        srcs_.insert({_name, _source});
    }

    std::string get(const std::string &_name) const {
        auto iter = srcs_.find(_name);
        return iter == srcs_.end() ? "" : iter->second;
    }

    std::string build() {
        find_pragma_once();
        find_edges();
        find_degrees();
        toposort();

        // Replace includes.
        std::unordered_map<std::string, bool> visited;
        std::unordered_map<std::string, std::string> modified;
        std::string content;
        while (!sorted_.empty()) {
            const std::string &name = sorted_.top(); // Name of the source we are modifying.
            sorted_.pop();
            content = srcs_[name];
            const auto &included = out_edges_[name];

            // Replace #includes with content.
            for (const std::string &incl : included) {
                // If the content has #pragma once, only include it once.
                if (pragma_once_[incl] && visited[incl]) {
                    continue;
                }
                visited[incl] = true;
                std::regex spec(R"(^[\s]*#include[\s]+<)" + incl + R"(>)", std::regex::ECMAScript | std::regex::multiline);
                content = std::regex_replace(content, spec, modified[incl], std::regex_constants::format_first_only);
            }

            // Delete the rest of the #include. (Note the [\s\r\n] at the end of the regex.)
            for (const std::string &incl : included) {
                std::regex spec(R"(^[\s]*#include[\s]+<)" + incl + R"(>[\s\r\n]*)", std::regex::ECMAScript | std::regex::multiline);
                content = std::regex_replace(content, spec, "");
            }

            // Update modified content.
            modified[name] = content;
        }

        // Remove the #pragma once.
        content = remove_pragma_once(content);

        // Return the last modified content. This should contain all the sources combined.
        return content;
    }
};
}