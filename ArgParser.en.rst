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

:class:`ArgParser` is powerful and easy-to-use command line Parsing for ATS.
The program defines what it requires by adding commands and options.
Then :class:`ArgParser` will figure out what is related from the command line.
All parsed arguments and function will be put in a key value pairs structure
:class:`Arguments` available for users to use.

Usage
+++++

The usage of the ArgParser is straightforward. The user is expected to create an
ArgParser for the program. Commands and options can be added to the parser with details
including ENV variable, arguments expected, etc. After a single method `parse()` is called,
An object containing all useful information available for future use will be returned.

Create a parser
---------------
The first step to use argparse is to create a parser.
The parser can be created simply by calling:

.. code-block:: cpp

   ts::ArgParser parser;

or initialize with the following arguments: 
name, help description, environment variable, argument number expected, function

.. code-block:: cpp

   ts::ArgParser parser("name", "description", "ENV_VAR", 0, &function);

To add the usage for the help message of this program:

.. code-block:: cpp

   parser.add_global_usage("traffic_blabla [--SWITCH]");

Add commands and options
------------------------

We can perform all kinds of operations on the parser which is itself or command which is a :class:`Command`.

To add commands to the program or current command:

.. code-block:: cpp
   parser.add_command("command", "description");
   command1.add_command("command", "description");
   command2.add_command("command", "description", "ENV_VAR", 0);
   command3.add_command("command", "description", "ENV_VAR", 0, &function);

This function call returns the new :class:`Command` instance added. (0 is number of argument)

.. Note::

   The 0 here is the number of arguments we expected. It can be also set to `INFINITE_ARG_NUM`
   to specify that this command expect all the arguments comes later (infinite).

To add options to the parser or current command:

.. code-block:: cpp
    parser.add_option("--switch", "-s", "switch description");
    command1.add_option("--switch", "-s", "switch description");
    command2.add_option("--switch", "-s", "switch description", "", 0);

This function call returns the new :class:`Option` instance. (0 is also number of arguments)

We can also use the following way to add subcommand or option:

.. code-block:: cpp

    command.add_command("init", "description").add_command("subinit", "description");

which is equivalent to

.. code-block:: cpp

    ts::ArgParser::Command &init_command = command.add_command("init", "description");
    init_command.add_command("subinit", "description");

In this case, subinit is the subcommand of init.

Parsing Arguments
-----------------

ArgParser parses arguments through the `parse()` method. This will inspect the command line
and walk through it. A :class:`Arguments` object will built up from attributes
parsed out of the command line holding key-value pairs all the parsed data and the function.

.. code-block:: cpp

    Arguments args = parser.parse();

Invoke functions
----------------

To invoke the function associated, we can perform it by simply calling `invoke()` method from the
:class:`Arguments` object returned from the parsing. The function can be lambda.

.. code-block:: cpp

    args.invoke();
