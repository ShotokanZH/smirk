# *SMIRK* - *SM*irk *I* a *R*oot*K*it 🔥🥔🔥

a simple userland rootkit that:
- link himself on *ld.preload.so* and try to stay there
- spawn shell to the people
- hide himself hijacking syscalls

#### Designed and built with all the Love❤️ in the World🌍 by [Roberto Bindi](https://github.com/ShotokanZH) & [Gianluca Pericoli](https://github.com/gpericol)

## Compile
```
make
```

## Install
```
LS_PRELOAD=smirk.so ls
```

## Uninstall
```
touch /dev/shm/.smirkkill
```

## How does it work
...

## To Do
- aggiungere installer
- accept4
- blacklist porte
- spawn shell locale se uid = 0 con unixfile
- install unix socket per uuid = 0

## References
[Linux man pages](https://linux.die.net/man/)

[h0mbre's article](https://h0mbre.github.io/Learn-C-By-Creating-A-Rootkit/)

## LICENCE

This program is free software; you can redistribute it and/or modify it under the terms of the [GNU General Public License](https://www.gnu.org/licenses/gpl-3.0.html) as published by the Free Software Foundation; either version 3 of the License, or(at your option) any later version.