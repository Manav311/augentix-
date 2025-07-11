#!/bin/sh

if [ "${1}" == "-h" ]; then
  echo "#==================================#"
  echo "#== Active/Remove IR cut filter! ==#"
  echo "#==================================#"
  echo "# NOTE: This script is used to active or remove IR cut filter."
  echo ""
  echo "USAGE:"
  echo "  sh /system/mpp/script/ir_cut.sh [IR_CUT0] [IR_CUT1] [MODE]"
  echo ""
  echo "OPTION:"
  echo "  [IR_CUT*]: ir cut gpio pin number"
  echo "  [MODE]: active or remove"
  echo ""
  echo "EXAMPLE:"
  echo "  sh /system/mpp/script/ir_cut.sh 53 54 active"
  echo "  sh /system/mpp/script/ir_cut.sh 53 54 remove"
fi

IR_CUT0=$1
IR_CUT1=$2
MODE=$3

Path1="/sys/class/gpio/gpio${IR_CUT0}/"
Path2="/sys/class/gpio/gpio${IR_CUT1}/"


#Use two GPIO pin to contral ir cut
if [ "${IR_CUT1}" != "-1" ];then
  #GPIO is already exported
  if [ "${MODE}" == 'active' ] && [ -d "$Path1" -a -d "$Path2" ]; then

    # Active IR cut filter
    echo 0     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 1     > /sys/class/gpio/gpio${IR_CUT1}/value

    usleep 300000

    echo 0     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 0     > /sys/class/gpio/gpio${IR_CUT1}/value

  #GPIO is not exported
  elif [ "${MODE}" == 'active' ] && [ ! -d "$Path1" -a ! -d "$Path2" ]; then

    # Export IR cut GPIO
    echo ${IR_CUT0}    > /sys/class/gpio/export
    echo "out"  > /sys/class/gpio/gpio${IR_CUT0}/direction
    echo ${IR_CUT1}    > /sys/class/gpio/export
    echo "out"  > /sys/class/gpio/gpio${IR_CUT1}/direction

    # Active IR cut filter
    echo 0     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 1     > /sys/class/gpio/gpio${IR_CUT1}/value

    usleep 300000

    echo 0     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 0     > /sys/class/gpio/gpio${IR_CUT1}/value

  #GPIO is already exported
  elif [ "${MODE}" == 'remove' ] && [ -d "$Path1" -a -d "$Path2" ]; then

    # Remove IR cut filter
    echo 1     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 0     > /sys/class/gpio/gpio${IR_CUT1}/value

    usleep 300000

    echo 0     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 0     > /sys/class/gpio/gpio${IR_CUT1}/value

  #GPIO is not exported
  elif [ "${MODE}" == 'remove' ] && [ ! -d "$Path1" -a ! -d "$Path2" ]; then

    # Export IR cut GPIO
    echo ${IR_CUT0}    > /sys/class/gpio/export
    echo "out"  > /sys/class/gpio/gpio${IR_CUT0}/direction
    echo ${IR_CUT1}    > /sys/class/gpio/export
    echo "out"  > /sys/class/gpio/gpio${IR_CUT1}/direction

    # Remove IR cut filter
    echo 1     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 0     > /sys/class/gpio/gpio${IR_CUT1}/value

    usleep 300000

    echo 0     > /sys/class/gpio/gpio${IR_CUT0}/value
    echo 0     > /sys/class/gpio/gpio${IR_CUT1}/value

  #Unexport GPIO
  elif [ "${MODE}" == 'unexport'  ] && [ -d "$Path1" -a -d "$Path2"  ]; then

    # Export IR cut GPIO
    echo ${IR_CUT0}    > /sys/class/gpio/unexport
    echo ${IR_CUT1}    > /sys/class/gpio/unexport

  fi
#Use single GPIO pin to contral ir cut
else
  echo ${IR_CUT0} > /sys/class/gpio/export
  echo "out"  > /sys/class/gpio/gpio${IR_CUT0}/direction

  if [ "${MODE}" == 'active' ]; then

    # Active IR cut filter
    echo 0 > /sys/class/gpio/gpio${IR_CUT0}/value

  #GPIO is already exported
  elif [ "${MODE}" == 'remove' ]; then

    # Remove IR cut filter
    echo 1 > /sys/class/gpio/gpio${IR_CUT0}/value
  fi
fi
