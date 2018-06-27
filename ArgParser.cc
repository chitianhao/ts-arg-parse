/** @file

  Powerful and easy-to-use command line parsing for ATS

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

#include "ArgParser.h"
// #include "ink_file.h"
// #include "I_Version.h"

#include <iostream>

namespace ts
{
ArgParser::ArgParser() {}

ArgParser::ArgParser(std::string const &name, std::string const &description, std::string const &envvar, int arg_num,
                     Function const &f)
{
  // initialize _top_level_command according to the provided message
  _top_level_command = ArgParser::Command(name, description, envvar, arg_num, f);
}

ArgParser::~ArgParser() {}

ArgParser::Command &
ArgParser::top_command()
{
  return _top_level_command;
}

void
ArgParser::add_global_usage(std::string const &usage)
{
  _global_usage = usage;
}

// a graceful way to output help message
void
ArgParser::help_message() const
{
  // four steps for the help message
  // 1. output global usage
  if (_global_usage.size() > 0) {
    std::cout << "\nUsage: " + _global_usage << std::endl;
  }
  // find the exact level to output help message
  ArgParser::Command command = _top_level_command;
  for (unsigned long i = 1; i < _argv.size(); i++) {
    if (command._subcommand_list.find(_argv[i]) == command._subcommand_list.end()) {
      break;
    }
    ArgParser::Command tmp = command._subcommand_list[_argv[i]];
    command                = tmp;
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
      // create the indent for output
      std::string indent;
      for (unsigned long i = 0; i < 30 - msg.size(); i++) {
        indent += " ";
      }
      std::cout << msg << ": " << indent << it.second._opt_description << std::endl;
    }
  }
  // 4. output example usage
  if (!command._example_usage.empty()) {
    std::cout << "\nExample Usage: " << command._example_usage << std::endl;
  }
  exit(0);
}

void
ArgParser::version_message() const
{
  // unified version message of ATS
  /*
  AppVersionInfo appVersionInfo;
  appVersionInfo.setup(PACKAGE_NAME, _argv[0].c_str(), PACKAGE_VERSION, __DATE__, __TIME__, BUILD_MACHINE, BUILD_PERSON, "");
  ink_fputln(stdout, appVersionInfo.FullVersionInfoStr);
  exit(0);
  */
}

// Top level call of parsing
ParsedArgs
ArgParser::parse(const char **argv)
{
  // deal with argv first
  int size = 0;
  _argv.clear();
  while (argv[size]) {
    _argv.push_back(argv[size]);
    size++;
  }
  if (size == 0) {
    std::cout << "Error: invalid argv provided" << std::endl;
    exit(1);
  }
  // the name of the program only
  _argv[0]                 = _argv[0].substr(_argv[0].find_last_of('/') + 1);
  _top_level_command._name = _argv[0];

  ParsedArgs ret; // the parsed arg object to return
  StringArray args = _argv;
  // call the recrusive parse method in Command
  _top_level_command.parse(*this, ret, args);
  // if there is anything left, then output usage
  if (!args.empty()) {
    std::string msg = "Unkown command, option or args:";
    for (unsigned long i = 0; i < args.size(); i++) {
      msg = msg + " '" + args[i] + "'";
    }
    std::cout << msg << std::endl;
    help_message();
  }
  return ret;
}

//=========================== Command class ================================
ArgParser::Command::Command() {}

ArgParser::Command::~Command() {}

ArgParser::Command::Command(std::string const &name, std::string const &description, std::string const &envvar, int arg_num,
                            Function const &f)
  : _name(name), _description(description), _arg_num(arg_num), _envvar(envvar), _f(f)
{
}

// check if this is a valid option before adding
void
ArgParser::Command::check_option(std::string const &name, std::string const &key) const
{
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
  if (_option_list.find(name) != _option_list.end() || _option_map.find(key) != _option_map.end()) {
    std::string msg;
    if (_option_list.find(name) != _option_list.end()) {
      msg = "Error: long option '" + name;
    } else {
      msg = "Error: short option '" + key;
    }
    if (_parent) {
      std::cerr << msg + "' already exists under command: " + _name;
    } else {
      std::cerr << msg + "' already exists in current program";
    }
    exit(1);
  }
  // search recursively up
  if (_parent) {
    _parent->check_option(name, key);
  }
}

