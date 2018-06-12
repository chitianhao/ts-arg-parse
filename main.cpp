
// for testing

#include "argparse.hpp"
#include <iostream>
#include <any>
#include <string>


int main(int argc, const char **argv)
{
    ts::ArgParser parser(std::string("traffic_shit"), std::string("parser for traffic shit"));
    parser.add_option("--switch", "-s", "some switch", "", "", 2);
    ts::ArgParser &init_command = parser.add_subcommand("init", "initialize traffic shit", "INIT_ENV", "", 1);
    ts::ArgParser &remove_command = parser.add_subcommand("remove", "remove traffic shit", "REMOVE_ENV", "", 0);
    parser.add_option("--arg", "-a", "some arg");

    init_command.add_option("--initswitch", "-i", "some initswitch");
    init_command.add_subcommand("subinit", "sub initialize traffic shit");
    init_command.add_subcommand("subinit2", "sub initialize traffic shit");
    remove_command.add_subcommand("subremove", "sub remove traffic shit");
    remove_command.add_option("--removeswitch", "-i", "some removeswitch");

    ts::ArgParser &sub_init_command = parser.get_subcommand("init").get_subcommand("subinit");
    ts::ArgParser &sub_remove_command = parser.get_subcommand("remove").get_subcommand("subremove");

    parser.parse(argc, argv);

    return 0;
}