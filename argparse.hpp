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

// steps: 1. store 2. parse 3. invoke 4. usage & help

#pragma once

#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#define RESERVE_LIST_SIZE 128

namespace ts {

// option class: -a --arg
class Option {
 public:
  Option();
  Option(std::string const &opt_name, std::string const &opt_key,
         std::string const &opt_description);
  Option(std::string const &opt_name, std::string const &opt_key,
         std::string const &opt_description, std::string const &opt_envvar,
         int opt_arg_num);
  ~Option();

  std::string get_envvar();
  std::vector<std::string> get_data();

 private:
  void show_option_info();
  std::string _opt_name;  // --arg
  std::string _opt_key;   // -a
  std::string _opt_description;
  int _opt_arg_num = 0;     // number of argument expected
  std::string _opt_envvar;  // stored envvar

  // data stored after parsing
  std::vector<std::string> _opt_data;

  friend class ArgParser;
};

// argparser class (nested set structure)
class ArgParser {
  typedef ArgParser self;  ///< Self reference type.

 public:
  // using AryAction = std::function<int(int argc, const char *argv[])>;
  // using NullaryAction = std::function<int()>;
  using Function = std::function<int(int argc, const char **argv)>;

  // constructor
  ArgParser();
  ArgParser(std::string const &name, std::string const &description);
  ArgParser(std::string const &name, std::string const &description,
            std::string const &envvar, int arg_num);

  // a constructor with action(function)
  ArgParser(std::string const &name, std::string const &description,
            Function const &f);
  ArgParser(std::string const &name, std::string const &description,
            std::string const &envvar, int arg_num, Function const &f);

  // desctructor
  ~ArgParser();

  // first step is parse
  void parse(int argc = 0, const char **argv = nullptr);
  // second step is invoke
  int invoke(int argc = 0, const char **argv = nullptr);

  // option without arg
  Option &add_option(std::string const &name, std::string const &key,
                     std::string const &description);
  // option with arg
  Option &add_option(std::string const &name, std::string const &key,
                     std::string const &description, std::string const &envvar,
                     int arg_num = 0);

  // command without arg
  ArgParser &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description);
  // command with arg
  ArgParser &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description,
                            std::string const &cmd_envvar, int cmd_arg_num);
  // command without arg with function
  ArgParser &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description,
                            Function const &f);
  // command without arg and function
  ArgParser &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description,
                            std::string const &cmd_envvar, int cmd_arg_num,
                            Function const &f);
  // get certain command from the command list to deal with
  ArgParser &get_subcommand(std::string const &cmd_name);

  // get the environment variable
  std::string get_envvar();
  // get the data
  std::vector<std::string> get_data();

  void show_all_configuration();

  void help(const std::vector<std::string> &args = std::vector<std::string>());
  void version_message();

 protected:
  std::vector<std::string> _argv;
  void check_option(std::string const &name, std::string const &key);
  void check_command(std::string const &name);
  // void append_cmd_data(std::vector<ArgParser> &list,
  //                      std::vector<std::string> &args);
  // void append_option_data(std::vector<Option> &list,
  //                         std::vector<std::string> &args);
  void append_parser_data(std::vector<std::string> &args);
  void append_option_data(std::vector<std::string> &args);
  void show_parser_info();
  // the parent of current parser
  ArgParser *_parent = nullptr;
  // information of the parser
  std::string _name;
  std::string _description;
  // list of all subcommands of current parser
  std::vector<ArgParser> _subcommand_list;
  // list of all options of current parser
  std::vector<Option> _option_list;
  // expected argument
  int _arg_num = 0;
  // stored envvar
  std::string _envvar;
  // function that should be invoked
  Function _func;

  // data stored after parsing
  std::vector<std::string> _data;

  friend class Option;
};  // Class ArgParser

}  // namespace ts
