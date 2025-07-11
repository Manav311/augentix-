#! /bin/sh

#set -x

loadUsage()
{
    echo "what.sh: Show MPP library information";
    echo "Usage:  what.sh <file1> [<file2> <file3> ...]";
    echo "example:";
    echo "    what.sh mpi_stream";
    echo "    what.sh enc.ko is.ko isp.ko senif.ko";
    echo "    what.sh libenc.a libis.a";
    echo "    what.sh *.ko";
    echo "    what.sh *.a";
    echo "    what.sh -h   (show this help)";
}

if [ $# -lt 1 ] || [ $1 = "-h" ]; then
    loadUsage;
    exit 0;
fi

echo "Number of arg: $#"
echo "List of all arg: $@"

infiles=$@
#echo $infiles;
strings $infiles | grep '^@(#)'

