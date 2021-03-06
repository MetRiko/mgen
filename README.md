# mgen - Simple makefile generator

Makefile generator using some flags and "vars" to create makefile easier.
It is crated for Windows but you can edit first function (i.e. GetExePath(...)) for properly work for another system.

## Getting started

Just add mgen folder to PATH string.

## User guide

There are some flags that you can use:

### First use:

1. Create directory _Test_ with simple _any.cpp_ file.
2. Go to path with _Test_ in console.
3. Type `mgen` in console.

Now put _any.cpp_ to any folder (e.g. _Test/Src_) and type `mgen` again.
It will find every single _.cpp_ file in folder with project.
Works with multiple files and folders.

### Assign flags:

Can have only one assigned value: `-S -O -a -c`

### Multiple flags:

Add more than one value to unique strings: `-I -L -l -v -f`

### Help command

For more infomation about flags type...
```
mgen help
```
...or read _help_ file in directory with mgen.

## Additional files

### Commands in one file

You can create _mgen_ file in the same place when the makefile will be created and put there any commands, separated by enters or spaces. When you run mgen with `-c` (clear commands) flag it will ignore searching _mgen_ file.

### Global _vars_ with commands

In directory with mgen just edit _adders_ file and put there some commands.
Separate commands `+VAR_NAME` for create new vars.
You can run all commands by typing `-v` flag with var name (e.g. `-vSFML`).

Example of _adders_ file:

```
+SFML
-lsfml-graphics -lsfml-system -lsfml-window
-ID:\Dev\Libraries\SFML\2.4.2\include
-LD:\Dev\Libraries\SFML\2.4.2\lib
+Boost
-ID:/Dev/CORE/boost_1_65_1
-LD:/Dev/CORE/boost_1_65_1/stage/lib
```

## Commands examples

```
mgen
mgen -Ssrc
mgen -fstd=c++11 -Ssrc -Oobj
mgen -ISFML/include -LSFML/lib -lsfml-window -lsfml-system -lsfml-
```

## Built With

* [Boost](http://www.boost.org/) - to search directories easier
