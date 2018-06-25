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

#define INFINITE_ARG_NUM -1

namespace ts
{
using StringArray = std::vector<std::string>;

struct ParserData {
  std::string env_data;
  StringArray arg_data;
};

// The class holding the stuff after parsing
class ParsedArgs
{
public:
  ParsedArgs();
  ~ParsedArgs();
  // get the related env variable given the name of command/option
  std::string get_env(std::string const &name) const;
  // get the related arguments given the name of command/option
  StringArray get_args(std::string const &name) const;
  // see if certain command/option is called
  bool called(std::string const &name) const;
  // append stuff to the datamap
  void append(std::string const &key, ParserData const &value);
  // TODO: some other method to deal with types of the parsed args

  // show what we have in the parsed data
  void show_all_configuration() const;

private:
  // a map of all the called parsed args/data
  // key: "command/option", value: ENV and args
  std::unordered_map<std::string, ParserData> _data_map;
  friend class ArgParser;
};

// basic container class of the parser
class ArgParser
{
  typedef ArgParser self; ///< Self reference type.
public:
  // option class: e.x. --arg -a
  struct Option {
    std::string _opt_name;        // --arg
    std::string _opt_key;         // -a
    std::string _opt_description; // help description
    std::string _opt_envvar;      // stored envvar
    int _opt_arg_num = 0;         // number of argument expected
  };

  // class for commands in a nested way
  class Command
  {
    typedef Command self;

  public:
    // constructor
    Command();
    Command(std::string const &name, std::string const &description);
    Command(std::string const &name, std::string const &description, std::string const &envvar, int arg_num);

    // desctructor
    ~Command();

    // option without arg
    Option &add_option(std::string const &name, std::string const &key, std::string const &description);
    // option with arg
    Option &add_option(std::string const &name, std::string const &key, std::string const &description, std::string const &envvar,
                       int arg_num = 0);

    // command with nothing
    Command &add_subcommand(std::string const &cmd_name, std::string const &cmd_description);
    // command with arg, ENV
    Command &add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                            int cmd_arg_num);

    // get certain command from the command list to deal with
    Command &get_subcommand(std::string const &cmd_name);
    void add_example_usage(std::string const &usage);

  protected:
    // some helper methods
    void check_option(std::string const &name, std::string const &key) const;
    void check_command(std::string const &name) const;
    void show_command_info() const;
    void parse(ArgParser &base, ParsedArgs &ret, StringArray &args);
    void append_data(ArgParser &me, ParsedArgs &ret, StringArray &args) const;
    void output_command(std::ostream &out, std::string const &prefix) const;

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
    // <short option: long option> map for fast searching
    std::unordered_map<std::string, std::string> _option_map;
    // expected argument
    int _arg_num = 0;
    // stored envvar
    std::string _envvar;
    // an example usage can be added for the help message
    std::string _example_usage;

  protected:
    friend class ArgParser;
  };

  ArgParser();
  ArgParser(std::string const &name, std::string const &description, std::string const &envvar, int arg_num);
  ~ArgParser();
  // return the _top_level_command to deal with
  Command &top_command();
  // main parsing function
  ParsedArgs parse(const char **argv);
  // show all information of the parser
  void show_parser_info() const;
  // help & version message
  void help_message() const;
  void version_message() const;
  // add the usage to global_usage for help_message()
  void add_global_usage(std::string const &usage);

protected:
  StringArray _argv; // for the use of parsing and help
  Command _top_level_command;
  std::string _global_usage;

  friend class Command;
  friend class ParsedArgs;
};

} // namespace ts
