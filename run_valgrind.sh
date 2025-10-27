#!/bin/bash
#run as ./run_valgrind.sh <EXEC> <SCRIPT> <suppression list filepath> <logfile>
EXEC=$1
SCRIPT=$2
SUPP=$3
LOG=$4

VG_SCRIPT=/tmp/vlgnd.tcl
cp $SCRIPT $VG_SCRIPT
# echo "quit" >> $VG_SCRIPT

valgrind --track-origins=yes --leak-check=full  --show-leak-kinds=all --show-reachable=yes --show-possibly-lost=yes --suppressions=$SUPP   --log-file=$LOG $EXEC $VG_SCRIPT
