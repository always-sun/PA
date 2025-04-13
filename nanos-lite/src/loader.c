#include "common.h"
#include "fs.h"
#include "memory.h"
#define DEFAULT_ENTRY ((void *)0x4000000)

extern void _map(_Protect *p, void *va, void *pa);
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
  filename = "/bin/pal";
   int fd = fs_open(filename, 0, 0);
  int bytes = fs_filesz(fd); 
  Log("Load [%d] %s with size: %d", fd, filename, bytes);

  void *pa, *va = DEFAULT_ENTRY;
   while (bytes > 0) {
    pa = new_page();                 // 分配物理页
    _map(as, va, pa);               // 映射虚拟地址到物理地址
    int len = (bytes >= PGSIZE) ? PGSIZE : bytes;
    fs_read(fd, pa, len);           // 读取 len 字节数据到物理内存
    va += PGSIZE;
    bytes -= len;
  }

  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
  }