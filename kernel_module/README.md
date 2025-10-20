# Kernel module
```bash
sudo insmod dump_process.ko
sudo dmesg 
sudo rmmod dump_process.ko

```
```bash
MAJOR_PID_ID=$(sudo dmesg | grep /dev/proc_pid | tail -n1 | sed -n 's/.*major \([0-9]*\).*/\1/p')
MAJOR_INFO_ID=$(sudo dmesg | grep /dev/proc_info | tail -n1 | sed -n 's/.*major \([0-9]*\).*/\1/p')
sudo mknod /dev/proc_pid  c $MAJOR_PID_ID 0
sudo mknod /dev/proc_info c $MAJOR_INFO_ID 0
sudo chmod 666 /dev/proc_pid /dev/proc_info
```

## Write PID to target device
```bash
echo 1 | sudo tee /dev/proc_pid
```

## Read info into named pipe
```bash
mkfifo /tmp/myfifo
cat /dev/proc_info > /tmp/myfifo &
cat /tmp/myfifo
```


## Write and Read process info
```bash 
echo 104 > /dev/proc_pid && cat /dev/proc_info
```
