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




static struct proc_dir_entry *proc_dir;/* proc 파일 생성을 위한 구조체 포인터 변수 선언*/
/* proc file에 정보를 출력하는 함수 - extern 함수를 통해 출력할 circular queue에 접근한다 */
static int hello_proc_show(struct seq_file *p, void *v) {
	extern int ptr_num2;
	extern struct q_entry mycircle2[2000];
        int i = ptr_num2;
        int j = 0;
        for(; i < 2000 ; i++)
        {
                
                        seq_printf(p, "time : %lld%ld || FS_name : %s || block_no : %lu\n", mycircle2[i].timestamp_sec, mycircle2[i].timestamp_msec, mycircle2[i].fsys_name, mycircle2[i].blk_num);
        }
        for(; j<ptr_num2 ; j++)
        {
                
                        seq_printf(p, "time : %lld%ld || FS_name : %s || block_no : %lu\n", mycircle2[j].timestamp_sec, mycircle2[j].timestamp_msec,mycircle2[j].fsys_name, mycircle2[j].blk_num);


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
     proc_dir = proc_mkdir("f2fs_proc", NULL);
     proc_create("f2fs_proc", 0700, proc_dir, &hello_proc_fops);
     return 0;
}

static void __exit hello_proc_exit(void) {
     remove_proc_entry("f2fs_proc", proc_dir);
     remove_proc_entry("f2fs_proc", NULL);
     return;
}

module_init(hello_proc_init);
module_exit(hello_proc_exit);

MODULE_AUTHOR("SANGJUN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AA");
MODULE_VERSION("NEW");
