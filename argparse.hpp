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

namespace ts {

// // Attrib can be any type!
// class Attrib
// {
// public:
//   // constructors and destructors
//   Attrib(const char *s) : _data(strdup(s)), _size(strlen(s)) {}
//   Attrib(const char *s, std::size_t size) : _data(strdup(s)), _size(size) {}
//   Attrib(const std::string &s) : _data(strdup(s.c_str())), _size(s.size()) {}
//   ~Attrib() { delete _data; };

//   // all types
//   int get_int() { return atoi(_data); }
//   int get_double() { return atof(_data); }
//   // int get_int64_t() { return ink_atoi64(_data); }
//   bool get_bool() { return (!strcmp(_data, "t") || !strcmp(_data, "T") ||
//   !strcmp(_data, "true")) ? true : false; } std::string get_string() { return
//   std::string(_data); } char *get_char_ptr() { return strdup(_data); }

//   // get the size of the Attrib
//   std::size_t get_size() { return _size; }

// private:
//   char *_data = nullptr;
//   std::size_t _size = 0;
// };

// option class: -a --arg
class Option {
 public:
  Option();
  Option(std::string const &opt_name, std::string const &opt_key,
         std::string const &opt_description);
  Option(std::string const &opt_name, std::string const &opt_key,
         std::string const &opt_description, std::string const &opt_envvar,
         std::string const &opt_arg_type, int opt_arg_num);
  ~Option();

  // TODO
  std::any get_envvar(std::string const &type);

 private:
  std::string _opt_name;  // --arg
  std::string _opt_key;   // -a
  std::string _opt_description;
  std::string _opt_arg_type;
  int _opt_arg_num;  // number of argument expected
  std::string _opt_envvar;
  /**
  The data is a list of any type. It will be filled after parse()
  */
  std::any _opt_data;

  friend class ArgParser;
};

// base container for argparser
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
            std::string const &envvar, std::string const &arg_type,
            int arg_num);

  // a constructor with action(function)
  ArgParser(std::string const &name, std::string const &description,
            Function const &f);
  ArgParser(std::string const &name, std::string const &description,
            std::string const &envvar, std::string const &arg_type, int arg_num,
            Function const &f);

  // desctructor
  ~ArgParser();

  void append_cmd_data(std::vector<ArgParser> &list,
                       const std::vector<std::string> &args);
  void append_option_data(std::vector<Option> &list,
                          const std::vector<std::string> &args);
  // first step is parse
  void parse(int argc, const char **argv);
  // second step is invoke
  int invoke(int argc, const char **argv);

  // option with arg
  Option &add_option(std::string const &name, std::string const &key,
                     std::string const &description, std::string const &envvar,
                     std::string const &arg_type, int arg_num = 0);
  // option without arg
  Option &add_option(std::string const &name, std::string const &key,
                     std::string const &description);
  // command with arg
  ArgParser &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description,
                            std::string const &cmd_envvar,
                            std::string const &cmd_arg_type,
                            int cmd_arg_num = 0);
  // command without arg
  ArgParser &add_subcommand(std::string const &cmd_name,
                            std::string const &cmd_description);
  // get certain command from the command list to deal with
  ArgParser &get_subcommand(std::string const &cmd_name);

  // TODO
  std::any &get_envvar(std::string const &type);

  void help();
  void version_message();

 protected:
  // the parent of current parser
  ArgParser *_parent = nullptr;
  // information of the parser
  std::string _name;
  std::string _description;
  // list of all options of current parser
  std::vector<Option> _option_list;
  // list of all subcommands of current parser
  std::vector<ArgParser> _subcommand_list;
  // expected argument
  std::string _arg_type;
  int _arg_num;
  // function that should be invoked
  Function _func;

  std::string _envvar;

  /**
  The data is a list of any type. It will be filled after parse()
  */
  std::any _data;

  friend class Option;
};  // Class ArgParser

}  // namespace ts
