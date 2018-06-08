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
#include <sstream>

namespace ts
{

// constructor and destructor
ts::Option::Option() {}

ts::Option::~Option() {}

ts::Option::Option(std::string const &opt_name, std::string const &opt_key, std::string const &opt_description)
    : _opt_name(opt_name), _opt_key(opt_key), _opt_description(opt_description) {}

ts::Option::Option(std::string const &opt_name, std::string const &opt_key, std::string const &opt_description, std::string const &opt_envvar)
    : _opt_name(opt_name), _opt_key(opt_key), _opt_description(opt_description), _opt_envvar(opt_envvar) {}

ts::Option::Option(std::string const &opt_name, std::string const &opt_key, std::string const &opt_description, std::string const &opt_arg_type, int opt_arg_num)
    : _opt_name(opt_name), _opt_key(opt_key), _opt_description(opt_description), _opt_arg_type(opt_arg_type), _opt_arg_num(opt_arg_num) {}

// constructor and destructor
ArgParser::ArgParser() {}

ArgParser::~ArgParser() {}

ArgParser::ArgParser(std::string const &name, std::string const &description)
    : _name(name), _description(description) {}

ArgParser::ArgParser(std::string const &name, std::string const &description, std::string const &arg_type, int arg_num)
    : _name(name), _description(description), _arg_type(arg_type), _arg_num(arg_num) {}

ArgParser::ArgParser(std::string const &name, std::string const &description, Function const &f)
    : _name(name), _description(description)
{
    _func = f;
}

ArgParser::ArgParser(std::string const &name, std::string const &description, std::string const &arg_type, int arg_num, Function const &f)
    : _name(name), _description(description), _arg_type(arg_type), _arg_num(arg_num)
{
    _func = f;
}

// option handling
Option &ArgParser::add_option(std::string const &name, std::string const &key, std::string const &description, std::string const &arg_type, int arg_num)
{
    _option_list.push_back(Option(name, key, description, arg_type, arg_num));
    return _option_list.back();
}

Option &ArgParser::add_option(std::string const &name, std::string const &key, std::string const &description)
{
    _option_list.push_back(Option(name, key, description));
    return _option_list.back();
}

ArgParser &ArgParser::add_subcommand(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_arg_type, int cmd_arg_num)
{
    ArgParser parser = ArgParser(cmd_name, cmd_description, cmd_arg_type, cmd_arg_num);
    parser._parent = this;
    _subcommand_list.push_back(parser);
    return _subcommand_list.back();
}

ArgParser &ArgParser::add_subcommand(std::string const &cmd_name, std::string const &cmd_description)
{
    ArgParser parser = ArgParser(cmd_name, cmd_description, "", 0);
    parser._parent = this;
    _subcommand_list.push_back(parser);
    return _subcommand_list.back();
}

ArgParser &ArgParser::get_subcommand(std::string const &cmd_name)
{
    for (int i = 0; i < _subcommand_list.size(); i++)
    {
        if (_subcommand_list[i]._name == cmd_name)
        {
            return _subcommand_list[i];
        }
    }
    std::ostringstream s;
    s << "Command " << cmd_name << " not found";
    exit(1);
}

void ArgParser::parse(int argc, const char **argv)
{
}

int ArgParser::invoke(int argc, const char **argv)
{
    return _func(argc, argv);
}

// for temporary testing
void ArgParser::help()
{
    std::cout << "help!" << std::endl;
}

} // namespace ts
