#!/bin/bash
i=0
case "$1" in
 help)
    echo "Usage: $0 {interval sec} {loop time} {pid}"
    exit 1
    ;;
 *)
    while [ $i != $2 ]
        do
        # Read /proc/stat file (for first datapoint)
        read cpu user nice system idle iowait irq softirq steal guest< /proc/stat

        # compute active and total utilizations
        cpu_active_prev=$((user+system+nice+softirq+steal))
        cpu_total_prev=$((user+system+nice+softirq+steal+idle+iowait))
        system_prev=$system
        cpu_idle_prev=$idle
        cpu_iowait_prev=$iowait
        read pid comm task_state ppid pgid sid tty_nr tty_pgrp flags min_flt cmin_flt maj_flt cmaj_flt utime stime cutime cstime priority nice num_threads it_real_value start_time vsize rss rsslim< /proc/$3/stat
        pid_active_prev=$((utime+stime+cutime+cstime))

        sleep $1

        # Read /proc/stat file (for second datapoint)
        read cpu user nice system idle iowait irq softirq steal guest< /proc/stat

        # compute active and total utilizations
        cpu_active_cur=$((user+system+nice+softirq+steal))
        cpu_total_cur=$((user+system+nice+softirq+steal+idle+iowait))
        system_cur=$system
        cpu_idle_cur=$idle
        cpu_iowait_cur=$iowait
        read pid comm task_state ppid pgid sid tty_nr tty_pgrp flags min_flt cmin_flt maj_flt cmaj_flt utime stime cutime cstime priority nice num_threads it_real_value start_time vsize rss rsslim< /proc/$3/stat
        pid_active_cur=$((utime+stime+cutime+cstime))

        # compute CPU utilization (%)
        cpu_total=$((cpu_total_cur-cpu_total_prev))
        cpu_util=$((100*( cpu_active_cur-cpu_active_prev ) / $cpu_total ))
        cpu_idle=$((100*( cpu_idle_cur-cpu_idle_prev ) / $cpu_total ))
        cpu_io_wait=$(( 100*(cpu_iowait_cur-cpu_iowait_prev) / $cpu_total ))
        system_util=$(( 100*(system_cur-system_prev) / $cpu_total ))
        pid_util=$((100*(pid_active_cur-pid_active_prev) / $cpu_total ))
        pid_rss=$((4*$rss))

        printf "Current CPU Util: %s , idle: %s, iowait: %s, system: %s pid: (%s)%s util: %s, rss: %s kB\n" "$cpu_util" "$cpu_idle" "$cpu_io_wait" "$system_util" "$comm" "$pid" "$pid_util" "$pid_rss"

        i=$(($i+1))

    done
    exit 0
    ;;
esac


