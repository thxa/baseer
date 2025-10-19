```bash
make
sudo insmod dump_process.ko
sudo dmesg 
sudo rmmod dump_process.ko
sudo sh create_devices.sh
sudo sh remove_devices.sh

```
```bash
sudo mknod /dev/proc_pid  c 240 0
sudo mknod /dev/proc_info c 241 0
sudo chmod 666 /dev/proc_pid /dev/proc_info
```

# Write PID to target device
```bash
echo 1 | sudo tee /dev/proc_pid
```

# Read info into named pipe
```bash
mkfifo /tmp/myfifo
cat /dev/proc_info > /tmp/myfifo &
cat /tmp/myfifo
```
