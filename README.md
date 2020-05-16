# **SMIRK** - **SM**irk **I**s a **R**oot**K**it 🔥🥔🔥

a simple userland rootkit that:
- links itself on *ld.preload.so* and tries to stay there
- spawns shells but only for the righteous
- hides itself hijacking syscalls

#### Designed and built with all the Love❤️ in the World🌍 by [Roberto Bindi](https://github.com/ShotokanZH) & [Gianluca Pericoli](https://github.com/gpericol)

## Compile
```bash
make
```

## Install
```bash
LD_PRELOAD=smirk.so id
```

## Uninstall (if killswitch is activated)
```bash
touch /dev/shm/.smirkkill
```

## How does it work
[TODO] fast description of hooked API and why

We explanatory comment everything and you can understand everthing looking at the sourcecode
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

This program is free software; you can redistribute it and/or modify it under the terms of the [GNU General Public License](https://www.gnu.org/licenses/gpl-3.0.html) as published by the Free Software Foundation; either version 3 of the License, or(at your option) any later version.
