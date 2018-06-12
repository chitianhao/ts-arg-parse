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
Option::Option() {}

Option::~Option() {}

Option::Option(std::string const& opt_name, std::string const& opt_key,
               std::string const& opt_description)
    : _opt_name(opt_name),
      _opt_key(opt_key),
      _opt_description(opt_description) {}

Option::Option(std::string const& opt_name, std::string const& opt_key,
               std::string const& opt_description,
               std::string const& opt_envvar, std::string const& opt_arg_type,
               int opt_arg_num)
    : _opt_name(opt_name),
      _opt_key(opt_key),
      _opt_description(opt_description),
      _opt_envvar(getenv(opt_envvar.c_str()) ? getenv(opt_envvar.c_str()) : ""),
      _opt_arg_type(opt_arg_type),
      _opt_arg_num(opt_arg_num) {}

// constructor and destructor
ArgParser::ArgParser() {}

ArgParser::~ArgParser() {}

ArgParser::ArgParser(std::string const& name, std::string const& description)
    : _name(name), _description(description) {
  // for safety purpose
  _subcommand_list.reserve(RESERVE_LIST_SIZE);
  _option_list.reserve(RESERVE_LIST_SIZE);
}

ArgParser::ArgParser(std::string const& name, std::string const& description,
                     std::string const& envvar, std::string const& arg_type,
                     int arg_num)
    : _name(name),
      _description(description),
      _envvar(getenv(envvar.c_str()) ? getenv(envvar.c_str()) : ""),
      _arg_type(arg_type),
      _arg_num(arg_num) {
  // for safety purpose
  _subcommand_list.reserve(RESERVE_LIST_SIZE);
  _option_list.reserve(RESERVE_LIST_SIZE);
}

ArgParser::ArgParser(std::string const& name, std::string const& description,
                     Function const& f)
    : _name(name), _description(description) {
  _subcommand_list.reserve(RESERVE_LIST_SIZE);
  _option_list.reserve(RESERVE_LIST_SIZE);
  _func = f;
}

ArgParser::ArgParser(std::string const& name, std::string const& description,
                     std::string const& envvar, std::string const& arg_type,
                     int arg_num, Function const& f)
    : _name(name),
      _description(description),
      _envvar(getenv(envvar.c_str()) ? getenv(envvar.c_str()) : ""),
      _arg_type(arg_type),
      _arg_num(arg_num) {
  _subcommand_list.reserve(RESERVE_LIST_SIZE);
  _option_list.reserve(RESERVE_LIST_SIZE);
  _func = f;
}

// check if this is a valid option before adding
void ArgParser::check_option(std::string const& name, std::string const& key) {
  if (name.size() < 3 || name[0] != '-' || name[1] != '-') {
    // invalid name
    std::cout << "invalid long option added: " + name << std::endl;
    exit(1);
  }
  if (key.size() > 0 && key[0] != '-') {
    // invalid key
    std::cout << "invalid short option added: " + key << std::endl;
    exit(1);
  }
  // find if existing in option list
  for (int i = 0; i < _option_list.size(); i++) {
    if (_option_list[i]._opt_name == name) {
      // name exist
      std::cout << "long option already exists: " + name << std::endl;
      exit(1);
    }
    if (_option_list[i]._opt_key == key) {
      // key exist
      std::cout << "short option already exists: " + key << std::endl;
      exit(1);
    }
  }
}

// check if this is a valid command before adding
void ArgParser::check_command(std::string const& name) {
  if (name.empty()) {
    // invalid name
    std::cout << "cannot add empty command" << std::endl;
    exit(1);
  }
  // find if existing in option list
  for (int i = 0; i < _subcommand_list.size(); i++) {
    if (_subcommand_list[i]._name == name) {
      // name exist
      std::cout << "command already exists: " + name << std::endl;
      exit(1);
    }
  }
}

// option handling
Option& ArgParser::add_option(std::string const& name, std::string const& key,
                              std::string const& description,
                              std::string const& envvar,
                              std::string const& arg_type, int arg_num) {
  check_option(name, key);

  _option_list.push_back(Option(name, key == "-" ? "" : key, description,
                                envvar, arg_type, arg_num));
  return _option_list.back();
}

Option& ArgParser::add_option(std::string const& name, std::string const& key,
                              std::string const& description) {
  check_option(name, key);
  _option_list.push_back(Option(name, key == "-" ? "" : key, description));
  return _option_list.back();
}

