#include "common.h"

#define DEFAULT_ENTRY ((void *)0x80400000)

extern void* new_page(void);
extern void ramdisk_read(void* buf, off_t offset, size_t len);
extern void ramdisk_write(const void* buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_filesz(int fd);
extern size_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
   /*ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  return (uintptr_t)DEFAULT_ENTRY;*/
   filename = "/bin/text";
  int fd = fs_open(filename, 0, 0);
  int bytes = fs_filesz(fd); 
  Log("Load [%d] %s with size: %d", fd, filename, bytes);

  void *pa;
  void *va = (void *)DEFAULT_ENTRY;  // 默认程序加载虚拟地址

  // 按页加载程序内容，并为每一页分配物理页并建立页表映射
  while (bytes > 0) {
    pa = new_page();             // 分配一页物理内存
    _map(as, va, pa);            // 建立虚拟地址 va 到物理地址 pa 的映射
    fs_read(fd, pa, PGSIZE);     // 从文件读取一页内容到物理内存
    va += PGSIZE;
    bytes -= PGSIZE;
  }

  fs_close(fd);  // 关闭文件
  return (uintptr_t)DEFAULT_ENTRY;  // 返回程序入口地址

}
