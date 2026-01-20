OpenSovix
OpenSovix is a micro-kernel that is still developing which published with GPL-3.0, aim to replace Windows

以下为简体中文：
注意⚠️：此项目采用GPL-3.0--or-later许可证授权。
一个高度模块化的微内核系统，采用分层架构和插件化设计，支持动态模块加载和扩展。这个系统设计遵循微内核核心最小化原则，所有非核心功能都作为可加载模块实现。
设计特点总结

1. 真正的微内核架构：内核只包含最基本功能（进程、IPC、调度）
2. 完全模块化：所有非核心功能都作为可加载模块
3. 动态扩展：支持运行时加载/卸载模块
4. 内存管理灵活：支持多内存池，预留GC接口
5. 文件系统支持：设计支持NTFS、ext2、ext3等
6. 可移植接口：为每个功能模块提供10-15个标准接口
7. 进程间通信：支持多种IPC机制
8. 系统调用标准化：统一的系统调用接口

仍在开发图形操作界面，文件管理系统，和底层硬件支持。
需要使用wsl或Linux进行编译运行，依赖宿主系统。
# 构建系统
1.克隆项目
     git clone <repository-url>
       cd microkernel 

2.安装依赖
     sudo apt-get install nasm gcc-multilib qemu-system-x86 grub-pc-bin xorriso 

3.构建系统
    make clean
      make all 

4.运行
    make run  



# 模块开发流程

1. 复制 template.c 作为起点
2. 修改模块信息（名称、版本、描述等）
3. 实现功能函数（最多15个）
4. 编译模块并加载模块
     bash
     make template-module


# 系统命令示例
查看系统状态
> sysinfo

列出进程
> ps

列出模块
> modules

列出内存池
> mempools

加载内存管理模块
> module load memory_gc.bin

创建内存池
> mempool create small_objects SMALL 1024

加载文件系统模块
> module load fs_ntfs.bin

挂载文件系统
> mount /dev/hda1 /mnt NTFS

卸载模块
> module unload fs_ntfs 
