flobby
======
flobby is a [spring](https://springrts.com) lobby client written in C++.  
flobby is developed and tested on archlinux, use AUR package [flobby-git](https://aur.archlinux.org/packages/flobby-git/) for easy install and update.

Dependencies worth noting
-------------------------
* FLTK 1.4 dependencies (FLTK is a submodule and linked to statically)
* jsoncpp
* libxpm
* libxss
* GraphicsMagick
* curl
* boost
* C++11 (gcc 4.6 should be enough)

Ubuntu Instructions
------------------
* see script: ubuntu/flobby_installer.sh
* essential build packages:

```
sudo apt-get update
sudo apt-get install -y apt-utils git cmake build-essential pkg-config libboost-system1.67.0 libboost1.67-dev libboost-filesystem-dev libboost-chrono1.67-dev libboost-regex-dev libboost-thread1.67-dev libjsoncpp-dev libjsoncpp1 libgraphicsmagick++1-dev libcurl4-gnutls-dev libminizip-dev clang-format libxpm-dev libxcb-screensaver0-dev libxss-dev libqt5opengl5-dev
cmake .
make -j 4 ( depends on your CPUspeed, use -j 12 for very fast compiling )
sudo make install
```

Build and run
-------------
    git clone --recursive https://github.com/cleanrock/flobby.git
    cd flobby
    cmake .
    make
    src/flobby

Directories and files
---------------------
    ~/.config/flobby/  # can be changed with -d argument or XDG_CONFIG_HOME
        flobby.prefs  # flobby settings, delete this file to reset all settings to default 
    ~/.cache/flobby/  # can be changed with -d argument or XDG_CACHE_HOME
        flobby.log  # flobby debug log
        flobby_script.txt  # spring script file, written on spring launch
        flobby_process_pr-downloader.log  # pr-downloader output
        map/  # map cache files
        log/  # logs of all chats, logging to files can be disabled in flobby

Command line arguments
----------------------
    $ flobby -h
    usage: flobby [options]
        -d | --dir <dir> : use <dir> for flobby config and cache instead of XDG
        -z | --zerok     : use zero-k lobby protocol, uses server address lobby.zero-k.info:8200 by default
        -v | --version   : print flobby version
        -h | --help      : print help message
    plus standard fltk options:
        -bg2 color
        -bg color
        -di[splay] host:n.n
        -dn[d]
        -fg color
        -g[eometry] WxH+X+Y
        -i[conic]
        -k[bd]
        -na[me] classname
        -nod[nd]
        -nok[bd]
        -not[ooltips]
        -s[cheme] scheme
        -ti[tle] windowtitle
        -to[oltips]

Non-obvious features
--------------------
- user name tab completion (case-insensitive), matches first "starting with" then "containing", cycles through multiple matches
- Alt+[1-9,0] - go to chat tab #, 0 will go to last tab
- Alt+[Left|Right] - go one chat tab left/right, works when in a chat tab
- supports watching replays from Zero-K website
