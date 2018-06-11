ts-argparse
============

WIP Arg Parsing for ATS

traffic server executables
------------
- traffic_cache_tool: command, subcommand, option.
- traffic_crashlog: option.
- traffic_ctl: command, subcommand, option, file arguments.
- traffic_layout: command, option.
- traffic_logcat: option, file arguments.
- traffic_logstats: option.
- traffic_manager: option.
- traffic_server: command option.
- traffic_top: option, file arguments.
- traffic_via: option, file arguments.

Compilation
------------
- Normal build: ``clang++(or g++) argparse.cpp main.cpp -o main -std=c++17``
- On MAC: ``/usr/local/Cellar/llvm/6.0.0/bin/clang++ argparse.cpp main.cpp -o main -std=c++17``
