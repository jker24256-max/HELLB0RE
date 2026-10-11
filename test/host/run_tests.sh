#!/usr/bin/env bash
set -euo pipefail
c++ -std=c++17 -Wall -Wextra -Werror -pedantic test/host/command_parser_test.cpp -o /tmp/hellebore-parser-test
/tmp/hellebore-parser-test
