#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>

/*硬件相关的头文件*/
#include <mach/at91sam9_smc.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/gpio.h>
#include <mach/at91sam9g45_matrix.h>
#include <mach/at91sam9g45.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>

#include <mach/gpio.h>

//理解file_operations、file、inode 等内核结构的作用 和 联系 ！！！

static dev_t Fcon_devt;
static struct class *cls;

//设备结构体
struct Fcon{
	struct cdev cdev;
	struct device *class_dev;
	/**
	int irq;
	struct work_struct irq_wq;//内核工作队列结构体
	int data;
	spinlock_t irq_lock;
	*/
	//void __iomem *piob_codr,*piob_sodr;
	//void __iomem *piod_pdsr;
};
struct Fcon Fcon_dev;

int Fcon_open(struct inode *inode,struct file *file)
{
	//打开设备后的初始操作
	//struct irq *chip = container_of(inode->i_cdev,struct irq,cdev);
	//file->private_data = chip;
	if(!at91_get_gpio_value(AT91_PIN_PB30))
		printk(KERN_ALERT "CONF_DONE = 0\n");
	
	if( at91_get_gpio_value(AT91_PIN_PB29) && at91_get_gpio_value(AT91_PIN_PD12) )
		printk(KERN_ALERT "restarting!\n");
	
	at91_set_gpio_value(AT91_PIN_PB29,0);
	at91_set_gpio_value(AT91_PIN_PB23,0);
	
	udelay(35);
	
	while(at91_get_gpio_value(AT91_PIN_PD12));
	at91_set_gpio_value(AT91_PIN_PB29,1);
	while(!at91_get_gpio_value(AT91_PIN_PD12));
	printk(KERN_ALERT "Configing!\n");
	
	return 0;
}

ssize_t Fcon_read(struct file *file,char __user *buf,size_t size,loff_t *offset ){
	//struct irq *idev = file->private_data;
	int tag = 0;
	while(!at91_get_gpio_value(AT91_PIN_PB30));
	tag = at91_get_gpio_value(AT91_PIN_PB30);
	
	copy_to_user(buf,&tag,sizeof(int));
	return size;
}

ssize_t Fcon_write(struct file *file,char __user *buf,size_t size,loff_t *offset ){
	unsigned char a;
	
	//struct Fcon *fdev = file->private_data;
	void __iomem *piob_codr,*piob_sodr;
	piob_codr=(void __iomem*)(0xFEF78000)+0x87434;
	piob_sodr=(void __iomem*)(0xFEF78000)+0x87430;
	
	copy_from_user(&a,buf,sizeof(unsigned char));
	printk(KERN_ALERT "0x%02x",a);
			
			if(a & 0x01)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(a & 0x02)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(a & 0x04)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(a & 0x08)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(a & 0x10)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(a & 0x20)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(a & 0x40)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(a & 0x80)
				__raw_writel(0x01000000,piob_sodr);
			else
				__raw_writel(0x01000000,piob_codr);
			__raw_writel(0x00800000,piob_sodr);
			__raw_writel(0x00800000,piob_codr);
			
			if(!at91_get_gpio_value(AT91_PIN_PD12)){
				printk(KERN_ALERT "error ");
				return 0;
			}
			/**
			if( !((__raw_readl(&(fdev->piod_pdsr)))&0x00001000) ){
				printk(KERN_ALERT "error ");
				return 0;
			}
			*/
	return size;
}

//ssize_t irq_write(struct file *file,const char __user *buf,size_t size, loff_t *offset){}
/**
static irqreturn_t irq_interrupt(int irq,void *dev_id)
{
	struct irq *idev = dev_id;
	spin_lock_irq(&idev->irq_lock);
	idev->data++;
	spin_unlock_irq(&idev->irq_lock);
	//schedule_work(&idev->irq_wq);
	return IRQ_HANDLED;
}

static int irq_do_work(struct work_struct *work)
{
	struct irq *idev = container_of(work,struct irq,irq_wq);
	return 1;
	//中断底部 将读取的数据缓存
}
*/
//file_operations 是设备编号 与 驱动程序的 连接口
static struct file_operations Fcon_fops = {
	.owner = THIS_MODULE,
	.open = Fcon_open,
	//.release = NULL,
	.read = Fcon_read,
	.write = Fcon_write,
};



static int Fcon_drv_init(void){
	int ret;
	//struct irq *irq_dev;
	//irq_dev = kmalloc(sizeof(struct irq),GFP_KERNEL);
	
	//待解决！！！
	cls = class_create(THIS_MODULE, "myFcon_class");
	if(IS_ERR(cls))
		{
		  return PTR_ERR(cls);
		}
	
	//①获取设备编号
	ret = alloc_chrdev_region(&Fcon_devt, 0, 2, "my_Fcon");
	if(ret < 0){
		class_destroy(cls);
		return ret;
	}
	
	//②注册设备驱动
	//&irq_dev.cdev = cdev_alloc();
	cdev_init(&Fcon_dev.cdev,&Fcon_fops);
	Fcon_dev.cdev.owner = THIS_MODULE;
	Fcon_dev.cdev.ops = &Fcon_fops;
	ret = cdev_add(&Fcon_dev.cdev,Fcon_devt,1);	
	if(ret<0){
	  printk(KERN_ALERT "error\n");	
	}
	
	//③创建设备节点
	Fcon_dev.class_dev = device_create(cls,NULL,Fcon_devt,&Fcon_dev,"myFcon");	
	if(IS_ERR(Fcon_dev.class_dev)){
	  printk(KERN_ALERT "error\n");	
	}
	/**
	//申请中断
	void __iomem* aic_ffer;
	aic_ffer=(void __iomem*)(0xFEF78000) +0x87000 + 0x140;
	__raw_writel(0x00000008,aic_ffer);
	spin_lock_init(&irq_dev.irq_lock);
	irq_dev.data = 0;
	INIT_WORK(&irq_dev.irq_wq,irq_do_work);
	irq_dev.irq = AT91_PIN_PB9;
	at91_set_gpio_input(irq_dev.irq,1);
	at91_set_deglitch(irq_dev.irq,1);
	ret = request_irq(irq_dev.irq,irq_interrupt,IRQ_TYPE_EDGE_RISING ,"my_irq",&irq_dev);
	*/
	
	//Fcon_dev.piob_codr=(void __iomem*)(0xFEF78000)+0x87434;
	//Fcon_dev.piob_sodr=(void __iomem*)(0xFEF78000)+0x87430;
	//Fcon_dev.piod_pdsr=(void __iomem*)(0xFEF78000)+0x8783C;
	
	at91_set_gpio_output(AT91_PIN_PB29,1);
	at91_set_gpio_input(AT91_PIN_PB30,1);
	at91_set_gpio_output(AT91_PIN_PB23,1);
	at91_set_gpio_output(AT91_PIN_PB24,1);
	at91_set_gpio_input(AT91_PIN_PD12,1);
	
	
	return 0;
}

static int Fcon_drv_exit(void){
	/**
	//中断释放
	free_irq(irq_dev.irq,&irq_dev);
	*/
	//①注销设备节点
	device_unregister(Fcon_dev.class_dev);
	//②注销设备
	cdev_del(&Fcon_dev);
	//③释放设备编号
	unregister_chrdev_region(Fcon_devt,2);
	
	return 0;
}

module_init(Fcon_drv_init);
module_exit(Fcon_drv_exit);

MODULE_LICENSE("GPL"); 