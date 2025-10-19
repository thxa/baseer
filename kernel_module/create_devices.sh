MAJOR_PID_ID=$(sudo dmesg | grep /dev/proc_pid | tail -n1 | sed -n 's/.*major \([0-9]*\).*/\1/p')
MAJOR_INFO_ID=$(sudo dmesg | grep /dev/proc_info | tail -n1 | sed -n 's/.*major \([0-9]*\).*/\1/p')

sudo mknod /dev/proc_pid  c $MAJOR_PID_ID 0
sudo mknod /dev/proc_info c $MAJOR_INFO_ID 0
sudo chmod 666 /dev/proc_pid /dev/proc_info

