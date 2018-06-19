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

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define RESERVE_LIST_SIZE 128

namespace ts {

struct ParserData {
  std::string env_data;
  std::vector<std::string> arg_data;
};

class ParsedArgs {
 public:
  // TODO: some method to deal with the parsed args
  // std::string get_env(std::string const &name) {
  //   if (_data_map.find(name) != _data_map.end()) {
  //     return _data_map[name].env_data;
  //   } else {
  //     return "";
  //   }
  // }
  // std::vector<std::string> get(std::string const &name) {
  //   if (_data_map.find(name) != _data_map.end()) {
  //     return _data_map[name].arg_data;
  //   } else {
  //     return std::vector<std::string>();
  //   }
  // }

  // show what we have in the parsed data
  void show_all_configuration() {
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

  // "command/option": command data
  // key value pared stored only when they are called
  std::unordered_map<std::string, ParserData> _data_map;
};

// base class of the parser
class ArgParser {
  typedef ArgParser self;  ///< Self reference type.
 public:
  // two way of construction
  ArgParser();
  ArgParser(std::string const &name, std::string const &description,
            std::string const &envvar, int arg_num);
  ~ArgParser();

  // option class: e.x. --arg -a
  class Option {
    typedef Option self;

   public:
    Option();
    Option(std::string const &opt_name, std::string const &opt_key,
           std::string const &opt_description);
    Option(std::string const &opt_name, std::string const &opt_key,
           std::string const &opt_description, std::string const &opt_envvar,
           int opt_arg_num);
    ~Option();

   protected:
    void show_option_info();
    std::string _opt_name;         // --arg
    std::string _opt_key;          // -a
    std::string _opt_description;  // description
    int _opt_arg_num = 0;          // number of argument expected
    std::string _opt_envvar;       // stored envvar

    friend class ArgParser;
    friend class Command;
  };

  class Command {
    typedef Command self;

   public:
    // constructor
    Command();
    Command(std::string const &name, std::string const &description);
    Command(std::string const &name, std::string const &description,
            std::string const &envvar, int arg_num);

    // desctructor
    ~Command();

    // option without arg
    Option &add_option(std::string const &name, std::string const &key,
                       std::string const &description);
    // option with arg
    Option &add_option(std::string const &name, std::string const &key,
                       std::string const &description,
                       std::string const &envvar, int arg_num = 0);

    // command with nothing
    Command &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description);
    // command with arg, ENV
    Command &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description,
                            std::string const &cmd_envvar, int cmd_arg_num);

    // get certain command from the command list to deal with
    Command &get_subcommand(std::string const &cmd_name);

   protected:
    // some helper methods
    void check_option(std::string const &name, std::string const &key);
    void check_command(std::string const &name);
    void show_command_info();
    bool append_data(ArgParser &me, ParsedArgs &ret,
                     std::vector<std::string> &args);
    void append_option_data(ArgParser &base, ParsedArgs &ret,
                            std::vector<std::string> &args);
    // the parent of current command
    Command *_parent = nullptr;
    // information of the command
    std::string _name;
    std::string _description;
    // list of all subcommands of current command
    // key: command name
    std::unordered_map<std::string, Command> _subcommand_list;
    // list of all options of current command
    // key: option name
    std::unordered_map<std::string, Option> _option_list;
    // long option and short option map for fast searching
    // key: short option, value: long option
    std::unordered_map<std::string, std::string> _option_map;
    // expected argument
    int _arg_num = 0;
    // stored envvar
    std::string _envvar;

   protected:
    friend class ArgParser;
    friend class Option;
  };

  // return the _top_level_command to deal with
  Command &top_command();
  // main parsing function
  ParsedArgs parse(const char **argv);
  // show all information of the parser
  void show_parser_info();
  // help & version
  void help_message();
  void version_message();

 protected:
  std::vector<std::string> _argv;  // for the use of parsing and help
  Command _top_level_command;

  friend class Option;
  friend class Command;
};

}  // namespace ts