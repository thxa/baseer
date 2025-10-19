#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>

#define DEVICE_NAME_PID  "proc_pid"
#define DEVICE_NAME_INFO "proc_info"

static int pid_major, info_major;
static int target_pid = -1;
static struct task_struct *target_task = NULL;

/* ---------- /dev/proc_pid ---------- */
static ssize_t pid_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    char kbuf[16];
    if (len >= sizeof(kbuf)) return -EINVAL;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';
    if (kstrtoint(kbuf, 10, &target_pid) != 0)
        return -EINVAL;

    target_task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
    if (!target_task) {
        pr_info("procmod: PID %d not found\n", target_pid);
        target_pid = -1;
    } else {
        pr_info("procmod: Target PID set to %d (%s)\n", target_pid, target_task->comm);
    }

    return len;
}

static struct file_operations pid_fops = {
    .owner = THIS_MODULE,
    .write = pid_write,
};

/* ---------- /dev/proc_info ---------- */
static ssize_t info_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    char info[512];
    int written = 0;

    if (*off > 0 || target_pid < 0 || !target_task)
        return 0;

    struct mm_struct *mm = target_task->mm;
    if (!mm)
        written = snprintf(info, sizeof(info),
            "PID %d (%s): kernel thread, no mm_struct\n", target_pid, target_task->comm);
    else
        written = snprintf(info, sizeof(info),
            "Process: %s (pid=%d)\n"
            "State: %ld\n"
            "Priority: %d\n"
            "Code: 0x%lx - 0x%lx\n"
            "Data: 0x%lx - 0x%lx\n"
            "Stack: 0x%lx\n",
            target_task->comm, target_pid,
            target_task->__state, target_task->prio,
            mm->start_code, mm->end_code,
            mm->start_data, mm->end_data,
            mm->start_stack);

    if (copy_to_user(buf, info, written))
        return -EFAULT;

    *off += written;
    return written;
}

static struct file_operations info_fops = {
    .owner = THIS_MODULE,
    .read = info_read,
};

/* ---------- Init / Exit ---------- */
static int __init procmod_init(void)
{
    pid_major = register_chrdev(0, DEVICE_NAME_PID, &pid_fops);
    info_major = register_chrdev(0, DEVICE_NAME_INFO, &info_fops);
    if (pid_major < 0 || info_major < 0)
        return -EINVAL;


    // static struct class *proc_class;
    // proc_class = class_create(THIS_MODULE, "procmod_class");
    // device_create(proc_class, NULL, MKDEV(pid_major, 0), NULL, "proc_pid");
    // device_create(proc_class, NULL, MKDEV(info_major, 0), NULL, "proc_info");

    pr_info("procmod loaded: /dev/%s (major %d)\n",
            DEVICE_NAME_INFO, info_major);

    pr_info("procmod loaded: /dev/%s (major %d) \n",
            DEVICE_NAME_PID, pid_major);
    return 0;
}

static void __exit procmod_exit(void)
{
    // device_destroy(proc_class, pid_dev);
    // device_destroy(proc_class, info_dev);
    // class_destroy(proc_class);

    unregister_chrdev(pid_major, DEVICE_NAME_PID);
    unregister_chrdev(info_major, DEVICE_NAME_INFO);
    pr_info("procmod unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(procmod_init);
module_exit(procmod_exit);

