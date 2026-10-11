#include <cassert>
#include <iostream>
#include "../../src/core/CommandParser.h"

int main() {
    using hellebore::Command;
    assert(hellebore::parseCommand("help") == Command::Help);
    assert(hellebore::parseCommand(" HELP ") == Command::Help);
    assert(hellebore::parseCommand("\tAbout\n") == Command::About);
    assert(hellebore::parseCommand("status") == Command::Status);
    assert(hellebore::parseCommand("clear") == Command::Clear);
    assert(hellebore::parseCommand("") == Command::Unknown);
    assert(hellebore::parseCommand("reboot") == Command::Unknown);
    assert(hellebore::parseCommand("help now") == Command::Unknown);
    std::cout << "HELLEBORE parser tests passed\n";
    return 0;
}
