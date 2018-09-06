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
#include <set>
#include <sstream>

std::string global_usage;

namespace ts
{
ArgParser::ArgParser() {}

ArgParser::ArgParser(std::string const &name, std::string const &description, std::string const &envvar, unsigned arg_num,
                     Function const &f)
{
  // initialize _top_level_command according to the provided message
  _top_level_command = ArgParser::Command(name, description, envvar, arg_num, f);
}

ArgParser::~ArgParser() {}

// add new options with args
ArgParser::Command &
ArgParser::add_option(std::string const &long_option, std::string const &short_option, std::string const &description,
                      std::string const &envvar, unsigned arg_num, std::string const &default_value, std::string const &key)
{
  return _top_level_command.add_option(long_option, short_option, description, envvar, arg_num, default_value, key);
}

// add sub-command with only function
ArgParser::Command &
ArgParser::add_command(std::string const &cmd_name, std::string const &cmd_description, Function const &f, std::string const &key)
{
  return _top_level_command.add_command(cmd_name, cmd_description, f, key);
}

// add sub-command without args and function
ArgParser::Command &
ArgParser::add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                       unsigned cmd_arg_num, Function const &f, std::string const &key)
{
  return _top_level_command.add_command(cmd_name, cmd_description, cmd_envvar, cmd_arg_num, f, key);
}

void
ArgParser::add_global_usage(std::string const &usage)
{
  global_usage = usage;
}

void
ArgParser::help_message(std::string_view err) const
{
  return _top_level_command.help_message(err);
}

// handle the output of arguments for help message
static std::string
argument_number_output(unsigned num)
{
  std::string arg_num_msg;
  if (num == 0) {
    arg_num_msg = "";
  } else if (num == MORE_THAN_ONE_ARG_N) {
    arg_num_msg = "1+";
  } else if (num == MORE_THAN_ZERO_ARG_N) {
    arg_num_msg = "0+";
  } else {
    arg_num_msg = std::to_string(num);
  }
  return arg_num_msg;
}

// a graceful way to output help message
void
ArgParser::Command::help_message(std::string_view err) const
{
  if (!err.empty()) {
    std::cout << "Error: " << err << std::endl;
  }
  // output global usage
  if (global_usage.size() > 0) {
    std::cout << "\nUsage: " + global_usage << std::endl;
  }
  // output subcommands
  std::cout << "\nCommands -------------- Args -- Description -----------------------" << std::endl;
  std::string prefix = "- ";
  output_command(std::cout, prefix);
  // output options
  if (_option_list.size() > 0) {
    std::cout << "\nOptions =============== Args == Default ===== Description =============" << std::endl;
    for (auto it : _option_list) {
      // nice formated way for output
      std::string msg = it.first;
      if (!it.second.short_option.empty()) {
        msg = msg + ", " + it.second.short_option;
      }
      if (INDENT_ONE - static_cast<int>(msg.size()) < 0) {
        msg = msg.substr(0, INDENT_ONE - 3) + "...";
      }
      msg = msg + std::string(INDENT_ONE - msg.size(), ' ') + argument_number_output(it.second.arg_num);
      if (INDENT_TWO - static_cast<int>(msg.size()) < 0) {
        msg = msg.substr(0, INDENT_TWO - 3) + "...";
      }
      msg = msg + std::string(INDENT_TWO - msg.size(), ' ') + it.second.default_value;
      if (INDENT_THREE - static_cast<int>(msg.size()) < 0) {
        msg = msg.substr(0, INDENT_THREE - 3) + "...";
      }
      std::cout << msg << std::string(INDENT_THREE - msg.size(), ' ') << it.second.description << std::endl;
    }
  }
  // output example usage
  if (!_example_usage.empty()) {
    std::cout << "\nExample Usage: " << _example_usage << std::endl;
  }
  exit(0);
}

void
ArgParser::Command::version_message() const
{
  // unified version message of ATS
  /*
  AppVersionInfo appVersionInfo;
  appVersionInfo.setup(PACKAGE_NAME, _name.c_str(), PACKAGE_VERSION, __DATE__, __TIME__, BUILD_MACHINE, BUILD_PERSON, "");
  ink_fputln(stdout, appVersionInfo.FullVersionInfoStr);
  exit(0);
  */
}

