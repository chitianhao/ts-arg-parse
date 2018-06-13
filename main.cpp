
// for testing purpose

#include <any>
#include <iostream>
#include <string>
#include "argparse.hpp"

int main(int argc, const char **argv) {
  ts::ArgParser parser(std::string("main"), std::string("parser for main"), "",
                       1);

  parser.add_option("--switch", "-s", "some switch", "", 2);
  ts::ArgParser &init_command =
      parser.add_subcommand("init", "initialize traffic shit", "INIT_ENV", 1);
  ts::ArgParser &remove_command =
      parser.add_subcommand("remove", "remove traffic shit", "REMOVE_ENV", 0);

  init_command.add_option("--initswitch", "-i", "some initswitch");
  init_command.add_subcommand("subinit", "sub initialize traffic shit");

  init_command.add_subcommand("subinit2", "sub initialize traffic shit", "", 2)
      .add_option("--abc", "-x", "some testing shit", "", 1);

  remove_command.add_subcommand("subremove", "sub remove traffic shit");
  remove_command.add_option("--removeswitch", "-i", "some removeswitch");

  parser.parse(argc, argv);

  parser.show_all_configuration();

  return 0;
}