/** @file

  arg parsing for ATS

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include "argparse.hpp"
#include <iostream>
#include <queue>
#include <sstream>

namespace ts {

// constructor and destructor
ts::Option::Option() {}

ts::Option::~Option() {}

ts::Option::Option(std::string const &opt_name, std::string const &opt_key,
                   std::string const &opt_description)
    : _opt_name(opt_name),
      _opt_key(opt_key),
      _opt_description(opt_description) {}

ts::Option::Option(std::string const &opt_name, std::string const &opt_key,
                   std::string const &opt_description,
                   std::string const &opt_envvar,
                   std::string const &opt_arg_type, int opt_arg_num)
    : _opt_name(opt_name),
      _opt_key(opt_key),
      _opt_description(opt_description),
      _opt_envvar(getenv(opt_envvar.c_str()) ? getenv(opt_envvar.c_str()) : ""),
      _opt_arg_type(opt_arg_type),
      _opt_arg_num(opt_arg_num) {}

// constructor and destructor
ArgParser::ArgParser() {}

ArgParser::~ArgParser() {}

ArgParser::ArgParser(std::string const &name, std::string const &description)
    : _name(name), _description(description) {}

ArgParser::ArgParser(std::string const &name, std::string const &description,
                     std::string const &envvar, std::string const &arg_type,
                     int arg_num)
    : _name(name),
      _description(description),
      _envvar(getenv(envvar.c_str()) ? getenv(envvar.c_str()) : ""),
      _arg_type(arg_type),
      _arg_num(arg_num) {}

ArgParser::ArgParser(std::string const &name, std::string const &description,
                     Function const &f)
    : _name(name), _description(description) {
  _func = f;
}

ArgParser::ArgParser(std::string const &name, std::string const &description,
                     std::string const &envvar, std::string const &arg_type,
                     int arg_num, Function const &f)
    : _name(name),
      _description(description),
      _envvar(getenv(envvar.c_str()) ? getenv(envvar.c_str()) : ""),
      _arg_type(arg_type),
      _arg_num(arg_num) {
  _func = f;
}

// option handling
Option &ArgParser::add_option(std::string const &name, std::string const &key,
                              std::string const &description,
                              std::string const &envvar,
                              std::string const &arg_type, int arg_num) {
  _option_list.push_back(
      Option(name, key, description, envvar, arg_type, arg_num));
  return _option_list.back();
}

Option &ArgParser::add_option(std::string const &name, std::string const &key,
                              std::string const &description) {
  _option_list.push_back(Option(name, key, description));
  return _option_list.back();
}

ArgParser &ArgParser::add_subcommand(std::string const &cmd_name,
                                     std::string const &cmd_description,
                                     std::string const &cmd_envvar,
                                     std::string const &cmd_arg_type,
                                     int cmd_arg_num) {
  ArgParser parser = ArgParser(cmd_name, cmd_description, cmd_envvar,
                               cmd_arg_type, cmd_arg_num);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser &ArgParser::add_subcommand(std::string const &cmd_name,
                                     std::string const &cmd_description) {
  ArgParser parser = ArgParser(cmd_name, cmd_description, "", "", 0);
  parser._parent = this;
  // segfault here
  _subcommand_list.push_back(parser);
  std::cout << "d" << std::endl;
  return _subcommand_list.back();
}

ArgParser &ArgParser::get_subcommand(std::string const &cmd_name) {
  for (int i = 0; i < _subcommand_list.size(); i++) {
    if (_subcommand_list[i]._name == cmd_name) {
      return _subcommand_list[i];
    }
  }
  std::ostringstream s;
  s << "Command " << cmd_name << " not found";
  exit(1);
}

// -------------------- start of the real world ----------------------------
void ArgParser::append_cmd_data(std::vector<ArgParser> &list,
                                const std::vector<std::string> &args) {
  int conflict_flag = 0;
  for (int i = 0; i < list.size(); i++) {
    for (int j = 0; j < args.size(); j++) {
      if (list[i]._name == args[j]) {
        if (conflict_flag == 1) {
          // multiple commands found
          help();
        }
        conflict_flag = 1;
        // handle args first
        for (int k = 0; k < _arg_num; k++) {
          if (args[j + k + 1][0] == '-') {
            help();
          }
          std::string arg = args[j + k + 1];
          // TODO transfer arg to the stuff we want
          list[i]._data = arg;
        }
        // handle options
        append_option_data(
            list[i]._option_list,
            std::vector<std::string>(args.begin() + j, args.end()));
      }
    }
  }
}

// for opt_cmd_flag: 0 is option, 1 is cmd
void ArgParser::append_option_data(std::vector<Option> &list,
                                   const std::vector<std::string> &args) {
  for (int i = 0; i < list.size(); i++) {
    for (int j = 0; j < args.size(); j++) {
      if (list[i]._opt_name == args[j] || list[i]._opt_key == args[j]) {
        for (int k = 0; k < _arg_num; k++) {
          if (args[j + k + 1][0] == '-') {
            help();
          }
          // TODO or =
          std::string arg = args[j + k + 1];
          // TODO transfer arg to the stuff we want
          list[i]._opt_data = arg;
        }
      }
    }
  }
}

/**
 * Flow:
 * 1. iterate global option list to find match & put the variable in (include
 * help & version)
 * 2. iterate global command list to find match, if multiple found: help
 *     if found iterate the option list of command
 *         iterate subcommand & option
 *
 * Note:
 * option must follow command
 * arg must follow option
 * stuff stored in _data
 */
void ArgParser::parse(int argc, const char **argv) {
  // put argv into vector<string>
  std::vector<std::string> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(argv[i]);
  }
  // output some version message as needed
  for (int i = 0; i < args.size(); i++) {
    if (args[i] == "--version" || args[i] == "-V") {
      version_message();
    }
  }

  // 1. expected arguments from the top level
  for (int i = 0; i < _arg_num; i++) {
    if (args[i + 1][0] == '-') {
      help();
    }
    std::string arg = args[i];
    // TODO transfer arg to the stuff we want
    _data = arg;
  }

  // 2. iterate over global option list
  append_option_data(_option_list, args);

  // 3. bfs over all subcommand lists
  std::queue<std::vector<ArgParser>> command_queue;
  std::vector<ArgParser> cur_list = _subcommand_list;

  while (!command_queue.empty() && !cur_list.empty()) {
    if (!cur_list.empty()) {
      append_cmd_data(cur_list, args);
    }
    // enqueue
    for (int i = 0; i < cur_list.size(); i++) {
      command_queue.push(cur_list[i]._subcommand_list);
    }
    // dequeue
    cur_list = command_queue.front();
    command_queue.pop();
  }
}

int ArgParser::invoke(int argc, const char **argv) { return _func(argc, argv); }

// for temporary testing
void ArgParser::help() {
  std::cout << "help!" << std::endl;
  exit(0);
}
void ArgParser::version_message() {
  std::cout << "version" << std::endl;
  exit(0);
}

}  // namespace ts