// Top level call of parsing
Arguments
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
  _top_level_command._key  = _argv[0];
  Arguments ret; // the parsed arg object to return
  StringArray args = _argv;
  // call the recrusive parse method in Command
  _top_level_command.parse(ret, args);
  // if there is anything left, then output usage
  if (!args.empty()) {
    std::string msg = "Unkown command, option or args:";
    for (auto it : args) {
      msg = msg + " '" + it + "'";
    }
    _top_level_command.help_message(msg);
  }
  return ret;
}

ArgParser::Command &
ArgParser::require_commands()
{
  return _top_level_command.require_commands();
}

//=========================== Command class ================================
ArgParser::Command::Command() {}

ArgParser::Command::~Command() {}

ArgParser::Command::Command(std::string const &name, std::string const &description, std::string const &envvar, unsigned arg_num,
                            Function const &f, std::string const &key)
  : _name(name), _description(description), _arg_num(arg_num), _envvar(envvar), _f(f), _key(key)
{
}

// check if this is a valid option before adding
void
ArgParser::Command::check_option(std::string const &long_option, std::string const &short_option, std::string const &key) const
{
  if (long_option.size() < 3 || long_option[0] != '-' || long_option[1] != '-') {
    // invalid name
    std::cerr << "Error: invalid long option added: '" + long_option + "'" << std::endl;
    exit(1);
  }
  if (short_option.size() > 2 || (short_option.size() > 0 && short_option[0] != '-')) {
    // invalid short option
    std::cerr << "Error: invalid short option added: '" + short_option + "'" << std::endl;
    exit(1);
  }
  // find if existing in option list
  auto long_it  = _option_list.find(long_option);
  auto short_it = _option_map.find(short_option);
  if (long_it != _option_list.end() || short_it != _option_map.end()) {
    std::string msg;
    if (long_it != _option_list.end()) {
      msg = "Error: long option '" + long_option;
    } else {
      msg = "Error: short option '" + short_option;
    }
    if (_parent) {
      std::cerr << msg + "' already exists under command: " + _name << std::endl;
    } else {
      std::cerr << msg + "' already exists in current program" << std::endl;
    }
    exit(1);
  }
}

// check if this is a valid command before adding
void
ArgParser::Command::check_command(std::string const &name, std::string const &key) const
{
  if (name.empty()) {
    // invalid name
    std::cerr << "Error: empty command cannot be added" << std::endl;
    exit(1);
  }
  // find if existing in subcommand list
  if (_subcommand_list.find(name) != _subcommand_list.end()) {
    std::cerr << "Error: command already exists: '" + name + "'" << std::endl;
    exit(1);
  }
}

// add new options with args
ArgParser::Command &
ArgParser::Command::add_option(std::string const &long_option, std::string const &short_option, std::string const &description,
                               std::string const &envvar, unsigned arg_num, std::string const &default_value,
                               std::string const &key)
{
  std::string lookup_key = key.empty() ? long_option.substr(2) : key;
  check_option(long_option, short_option, lookup_key);
  _option_list[long_option] = {long_option, short_option == "-" ? "" : short_option, description, envvar, arg_num, default_value,
                               lookup_key};
  if (short_option != "-" && !short_option.empty()) {
    _option_map[short_option] = long_option;
  }
  return *this;
}

// add sub-command with only function
ArgParser::Command &
ArgParser::Command::add_command(std::string const &cmd_name, std::string const &cmd_description, Function const &f,
                                std::string const &key)
{
  std::string lookup_key = key.empty() ? cmd_name : key;
  check_command(cmd_name, lookup_key);
  ArgParser::Command command = ArgParser::Command(cmd_name, cmd_description, "", 0, f, lookup_key);
  command._parent            = this;
  _subcommand_list[cmd_name] = command;
  return _subcommand_list[cmd_name];
}

// add sub-command without args and function
ArgParser::Command &
ArgParser::Command::add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                                unsigned cmd_arg_num, Function const &f, std::string const &key)
{
  std::string lookup_key = key.empty() ? cmd_name : key;
  check_command(cmd_name, lookup_key);
  ArgParser::Command command = ArgParser::Command(cmd_name, cmd_description, cmd_envvar, cmd_arg_num, f, lookup_key);
  command._parent            = this;
  _subcommand_list[cmd_name] = command;
  return _subcommand_list[cmd_name];
}

ArgParser::Command &
ArgParser::Command::add_example_usage(std::string const &usage)
{
  _example_usage = usage;
  return *this;
}

