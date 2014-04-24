CS:GO Weapon Attribute Parser
================

This tool uses Valve's own libraries to parse CS:GO's weapon scripts, and exports the attributes of each weapon to a JSON document.

## Compiling

You need the reverse-engineered plugin SDK from [Allied Modders][allied_mods]- specifically, [hl2sdk-ob-valve][hl2sdk]. You also need an installation of the CS:GO dedicated server. Edit the paths at the top of the Makefile to point to these directories, and you should be good to go.

If you want to build the tests, you will need cppunit installed.

[allied_mods]: http://www.alliedmods.net/
[hl2sdk]: https://hg.alliedmods.net/hl2sdks/hl2sdk-ob-valve/
