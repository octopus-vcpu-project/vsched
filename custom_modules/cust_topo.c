#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/topology.h>
#include <linux/arch_topology.h>
#include <linux/sched/topology.h>
#include <linux/cpumask.h>
#include <linux/proc_fs.h>
#include <linux/acpi.h>
#include <linux/cacheinfo.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>
#include <linux/cpuset.h>
#include <linux/cpufreq.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#define PROCFS_NAME "edit_topology"
#define BUFFER_SIZE 5096
#define NR_CPUS 32

static struct proc_dir_entry *topology_proc;
extern cpumask_var_t cpu_l2c_shared_map;
extern void set_l2c_shared_mask(int cpu,struct cpumask new_mask);
extern void set_llc_shared_mask(int cpu,struct cpumask new_mask);

const struct cpumask *grapperFunction(int cpu) {
    static cpumask_t mask;

    // Clear all bits in the mask
    cpumask_clear(&mask);

    // Set only the specified CPU bit
    if (cpu >= 0 && cpu < nr_cpu_ids) {
        cpumask_set_cpu(cpu, &mask);
	if(cpu%2==1){
		cpumask_set_cpu(cpu-1,&mask);
	}else{
		cpumask_set_cpu(cpu+1,&mask);
	}
    }

    return &mask;
}




typedef const struct cpumask *(*resFunc)(int);

resFunc wrapperFunction(cpumask_t** masklist) {
	const cpumask_t* index_function(int index) {
		return masklist[index];
	}
	return index_function;
}



cpumask_t *str = NULL;

union {
  cpumask_t *mutable_field_p;
  const cpumask_t *const_field_p;
} u;



void my_custom_function(char *data) {
    printk(KERN_INFO "Registered Write to edit_topology...\n");
    struct sched_domain_topology_level *topology = get_sched_topology();
    if (topology != NULL) {
	printk(KERN_INFO "Scheduling Domain Topology retrieved.\n");
        printk(KERN_INFO "Data received: %s\n", data);
	cpumask_t use_cpumask;
	cpumask_clear(&use_cpumask);
	cpumask_t* cpu_mask_list[NR_CPUS];
	for (int i = 0; i < NR_CPUS; i++) {
	    cpu_mask_list[i] = kmalloc(sizeof(cpumask_t), GFP_KERNEL);
            cpumask_clear(cpu_mask_list[i]);
        }
	//resFunc res_func_list[5];
	resFunc* test_func = kmalloc(sizeof(test_func), GFP_KERNEL);;
	int cpu=0;
        //NOTE: 0 =SMT 1=CLUSTER 2=NUMA 3=COMPUTER
        int sched_domain=0;
	int comp_cpu=0;
	char currentChar = *data;
        while(*data != '\0') {
            currentChar = *data;
	    printk(KERN_INFO "Characsster: %c\n", currentChar);
            if(currentChar == ';') {
		if(sched_domain==0){
			cpumask_copy(topology[1].mask(cpu),&use_cpumask);
		}
		cpumask_copy(topology[sched_domain].mask(cpu),&use_cpumask);
		cpumask_clear(&use_cpumask);
		cpu++;
                comp_cpu=0;
            }else if(currentChar == ':') {
		sched_domain++;
		if(sched_domain==1){
			sched_domain++;
		}
                comp_cpu=0;
                cpu=0;
            }else{
                if(currentChar=='1') {
                    cpumask_set_cpu(comp_cpu,&use_cpumask);
		}
                comp_cpu++;
            }
	    data++;
        }
	int cpus;
        set_live_topology(topology);
    } else {
        printk(KERN_WARNING "Failed to retrieve Scheduling Domain Topology.\n");
    }
}

static ssize_t procfile_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos)
{
        printk( KERN_DEBUG "read handler\n");
        return -1;
}


ssize_t procfile_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset) {
    char *procfs_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

    if (count > BUFFER_SIZE) {
        count = BUFFER_SIZE;
    }

    if (copy_from_user(procfs_buffer, buffer, count)) {
        kfree(procfs_buffer);
        return -EFAULT;
    }

    // Here, you'll need to parse procfs_buffer to convert it into a list of lists format
    // For now, it's just passed as a string
    my_custom_function(procfs_buffer);

    kfree(procfs_buffer);
    return count;
}

static const struct proc_ops proc_file_fops = {
    .proc_write = procfile_write,
    .proc_read = procfile_read,
};


static int __init sched_topology_module_init(void) {
    topology_proc = proc_create(PROCFS_NAME, 0666, NULL, &proc_file_fops);
    if (topology_proc == NULL) {
        proc_remove(topology_proc);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
    return 0;
}


static void __exit sched_topology_module_exit(void)
{
    proc_remove(topology_proc);
    printk(KERN_INFO "Exiting sched_topology_module.\n");
}

module_init(sched_topology_module_init);
module_exit(sched_topology_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Linux module using get_sched_topology.");