// method used by help_message()
void
ArgParser::Command::output_command(std::ostream &out, std::string const &prefix) const
{
  // a nicely formated way to output command usage
  std::string msg = prefix + _name;
  if (INDENT_ONE - static_cast<int>(msg.size()) < 0) {
    msg = msg.substr(0, INDENT_ONE - 3) + "...";
  }
  msg = msg + std::string(INDENT_ONE - msg.size(), ' ') + argument_number_output(_arg_num);
  if (INDENT_TWO - static_cast<int>(msg.size()) < 0) {
    msg = msg.substr(0, INDENT_TWO - 3) + "...";
  }
  // output with a nice format
  std::cout << msg << std::string(INDENT_TWO - msg.size(), ' ') << _description << std::endl;
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

// helper method to handle the arguments and put them nicely in arguments
// can be switched to ts::errata
static std::string
handle_args(Arguments &ret, StringArray &args, std::string const &name, unsigned arg_num, unsigned &index)
{
  ArgumentData data;
  ret.append(name, data);
  // handle the args
  if (arg_num == MORE_THAN_ZERO_ARG_N || arg_num == MORE_THAN_ONE_ARG_N) {
    // infinite arguments
    if (arg_num == MORE_THAN_ONE_ARG_N && args.size() <= index + 1) {
      return "at least one argument expected by " + name;
    }
    for (unsigned j = index + 1; j < args.size(); j++) {
      ret.append_arg(name, args[j]);
    }
    args.erase(args.begin() + index, args.end());
    return "";
  }
  // finite number of argument handling
  for (unsigned j = 0; j < arg_num; j++) {
    if (args.size() < index + j + 2 || args[index + j + 1].empty()) {
      return std::to_string(arg_num) + " argument(s) expected by " + name;
    }
    ret.append_arg(name, args[index + j + 1]);
  }
  // erase the used arguments and append the data to the return structure
  args.erase(args.begin() + index, args.begin() + index + arg_num + 1);
  index -= 1;
  return "";
}

// Append the args of option to parsed data. Return true if there is any option called
void
ArgParser::Command::append_option_data(Arguments &ret, StringArray &args, int index)
{
  std::unordered_map<std::string, unsigned> check_map;
  for (unsigned i = index; i < args.size(); i++) {
    // output version message
    if (args[i] == "--version" || args[i] == "-V") {
      version_message();
    }
    // output help message
    ArgParser::Command command = *this;
    if (args[i] == "--help" || args[i] == "-h") {
      // find the correct level to output help messsage
      for (unsigned i = 1; i < args.size(); i++) {
        auto it = command._subcommand_list.find(args[i]);
        if (it == _subcommand_list.end()) {
          break;
        }
        ArgParser::Command tmp = it->second;
        command                = tmp;
      }
      command.help_message();
    }
    // find matches of the arg
    if (args[i][0] == '-' && args[i][1] == '-' && args[i].find('=') != std::string::npos) {
      // deal with --args=
      std::string option_name = args[i].substr(0, args[i].find_first_of('='));
      std::string value       = args[i].substr(args[i].find_last_of('=') + 1);
      if (value.empty()) {
        help_message("missing argument for '" + option_name + "'");
      }
      auto it = _option_list.find(option_name);
      if (it != _option_list.end()) {
        ArgParser::Option cur_option = it->second;
        // handle environment variable
        if (!cur_option.envvar.empty()) {
          ret.set_env(cur_option.key, getenv(cur_option.envvar.c_str()) ? getenv(cur_option.envvar.c_str()) : "");
        }
        ret.append_arg(cur_option.key, value);
        check_map[cur_option.long_option] += 1;
        args.erase(args.begin() + i);
        i -= 1;
      }
    } else {
      // deal with normal --arg val1 val2 ...
      auto long_it  = _option_list.find(args[i]);
      auto short_it = _option_map.find(args[i]);
      // long option match or short option match
      if (long_it != _option_list.end() || short_it != _option_map.end()) {
        ArgParser::Option cur_option;
        if (long_it != _option_list.end()) {
          cur_option = long_it->second;
        } else {
          cur_option = _option_list.at(short_it->second);
        }
        // handle the arguments
        std::string err = handle_args(ret, args, cur_option.key, cur_option.arg_num, i);
        if (!err.empty()) {
          help_message(err);
        }
        // handle environment variable
        if (!cur_option.envvar.empty()) {
          ret.set_env(cur_option.key, getenv(cur_option.envvar.c_str()) ? getenv(cur_option.envvar.c_str()) : "");
        }
      }
    }
  }
  // check for wrong number of arguments for --arg=...
  for (auto it : check_map) {
    unsigned num = _option_list.at(it.first).arg_num;
    if (num != it.second && num < MORE_THAN_ONE_ARG_N) {
      help_message(std::to_string(_option_list.at(it.first).arg_num) + " arguments expected by " + it.first);
    }
  }
  // put in the default value of options
  for (auto it : _option_list) {
    if (!it.second.default_value.empty() && ret.get(it.second.key).empty()) {
      std::istringstream ss(it.second.default_value);
      std::string token;
      while (std::getline(ss, token, ' ')) {
        ret.append_arg(it.second.key, token);
      }
    }
  }
}

// Main recursive logic of Parsing
bool
ArgParser::Command::parse(Arguments &ret, StringArray &args)
{
  bool command_called = false;
  // iterate through all arguments
  for (unsigned i = 0; i < args.size(); i++) {
    if (_name == args[i]) {
      command_called = true;
      // handle the option
      append_option_data(ret, args, i);
      // handle the action
      if (_f) {
        ret._action = _f;
      }
      std::string err = handle_args(ret, args, _key, _arg_num, i);
      if (!err.empty()) {
        help_message(err);
      }
      // set ENV var
      if (!_envvar.empty()) {
        ret.set_env(_key, getenv(_envvar.c_str()) ? getenv(_envvar.c_str()) : "");
      }
      break;
    }
  }
  if (command_called) {
    bool flag = false;
    // recursively call subcommand
    for (auto it : _subcommand_list) {
      if (it.second.parse(ret, args)) {
        flag = true;
        break;
      }
    }
    // check for command required
    if (!flag && _command_required) {
      help_message("No subcommand found for " + _name);
    }
  }
  return command_called;
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
    std::cout << "Long option: " + it.second.long_option << std::endl;
    std::cout << "Short option: " + it.second.short_option << std::endl;
    std::cout << "Option description: " + it.second.description << std::endl;
    std::cout << "Option ENV variable: " + it.second.envvar << std::endl;
    std::cout << "Option expected arguments: " << it.second.arg_num << std::endl;
    std::cout << "Option parent: " + _name << std::endl << std::endl;
  }
  // recusive call
  for (auto it : _subcommand_list) {
    it.second.show_command_info();
  }
}

