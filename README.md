# OpenSovix Micro-Kernel


OpenSovix 是一个遵循 GPL-3.0 许可证开源，正在积极开发中的真正的微内核操作系统项目。它采用分层与插件化设计，旨在为个人桌面计算构建一个高度模块化、安全且可扩展的现代系统基石，其长期目标是成为一个可替代传统桌面操作系统的新选择。

# ⚠️ 许可证声明: 本项目严格采用 GNU General Public License v3.0 许可证授权。所有分发和衍生作品必须遵循相同的开源条款。

# 🎯 项目愿景：为什么是另一个微内核？

我们致力于探索操作系统设计的另一种范式。与主流的宏内核（如Linux、Windows NT）不同，OpenSovix坚持微内核架构，将内核本身精简到极致——仅包含进程调度、进程间通信（IPC）和最基本的内存管理。所有其他服务（文件系统、驱动、网络栈等）均作为独立的用户态进程运行。

这种设计的核心优势在于：

· 高安全性与可靠性：服务的崩溃不会导致整个系统崩溃，可被独立重启。
· 极致模块化：系统功能可像乐高一样动态组装、替换和升级。
· 形式化验证潜力：精简的核心更易于进行数学证明，为构建高可信系统奠定基础。

虽然构建一个功能完备的微内核系统面临巨大的工程挑战（如性能优化、驱动生态），但我们相信，这对于深入理解计算机系统原理、推动操作系统技术向前发展具有重要的教育和实践价值。

# ✨ 核心设计特点

· 真正的微内核架构：内核仅包含最基本功能（进程/线程、IPC、调度），拒绝任何“非核心”代码。
· 完全模块化：所有非核心功能均作为可加载模块实现，支持运行时动态加载与卸载。
· 灵活的内存管理：支持多内存池机制，并为未来的垃圾回收（GC）预留了接口。
· 广泛的文件系统支持：架构设计支持 NTFS、ext2、ext3 等主流文件系统（通过模块实现）。
· 标准化接口：
  · 为每个功能模块提供 10-15 个标准操作接口。
  · 为上层应用提供统一的系统调用接口。
· 高效的进程间通信：实现了多种IPC机制，作为模块与服务间通信的“血管”。

# 🛠️ 快速开始

目前，项目的编译和运行依赖于宿主Linux环境（或WSL）。

环境准备与构建

```bash
# 1. 克隆仓库
git clone https://github.com/Yankeelucas/OpenSovix.git
cd OpenSovix

# 2. 安装必要的构建工具链和模拟器（以 Debian/Ubuntu 为例）
sudo apt-get update
sudo apt-get install nasm gcc-multilib qemu-system-x86 grub-pc-bin xorriso

# 3. 编译系统
make clean
make all

# 4. 在 QEMU 模拟器中运行
make run
```

# 📦 模块：系统的活力之源

模块是扩展OpenSovix功能的唯一方式。开发新模块非常简单：

1. 复制模板：以 template.c 文件作为起点。
2. 修改元信息：更新模块的名称、版本、描述等。
3. 实现功能：在规定的框架内（最多15个标准接口函数）编写您的逻辑。
4. 编译与加载：使用 make template-module 编译，并在系统运行时使用 module load <模块名.bin> 动态加载。

# 💻 系统命令速查

系统启动后，您可以使用以下命令进行交互和管理：

```命令 功能描述 示例
# 显示系统概要信息（版本、内存使用等）
 sysinfo 
# 列出当前系统中所有的进程。
 sysinfops
# 显示所有已加载的内核模块
 psmodules
# 列出所有已创建的内存池及其状态
 modulesmempools
# 创建一个新的内存池
 mempoolsmempool create eg: mempool create small_objects SMALL 1024
# 动态加载或卸载一个功能模块
 module load/unload eg :module load memory_gc.bin
# 挂载文件系统（需先加载对应模块）
 mount eg:mount /dev/hda1 /mnt NTFS
```

# 🚧 当前开发状态与路线图

项目处于早期积极开发阶段。目前的核心代码主要集中于搭建基础框架、底层硬件抽象和核心数据结构。

· 当前开发焦点：
  · 🖥️ 图形用户界面（GUI）框架
  · 📁 完整的文件管理子系统
  · 🔧 更广泛的底层硬件驱动支持
· 已完成核心：
  · 基础的多核调度器
  · 虚拟内存管理
  · 进程间通信（IPC）框架
  · 模块加载器

我们坦诚面对挑战：构建一个媲美宏内核性能的微内核、编写大量硬件驱动、培育应用生态，这些都是长期而艰巨的任务。本项目更多是一个深入实践操作系统原理的绝佳平台。

# 📁 项目结构概览

