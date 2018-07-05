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

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <string_view>

// For unspecified-number args
static constexpr unsigned INFINITE_ARG_NUM = ~0;

namespace ts
{
// Some type def
using StringArray = std::vector<std::string>;
using Function    = std::function<int()>;

/// Struct holding both the ENV and String arguments
struct ParserData {
  std::string env_data;
  StringArray arg_data;
};

// The class holding all the parsed data after ArgParser::parse()
class ParsedArgs
{
public:
  ParsedArgs();
  ~ParsedArgs();

  /** Get the related env variable given the name of command/option.
      @return The ENV variable associated with certain command/option.
  */
  std::string get_env(std::string const &name) const;
  /** Get the related arguments given the name of command/option.
      @return The parsed arguments associated with certain command/option.
  */
  StringArray get_args(std::string const &name) const;
  /** Get the argument at certain index given the name of command/option.
      @return The parsed argument associated with certain command/option.
  */
  std::string get_arg(std::string const &name, unsigned index) const;
  /** To check if certain command/option is called
      @return true if called, else false
  */
  bool called(std::string const &name) const;
  // Append key value pairs to the datamap
  void append(std::string const &key, ParserData const &value);
  // Print all we have in the parsed data to the console
  void show_all_configuration() const;
  /** Invoke the function associated with the parsed command.
      @return The return value of the executed command (int).
  */
  int invoke();
  // TODO: Some other method to deal with types of the parsed args
  // get_args_int, get_args_long, get_args_bool ...
  
private:
  // A map of all the called parsed args/data
  // Key: "command/option", value: ENV and args
  std::unordered_map<std::string, ParserData> _data_map;
  // The function associated. invoke() will call this func
  Function _action;
  friend class ArgParser;
};

// Basic container class of the parser
class ArgParser
{
public:
  // Option structure: e.x. --arg -a
  // Contains all information about certain option(--switch)
  struct Option {
    std::string name;        // long option: --arg
    std::string key;         // short option: -a
    std::string description; // help description
    std::string envvar;      // stored ENV variable
    unsigned arg_num = 0;         // number of argument expected
  };

  // Class for commands in a nested way
  class Command
  {
  public:
    // Constructor
    Command();
    Command(std::string const &name, std::string const &description, std::string const &envvar, unsigned arg_num, Function const &f);
    // Desctructor
    ~Command();

    /** Add an option to current command without arguments
        @return The Option object.
    */
    Option &add_option(std::string const &name, std::string const &key, std::string const &description);
    /** Add an option to current command with arguments
        @return The Option object.
    */
    Option &add_option(std::string const &name, std::string const &key, std::string const &description, std::string const &envvar,
                       unsigned arg_num = 0);

    /** Three ways of adding a sub-command to current command:
     * 1. Without arguments.
     * 2. With arguments.
     * 3. With arguments and function
        @return The new sub-command instance.
    */
    Command &add_subcommand(std::string const &cmd_name, std::string const &cmd_description);
    Command &add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                            unsigned cmd_arg_num);
    Command &add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                            unsigned cmd_arg_num, Function const &f);
    // add an example usage of current command for the help message
    void add_example_usage(std::string const &usage);

  protected:
    // Helper method for add_option
    void check_option(std::string const &name, std::string const &key) const;
    // Helper method for add_subcommand
    void check_command(std::string const &name) const;
    // Helper method for ArgParser::show_parser_info
    void show_command_info() const;
    // Helper method for ArgParser::help_message
    void output_command(std::ostream &out, std::string const &prefix) const;
    // Helper method for ArgParser::parse
    void parse(ArgParser &base, ParsedArgs &ret, StringArray &args);

    // The parent of current command
    Command *_parent = nullptr;
    // The command name and help message
    std::string _name;
    std::string _description;

    // Expected argument number
    unsigned _arg_num = 0;
    // Stored Env variable
    std::string _envvar;
    // An example usage can be added for the help message
    std::string _example_usage;
    // Function associated with this command
    Function _f;

    // list of all subcommands of current command
    // Key: command name. Value: Command object
    std::unordered_map<std::string, Command> _subcommand_list;
    // list of all options of current command
    // Key: option name. Value: Option object
    std::unordered_map<std::string, Option> _option_list;

    // Map for fast searching: <short option: long option>
    std::unordered_map<std::string, std::string> _option_map;

  protected:
    friend class ArgParser;
  };
  // Base class constructor and destructor
  ArgParser();
  ArgParser(std::string_view name, std::string_view description, std::string_view envvar, unsigned arg_num, Function const &f);
  ~ArgParser();

  /** Get the top level command object to deal with
      @return The _top_level_command
  */
  Command &top_command();
  /** Main parsing function
      @return The ParsedArgs object available for program using
  */
  ParsedArgs parse(const char **argv);
  // Show all information(command, option, envvar ...) of the parser
  void show_parser_info() const;
  // The help & version messages
  void help_message() const;
  void version_message() const;
  // Add the usage to global_usage for help_message()
  // Example: traffic_blabla [--SWITCH [ARG]]
  void add_global_usage(std::string const &usage);

protected:
  // Converted from 'const char **argv' for the use of parsing and help
  StringArray _argv;
  // the top level command object for program use
  Command _top_level_command;
  // global usage of the program
  std::string _global_usage;

  friend class Command;
  friend class ParsedArgs;
};

} // namespace ts