ArgParser::Command &
ArgParser::Command::require_commands()
{
  _command_required = true;
  return *this;
}

//=========================== Arguments class ================================

Arguments::Arguments() {}
Arguments::~Arguments() {}

ArgumentData
Arguments::get(std::string const &name)
{
  if (_data_map.find(name) != _data_map.end()) {
    _data_map[name]._is_called = true;
    return _data_map[name];
  }
  return ArgumentData();
}

void
Arguments::append(std::string const &key, ArgumentData const &value)
{
  // perform overwrite for now
  _data_map[key] = value;
}

void
Arguments::append_arg(std::string const &key, std::string const &value)
{
  _data_map[key]._values.push_back(value);
}

void
Arguments::set_env(std::string const &key, std::string const &value)
{
  // perform overwrite for now
  _data_map[key]._env_value = value;
}

void
Arguments::show_all_configuration() const
{
  for (auto it : _data_map) {
    std::cout << "name: " + it.first << std::endl;
    std::string msg;
    msg = "args value:";
    for (auto it_data : it.second._values) {
      msg += " " + it_data;
    }
    std::cout << msg << std::endl;
    std::cout << "env value: " + it.second._env_value << std::endl << std::endl;
  }
}

// invoke the function with the args
int
Arguments::invoke()
{
  if (_action) {
    // call the std::function
    return _action();
  } else {
    throw std::runtime_error("no function to invoke");
  }
}

//=========================== ArgumentData class ================================

std::string const &
ArgumentData::env() const noexcept
{
  return _env_value;
}

std::string const &
ArgumentData::at(unsigned index) const
{
  if (index >= _values.size()) {
    throw std::out_of_range("argument not fonud at index: " + std::to_string(index));
  }
  return _values.at(index);
}

std::string const &
ArgumentData::value() const noexcept
{
  if (_values.empty()) {
    // To prevent compiler warning
    static const std::string empty = "";
    return empty;
  }
  return _values.at(0);
}

size_t
ArgumentData::size() const noexcept
{
  return _values.size();
}

bool
ArgumentData::empty() const noexcept
{
  return _values.empty() && _env_value.empty();
}

StringArray::const_iterator
ArgumentData::begin() const noexcept
{
  return _values.begin();
}

StringArray::const_iterator
ArgumentData::end() const noexcept
{
  return _values.end();
}

} // namespace ts