// check if this is a valid command before adding
void
ArgParser::Command::check_command(std::string const &name) const
{
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

// add new options without args
ArgParser::Option &
ArgParser::Command::add_option(std::string const &name, std::string const &key, std::string const &description)
{
  check_option(name, key);
  _option_list[name] = {name, key == "-" ? "" : key, description, "", 0};
  if (key != "-" && !key.empty()) {
    _option_map[key] = name;
  }
  return _option_list[name];
}

// add new options with args
ArgParser::Option &
ArgParser::Command::add_option(std::string const &name, std::string const &key, std::string const &description,
                               std::string const &envvar, int arg_num)
{
  check_option(name, key);
  _option_list[name] = {name, key == "-" ? "" : key, description, envvar, arg_num};
  if (key != "-" && !key.empty()) {
    _option_map[key] = name;
  }
  return _option_list[name];
}

// add sub-command without args
ArgParser::Command &
ArgParser::Command::add_subcommand(std::string const &cmd_name, std::string const &cmd_description)
{
  check_command(cmd_name);
  ArgParser::Command command = ArgParser::Command(cmd_name, cmd_description, "", 0, nullptr);
  command._parent            = this;
  _subcommand_list[cmd_name] = command;
  return _subcommand_list[cmd_name];
}

// add sub-command with args
ArgParser::Command &
ArgParser::Command::add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                                   int cmd_arg_num)
{
  check_command(cmd_name);
  ArgParser::Command command = ArgParser::Command(cmd_name, cmd_description, cmd_envvar, cmd_arg_num, nullptr);
  command._parent            = this;
  _subcommand_list[cmd_name] = command;
  return _subcommand_list[cmd_name];
}

// add sub-command without args and function
ArgParser::Command &
ArgParser::Command::add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                                   int cmd_arg_num, Function const &f)
{
  check_command(cmd_name);
  ArgParser::Command command = ArgParser::Command(cmd_name, cmd_description, cmd_envvar, cmd_arg_num, f);
  command._parent            = this;
  _subcommand_list[cmd_name] = command;
  return _subcommand_list[cmd_name];
}

void
ArgParser::Command::add_example_usage(std::string const &usage)
{
  _example_usage = usage;
}

// method used by help_message()
void
ArgParser::Command::output_command(std::ostream &out, std::string const &prefix) const
{
  // create indent
  std::string indent;
  for (unsigned long i = 0; i < 30 - (prefix.size() + _name.size()); i++) {
    indent += " ";
  }
  out << prefix << _name << ": " << indent << _description << std::endl;
  // recursive call
  for (auto it : _subcommand_list) {
    it.second.output_command(out, "  " + prefix);
  }
}

void
ArgParser::show_parser_info() const
{
  std::cout << "Parser information:\n" << std::endl;
  _top_level_command.show_command_info();
}

// helper method to handle the arguments and put them nicely in data
static void
handle_args(ArgParser &base, ParsedArgs &ret, StringArray &args, ParserData &data, std::string const &name, int arg_num, unsigned long &index)
{
  // handle the args
  if (arg_num == INFINITE_ARG_NUM) {
    // infinite arguments
    for (unsigned long j = index + 1; j < args.size(); j++) {
      data.arg_data.push_back(args[j]);
    }
    args.erase(args.begin() + index, args.end());
    ret.append(name, data);
    return;
  }
  // finite number of argument handling
  for (int j = 0; j < arg_num; j++) {
    if (args.size() < index + j + 2 || args[index + j + 1].empty() || args[index + j + 1][0] == '-') {
      std::cout << "Error: " << arg_num << " argument(s) expected by " << name << std::endl << std::endl;
      base.help_message();
    }
    data.arg_data.push_back(args[index + j + 1]);
  }
  // erase the used arguments and append the data to the return structure
  args.erase(args.begin() + index, args.begin() + index + arg_num + 1);
  ret.append(name, data);
  index -= 1;
  return;
}

// append the args of option to parsed data
static void
append_option_data(ArgParser &base, ParsedArgs &ret, StringArray &args,
                   std::unordered_map<std::string, ArgParser::Option> const &option_list,
                   std::unordered_map<std::string, std::string> const &option_map, int index)
{
  for (unsigned long i = index; i < args.size(); i++) {
    // find matches of the arg
    if (args[i][0] == '-' && args[i][1] == '-' && args[i].find('=') != std::string::npos) {
      // deal with --args=
      std::string option_name = args[i].substr(0, args[i].find_first_of('='));
      if (option_list.find(option_name) != option_list.end() && option_list.at(option_name)._opt_arg_num == 1) {
        ParserData option_data;
        ArgParser::Option cur_option = option_list.at(option_name);
        option_data.arg_data.push_back(args[i].substr(args[i].find_last_of('=') + 1));
        args.erase(args.begin() + i);
        ret.append(cur_option._opt_name, option_data);
        i -= 1;
      }
    } else if (option_list.find(args[i]) != option_list.end() || option_map.find(args[i]) != option_map.end()) {
      // arg name match or key match
      // output version or help message
      if (args[i] == "--version" || args[i] == "-V") {
        base.version_message();
      }
      if (args[i] == "--help" || args[i] == "-h") {
        base.help_message();
      }
      ParserData option_data;
      ArgParser::Option cur_option;
      if (option_list.find(args[i]) != option_list.end()) {
        cur_option = option_list.at(args[i]);
      } else {
        cur_option = option_list.at(option_map.at(args[i]));
      }
      handle_args(base, ret, args, option_data, cur_option._opt_name, cur_option._opt_arg_num, i);
    }
  }
}

