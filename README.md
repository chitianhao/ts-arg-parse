ts-argparse
============

WIP Arg Parsing for ATS

traffic server executables
------------
- traffic_cache_tool: command, subcommand, option (command implemented: command.cc, command.h)
- traffic_crashlog: option
- traffic_ctl: command, subcommand, option, file arguments (command implemented: traffic_ctl.cc)
- traffic_layout: command, option (command implemented: traffic_layout.cc) 
- traffic_logcat: option, file arguments.
- traffic_logstats: option.
- traffic_manager: option.
- traffic_server: command option.
- traffic_top: option, file arguments.
- traffic_via: option, file arguments.

Compilation
------------
clang++(or g++) argparse.cpp main.cpp -o main -std=c++17
