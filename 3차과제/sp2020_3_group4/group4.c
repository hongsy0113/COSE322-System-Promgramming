#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/netfilter_arp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#define PROC_DIRNAME "group4"
#define MAX_RULE 128
#define BUFSIZE 128

static struct proc_dir_entry *proc_dir;/* proc 파일 생성을 위한 구조체 포인터 변수 선언*/

struct my_rules //This is in the format of blacklist. Port numbers with certain type in this list  will be somehow restricted. 
{
	char type;	//rule type
	short port_num;	//port number
};

/*given functions ... string address to valid network address*/
unsigned int as_addr_to_net(char *str); // string address to network address
char *as_net_to_addr(unsigned int addr, char str[]); // given an empty char array(size of 16), network address to string address
static struct my_rules rules[MAX_RULE]; // make rule array of MAX_RULE
static int idx;	// the number of rules in array

/*hook function ************************/
static unsigned int inbound_hook( void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{

	int i; // integer for 'for loop'
	struct iphdr * ih = ip_hdr(skb); // get the ip header of skb
	struct tcphdr * th = tcp_hdr(skb); // get the tcp head of skb
	char str_s[16]; char str_d[16]; // empty character arrays
	as_net_to_addr(ih->saddr, str_s); // get string address of 'source ip address' from the real address
	as_net_to_addr(ih->daddr, str_d); // get string address of 'destination ip address' from the real address

	for(i=0; i<idx; i++)
	{ 
		if( (rules[i].type == 'I') && (be16_to_cpu(th->source) == rules[i].port_num)) // This demonstrates the condition which the packet is inbound and port number is the same as in the inbound rule
		{
			printk(KERN_ALERT "%-15s:%2u,%5d,%5d,%-15s,%-15s,%d%d%d%d\n", "DROP(INBOUND)", ih->protocol, be16_to_cpu(th->source), be16_to_cpu(th->dest), str_s, str_d, th->syn, th->fin, th->ack, th->rst); //print a message out on type, prot, sport, dport, saddr, daddr, tcp bit(syn fin ack rst) in order. be16_to_cpu is convert function from big endian to little endian which is our cpu is using.
			return NF_DROP; // this case must be dropped
		}

		else if( (rules[i].type == 'P') && (be16_to_cpu(th->source) == rules[i].port_num)) // This is for the proxy rule. when one of the rules is on proxy and port number is the same as in it.
		{
			
			ih->daddr = as_addr_to_net("131.1.1.1"); // 131.1.1.1 is given in the given slides. change the destination ip address to 131.1.1.1
			th->dest = th->source; // make the destination port same with the source port.
		 
			as_net_to_addr(ih->saddr, str_s); // get string address of 'source ip address' from the real address
			as_net_to_addr(ih->daddr, str_d); // destination ip address has changed. So, convert it  once more.
			printk(KERN_ALERT "%-15s:%2u,%5d,%5d,%-15s,%-15s,%d%d%d%d\n", "PROXY(INBOUND)", ih->protocol,  be16_to_cpu(th->source), be16_to_cpu(th->dest), str_s, str_d, th->syn, th->fin, th->ack, th->rst);// print a message out on type, prot, sport, dport, saddr, daddr, tcp bit(syn fin ack rst) in order.be16_to_cpu is convert function from big endian to little endian which is our cpu is using.
			return NF_ACCEPT; // this packet must be accepted and be sent to 131.1.1.1
		}
	}

	printk(KERN_ALERT "%-15s:%2u,%5d,%5d,%-15s,%-15s,%d%d%d%d\n", "INBOUND", ih->protocol, be16_to_cpu(th->source), be16_to_cpu(th->dest), str_s, str_d, th->syn, th->fin, th->ack, th->rst);
	// message for the packets which don't match any of the blacklist(rule array)
	return NF_ACCEPT; 
}

static unsigned int outbound_hook( void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{

	int i;
	struct iphdr * ih = ip_hdr(skb);
	struct tcphdr * th = tcp_hdr(skb);
	char str_s[16]; char str_d[16];
	as_net_to_addr(ih->saddr, str_s);
	as_net_to_addr(ih->daddr, str_d);
	// Explanation for these codes above is the same as in "inbound_hook".

	for(i = 0; i<idx; i++)
	{ 
		if( (rules[i].type == 'O') && (th->dest == ( cpu_to_be16(rules[i].port_num)))) // This condition is for when the port number of arrived packet is the same as the one the outbound rule is on

		{
			printk(KERN_ALERT "%-15s:%2u,%5d,%5d,%-15s,%-15s,%d%d%d%d\n", "DROP(OUTBOUND)", ih->protocol, be16_to_cpu(th->source), be16_to_cpu(th->dest), str_s, str_d, th->syn, th->fin, th->ack, th->rst); // print a message out on type, prot, sport, dport, saddr, daddr, tcp bit(syn fin ack rst) in order.be16_to_cpu is convert function from big endian to little endian which is our cpu is using.
			return NF_DROP;
		}
	}
	printk(KERN_ALERT "%-15s:%2u,%5d,%5d,%-15s,%-15s,%d%d%d%d\n", "OUTBOUND", ih->protocol, be16_to_cpu(th->source), be16_to_cpu(th->dest), str_s, str_d, th->syn, th->fin, th->ack, th->rst);
	//print a message out on type, prot, sport, dport, saddr, daddr, tcp bit(syn fin ack rst) in order.be16_to_cpu is convert function from big endian to little endian which is our cpu is using.
	return NF_ACCEPT;

}

static unsigned int forward_hook( void *priv, struct sk_buff * skb, const struct nf_hook_state *state)
{

	int i;
	struct iphdr * ih = ip_hdr(skb);
	struct tcphdr * th = tcp_hdr(skb);
	char str_s[16]; char str_d[16];
	as_net_to_addr(ih->saddr, str_s);
	as_net_to_addr(ih->daddr, str_d);
	for(i = 0; i<idx; i++)
	{ 
		if( (rules[i].type == 'F') && (th->source == ( cpu_to_be16(rules[i].port_num)))) //This condition is for when the port number of arrived packet is the same as the one the outbound rule is on 
		{
	printk(KERN_ALERT "%-15s:%2u,%5d,%5d,%-15s,%-15s,%d%d%d%d\n", "DROP(FORWARD)", ih->protocol, be16_to_cpu(th->source), be16_to_cpu(th->dest), str_s, str_d, th->syn, th->fin, th->ack, th->rst);
			//print a message out on type, prot, sport, dport, saddr, daddr, tcp bit(syn fin ack rst) in order. be16_to_cpu is convert function from big endian to little endian which is our cpu is using.
	return NF_DROP;
		}
	}

	printk(KERN_ALERT "%-15s:%2u,%5d,%5d,%-15s,%-15s,%d%d%d%d\n", "FORWARD", ih->protocol, be16_to_cpu(th->source), be16_to_cpu(th->dest), str_s, str_d, th->syn, th->fin, th->ack, th->rst);
	//print a message out on type, prot, sport, dport, saddr, daddr, tcp bit(syn fin ack rst) in order.be16_to_cpu is convert function from big endian to little endian which is our cpu is using.
	return NF_ACCEPT;

}


static struct nf_hook_ops inbound_ops= {
	.hook = inbound_hook, // hook function
	.pf = PF_INET, // 'cause using TCP/IP protocol
	.hooknum = NF_INET_PRE_ROUTING, // hook point
	.priority = NF_IP_PRI_FIRST, // priority  = 0 
	
};
static struct nf_hook_ops outbound_ops={
	.hook = outbound_hook, // hook function
	.pf = PF_INET, // 'cause using TCP/IP protocol
	.hooknum = NF_INET_POST_ROUTING,  // hook point
	.priority = NF_IP_PRI_FIRST, // priority = 0

};
static struct nf_hook_ops forward_ops={
	.hook = forward_hook, // hook function
	.pf = PF_INET, // 'cause using TCP/IP protocol
	.hooknum = NF_INET_FORWARD,// hook point
	.priority = NF_IP_PRI_FIRST, // priority = 0

};
/**********************************************************/

static int my_open(struct inode * inode, struct file *file) // function for module open
{
	printk(KERN_INFO "Module open!!\n"); // when this message is shown on dmesg, module has been open.
	return 0;
}

static ssize_t add(struct file *file, const char __user*ubuf, size_t size, loff_t * ppos) // adding rules 
{
	int len;
	char buf[BUFSIZE]; // buf to get from user buf
	char r; //which rule
	int  p; //port num

	copy_from_user(buf, ubuf, size); 

	sscanf(buf, "%c %d", &r, &p);
	
	// making a rule
	switch (r){ 
		case 'I':
			rules[idx].type = r;
			break;
		case 'O':
			rules[idx].type = r;
			break;
		case 'F':
			rules[idx].type = r;
			break;
		case 'P':
			rules[idx].type = r;
			break;
		default:
			printk(KERN_INFO "unvalid rule\n");
	}
	rules[idx].port_num = p;

	idx++; // the number of rule is increased by 1
	len = strlen(buf); // how much came from user buffer

	return len;
}

static ssize_t show(struct file *file, char __user*ubuf, size_t size, loff_t *ppos) // show the whole rules in rule array
{
	int len = 0;
	char buf[BUFSIZE] = {0};
	int i ;
	
	if(*ppos == 0) {
		for (i= 0; i<idx ; i++){
			len += sprintf(buf+len, "%d(%c): %d\n", i, rules[i].type, rules[i].port_num); // write on buffer
		}
		copy_to_user(ubuf, buf, len); // send to user buffer
	}
	(*ppos) = len;
	
	return len;
}

static ssize_t del(struct file *file, const char __user*ubuf, size_t size, loff_t * ppos) // delete a rule in rule array
{
	int len;
	char buf[BUFSIZE];
	int i;
	copy_from_user(buf, ubuf, size); // get instruction from user 

	sscanf(buf, "%d", &i);
	
	if (i>idx || i<0) {
		printk(KERN_INFO "unvalid index\n");	
	}
	else {
		memmove(rules+i, rules+i+1, sizeof(struct my_rules)*(idx-i-1)); // remove rule what the instruction is pointing
		idx --;
	}

	len = strlen(buf);
	return len;

}

static const struct file_operations add_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.write = &add,
};

static const struct file_operations show_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = &show,
};

