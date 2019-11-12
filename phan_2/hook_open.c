#include<linux/module.h> 
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/unistd.h>
#include<linux/fs.h>

void **syscall_table_addr = NULL;

asmlinkage int (*custom_syscall)(char *name);

asmlinkage int hook_open(const char *pathname, int flags)
{
	printk(KERN_INFO "This is my hook_open()\n");
	
	printk(KERN_INFO"Opening file: %s\n",pathname);
	
	return custom_syscall(const char *pathname, int flags);
}



static int __init entry_point(void)
{
	printk(KERN_INFO "loaded mysyscall hook\n");

	syscall_table_addr =(void*)0xffffffff820001e0;
	
	custom_syscall = syscall_table_addr[__NR_open];
	
	make_rw((unsigned long)syscall_table_addr);

	syscall_table_addr[__NR_open] = hook_open;
	
	return 0;
	
}



static int __exit exit_point(void)
{
	printk(KERN_INFO " removed mysyscall hook\n");
	syscall_table_addr[__NR_open] = custom_syscall;

	make_ro((unsigned long)syscall_table_addr);

	return 0;
}

module_init(entry_point);
module_exit(exit_point);

