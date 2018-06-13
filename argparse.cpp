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

// ---------------------- implementation of Option ------------------------
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
               std::string const& opt_envvar, int opt_arg_num)
    : _opt_name(opt_name),
      _opt_key(opt_key),
      _opt_description(opt_description),
      _opt_envvar(opt_envvar),
      _opt_arg_num(opt_arg_num) {}

std::string Option::get_envvar() {
  return getenv(_opt_envvar.c_str()) ? getenv(_opt_envvar.c_str()) : "";
}

std::vector<std::string> Option::get_data() { return _opt_data; }

// ---------------------- implementation of ArgParser ------------------------
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
                     std::string const& envvar, int arg_num)
    : _name(name),
      _description(description),
      _envvar(envvar),
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
                     std::string const& envvar, int arg_num, Function const& f)
    : _name(name),
      _description(description),
      _envvar(envvar),
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
                              std::string const& envvar, int arg_num) {
  check_option(name, key);
  _option_list.push_back(
      Option(name, key == "-" ? "" : key, description, envvar, arg_num));
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
                                     int cmd_arg_num) {
  check_command(cmd_name);
  ArgParser parser =
      ArgParser(cmd_name, cmd_description, cmd_envvar, cmd_arg_num);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser& ArgParser::add_subcommand(std::string const& cmd_name,
                                     std::string const& cmd_description) {
  check_command(cmd_name);
  ArgParser parser = ArgParser(cmd_name, cmd_description, "", 0);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser& ArgParser::add_subcommand(std::string const& cmd_name,
                                     std::string const& cmd_description,
                                     std::string const& cmd_envvar,
                                     int cmd_arg_num, Function const& f) {
  check_command(cmd_name);
  ArgParser parser =
      ArgParser(cmd_name, cmd_description, cmd_envvar, cmd_arg_num, f);
  parser._parent = this;
  _subcommand_list.push_back(parser);
  return _subcommand_list.back();
}

ArgParser& ArgParser::add_subcommand(std::string const& cmd_name,
                                     std::string const& cmd_description,
                                     Function const& f) {
  check_command(cmd_name);
  ArgParser parser = ArgParser(cmd_name, cmd_description, "", 0, f);
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

std::string ArgParser::get_envvar() {
  return getenv(_envvar.c_str()) ? getenv(_envvar.c_str()) : "";
}

std::vector<std::string> ArgParser::get_data() { return _data; }

void ArgParser::show_parser_info() {
  if (_parent) {
    std::cout << "Parent parser: " + _parent->_name << std::endl;
  }
  std::cout << "name: " + _name << std::endl;
  std::cout << "description: " + _description << std::endl;
  std::cout << "ENV variable: " + _envvar << std::endl;
  std::cout << "expected arguments: " << _arg_num << std::endl;
  std::string args_string;
  for (int i = 0; i < _data.size(); i++) {
    args_string = args_string + _data[i] + " ";
  }
  std::cout << "Parser data stored: " + args_string << std::endl << std::endl;
}

void Option::show_option_info() {
  std::cout << "Long option: " + _opt_name << std::endl;
  std::cout << "Short option: " + _opt_key << std::endl;
  std::cout << "Option description: " + _opt_description << std::endl;
  std::cout << "Option ENV variable: " + _opt_envvar << std::endl;
  std::cout << "Option expected arguments: " << _opt_arg_num << std::endl;
  std::string args_string;
  for (int i = 0; i < _opt_data.size(); i++) {
    args_string = args_string + _opt_data[i] + " ";
  }
  std::cout << "Option data stored: " + args_string << std::endl << std::endl;
}

// need polish
void ArgParser::show_all_configuration() {
  std::cout << "Argument Configuration:\n" << std::endl;

  std::queue<ArgParser> queue;
  queue.push(*this);

  while (!queue.empty()) {
    ArgParser cur = queue.front();
    cur.show_parser_info();
    for (int i = 0; i < cur._option_list.size(); i++) {
      std::cout << "Option parent: " + cur._name << std::endl;
      cur._option_list[i].show_option_info();
    }
    queue.pop();
    for (int i = 0; i < cur._subcommand_list.size(); i++) {
      queue.push(cur._subcommand_list[i]);
    }
  }
}

// -------------------- start of the real world ----------------------------

void ArgParser::append_parser_data(std::vector<std::string>& args) {
  int conflict_flag = 0;
  for (int i = 0; i < args.size(); i++) {
    if (_name == args[i]) {
      args.erase(args.begin() + i);
      if (conflict_flag == 1) {
        // multiple commands found
        help(_argv);
      }
      conflict_flag = 1;

      // TODO: BUG
      // // how the current command know subcommand is called
      // if (args.size() > i) {
      //   for (auto it : _subcommand_list) {
      //     if (it._name == args[i]) {
      //       return;
      //     }
      //   }
      // }

      // handle the args
      for (int j = 0; j < _arg_num; j++) {
        if (args.size() < i + j + 1 || args[i + j].empty() ||
            args[i + j][0] == '-') {
          help(_argv);
        }
        std::string arg = args[i + j];
        _data.push_back(arg);
      }
      args.erase(args.begin() + i, args.begin() + i + _arg_num);
    }
  }
}

void ArgParser::append_option_data(std::vector<std::string>& args) {
  for (int i = 0; i < _option_list.size(); i++) {
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
      if (_option_list[i]._opt_name == args[j] ||
          _option_list[i]._opt_key == args[j]) {
        args.erase(args.begin() + j);
        // deal with --args=...
        for (int k = 0; k < _option_list[i]._opt_arg_num; k++) {
          if (args.size() < j + k + 1 || args[j + k].empty() ||
              args[j + k][0] == '-') {
            help(_argv);
          }
          std::string arg = args[j + k];
          _option_list[i]._opt_data.push_back(arg);
        }
        args.erase(args.begin() + j,
                   args.begin() + j + _option_list[i]._opt_arg_num);
      }
    }
  }
}

/**
 * Flow: A queue of BFS order to append data to command and option data
 */
void ArgParser::parse(int argc, const char** argv) {
  // put argv into vector<string>
  for (int i = 0; i < argc; i++) {
    _argv.push_back(argv[i]);
  }

  // args variable for the tweaking of each commands and options
  std::vector<std::string> args = _argv;
  // handle the first argument
  args[0] = args[0].substr(args[0].find_last_of('/') + 1);

  std::queue<ArgParser> parser_queue;
  parser_queue.push(*this);

  while (!parser_queue.empty()) {
    // BUG: not setting correctly
    ArgParser& cur_parser = parser_queue.front();
    cur_parser.append_parser_data(args);
    cur_parser.append_option_data(args);
    parser_queue.pop();
    for (int i = 0; i < cur_parser._subcommand_list.size(); i++) {
      parser_queue.push(cur_parser._subcommand_list[i]);
    }
  }

  if (!args.empty()) {
    for (int i = 0; i < args.size(); i++) {
      std::cout << args[i] << std::endl;
    }
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
