# **SMIRK** - **SM**irk **I**s a **R**oot**K**it 🔥🥔🔥

...A simple userland rootkit that:
- links itself on *ld.preload.so* and tries to stay there
- spawns shells but only for the righteous
- hides itself hijacking syscalls

#### Designed and built with all the Love❤️ in the World🌍 by [Roberto Bindi](https://github.com/ShotokanZH) & [Gianluca Pericoli](https://github.com/gpericol)

## Compile
```bash
$ make
```

## Install
```bash
# LD_PRELOAD=smirk.so id
```

## Uninstall (if killswitch is activated)
```bash
$ touch /dev/shm/.smirkkill
```

## How does it work
The shared library, once installed, will hook the following functions:
### open
Used to hide specific files, i.e. files prefixed with a "magix prefix" (default: `.smirk`) or network files.

Hooked functions for this scope are:
- `open`
- `fopen`
- `fopen64`
### ioctl
Any file with `FS_IMMUTABLE_FL | FS_APPEND_FL` bits set is protected from their possible removal.

Hooked functions for this scope are:
- `ioctl`
### accept
A magic backdoor!

Just connect to any socket (started after the library injection) with a specific source port (`ncat IP PORT -p SOURCE_IP`, default: `65535`) to see a magical shell spawn.

Did you ask for security? It's password protected! (default pw: `SmirkFTW`)

Hooked functions for this scope are:
- `accept`
- `accept4`

### mount
If source or destination are one of the magicfiles it just returns 'ENOENT' (-1), thus preventing some nice mount tricks!

Hooked functions for this scope are:
- `mount`

### xstat
If a stat'd file has the "magix prefix" (default: `.smirk`) just returns "ENOENT" (-1).

Hooked functions for this scope are:
- `xstat`
- `xstat64`
- `lxstat`
- `lxstat64`


## In any case...
...We commented everything and you can easily understand what a function does just by reading the source.
```text
                    ____
                 _.' :  `._
             .-.'`.  ;   .'`.-.
    __      / : ___\ ;  /___ ; \      __
  ,'_ ""--.:__;".-.";: :".-.":__;.--"" _`,
  :' `.t""--.. '<@.`;_  ',@>` ..--""j.' `;
       `:-.._J '-.-'L__ `-- ' L_..-;'
         "-.__ ;  .-"  "-.  : __.-"
             L ' /.------.\ ' J
              "-.   "--"   .-"
             __.l"-:_JL_;-";.__
          .-j/'.;  ;""""  / .'\"-.
        .' /:`. "-.:     .-" .';  `.
     .-"  / ;  "-. "-..-" .-"  :    "-.
  .+"-.  : :      "-.__.-"      ;-._   \
  ; \  `.; ;                    : : "+. ;
  :  ;   ; ;                    : ;  : \:
 : `."-; ;  ;                  :  ;   ,/;
  ;    -: ;  :                ;  : .-"'  :
  :\     \  : ;             : \.-"      :
   ;`.    \  ; :            ;.'_..--  / ;
   :  "-.  "-:  ;          :/."      .'  :
     \       .-`.\        /t-""  ":-+.   :
      `.  .-"    `l    __/ /`. :  ; ; \  ;
        \   .-" .-"-.-"  .' .'j \  /   ;/
         \ / .-"   /.     .'.' ;_:'    ;
          :-""-.`./-.'     /    `.___.'
                \ `t  ._  / 
                 "-.t-._:'
             __            _                
            |  |   ___ ___| |_              
            |  |__| . | . | '_|             
       _   _|_____|___|___|_,_| _     
      | |_| |_ ___    ___ ___ _| |___ 
      |  _|   | -_|  |  _| . | . | -_|
      |__ |_|_|___|  |___|___|___|___|
            |  |   _ _| |_ ___              
            |  |__| | | '_| -_|             
            |_____|___|_,_|___|
```

## References
[Linux man pages](https://linux.die.net/man/)

[h0mbre's article](https://h0mbre.github.io/Learn-C-By-Creating-A-Rootkit/)

## LICENCE

This project is libre, and licenced under the terms of the DO WHAT THE FUCK YOU WANT TO PUBLIC LICENCE, version 3.1, as published by dtf on July 2019. See the LICENCE file or https://ph.dtf.wtf/w/wtfpl/#version-3-1 for more details.
