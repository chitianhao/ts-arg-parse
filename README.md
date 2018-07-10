ArgParser
============

Introduction
------------
ArgParser is powerful and easy-to-use command line Parsing for ATS.
The program defines what it requires by adding commands and options.
Then ArgParser will figure out what is related from the command line.
All parsed arguments and function will be put in ParsedArgs available
for users to use.

Create a parser
------------
The first step to use argparse is to create a parser. <br />
The parser can be created simply by calling: <br />
```
ts::ArgParser parser
```
or initialize with some arguments: name, help description, environment variable, argument number expected, function:
```
ts::ArgParser parser("name", "description", "ENV_VAR", 0, &function)
```

To add the usage for the help message of this program:
```
parser.add_global_usage("traffic_blabla [--SWITCH]")
```

Add commands and options
------------
All commands and options are dealt with in the top level command. <br />
So the top level command should be called first to be dealt with:
```
ts::ArgParser::Command &base_command = parser.base_command()
```
Then we can perform all kinds of operations on the top level command.

To add subcommands to the current command:
```
base_command.add_subcommand("command", "description")
base_command.add_subcommand("command", "description", "ENV_VAR", 0)
base_command.add_subcommand("command", "description", "ENV_VAR", 0, &function)
```
This function call returns the new sub-command instance. (0 is number of argument)

To add options to the current command:
```
base_command.add_option("--switch", "-s", "switch description")
base_command.add_option("--switch", "-s", "switch description", "", 0)
```
This function call returns the new Option instance. (0 is number of argument)

We can also use the following way to add subcommand or option:
```
base_command.add_subcommand("init", "description").add_subcommand("subinit", "description")
```
or
```
ts::ArgParser::Command &init_command = base_command.add_subcommand("init", "description")
init_command.add_subcommand("subinit", "description")
```
In this case, subinit is the subcommand of init. Option is added the same way as commands.

Parsing arguments
------------
ArgParser parses arguments through the ``parse()`` method. This will inspect the command line and walk through it.
A ParsedArg object will built up from attributes parsed out of the command line holding key-value pairs all the parsed data and the function.

Invoke functions
------------
To invoke the function associated, we can perform it by simply calling ``invoke()`` method from the ParsedArg object after the parsing. The function can be lambda.

ParsedArgs Class
------------
The ParsedArgs is the class holding the parsed arguments and function to invoke. It basically constains a function to invoke and a private map holding key value pairs. The key is the command or option name string and the value is the Parsed data object which contains the environment variable and arguments that belongs to this certain command or option.

Methods description:

- ``get_env(std::string const &name)`` will return the ENV variable given the name of command or option
- ``get_args(std::string const &name)`` will return the arguments string array given the name of command or option
- ``called(std::string const &name)`` is able to check if certain command or option is called
- ``append(std::string const &key, ParserData const &value)`` is able to append key-value pairs to the map

Help and Version messages
------------
Help message will show up when wrong using of the program is detected or ``--help`` option found. <br />
Version message is defined unified in ArgParser::version_message().

Other Helper methods
------------
There are some helper methods for debugging and understanding.

``ts::ArgParser::show_parser_info()`` is able to show information of all the commands and option we added to the parser.

``ts::ParsedArgs::show_all_configuration()`` is able to show all the called commands, options and associated arguments.

Example
------------
Below is a short example of using the ArgParser. 
We add some switch and some commands to it using different ways.
This program will have such functionality:
- ``--switch``, ``-s`` as a global switch
- Command ``func`` will call ``function()`` and this command takes 2 argument.
- Command ``func2`` will call ``function2(int num)``.
- Command ``init`` has subcommand ``subinit`` and option ``--path`` which take 1 argument.
- Command ``remove`` has option ``--path`` which takes 1 argument and has ``HOME`` as the environment variable.

```
int function() {
    ...
}

int function2(int num) {
    ...
}

int main (int, const char **argv) {
    ts::ArgParser parser;
    parser.add_global_usage("traffic_blabla [some stuff]");
    ts::ArgParser::Command &base_command = parser.base_command();

    base_command.add_option("--switch", "-s", "top level switch");
    base_command.add_subcommand("func", "some function", "", 2, &function);
    base_command.add_subcommand("func2", "some function2", "", 0, [&]() { return function2(100); });

    ts::ArgParser::Command &init_command = base_command.add_subcommand("init", "initialize");
    init_command.add_option("--path", "-p", "specify the path", "", 1);
    init_command.add_subcommand("subinit", "sub initialize");

    base_command.add_subcommand("remove", "remove things").add_option("--path", "-p", "specify the path", "HOME", 1);
    
    ts::ParsedArgs parsed_data = parser.parse(argv);
    parsed_data.invoke();
    ...
}

```

Compilation
------------
After including catch.hpp: ``clang++(or g++) ArgParser.cc test_ArgParser.cc -o test -std=c++17``