// Main recursive logic of Parsing
void
ArgParser::Command::parse(ArgParser &base, ParsedArgs &ret, StringArray &args)
{
  // check for conflict commands
  int conflict_flag = 0;
  for (unsigned long i = 0; i < args.size(); i++) {
    if (_subcommand_list.find(args[i]) != _subcommand_list.end()) {
      if (conflict_flag == 1) {
        std::cout << "Error: Multiple commands found" << std::endl;
        base.help_message();
      }
      conflict_flag = 1;
    }
  }
  // iterate through all arguments
  for (unsigned long i = 0; i < args.size(); i++) {
    if (_name == args[i]) {
      // get ENV var first
      ParserData cmd_data;
      if (_envvar.size() > 0) {
        cmd_data.env_data = getenv(_envvar.c_str()) ? getenv(_envvar.c_str()) : "";
      }
      // handle the option
      append_option_data(base, ret, args, _option_list, _option_map, i);
      // handle the action
      if (_f) {
        ret._action = _f;
      }
      // when subcommand is called, continue
      if (_subcommand_list.find(args[i + 1]) != _subcommand_list.end()) {
        args.erase(args.begin() + i);
        ret.append(_name, cmd_data);
        break;
      }
      handle_args(base, ret, args, cmd_data, _name, _arg_num, i);
      return;
    }
  }
  // recursively call subcommand
  for (auto it : _subcommand_list) {
    it.second.parse(base, ret, args);
  }
}

void
ArgParser::Command::show_command_info() const
{
  // show the command information
  std::cout << "name: " + _name << std::endl;
  std::cout << "description: " + _description << std::endl;
  std::cout << "ENV variable: " + _envvar << std::endl;
  std::cout << "expected arguments: " << _arg_num << std::endl;
  if (_parent) {
    std::cout << "Parent Command: " + _parent->_name << std::endl;
  }
  std::cout << std::endl;
  // show the options information
  for (auto it : _option_list) {
    std::cout << "Long option: " + it.second._opt_name << std::endl;
    std::cout << "Short option: " + it.second._opt_key << std::endl;
    std::cout << "Option description: " + it.second._opt_description << std::endl;
    std::cout << "Option ENV variable: " + it.second._opt_envvar << std::endl;
    std::cout << "Option expected arguments: " << it.second._opt_arg_num << std::endl;
    std::cout << "Option parent: " + _name << std::endl << std::endl;
  }
  // recusive call
  for (auto it : _subcommand_list) {
    it.second.show_command_info();
  }
}

//=========================== ParsedArgs class ================================

ParsedArgs::ParsedArgs() {}
ParsedArgs::~ParsedArgs() {}

std::string
ParsedArgs::get_env(std::string const &name) const
{
  if (_data_map.find(name) != _data_map.end()) {
    return _data_map.at(name).env_data;
  } else {
    return "";
  }
}
StringArray
ParsedArgs::get_args(std::string const &name) const
{
  if (_data_map.find(name) != _data_map.end()) {
    return _data_map.at(name).arg_data;
  } else {
    return StringArray();
  }
}

bool
ParsedArgs::called(std::string const &name) const
{
  if (_data_map.find(name) != _data_map.end()) {
    return true;
  } else {
    return false;
  }
}

void
ParsedArgs::append(std::string const &key, ParserData const &value)
{
  // perform overwrite for now
  _data_map[key] = value;
}

void
ParsedArgs::show_all_configuration() const
{
  for (auto it : _data_map) {
    std::cout << "name: " + it.first << std::endl;
    std::string msg;
    msg = "args value:";
    for (unsigned long i = 0; i < it.second.arg_data.size(); i++) {
      msg += " " + it.second.arg_data[i];
    }
    std::cout << msg << std::endl;
    std::cout << "env value: " + it.second.env_data << std::endl << std::endl;
  }
}

// invoke the function with the args
int
ParsedArgs::invoke()
{
  if (_action) {
    // call the std::functin
    return _action();
  } else {
    std::cerr << "Error: no function to invoke";
    exit(1);
  }
}

} // namespace ts
