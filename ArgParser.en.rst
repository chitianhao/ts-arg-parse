.. Licensed to the Apache Software Foundation (ASF) under one
   or more contributor license agreements.  See the NOTICE file
   distributed with this work for additional information
   regarding copyright ownership.  The ASF licenses this file
   to you under the Apache License, Version 2.0 (the
   "License"); you may not use this file except in compliance
   with the License.  You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing,
   software distributed under the License is distributed on an
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.  See the License for the
   specific language governing permissions and limitations
   under the License.

.. include:: ../../common.defs

.. highlight:: cpp
.. default-domain:: cpp

.. _ArgParser:

ArgParser
*********

Synopsis
++++++++

.. code-block:: cpp

   #include <ts/ArgParser.h>

Description
+++++++++++

:class:`ArgParser` is a powerful and easy-to-use command line Parsing library for ATS.
The program defines what it requires by adding commands and options.
Then :class:`ArgParser` will figure out what is related from the command line.
All parsed arguments and function will be put in a key-value pairs structure
:class:`Arguments` available for users to use.

Usage
+++++

The usage of the ArgParser is straightforward. The user is expected to create an
ArgParser for the program. Commands and options can be added to the parser with details
including ENV variable, arguments expected, etc. After a single method :code:`parse(argv)` is called,
An object containing all information and parsed arguments available to use will be returned.

Create a parser
---------------
The first step to use argparse is to create a parser.
The parser can be created simply by calling:

.. code-block:: cpp

   ts::ArgParser parser;

or initialize with the following arguments: 
*name, help description, environment variable, argument number expected, function*

.. code-block:: cpp

   ts::ArgParser parser("name", "description", "ENV_VAR", 0, &function);

To add the usage for the help message of this program:

.. code-block:: cpp

   parser.add_global_usage("traffic_blabla [--SWITCH]");

Add commands and options
------------------------

We can perform all kinds of operations on the parser which is the :class:`ArgParser` or command which is a :class:`Command`.

To add commands to the program or current command:

.. code-block:: cpp

   parser.add_command("command", "description");
   command1.add_command("command", "description");
   command2.add_command("command", "description", "ENV_VAR", 0);
   command3.add_command("command", "description", "ENV_VAR", 0, &function);

This function call returns the new :class:`Command` instance added.

.. Note::

   The 0 here is the number of arguments we expected. It can be also set to `INFINITE_ARG_N`
   to specify that this command expect all the arguments comes later (infinite).

To add options to the parser or current command:

.. code-block:: cpp

    parser.add_option("--switch", "-s", "switch description");
    command1.add_option("--switch", "-s", "switch description");
    command2.add_option("--switch", "-s", "switch description", "", 0);

This function call returns the new :class:`Option` instance. (0 is also number of arguments expected)

We can also use the following way to add subcommand or option:

.. code-block:: cpp

    command.add_command("init", "description").add_command("subinit", "description");
    command.add_command("init", "description").add_option("-i", "--initoption");

which is equivalent to

.. code-block:: cpp

    ts::ArgParser::Command &init_command = command.add_command("init", "description");
    init_command.add_command("subinit", "description");
    init_command.add_option("-i", "--initoption");

In this case, `subinit` is the subcommand of `init` and `--initoption` is a switch of `init`.


Parsing Arguments
-----------------

:class:`ArgParser` parses arguments through the :code:`parse(argv)` method. This will inspect the command line
and walk through it. A :class:`Arguments` object will be built up from attributes
parsed out of the command line holding key-value pairs all the parsed data and the function.

.. code-block:: cpp

    Arguments args = parser.parse(argv);

Invoke functions
----------------

To invoke the function associated with certain commands, we can perform this by simply calling :code:`invoke()`
from the :class:`Arguments` object returned from the parsing. The function can be lambda.

.. code-block:: cpp

    args.invoke();

Help and Version messages
-------------------------

- Help message will outputed when wrong usage of the program is detected or `--help` option found.

- Version message is defined unified in :code:`ArgParser::version_message()`.

Classes
+++++++

.. class:: Arguments

   :class:`Arguments` holds the parsed arguments and function to invoke.
   It basically contains a function to invoke and a private map holding key-value pairs.
   The key is the command or option name string and the value is the Parsed data object which
   contains the environment variable and arguments that belong to this certain command or option.

   Note: More methods can be added later to get specific type from the arguments.

   .. function:: std::string get_env(std::string const &name)

      Return the ENV variable given the name of command or option.

   .. function:: std::vector<std::string> get(std::string const &name)

      Return the arguments string array given the name of command or option.

   .. function:: std::string get(std::string const &name, unsigned index)

      Return the argument string at the certain index given the name of command or option.
      This can be an easy use to get the arg of command or option that only expect one argument by
      simply calling, for example, :code:`std::string path = get("--path", 0);`.

   .. function:: bool called(std::string const &name)

      Check if certain command or option is called. Return true or false.

   .. function:: void append(std::string const &key, ArgumentData const &value)

      Append key-value pairs to the map in :class:`Arguments`.

   .. function:: void show_all_configuration() const

      Show all the called commands, options and associated arguments