ArgParser& ArgParser::add_subcommand(std::string const& cmd_name,
                                     std::string const& cmd_description,
                                     std::string const& cmd_envvar,
                                     std::string const& cmd_arg_type,
                                     int cmd_arg_num) {
  check_command(cmd_name);
  ArgParser parser = ArgParser(cmd_name, cmd_description, cmd_envvar,
                               cmd_arg_type, cmd_arg_num);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser& ArgParser::add_subcommand(std::string const& cmd_name,
                                     std::string const& cmd_description) {
  check_command(cmd_name);
  ArgParser parser = ArgParser(cmd_name, cmd_description, "", "", 0);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser& ArgParser::add_subcommand(std::string const& cmd_name,
                                     std::string const& cmd_description,
                                     std::string const& cmd_envvar,
                                     std::string const& cmd_arg_type,
                                     int cmd_arg_num, Function const& f) {
  check_command(cmd_name);
  ArgParser parser = ArgParser(cmd_name, cmd_description, cmd_envvar,
                               cmd_arg_type, cmd_arg_num, f);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser& ArgParser::add_subcommand(std::string const& cmd_name,
                                     std::string const& cmd_description,
                                     Function const& f) {
  check_command(cmd_name);
  ArgParser parser = ArgParser(cmd_name, cmd_description, "", "", 0, f);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser& ArgParser::get_subcommand(std::string const& cmd_name) {
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
void ArgParser::append_cmd_data(std::vector<ArgParser>& list,
                                std::vector<std::string>& args) {
  int conflict_flag = 0;
  for (int i = 0; i < list.size(); i++) {
    for (int j = 0; j < args.size(); j++) {
      if (list[i]._name == args[j]) {
        args.erase(args.begin() + j);
        if (conflict_flag == 1) {
          // multiple commands found
          help(_argv);
        }
        conflict_flag = 1;
        // handle args first
        for (int k = 0; k < list[i]._arg_num; k++) {
          if (args.size() < j + k + 1 || args[j + k].empty() ||
              args[j + k][0] == '-') {
            help(_argv);
          }
          std::string arg = args[j + k];
          // TODO transfer arg to the stuff we want
          list[i]._data = arg;
        }
        args.erase(args.begin() + j, args.begin() + j + list[i]._arg_num);
        // handle options
        append_option_data(list[i]._option_list, args);
      }
    }
  }
}

// for opt_cmd_flag: 0 is option, 1 is cmd
void ArgParser::append_option_data(std::vector<Option>& list,
                                   std::vector<std::string>& args) {
  for (int i = 0; i < list.size(); i++) {
    // each element in the option list
    for (int j = 0; j < args.size(); j++) {
      // output version message
      if (args[j] == "--version" || args[j] == "-V") {
        version_message();
      }
      // output usage message
      if (args[j] == "--help" || args[j] == "-h") {
        help(_argv);
      }
      // find match from the args
      if (list[i]._opt_name == args[j] || list[i]._opt_key == args[j]) {
        args.erase(args.begin() + j);
        for (int k = 0; k < list[i]._opt_arg_num; k++) {
          if (args.size() < j + k + 1 || args[j + k].empty() ||
              args[j + k][0] == '-') {
            help(_argv);
          }
          std::string arg = args[j + k];
          // TODO transfer arg to the stuff we want
          list[i]._opt_data = arg;
        }
        args.erase(args.begin() + j, args.begin() + j + list[i]._opt_arg_num);
      }
    }
  }
}

/**
 * Flow:
 * 1. iterate global option list to find match & put the variable in (include
 * help & version)
 * 2. iterate global command list to find match, if multiple found: help
 *    BFS iterate through all subcommands
 *
 * Note:
 * option must follow command
 * arg must follow option
 * stuff stored in _data
 */
void ArgParser::parse(int argc, const char** argv) {
  // put argv into vector<string>
  for (int i = 0; i < argc; i++) {
    _argv.push_back(argv[i]);
  }

  // args variable for the tweaking of each commands and options
  std::vector<std::string> args = _argv;
  args.erase(args.begin());

  // 1. expected arguments from the top level
  for (int i = 0; i < _arg_num; i++) {
    if (args.size() < i + 1 || args[i][0] == '-') {
      help(_argv);
    }
    std::string arg = args[i];
    // TODO transfer arg to the stuff we want
    _data = arg;
    args.erase(args.begin() + i);
  }

  // 2. iterate over global option list
  append_option_data(_option_list, args);

  // 3. bfs over all subcommand lists
  std::queue<std::vector<ArgParser>> command_queue;
  command_queue.push(_subcommand_list);
  while (!command_queue.empty()) {
    // get the first element to deal with (dequeue)
    std::vector<ArgParser> cur_list = command_queue.front();
    command_queue.pop();
    // visit
    if (!cur_list.empty()) {
      append_cmd_data(cur_list, args);
    }
    // enqueue
    for (int i = 0; i < cur_list.size(); i++) {
      command_queue.push(cur_list[i]._subcommand_list);
    }
  }

  if (!args.empty()) {
    std::cout << "Error in parsing" << std::endl;
    help();
  }
}

int ArgParser::invoke(int argc, const char** argv) { return _func(argc, argv); }

// for temporary testing
void ArgParser::help(std::vector<std::string> const& args) {
  std::cout << "help!" << std::endl;
  // TODO: find the level from the args
  exit(0);
}
void ArgParser::version_message() {
  std::cout << "version" << std::endl;
  exit(0);
}

}  // namespace ts
