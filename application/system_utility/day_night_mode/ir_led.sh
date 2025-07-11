#!/bin/sh

if [ "${1}" == "-h" ]; then
  echo "#=========================#"
  echo "#== Turn on/off IR LED! ==#"
  echo "==========================#"
  echo "# NOTE: This script is used to turn on or turn off IR LED."
  echo ""
  echo "USAGE:"
  echo "  sh /system/mpp/script/ir_led.sh [IR_LED] [MODE]"
  echo ""
  echo "OPTION:"
  echo "  [IR_LED]: ir led gpio pin number"
  echo "  [MODE]: on or off"
  echo ""
  echo "EXAMPLE:"
  echo "  sh /system/mpp/script/ir_cut.sh 60 on"
  echo "  sh /system/mpp/script/ir_cut.sh 60 off"
fi

IR_LED=$1
MODE=$2

Path1="/sys/class/gpio/gpio${IR_LED}/"

#GPIO is already exported
if [ "${MODE}" == 'on' ] && [ -d "$Path1" ]; then

  # Turn on IR LED
  echo 1     > /sys/class/gpio/gpio${IR_LED}/value
 
#GPIO is not exported
elif [ "${MODE}" == 'on' ] && [ ! -d "$Path1" ]; then
    
  # Export IR LED GPIO
  echo ${IR_LED}    > /sys/class/gpio/export
  echo "out"  > /sys/class/gpio/gpio${IR_LED}/direction

  # Turn on IR LED
  echo 1     > /sys/class/gpio/gpio${IR_LED}/value

#GPIO is already exported
elif [ "${MODE}" == 'off' ] && [ -d "$Path1" ]; then

  # Turn off IR LED
  echo 0     > /sys/class/gpio/gpio${IR_LED}/value
  

#GPIO is not exported
elif [ "${MODE}" == 'off' ] && [ ! -d "$Path1" ]; then
  
  # Export IR LED GPIO
  echo ${IR_LED}    > /sys/class/gpio/export
  echo "out"  > /sys/class/gpio/gpio${IR_LED}/direction

  # Turn off IR LED
  echo 0     > /sys/class/gpio/gpio${IR_LED}/value 
  

#Unexport GPIO
elif [ "${MODE}" == 'unexport'  ] && [ -d "$Path1" ]; then

  # Unexport IR cut GPIO
  echo ${IR_LED}    > /sys/class/gpio/unexport

fi
