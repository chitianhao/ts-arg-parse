ArgParser
============

Introduction
------------
ArgParser is powerful and easy-to-use command line Parsing for ATS.
The program defines what it requires by adding commands and options.
Then ArgParser will figure out what is related from the command line.
All parsed arguments and function will be put in ParsedArgs available
for user to use.

Create a parser
------------
The first step to use argparse is to create a parser. <br />
The parser can be created simply by calling: <br />
```
ts::ArgParser parser
```
or
```
ts::ArgParser parser("name", "description", "ENV_VAR", 0, &function)
```
The arguments required are: name, help description, environment variable, argument number expected, function.

To add the usage for the help message of this program:
```
parser.add_global_usage("traffic_blabla [--SWITCH]")
```

Add commands and options
------------
All commands and options are dealt with in the top level command. <br />
So the top level command should be called first to be dealt with:
```
ts::ArgParser::Command &top_command = parser.top_command()
```
Then we can perform following operations on the top level command.

To add subcommands to the current command:
```
top_command.add_subcommand("command", "description")
top_command.add_subcommand("command", "description", "ENV_VAR", 0)
top_command.add_subcommand("command", "description", "ENV_VAR", 0, &function)
```
This function call returns the new sub-command instance.

To add options to the current command:
```
top_command.add_option("--switch", "-s", "switch description")
top_command.add_option("--switch", "-s", "switch description", "", 0)
```
This function call returns the new Option instance.

We can use the following way to add subcommand or option:
```
top_command.add_subcommand("init", "description").add_subcommand("subinit", "description")
```
or
```
ts::ArgParser::Command &init_command = top_command.add_subcommand("init", "description")
init_command.add_subcommand("subinit", "description")
```
In this case, subinit is the subcommand of init.

Parsing arguments
------------
ArgParser parses arguments through the ``parse()`` method. This will inspect the command line and walk through it.
A ParsedArg object will be returned holding key value pairs all the parsed data and the function.

Invoke functions
------------
To invoke the function associated, we can perform it by simply calling ``invoke()`` method from the ParsedArg object after the parsing.

Helper methods
------------
There are some helper method for debugging and understanding.

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
- Command ``remove`` has option ``--path`` which takes 1 argument and has ``HOME`` as environment variable.

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
    ts::ArgParser::Command &top_command = parser.top_command();

    top_command.add_option("--switch", "-s", "top level switch");
    top_command.add_subcommand("func", "some function", "", 2, &function);
    top_command.add_subcommand("func2", "some function2", "", 0, [&]() { return function2(100); });

    ts::ArgParser::Command &init_command = top_command.add_subcommand("init", "initialize");
    init_command.add_option("--path", "-p", "specify the path", "", 1);
    init_command.add_subcommand("subinit", "sub initialize");

    top_command.add_subcommand("remove", "remove things").add_option("--path", "-p", "specify the path", "HOME", 1);
    
    ts::ParsedArgs parsed_data = parser.parse(argv);
    parsed_data.invoke();
    ...
}

```

Compilation
------------
 ``clang++(or g++) argparse.cpp main.cpp -o main -std=c++17``
