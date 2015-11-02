#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sree Surendran");
MODULE_DESCRIPTION("My first foray into the world of netlink sockets");

#define NETLINK_USER 31

struct sock* nl_sk = NULL;

static void nl_rcv_msg(struct sk_buff* skb){
    struct nlmsghdr* nlh;
    int pid;
    struct sk_buff* skb_out;
    int msg_size;
    char* msg = "From Kernel";
    int res;
    
    printk(KERN_INFO "Entering %s\n", __FUNCTION__);
    msg_size = strlen(msg);
    
    nlh = (struct nlmsghdr*)skb->data;
    printk(KERN_INFO "Netlink received message payload: %s\n", (char*)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid;

    skb_out = nlmsg_new(msg_size, 0);
    if(!skb_out){
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0){
        printk(KERN_INFO "Error returning to user\n");
    }

}

int __init my_init(void){
    printk(KERN_INFO "Entering %s\n", __FUNCTION__);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
    struct netlink_kernel_cfg cfg = {
        .input = nl_rcv_msg,
    };    
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
#else
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, nl_rcv_msg, NULL, THIS_MODULE);
#endif
    if(!nl_sk){
        printk(KERN_ALERT "Error creating socket\n");
        return -10;
    }

    return 0;
}

void __exit my_fini(void){
    printk(KERN_INFO "Entering %s\n", __FUNCTION__);
    if(nl_sk){
        netlink_kernel_release(nl_sk);
    }
}

module_init(my_init);
module_exit(my_fini);
