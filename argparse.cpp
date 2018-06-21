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

namespace ts {

ArgParser::ArgParser() {}

ArgParser::ArgParser(std::string const& name, std::string const& description,
                     std::string const& envvar, int arg_num) {
  _top_level_command = ArgParser::Command(name, description, envvar, arg_num);
}

ArgParser::~ArgParser() {}

ArgParser::Command& ArgParser::top_command() { return _top_level_command; }

void ArgParser::add_global_usage(std::string const& usage) {
  _global_usage = usage;
}

void ArgParser::help_message() const {
  // 1. output global usage
  if (_global_usage.size() > 0) {
    std::cout << "Usage: " + _global_usage << std::endl;
  }
  // find the level to output help message
  ArgParser::Command command = _top_level_command;
  for (int i = 1; i < _argv.size(); i++) {
    if (command._subcommand_list.find(_argv[i]) ==
        command._subcommand_list.end()) {
      break;
    }
    ArgParser::Command tmp = command._subcommand_list[_argv[i]];
    command = tmp;
  }
  // 2. output subcommands
  std::cout << "\nCommands: " << std::endl;
  std::string prefix = "- ";
  command.output_command(std::cout, prefix);
  // 3. output options
  if (command._option_list.size() > 0) {
    std::cout << "\nOptions: " << std::endl;
    for (auto it : command._option_list) {
      std::string msg = "  " + it.first;
      if (!it.second._opt_key.empty()) {
        msg = msg + ", " + it.second._opt_key;
      }
      std::cout << msg << ": " << it.second._opt_description << std::endl;
    }
  }
  // 4. output example usage
  if (!command._example_usage.empty()) {
    std::cout << "\nExample Usage: " << command._example_usage << std::endl;
  }
  exit(0);
}

void ArgParser::version_message() const {
  /*
  AppVersionInfo appVersionInfo;
  appVersionInfo.setup(PACKAGE_NAME, _name, PACKAGE_VERSION, __DATE__, __TIME__, BUILD_MACHINE, BUILD_PERSON, "");
  ink_fputln(stdout, appinfo->FullVersionInfoStr);
  */
  exit(0);
}

// Main logic of parsing
ParsedArgs ArgParser::parse(const char** argv) {
  // deal with argv first
  int size = 0;
  while (argv[size]) {
    _argv.push_back(argv[size]);
    size++;
  }
  if (size == 0) {
    std::cout << "Error: invalid argv provided" << std::endl;
    exit(1);
  }
  // the name of the program only
  _argv[0] = _argv[0].substr(_argv[0].find_last_of('/') + 1);
  _top_level_command._name = _argv[0];

  // start of parsing
  ParsedArgs ret;
  std::vector<std::string> args = _argv;
  // A queue of BFS (implemented using vector)
  std::vector<ArgParser::Command> cmd_queue;
  cmd_queue.push_back(_top_level_command);
  while (!cmd_queue.empty()) {
    ArgParser::Command cur = cmd_queue.front();
    // find conflict commands
    int conflict_flag = 0;
    for (int i = 0; i < args.size(); i++) {
      if (conflict_flag == 1) {
        std::cout << "Error: Multiple commands found" << std::endl;
        help_message();
      }
      if (cur._subcommand_list.find(args[i]) != cur._subcommand_list.end()) {
        conflict_flag = 1;
      }
    }
    // visit
    bool called = cur.append_data(*this, ret, args);
    // dequeue
    cmd_queue.erase(cmd_queue.begin());
    // enqueue
    if (called) {
      for (auto it : cur._subcommand_list) {
        cmd_queue.push_back(it.second);
      }
    }
  }
  // if there is anything left, then output usage
  if (!args.empty()) {
    std::string msg = "Unkown command, option or args:";
    for (int i = 0; i < args.size(); i++) {
      msg = msg + " '" + args[i] + "'";
    }
    std::cout << msg << std::endl;
    help_message();
  }
  return ret;
}

//===================== Command class =============================
ArgParser::Command::Command() {}

ArgParser::Command::~Command() {}

ArgParser::Command::Command(std::string const& name,
                            std::string const& description)
    : _name(name), _description(description) {}

ArgParser::Command::Command(std::string const& name,
                            std::string const& description,
                            std::string const& envvar, int arg_num)
    : _name(name),
      _description(description),
      _envvar(envvar),
      _arg_num(arg_num) {}

// check if this is a valid option before adding
void ArgParser::Command::check_option(std::string const& name,
                                      std::string const& key) const {
  if (name.size() < 3 || name[0] != '-' || name[1] != '-') {
    // invalid name
    std::cerr << "Error: invalid long option added: '" + name + "'";
    exit(1);
  }
  if (key.size() > 0 && key[0] != '-') {
    // invalid key
    std::cerr << "Error: invalid short option added: '" + key + "'";
    exit(1);
  }
  // find if existing in option list
  if (_option_list.find(name) != _option_list.end() ||
      _option_map.find(key) != _option_map.end()) {
    std::string msg;
    if (_option_list.find(name) != _option_list.end()) {
      msg = "Error: long option '" + name;
    } else {
      msg = "Error: short option '" + key;
    }
    if (_parent) {
      std::cerr << msg + "' already exists under current command: " + _name;
    } else {
      std::cerr << msg + "' already exists in current program";
    }
    exit(1);
  }
  // find recursively up to see if there is any conflict
  Command* parent = _parent;
  while (parent) {
    if (parent->_option_list.find(name) != parent->_option_list.end() ||
        parent->_option_map.find(key) != parent->_option_map.end()) {
      std::string msg;
      if (_option_list.find(name) != _option_list.end()) {
        msg = "Error: long option '" + name;
      } else {
        msg = "Error: short option '" + key;
      }
      if (parent->_parent) {
        std::cerr << msg + "' already exists under parent command: " + _name;
      } else {
        std::cerr << msg + "' already exists in current program";
      }
      exit(1);
    }
    parent = parent->_parent;
  }
}

// check if this is a valid command before adding
void ArgParser::Command::check_command(std::string const& name) const {
  if (name.empty()) {
    // invalid name
    std::cerr << "Error: empty command cannot be added";
    exit(1);
  }
  // find if existing in subcommand list
  if (_subcommand_list.find(name) != _subcommand_list.end()) {
    std::cerr << "Error: command already exists: '" + name + "'";
    exit(1);
  }
}

// add new options
ArgParser::Option& ArgParser::Command::add_option(
    std::string const& name, std::string const& key,
    std::string const& description, std::string const& envvar, int arg_num) {
  check_option(name, key);
  _option_list[name] =
      Option(name, key == "-" ? "" : key, description, envvar, arg_num);
  if (key != "-" && !key.empty()) {
    _option_map[key] = name;
  }
  return _option_list[name];
}

ArgParser::Option& ArgParser::Command::add_option(
    std::string const& name, std::string const& key,
    std::string const& description) {
  check_option(name, key);
  _option_list[name] = Option(name, key == "-" ? "" : key, description);
  if (key != "-" && !key.empty()) {
    _option_map[key] = name;
  }
  return _option_list[name];
}

// add new subcommands
ArgParser::Command& ArgParser::Command::add_subcommand(
    std::string const& cmd_name, std::string const& cmd_description,
    std::string const& cmd_envvar, int cmd_arg_num) {
  check_command(cmd_name);
  ArgParser::Command command =
      ArgParser::Command(cmd_name, cmd_description, cmd_envvar, cmd_arg_num);
  command._parent = this;
  _subcommand_list[cmd_name] = command;
  return _subcommand_list[cmd_name];
}

ArgParser::Command& ArgParser::Command::add_subcommand(
    std::string const& cmd_name, std::string const& cmd_description) {
  check_command(cmd_name);
  ArgParser::Command command =
      ArgParser::Command(cmd_name, cmd_description, "", 0);
  command._parent = this;
  _subcommand_list[cmd_name] = command;
  return _subcommand_list[cmd_name];
}

ArgParser::Command& ArgParser::Command::get_subcommand(
    std::string const& cmd_name) {
  // TODO: make it search the whole tree
  if (_subcommand_list.find(cmd_name) != _subcommand_list.end()) {
    return _subcommand_list[cmd_name];
  }
  std::cerr << "Error: command " + cmd_name + " not found";
  exit(1);
}

void ArgParser::Command::show_command_info() const {
  std::cout << "name: " + _name << std::endl;
  std::cout << "description: " + _description << std::endl;
  std::cout << "ENV variable: " + _envvar << std::endl;
  std::cout << "expected arguments: " << _arg_num << std::endl;
  if (_parent) {
    std::cout << "Parent Command: " + _parent->_name << std::endl;
  }
  std::cout << std::endl;
}

// need polish
void ArgParser::show_parser_info() const {
  std::cout << "Parser information:\n" << std::endl;
  // output the parser information BFS
  std::vector<ArgParser::Command> cmd_queue;
  cmd_queue.push_back(_top_level_command);

  while (!cmd_queue.empty()) {
    ArgParser::Command cur = cmd_queue.front();
    cur.show_command_info();
    for (auto it : cur._option_list) {
      it.second.show_option_info();
      std::cout << "Option parent: " + cur._name << std::endl << std::endl;
    }
    cmd_queue.erase(cmd_queue.begin());
    for (auto it : cur._subcommand_list) {
      cmd_queue.push_back(it.second);
    }
  }
}

// append the args of option to parsed data
void ArgParser::Command::append_option_data(ArgParser& base, ParsedArgs& ret,
                                            std::vector<std::string>& args,
                                            int index) {
  for (int i = index; i < args.size(); i++) {
    // output version or help message
    if (args[i] == "--version" || args[i] == "-V") {
      base.version_message();
    }
    if (args[i] == "--help" || args[i] == "-h") {
      base.help_message();
    }
    // find matches of the arg
    if (args[i][0] == '-' && args[i][1] == '-' &&
        args[i].find('=') != std::string::npos) {
      // deal with --args=
      std::string option_name = args[i].substr(0, args[i].find_first_of('='));
      if (_option_list.find(option_name) != _option_list.end()) {
        ParserData option_data;
        Option cur_option = _option_list[option_name];
        option_data.arg_data.push_back(
            args[i].substr(args[i].find_last_of('=') + 1));
        args.erase(args.begin() + i);
        ret._data_map[cur_option._opt_name] = option_data;
        i -= 1;
      }
    } else if (_option_list.find(args[i]) != _option_list.end() ||
               _option_map.find(args[i]) != _option_map.end()) {
      // arg name match or key match
      ParserData option_data;
      Option cur_option;
      if (_option_list.find(args[i]) != _option_list.end()) {
        cur_option = _option_list[args[i]];
      } else {
        cur_option = _option_list[_option_map[args[i]]];
      }
      // handle the options
      if (cur_option._opt_arg_num == INFINITE_ARG_NUM) {
        // infinite arguments
        for (int j = i + 1; j < args.size(); j++) {
          option_data.arg_data.push_back(args[j]);
        }
        args.erase(args.begin() + i, args.end());
        ret._data_map[cur_option._opt_name] = option_data;
        return;
      }
      // normal finite number of argument handling
      for (int j = 0; j < cur_option._opt_arg_num; j++) {
        if (args.size() < i + j + 2 || args[i + j + 1].empty() ||
            args[i + j + 1][0] == '-') {
          std::cout << "Error: " << _arg_num << " arguments expected by "
                    << cur_option._opt_name << std::endl
                    << std::endl;
          base.help_message();
        }
        option_data.arg_data.push_back(args[i + j + 1]);
      }
      args.erase(args.begin() + i,
                 args.begin() + i + cur_option._opt_arg_num + 1);
      ret._data_map[cur_option._opt_name] = option_data;
      i -= 1;
    }
  }
}

// append the args of command to parsed data
bool ArgParser::Command::append_data(ArgParser& base, ParsedArgs& ret,
                                     std::vector<std::string>& args) {
  // iterate through all arguments
  for (int i = 0; i < args.size(); i++) {
    if (_name == args[i]) {
      // get ENV var first
      ParserData cmd_data;
      if (_envvar.size() > 0) {
        cmd_data.env_data =
            getenv(_envvar.c_str()) ? getenv(_envvar.c_str()) : "";
      }
      // handle the option
      append_option_data(base, ret, args, i);
      // when subcommand is called
      if (_subcommand_list.find(args[i + 1]) != _subcommand_list.end()) {
        args.erase(args.begin() + i);
        ret._data_map[_name] = cmd_data;
        return true;
      }
      // handle the args
      if (_arg_num == INFINITE_ARG_NUM) {
        // infinite arguments
        for (int j = i + 1; j < args.size(); j++) {
          cmd_data.arg_data.push_back(args[j]);
        }
        args.erase(args.begin() + i, args.end());
        ret._data_map[_name] = cmd_data;
        return true;
      }
      // finite number of argument handling
      for (int j = 0; j < _arg_num; j++) {
        if (args.size() < i + j + 2 || args[i + j + 1].empty() ||
            args[i + j + 1][0] == '-') {
          std::cout << "Error: " << _arg_num << " arguments expected by "
                    << _name << std::endl
                    << std::endl;
          base.help_message();
        }
        cmd_data.arg_data.push_back(args[i + j + 1]);
      }
      args.erase(args.begin() + i, args.begin() + i + _arg_num + 1);
      ret._data_map[_name] = cmd_data;
      i -= 1;
      return true;
    }
  }
  return false;
}

void ArgParser::Command::add_example_usage(std::string const& usage) {
  _example_usage = usage;
}

// method used by help_message()
void ArgParser::Command::output_command(std::ostream& out,
                                        std::string const& prefix) const {
  out << prefix << _name << ": " << _description << std::endl;
  for (auto it : _subcommand_list) {
    it.second.output_command(out, "  " + prefix);
  }
}

//===================== Option class =============================
ArgParser::Option::Option() {}
ArgParser::Option::~Option() {}
ArgParser::Option::Option(std::string const& opt_name,
                          std::string const& opt_key,
                          std::string const& opt_description)
    : _opt_name(opt_name),
      _opt_key(opt_key),
      _opt_description(opt_description) {}
ArgParser::Option::Option(std::string const& opt_name,
                          std::string const& opt_key,
                          std::string const& opt_description,
                          std::string const& opt_envvar, int opt_arg_num)
    : _opt_name(opt_name),
      _opt_key(opt_key),
      _opt_description(opt_description),
      _opt_envvar(opt_envvar),
      _opt_arg_num(opt_arg_num) {}

// For temporary testing
void ArgParser::Option::show_option_info() const {
  std::cout << "Long option: " + _opt_name << std::endl;
  std::cout << "Short option: " + _opt_key << std::endl;
  std::cout << "Option description: " + _opt_description << std::endl;
  std::cout << "Option ENV variable: " + _opt_envvar << std::endl;
  std::cout << "Option expected arguments: " << _opt_arg_num << std::endl;
  ;
}

//===================== ParsedArgs class =============================

ParsedArgs::ParsedArgs() {}
ParsedArgs::~ParsedArgs() {}

std::string ParsedArgs::get_env(std::string const& name) {
  if (_data_map.find(name) != _data_map.end()) {
    return _data_map[name].env_data;
  } else {
    return "";
  }
}
std::vector<std::string> ParsedArgs::get_args(std::string const& name) {
  if (_data_map.find(name) != _data_map.end()) {
    return _data_map[name].arg_data;
  } else {
    return std::vector<std::string>();
  }
}

bool ParsedArgs::called(std::string const& name) {
  if (_data_map.find(name) != _data_map.end()) {
    return true;
  } else {
    return false;
  }
}

void ParsedArgs::show_all_configuration() const {
  for (auto it : _data_map) {
    std::cout << "name: " + it.first << std::endl;
    std::string s;
    s = "args value:";
    for (int i = 0; i < it.second.arg_data.size(); i++) {
      s += " " + it.second.arg_data[i];
    }
    std::cout << s << std::endl;
    std::cout << "env value: " + it.second.env_data << std::endl << std::endl;
  }
}

}  // namespace ts
