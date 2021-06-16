#!/bin/sh

DIR="$( cd "$( dirname "$0"  )" && pwd  )"
DSPathUnix="$DIR/DebugSymbol"
export DebugSymbolsPath="`cygpath -w $DSPathUnix`"
if [ -d $DSPathUnix ]; then
    rm $DSPathUnix -rf
fi
mkdir $DSPathUnix
xmake require -fvy
xmake -r
xmake r &
sleep 3
python ./Packing/Packing.py
'/c/Program Files (x86)/Inno Setup 6/ISCC.exe' ./Packing/setup.iss