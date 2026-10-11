#pragma once

#include <string>
#include <algorithm>
#include <cctype>

namespace hellebore {

inline std::string trim(const std::string& input) {
    const auto first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = input.find_last_not_of(" \t\r\n");
    return input.substr(first, last - first + 1);
}

inline std::string normalizeCommand(const std::string& input) {
    std::string result = trim(input);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

enum class Command {
    Help,
    About,
    Status,
    Clear,
    Unknown
};

inline Command parseCommand(const std::string& input) {
    const std::string command = normalizeCommand(input);
    if (command == "help") return Command::Help;
    if (command == "about") return Command::About;
    if (command == "status") return Command::Status;
    if (command == "clear") return Command::Clear;
    return Command::Unknown;
}

} // namespace hellebore
