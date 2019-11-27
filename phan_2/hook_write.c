#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/unistd.h>
#include<linux/fs.h>
#include <asm/pgtable_types.h>
#include <linux/foobar.h>
#include <asm/uaccess.h>
#include<linux/uaccess.h>
void **syscall_table_addr = NULL;
asmlinkage ssize_t (*original_write)(int fd, const void *buf, size_t cnt);

int make_rw(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    if(pte->pte &~_PAGE_RW){
        pte->pte |=_PAGE_RW;
    }
    return 0;
}

int make_ro(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}






asmlinkage ssize_t hook_write(int fd, const void *buf, size_t cnt)
{
	char *tmp;
	char *pathname;
	struct file *file;
	int fput_needed;
	mm_segment_t segment;
	int  written_bytes ;
	struct path *path;
	
	printk(KERN_INFO"This is my hook_write()\n");

	/*file = fget_light(fd, &fput_needed);
	if (file)
	{
		segment = get_fs();
		set_fs(KERNEL_DS);
		pathname = file->f_path.dentry->d_iname;		
		set_fs(segment);
	}
	fput_light(file, fput_needed);*/
	spin_lock(&current->files->file_lock);
file = fcheck_files(current->files, fd);
if (!file) {
    spin_unlock(&current->files->file_lock);
    return -ENOENT;
}

path = &file->f_path;
path_get(path);
spin_unlock(&current->files->file_lock);

tmp = (char *)__get_free_page(GFP_KERNEL);

if (!tmp) {
    path_put(path);
    return -ENOMEM;
}

pathname = d_path(path, tmp, PAGE_SIZE);
path_put(path);

if (IS_ERR(pathname)) {
    free_page((unsigned long)tmp);
    return PTR_ERR(pathname);
}

/* do something here with pathname */

free_page((unsigned long)tmp);

	printk(KERN_INFO "Written file: %s\n", pathname);

	written_bytes = original_write(fd, buf, cnt);

	printk(KERN_INFO"Number of written bytes: %d\n",written_bytes);

	return written_bytes;
}

static int __init init_mysyscall(void)
{
	printk(KERN_INFO "loaded mysyscall hook\n");
	
	syscall_table_addr = (void*)0xffffffff820001e0;

	origianl_write = syscall_table_addr[__NR_write];

	make_rw((unsigned long)syscall_table_addr);

	syscall_table_addr[__NR_write] = hook_write;

	return 0;
}


static void __exit exit_mysyscall(void)
{
	printk(KERN_INFO "removed mysyscall hook\n");

	syscall_table_addr[__NR_write] = original_write;

	make_ro((unsigned long)syscall_table_addr);

}


module_init(init_mysyscall);
module_exit(exit_mysyscall);
