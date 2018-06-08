
// for testing

#include "argparse.hpp"
#include <iostream>
#include <any>
#include <string>
// #include <varient>


int main()
{
    std::any a = "1";
    std::cout << std::any_cast<double>(a) << std::endl;

    // ts::Attrib attrib1("abc");
    // std::cout << attrib1.get_string() << std::endl;
    // ts::Attrib attrib2("1");
    // std::cout << attrib2.get_int() << std::endl;

    // ts::ArgParser parser(std::string("traffic_shit"), std::string("parser for traffic shit"));
    // parser.add_option("switch", "s", "some switch");

    // // must by reference
    // ts::ArgParser &commandget = parser.add_subcommand("init", "initialize stuff1");

    return 0;
}