/** @file

  Unit test for ArgParser

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

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
// #include "ts/ArgParser.h"
#include "ArgParser.h"

#include <iostream>

TEST_CASE("Parsing test", "[parse]")
{
  // initialize and construct the parser
  ts::ArgParser parser;
  parser.add_global_usage("traffic_blabla [--SWITCH]");
  ts::ArgParser::Command &top_command = parser.top_command();

  top_command.add_option("--globalx", "-x", "global switch x", "", 2);
  top_command.add_option("--globaly", "-y", "global switch y");

  ts::ArgParser::Command &init_command   = top_command.add_subcommand("init", "initialize traffic blabla", "HOME", 1, nullptr);
  ts::ArgParser::Command &remove_command = top_command.add_subcommand("remove", "remove traffic blabla");

  init_command.add_option("--initoption", "-i", "init option");
  init_command.add_option("--initoption2", "-j", "init2 option", "", 1);
  init_command.add_subcommand("subinit", "sub initialize traffic blabla", "", 2, nullptr)
    .add_option("--subinitopt", "-s", "sub init option");

  remove_command.add_subcommand("subremove", "sub remove traffic blabla").add_subcommand("subsubremove", "sub sub remove");

  ts::ParsedArgs parsed_data;

  const char *argv1[] = {"traffic_blabla", "init", "a", "--initoption", "--globalx", "x", "y", NULL};

  parsed_data = parser.parse(argv1);
  REQUIRE(parsed_data.called("init") == true);
  REQUIRE(parsed_data.called("--globalx") == true);
  REQUIRE(parsed_data.called("--initoption") == true);
  REQUIRE(parsed_data.called("a") == false);
  REQUIRE(parsed_data.get_args("init").size() == 1);
  REQUIRE(parsed_data.get_args("init")[0] == "a");
  REQUIRE(parsed_data.get_args("--globalx").size() == 2);
  REQUIRE(parsed_data.get_args("--globalx")[0] == "x");
  REQUIRE(parsed_data.get_args("--globalx")[1] == "y");

  const char *argv2[] = {"traffic_blabla", "init", "subinit", "a", "b", "--initoption2=abc", "--subinitopt", "-y", NULL};

  parsed_data = parser.parse(argv2);
  REQUIRE(parsed_data.called("init") == true);
  REQUIRE(parsed_data.called("--subinitopt") == true);
  REQUIRE(parsed_data.called("--globaly") == true);
  REQUIRE(parsed_data.get_args("subinit").size() == 2);
  REQUIRE(parsed_data.get_args("--initoption2").size() == 1);
  REQUIRE(parsed_data.get_args("--initoption2")[0] == "abc");

  const char *argv3[] = {"traffic_blabla", "-x", "abc", "xyz", "remove", "subremove", "subsubremove", NULL};

  parsed_data = parser.parse(argv3);
  REQUIRE(parsed_data.called("remove") == true);
  REQUIRE(parsed_data.called("subremove") == true);
  REQUIRE(parsed_data.called("subsubremove") == true);
  REQUIRE(parsed_data.get_args("--globalx").size() == 2);
}

int
test_method_1()
{
  return 0;
}

int
test_method_2(int num)
{
  if (num == 1) {
    return 1;
  }
  return 2;
}

TEST_CASE("Invoke test", "[invoke]")
{
  ts::ArgParser parser;
  int num = 1;

  parser.add_global_usage("traffic_blabla [--SWITCH]");
  ts::ArgParser::Command &top_command = parser.top_command();
  // function by reference
  top_command.add_subcommand("func", "some test function 1", "", 0, &test_method_1);
  // lambda
  top_command.add_subcommand("func2", "some test function 2", "", 0, [&]() { return test_method_2(num); });

  ts::ParsedArgs parsed_data;

  const char *argv1[] = {"traffic_blabla", "func", NULL};

  parsed_data = parser.parse(argv1);
  REQUIRE(parsed_data.invoke() == 0);

  const char *argv2[] = {"traffic_blabla", "func2", NULL};

  parsed_data = parser.parse(argv2);
  REQUIRE(parsed_data.invoke() == 1);
  num = 3;
  REQUIRE(parsed_data.invoke() == 2);
}