// Copyright(c) 2024-present, Lim Ngian Xin Terry & mkr_glsl_include contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// glsl_include main header file.
// See README.md for usage example.

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
class glsl_include {
 private:
    std::unordered_map<std::string /* Name */, std::string /* Content */> srcs_;

    static std::string erase_header_guard(const std::string &_content) {
        static const std::regex spec(R"(^[\s]*#pragma[\s]+once[\s\r\n]*)", std::regex::ECMAScript | std::regex::multiline);
        return std::regex_replace(_content, spec, "");
    }

    static std::string extract_name(const std::string &_incl) {
        static const std::regex spec(R"(<[a-zA-z0-9_.]+>)", std::regex::ECMAScript);
        std::smatch match;
        std::regex_search(_incl, match, spec);
        return match.str().substr(1, match.str().length() - 2);
    }

    static std::unordered_set<std::string> extract_includes(const std::string &_src) {
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

    static std::unordered_map<std::string, bool> is_single_include(const std::unordered_map<std::string, std::string> &_srcs) {
        static const std::regex spec(R"(^[\s]*#pragma[\s]+once[\s\r\n]*)", std::regex::ECMAScript | std::regex::multiline);
        std::unordered_map<std::string, bool> single_include;
        for (auto &iter : _srcs) {
            const auto &name = iter.first;
            const auto &content = iter.second;
            std::smatch match;
            single_include[name] = std::regex_search(content, match, spec);
        }
        return single_include;
    }

    static std::unordered_map<std::string, std::unordered_set<std::string>> get_out_edges(const std::unordered_map<std::string, std::string> &_srcs) {
        std::unordered_map<std::string, std::unordered_set<std::string>> out_edges;
        for (auto &iter : _srcs) {
            const auto &name = iter.first;
            const auto &content = iter.second;
            out_edges[name] = extract_includes(iter.second);

            // Check that the edges are valid.
            for (const auto &to : out_edges[name]) {
                if (_srcs.contains(to)) { continue; }
                std::string err_msg = "glsl_include - Cannot include missing source " + to + ".";
                throw std::runtime_error(err_msg);
            }
        }
        return out_edges;
    }

    static std::unordered_map<std::string, std::unordered_set<std::string>> get_in_edges(std::unordered_map<std::string, std::unordered_set<std::string>> _out_edges) {
        std::unordered_map<std::string, std::unordered_set<std::string>> in_edges;
        for (auto &iter : _out_edges) {
            const auto &from = iter.first;
            for (const auto &to : iter.second) {
                in_edges[to].insert(from);
            }
        }
        return in_edges;
    }

    static std::unordered_map<std::string, size_t> get_degrees(const std::unordered_map<std::string, std::string> &_srcs,
                                                               const std::unordered_map<std::string, std::unordered_set<std::string>> &_edges) {
        std::unordered_map<std::string, size_t> degrees;
        for (auto &iter : _srcs) {
            const auto &name = iter.first;
            degrees[name] = _edges.contains(name) ? _edges.find(name)->second.size() : 0;
        }
        return degrees;
    }

    // Using toposort, we can ensure that there are no cyclic dependencies, and get the correct order to combine the sources.
    std::stack<std::string> toposort(std::unordered_map<std::string, std::unordered_set<std::string>> _out_edges,
                                     std::unordered_map<std::string, size_t> _in_degs) {
        std::stack<std::string> sorted;
        std::queue<std::string> topoqueue;
        for (const auto &iter : _in_degs) {
            if (iter.second == 0) {
                topoqueue.push(iter.first);
            }
        }

        if (topoqueue.size() != 1) {
            throw std::runtime_error("glsl_include - There must be exactly 1 file which is not included by any other file.");
        }

        while (!topoqueue.empty()) {
            const auto &from = topoqueue.front();
            topoqueue.pop();
            sorted.push(from);
            for (const auto &to : _out_edges[from]) {
                if (--_in_degs[to] == 0) {
                    topoqueue.push(to);
                }
            }
        }

        for (auto &iter : _in_degs) {
            if (iter.second != 0) {
                throw std::runtime_error("glsl_include - Cyclic dependency detected.");
            }
        }

        return sorted;
    }

 public:
    glsl_include() = default;

    ~glsl_include() = default;

    /**
     * Add a source. The content of the source will be used to replace wherever the #include directive is used.
     * For example, if the source name is `abc.frag`, use `#include <abc.frag>` in another source to include this.
     * @param _name The name of the source.
     * @param _content The content of the source. This is the actual contents of your shader.
     */
    void add(const std::string &_name, const std::string &_content) {
        srcs_.insert({_name, _content});
    }

    /**
     * Remove a source.
     * @param _name The name of the source.
     */
    void remove(const std::string &_name) {
        auto iter = srcs_.find(_name);
        if (iter != srcs_.end()) {
            srcs_.erase(iter);
        }
    }

    /**
     * Merge all added sources into a one.
     * @return The merger of all the sources added.
     */
    std::string merge() {
        auto single_include = is_single_include(srcs_);
        auto out_edges = get_out_edges(srcs_);
        auto in_edges = get_in_edges(out_edges);
        auto out_degrees = get_degrees(srcs_, out_edges);
        auto in_degrees = get_degrees(srcs_, in_edges);
        auto sorted_ = toposort(out_edges, in_degrees);

        // Replace includes.
        std::unordered_map<std::string, bool> visited;
        std::unordered_map<std::string, std::string> modified;
        std::string content;
        while (!sorted_.empty()) {
            const std::string &name = sorted_.top(); // Name of the source we are modifying.
            sorted_.pop();
            content = srcs_[name];
            const auto &included = out_edges[name];

            // Replace #includes with content.
            for (const std::string &incl : included) {
                // If the content has #pragma once, only include it once.
                if (single_include[incl] && visited[incl]) {
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

        // Remove the #pragma once macro.
        content = erase_header_guard(content);

        // Return the last modified content. This should contain all the sources combined.
        return content;
    }
};
}