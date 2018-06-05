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

/*
  need:
  constructor of function
  function type
  usage method
  help message & method
  invoke and structure storing stuff after the parse
 */

#pragma once

#include <string>
#include <vector>

namespace ts
{

// base container for argparser
class ArgParser
{
  typedef ArgParser self; ///< Self reference type.

public:
  // option class: -a --arg
  class Option
  {
  public:
    Option();
    Option(std::string const &name, std::string const &key, std::string const &description, std::string const &arg_type, int arg_num = 0);
    ~Option();

  private:
    std::string _name;
    std::string _key;
    std::string _description;
    std::string _arg_type;
    int _arg_num;
  }; // Class Option

  // command class
  class Command
  {
  public:
    // default constructor
    Command();
    // constructor with arg
    Command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_arg_type, int cmd_arg_num = 0);
    ~Command();

    // TODO constructor with function

    // option with arg
    Option &add_suboption(std::string const &name, std::string const &key, std::string const &description, std::string const &arg_type, int arg_num = 0);
    // option without arg
    Option &add_suboption(std::string const &name, std::string const &key, std::string const &description);
    // command with arg
    Command &add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_arg_type, int cmd_arg_num = 0);
    // command without arg
    Command &add_subcommand(std::string const &cmd_name, std::string const &cmd_description);

  protected:
    std::string _cmd_name;
    // command description
    std::string _cmd_description;
    // how many arguments expected
    int _cmd_arg_num = 0;
    // argument type
    std::string _cmd_arg_type;
    // option of this specified command
    std::vector<Option> _cmd_option_list;
    // list of subcommands
    std::vector<Command> _subcommand_list;

  }; // class Command

  // constructor
  ArgParser();
  ArgParser(std::string const &description);
  // desctructor
  ~ArgParser();

  // option with arg
  Option &add_option(std::string const &name, std::string const &key, std::string const &description, std::string const &arg_type, int arg_num = 0);
  // option without arg
  Option &add_option(std::string const &name, std::string const &key, std::string const &description);
  // command with arg
  Command &add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_arg_type, int cmd_arg_num = 0);
  // command without arg
  Command &add_command(std::string const &cmd_name, std::string const &cmd_description);

protected:
  std::string _description;
  std::vector<Option> _option_list;
  std::vector<Command> _command_list;
}; // Class ArgParser

} // namespace ts