```
opensovix/
├── Makefile
├── scripts/
│   ├── build.sh
│   ├── run-qemu.sh
│   └── mkbootimg.sh         
├── include/
│   ├── kernel/
│   │   ├── types.h
│   │   ├── mem.h
│   │   ├── ipc.h
│   │   ├── process.h
│   │   ├── vfs.h
│   │   ├── syscall.h
│   │   ├── fat32.h        
│   │   ├── device.h        
│   │   └── time.h         
│   ├── common/
│   │   ├── list.h
│   │   ├── string.h
│   │   ├── bitmap.h         
│   │   └── ringbuf.h        
│   ├── posix/              
│   │   ├── dirent.h
│   │   ├── fcntl.h
│   │   ├── stat.h
│   │   ├── unistd.h
│   │   ├── time.h
│   │   ├── sys/
│   │   │   ├── types.h
│   │   │   ├── stat.h
│   │   │   └── time.h
│   │   └── bits/
│   │       └── stat.h
│   └── errno.h           
├── kernel/
│   ├── main.c
│   ├── syscall.c
│   ├── linker.ld
│   ├── arch/
│   │   └── x86/
│   │       ├── boot.asm
│   │       ├── gdt.c
│   │       ├── idt.c
│   │       ├── io.c
│   │       ├── interrupt.asm
│   │       ├── timer.c       
│   │       ├── cmos.c        
│   │       └── pit.c       
│   ├── core/
│   │   ├── ipc.c
│   │   ├── process.c
│   │   ├── scheduler.c
│   │   └── memory.c
│   ├── fs/
│   │   ├── vfs.c
│   │   ├── fat32.c
│   │   ├── ramfs.c
│   │   ├── pipe.c          
│   │   ├── inode.c          
│   │   ├── path.c           
│   │   └── cache.c           
│   ├── mm/
│   │   ├── slab.c
│   │   ├── paging.c
│   │   ├── buddy.c
│   │   └── heap.c          
│   ├── drivers/
│   │   ├── serial.c
│   │   ├── ata.c
│   │   ├── keyboard.c    
│   │   ├── vga.c          
│   │   ├── rtc.c            
│   │   └── pci.c            
│   ├── sys/                
│   │   ├── file.c
│   │   ├── process.c
│   │   ├── ipc.c
│   │   ├── memory.c          
│   │   ├── time.c           
│   │   └── ioctl.c          
│   └── lib/                
│       ├── kstring.c
│       ├── kprintf.c
│       └── kassert.c
├── servers/
│   ├── init/
│   │   ├── main.c
│   │   └── Makefile
│   ├── fs/
│   │   ├── main.c
│   │   └── Makefile
│   ├── console/
│   │   ├── main.c
│   │   └── Makefile
│   └── tty/                 
│       ├── main.c
│       └── Makefile
├── lib/
│   ├── libc/
│   │   ├── string.c
│   │   ├── stdio.c
│   │   ├── stdlib.c          
│   │   ├── ctype.c           
│   │   ├── errno.c      
│   │   └── Makefile
│   └── posix/
│       ├── dirent.c
│       ├── fcntl.c
│       ├── stat.c
│       ├── unistd.c
│       ├── time.c
│       └── io.c              
├── user/
│   ├── ksh/
│   │   ├── main.c
│   │   ├── builtins.c      
│   │   └── Makefile
│   ├── test/                 
│   │   ├── fat_test.c
│   │   ├── posix_test.c
│   │   └── Makefile
│   └── utils/                
│       ├── ls.c
│       ├── cat.c
│       └── Makefile
├── iso/
│   ├── boot/
│   │   └── grub/
│   │       └── grub.cfg
│   └── modules/
└── tools/
    ├── mkinitrd.c
    ├── mkext2.c              
    └── mkfat.c               
```

# 🤝 加入我们：贡献指南

我们热烈欢迎任何形式的贡献！无论您是操作系统专家，还是充满热情的学习者。
```
1. 探索与学习：阅读 docs/ 下的文档和 src/ 下的代码，是理解微内核设计的最佳途径。
2. 报告问题与建议：如果您发现了Bug或有新想法，请在 GitHub Issues 中提出。
3. 提交代码：
   · Fork 本仓库。
   · 创建您的功能分支 (git checkout -b feature/AmazingFeature)。
   · 提交清晰的更改 (git commit -m 'Add some AmazingFeature')。
   · 推送到分支 (git push origin feature/AmazingFeature)。
   · 开启一个 Pull Request。
4. 完善文档：改进文档、注释或翻译同样价值连城。
```
在贡献前，请确保您理解并同意本项目的 GPL-3.0 开源协议条款。

# 📄 许可证

Copyright (c) 2024 Yankeelucas & OpenSovix Project Contributors.

本项目所有源代码均在 GNU General Public License v3.0 许可证下发布。详情请见项目根目录下的 LICENSE 文件。
