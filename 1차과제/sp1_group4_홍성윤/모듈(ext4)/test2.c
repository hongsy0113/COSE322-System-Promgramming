#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/myCircle-1.h>


/*
 작성자 : 윤상준
 작성일 : 20-10-31
*/


static struct proc_dir_entry *proc_dir; /* proc 파일 생성을 위한 구조체 포인터 변수 선언*/
/* proc file에 정보를 출력하는 함수 - extern 함수를 통해 출력할 circular queue에 접근한다 */
static int hello_proc_show(struct seq_file *p, void *v) {
	extern int ptr_num1;
	extern struct q_entry mycircle1[2000];
        int i = ptr_num1;
        int j = 0;
        for(; i < 2000 ; i++)
        {
                
                        seq_printf(p, "time : %lld%ld || FS_name : %s || block_no : %lu\n", mycircle1[i].timestamp_sec, mycircle1[i].timestamp_msec, mycircle1[i].fsys_name, mycircle1[i].blk_num);
        }
        for(; j<ptr_num1 ; j++)
        {
                
                        seq_printf(p, "time : %lld%ld || FS_name : %s || block_no : %lu\n", mycircle1[j].timestamp_sec, mycircle1[j].timestamp_msec,mycircle1[j].fsys_name, mycircle1[j].blk_num);


        }

        return 0;
}

static int hello_proc_open(struct inode *inode, struct  file *file) {
     return single_open(file, hello_proc_show, NULL);
}

static const struct file_operations hello_proc_fops = {
     .owner = THIS_MODULE,
     .open = hello_proc_open,
     .read = seq_read,
     .llseek = seq_lseek,
     .release = single_release,
};

static int __init hello_proc_init(void) {
     proc_dir = proc_mkdir("test3", NULL);
     proc_create("ext4_proc", 0700, proc_dir, &hello_proc_fops);
     return 0;
}

static void __exit hello_proc_exit(void) {
     remove_proc_entry("ext4_proc", proc_dir);
     remove_proc_entry("test3", NULL);
     return;
}

module_init(hello_proc_init);
module_exit(hello_proc_exit);

MODULE_AUTHOR("SANGJUN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AA");
MODULE_VERSION("NEW");