.. class:: ArgParser

   .. function:: Option &add_option(std::string const &name, std::string const &key, std::string const &description)

      Add an option to current command with only *long name* and *short name*. Return The Option object itself.

   .. function:: Option &add_option(std::string const &name, std::string const &key, std::string const &description, std::string const &envvar, unsigned arg_num = 0)

       Add an option to current command with *arguments* and *environment variable*. Return The Option object itself.

   .. function:: Command &add_command(std::string const &cmd_name, std::string const &cmd_description)

      Add a command with only *name* and *description*. Return the new Command object.

   .. function:: Command &add_command(std::string const &cmd_name, std::string const &cmd_description, Function const &f)

      Add a command with only *name* and *description* and *function to invoke*. Return the new Command object.

   .. function:: Command &add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar, unsigned cmd_arg_num)

      Add a command with *name*, *description*, *environment variable* and *number of arguments expected*.
      Return the new Command object.

   .. function:: Command &add_command(std::string const &cmd_name, std::string const &cmd_description, std::string const &cmd_envvar, unsigned cmd_arg_num, std::function<int()> const &f)

      Add a command with *name*, *description*, *environment variable*, *number of arguments expected* and *function to invoke*.
      The function can be passed by reference or be a lambda. It returns the new Command object.

   .. function:: void parse(const char **argv)

      Parse the command line by calling :code:`parser.parse(argv)`. Return the new :class:`Arguments` instance.

   .. function:: void show_parser_info() const

      Show information of all the commands and options added to the parser.

   .. function:: void help_message() const

      Output usage to the console.

   .. function:: void version_message() const

      Output version string to the console.

   .. function:: void add_global_usage(std::string const &usage)

      Add a global_usage for :code:`help_message()`. Example: `traffic_blabla [--SWITCH [ARG]]`.

   .. function:: void require_commands();

      Make the parser require commands. If no command is fonud, output help message.

.. class:: Option

   :class:`Option` is a data struct containing information about an option.

.. code-block:: cpp

   struct Option {
      std::string name;        // long option: --arg
      std::string key;         // short option: -a
      std::string description; // help description
      std::string envvar;      // stored ENV variable
      unsigned arg_num = 0;    // number of argument expected
   };

.. class:: Command

   :class:`Command` is a nested structure of command for :class:`ArgParser`. The :code:`add_option()` and :code:`add_command()`
   are the same with those in :class:`ArgParser`. When :code:`add_command()` is called under certain command, it will be
   added as a subcommand for the current command. For Example, :code:`command1.add_command("command2", "description")`
   will make :code:`command2` a subcommand of :code:`command1`. :code:`require_commands()` is also available within :class:`Command`.

   .. function:: void add_example_usage(std::string const &usage)

      Add an example usage for the command to output in `help_message`.
      For Example: :code:`command.add_example_usage("traffic_blabla init --path=/path/to/file")`.

.. class:: ArgumentData

   :class:`ArgumentData` is a struct containing the parsed Environment variable and commandline arguments.

Example
+++++++

Below is a short example of using the ArgParser. We add some options and some commands to it using different ways.
This program will have such functionality:

- ``--switch``, ``-s`` as a global switch.
- Command ``func`` will call ``function()`` and this command takes 2 argument.
- Command ``func2`` will call ``function2(int num)``.
- Command ``init`` has subcommand ``subinit`` and option ``--path`` which take 1 argument.
- Command ``remove`` has option ``--path`` which takes 1 argument and has ``HOME`` as the environment variable.

.. code-block:: cpp

    #include "ts/ArgParser.h"

    int function() {
        ...
    }

    int function2(int num) {
        ...
    }

    int main (int, const char **argv) {
        ts::ArgParser parser;
        parser.add_global_usage("traffic_blabla [some stuff]");

        parser.add_option("--switch", "-s", "top level switch");
        parser.add_command("func", "some function", "ENV_VAR", 2, &function);
        parser.add_command("func2", "some function2", [&]() { return function2(100); });

        ts::ArgParser::Command &init_command = parser.add_command("init", "initialize");
        init_command.add_option("--path", "-p", "specify the path", "", 1);
        init_command.add_command("subinit", "sub initialize");

        parser.add_command("remove", "remove things").add_option("--path", "-p", "specify the path", "HOME", 1);
        
        ts::Arguments parsed_data = parser.parse(argv);
        parsed_data.invoke();
        ...
    }
