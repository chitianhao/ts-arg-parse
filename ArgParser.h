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

// more than zero arguments
static constexpr unsigned INFINITE_ARG_N = ~0;
// more than one arguments
static constexpr unsigned MORE_THAN_ONE_ARG_N = ~0 - 1;
// some indent magic numbers
static constexpr int INDENT_ONE = 25;
static constexpr int INDENT_TWO = 74;

namespace ts
{
// Some type def
using StringArray = std::vector<std::string>;
using Function    = std::function<int()>;

/// Struct holding both the ENV and String arguments
struct ArgumentData {
  operator bool() const { return is_called; };
  std::string operator[](int x) { return arg_data[x]; }
  std::string env();
  StringArray args();
  std::string at(unsigned index);
  std::string value();
  size_t size();
  bool empty();

  // the arguments stored
  StringArray arg_data;
  // the environment variable
  std::string env_data;
  bool is_called = false;
};

// The class holding all the parsed data after ArgParser::parse()
class Arguments
{
public:
  Arguments();
  ~Arguments();

  ArgumentData get(std::string const &name);

  void append(std::string const &key, ArgumentData const &value);
  // Append value to the arg to the map of key
  void append_arg(std::string const &key, std::string const &value);
  // append env value to the map with key
  void append_env(std::string const &key, std::string const &value);
  // Print all we have in the parsed data to the console
  void show_all_configuration() const;
  /** Invoke the function associated with the parsed command.
      @return The return value of the executed command (int).
  */
  int invoke();

private:
  // A map of all the called parsed args/data
  // Key: "command/option", value: ENV and args
  std::unordered_map<std::string, ArgumentData> _data_map;
  // The function associated. invoke() will call this func
  Function _action;

  friend class ArgParser;
};

// Class of the ArgParser
class ArgParser
{
public:
  // Option structure: e.x. --arg -a
  // Contains all information about certain option(--switch)
  struct Option {
    std::string long_option;   // long option: --arg
    std::string short_option;  // short option: -a
    std::string description;   // help description
    std::string envvar;        // stored ENV variable
    unsigned arg_num;          // number of argument expected
    std::string default_value; // default value of option
    std::string key;           // look-up key
  };

  // Class for commands in a nested way
  class Command
  {
  public:
    // Constructor and destructor
    Command();
    ~Command();
    /** Add an option to current command
        @return The Option object.
    */
    Option &add_option(std::string const &long_option, std::string const &short_option, std::string const &description,
                       std::string const &envvar = "", unsigned arg_num = 0, std::string const &default_value = "",
                       std::string const &key = "");

    /** Two ways of adding a sub-command to current command:
        @return The new sub-command instance.
    */
    Command &add_command(std::string const &cmd_name, std::string const &cmd_description, Function const &f = nullptr,
                         std::string const &key = "");
    Command &add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                         unsigned cmd_arg_num, Function const &f = nullptr, std::string const &key = "");
    /** Add an example usage of current command for the help message
        @return The Command instance for chained calls.
    */
    Command &add_example_usage(std::string const &usage);
    /** Require subcommand/options for this command
        @return The Command instance for chained calls.
    */
    Command &require_commands();
    Command &require_options();

  protected:
    // Main constructor called by add_command()
    Command(std::string const &name, std::string const &description, std::string const &envvar, unsigned arg_num, Function const &f,
            std::string const &key = "");
    // Helper method for add_option to check the validity of option
    void check_option(std::string const &long_option, std::string const &short_option, std::string const &key) const;
    // Helper method for add_command to check the validity of command
    void check_command(std::string const &name, std::string const &key) const;
    // Helper method for ArgParser::show_parser_info
    void show_command_info() const;
    // Helper method for ArgParser::help_message
    void output_command(std::ostream &out, std::string const &prefix) const;
    // Helper method for ArgParser::parse
    bool parse(ArgParser &base, Arguments &ret, StringArray &args);

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
    // look up key
    std::string _key;

    // list of all subcommands of current command
    // Key: command name. Value: Command object
    std::unordered_map<std::string, Command> _subcommand_list;
    // list of all options of current command
    // Key: option name. Value: Option object
    std::unordered_map<std::string, Option> _option_list;
    // Map for fast searching: <short option: long option>
    std::unordered_map<std::string, std::string> _option_map;

    // require command / option for this parser
    bool _command_required = false;
    bool _option_required  = false;

    friend class ArgParser;
  };
  // Base class constructors and destructor
  ArgParser();
  ArgParser(std::string_view name, std::string_view description, std::string_view envvar, unsigned arg_num, Function const &f);
  ~ArgParser();

  /** Add an option to current command with arguments
      @return The Option object.
  */
  Option &add_option(std::string const &long_option, std::string const &short_option, std::string const &description,
                     std::string const &envvar = "", unsigned arg_num = 0, std::string const &default_value = "",
                     std::string const &key = "");

  /** Two ways of adding command to the parser:
      @return The new command instance.
  */
  Command &add_command(std::string const &cmd_name, std::string const &cmd_description, Function const &f = nullptr,
                       std::string const &key = "");
  Command &add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar,
                       unsigned cmd_arg_num, Function const &f = nullptr, std::string const &key = "");

  /** Main parsing function
      @return The Arguments object available for program using
  */
  Arguments parse(const char **argv);
  // Show all information(command, option, envvar ...) of the parser
  void show_parser_info() const;
  // The help & version messages
  void help_message() const;
  void version_message() const;
  // Add the usage to global_usage for help_message(). Something like: traffic_blabla [--SWITCH [ARG]]
  void add_global_usage(std::string const &usage);

  /** Require subcommand/options for this command
      @return The Command instance for chained calls.
  */
  Command &require_commands();
  Command &require_options();

protected:
  // Converted from 'const char **argv' for the use of parsing and help
  StringArray _argv;
  // the top level command object for program use
  Command _top_level_command;
  // global usage of the program
  std::string _global_usage;

  friend class Command;
  friend class Arguments;
};

} // namespace ts