static const struct file_operations del_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.write = &del,
};

static int __init simple_init(void) {
	idx = 0;
	proc_dir = proc_mkdir(PROC_DIRNAME, NULL); // proc files will be located in proc_dir

	/* make 3 proc files, add show del */
	proc_create("add", 0777, proc_dir, &add_fops); // 0777 is for the whole authority 
	proc_create("show", 0777, proc_dir, &show_fops);
	proc_create("del", 0777, proc_dir, &del_fops);

	/*register hook functions*/
	nf_register_hook(&inbound_ops);
	nf_register_hook(&outbound_ops);
	nf_register_hook(&forward_ops);
	return 0;
}

static void __exit simple_exit(void) { //when we call deload the module
	/* remove proc files*/
	remove_proc_entry("add", proc_dir);
	remove_proc_entry("show", proc_dir);
	remove_proc_entry("del", proc_dir);
	remove_proc_entry("group4", NULL);

	/*remove hook functions from hook point*/
	nf_unregister_hook(&inbound_ops);
	nf_unregister_hook(&outbound_ops);
	nf_unregister_hook(&forward_ops);
}

/*given functions for translating ip address*/
unsigned int as_addr_to_net(char *str)
{
	unsigned char arr[4];
	sscanf(str, "%d.%d.%d.%d", (unsigned int*)&arr[0], (unsigned int *) &arr[1], (unsigned int *)&arr[2], (unsigned int *)&arr[3]);
	return *(unsigned int *)arr;
}

char * as_net_to_addr(unsigned int addr, char str[])
{
	char add[16];
	unsigned char a = ((unsigned char *) &addr)[0];
	unsigned char b = ((unsigned char *) &addr)[1];
	unsigned char c = ((unsigned char *) &addr)[2];
	unsigned char d = ((unsigned char *) &addr)[3];
	sprintf(add, "%u.%u.%u.%u",a,b,c,d);
	sprintf(str,"%s", add);
	return str;
}



module_init(simple_init);
module_exit(simple_exit);

MODULE_AUTHOR("SUNGYOON");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("kernerl module by group4");
MODULE_VERSION("NEW");
