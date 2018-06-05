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

namespace ts
{
// constructor and destructor
ArgParser::ArgParser() {}
ArgParser::~ArgParser() {}
ArgParser::ArgParser(std::string const &description)
{
    _description = description;
}

ArgParser::Option::Option() {}
ArgParser::Option::~Option() {}
ArgParser::Option::Option(std::string const &name, std::string const &key, std::string const &description, std::string const &arg_type, int arg_num)
{
    _name = name;
    _key = key;
    _description = description;
    _arg_type = arg_type;
    _arg_num = arg_num;
}

// option handling
ArgParser::Option &ArgParser::add_option(std::string const &name, std::string const &key, std::string const &description, std::string const &arg_type, int arg_num)
{
    _option_list.push_back(Option(name, key, description, arg_type, arg_num));
    return _option_list.back();
}

ArgParser::Option &ArgParser::add_option(std::string const &name, std::string const &key, std::string const &description)
{
    _option_list.push_back(Option(name, key, description, "", 0));
    return _option_list.back();
}

ArgParser::Command &ArgParser::add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_arg_type, int cmd_arg_num)
{
    _command_list.push_back(Command(cmd_name, cmd_description, cmd_arg_type, cmd_arg_num));
    return _command_list.back();
}

ArgParser::Command &ArgParser::add_command(std::string const &cmd_name, std::string const &cmd_description)
{
    _command_list.push_back(Command(cmd_name, cmd_description, "", 0));
    return _command_list.back();
}

ArgParser::Command::Command() {}
ArgParser::Command::~Command() {}
ArgParser::Command::Command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_arg_type, int cmd_arg_num)
{
    _cmd_name = cmd_name;
    _cmd_description = cmd_description;
    _cmd_arg_type = cmd_arg_type;
    _cmd_arg_num = cmd_arg_num;
}

// option handling
ArgParser::Option &ArgParser::Command::add_suboption(std::string const &name, std::string const &key, std::string const &description, std::string const &arg_type, int arg_num)
{
    _cmd_option_list.push_back(Option(name, key, description, arg_type, arg_num));
    return _cmd_option_list.back();
}

ArgParser::Option &ArgParser::Command::add_suboption(std::string const &name, std::string const &key, std::string const &description)
{
    _cmd_option_list.push_back(Option(name, key, description, "", 0));
    return _cmd_option_list.back();
}

ArgParser::Command &ArgParser::Command::add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_arg_type, int cmd_arg_num)
{
    _subcommand_list.push_back(Command(cmd_name, cmd_description, cmd_arg_type, cmd_arg_num));
    return _subcommand_list.back();
}

ArgParser::Command &ArgParser::Command::add_subcommand(std::string const &cmd_name, std::string const &cmd_description)
{
    _subcommand_list.push_back(Command(cmd_name, cmd_description, "", 0));
    return _subcommand_list.back();
}

} // namespace ts
