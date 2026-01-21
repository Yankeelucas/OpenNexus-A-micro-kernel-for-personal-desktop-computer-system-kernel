// SPDX-License-Identifier: GPL-3.0-or-later
// fs_module.h - 文件系统模块接口定义
#ifndef __FS_MODULE_H__
#define __FS_MODULE_H__

#include "kernel.h"

// ======================
// 文件系统类型定义
// ======================
typedef enum {
    FS_TYPE_UNKNOWN = 0,
    FS_TYPE_FAT12,
    FS_TYPE_FAT16,
    FS_TYPE_FAT32,
    FS_TYPE_EXT2,
    FS_TYPE_EXT3,
    FS_TYPE_NTFS,
    FS_TYPE_ISO9660,
    FS_TYPE_RAMFS,
    FS_TYPE_TMPFS
} FileSystemType;

// ======================
// 文件属性定义
// ======================
typedef enum {
    FS_ATTR_READ_ONLY  = 0x01,
    FS_ATTR_HIDDEN     = 0x02,
    FS_ATTR_SYSTEM     = 0x04,
    FS_ATTR_DIRECTORY  = 0x10,
    FS_ATTR_ARCHIVE    = 0x20,
    FS_ATTR_DEVICE     = 0x40,
    FS_ATTR_SYMLINK    = 0x80
} FileAttributes;

// ======================
// 文件句柄定义
// ======================
typedef struct {
    u32 id;
    char path[256];
    u32 position;
    u32 size;
    FileAttributes attributes;
    u64 create_time;
    u64 modify_time;
    u64 access_time;
    FileSystemType fs_type;
    void* fs_private;      // 文件系统私有数据
    void* driver_private;  // 驱动私有数据
} FileHandle;

// ======================
// 目录项定义
// ======================
typedef struct {
    char name[256];
    FileAttributes attributes;
    u32 size;
    u64 create_time;
    u64 modify_time;
} DirEntry;

// ======================
// 文件系统操作接口 (15个函数)
// ======================

typedef struct {
    // 基本函数
    ErrorCode (*init)(void* params);
    ErrorCode (*exit)(void);
    ErrorCode (*query)(ModuleInfo* info);
    
    // 文件操作接口 (函数1-5)
    FileHandle* (*open)(const char* path, u32 mode);
    ErrorCode (*close)(FileHandle* file);
    u32 (*read)(FileHandle* file, void* buffer, u32 size);
    u32 (*write)(FileHandle* file, const void* buffer, u32 size);
    ErrorCode (*seek)(FileHandle* file, u32 offset, u32 whence);
    
    // 目录操作接口 (函数6-10)
    ErrorCode (*mkdir)(const char* path);
    ErrorCode (*rmdir)(const char* path);
    DirEntry* (*readdir)(const char* path, u32* count);
    ErrorCode (*find)(const char* pattern, DirEntry** results, u32* count);
    ErrorCode (*stat)(const char* path, FileHandle* info);
    
    // 高级功能接口 (函数11-15)
    ErrorCode (*format)(const char* device, FileSystemType type, void* params);
    ErrorCode (*mount)(const char* device, const char* mountpoint, FileSystemType type);
    ErrorCode (*unmount)(const char* mountpoint);
    ErrorCode (*fsck)(const char* device);  // 文件系统检查
    ErrorCode (*defrag)(const char* device); // 碎片整理
    
    // 模块私有数据
    void* private_data;
} FileSystemModuleExports;

// ======================
// 具体文件系统实现接口
// ======================

// NTFS文件系统操作
typedef struct {
    // NTFS特定函数
    ErrorCode (*ntfs_init)(void* params);
    ErrorCode (*ntfs_read_mft)(void);
    ErrorCode (*ntfs_read_attr)(FileHandle* file, const char* attr_name, void* buffer, u32 size);
    ErrorCode (*ntfs_write_attr)(FileHandle* file, const char* attr_name, const void* buffer, u32 size);
    ErrorCode (*ntfs_compress)(FileHandle* file);
    ErrorCode (*ntfs_decompress)(FileHandle* file);
    
    // 通用文件系统接口
    FileSystemModuleExports common;
} NTFSModuleExports;

// ext2/ext3文件系统操作
typedef struct {
    // ext2/ext3特定函数
    ErrorCode (*ext_init)(void* params);
    ErrorCode (*ext_read_inode)(u32 inode, void* buffer);
    ErrorCode (*ext_write_inode)(u32 inode, const void* data);
    ErrorCode (*ext_read_superblock)(void* buffer);
    ErrorCode (*ext_journal_begin)(void);
    ErrorCode (*ext_journal_commit)(void);
    
    // 通用文件系统接口
    FileSystemModuleExports common;
} ExtModuleExports;

// ======================
// 虚拟文件系统层 (VFS)
// ======================

// 挂载点结构
typedef struct MountPoint {
    char device[64];
    char mountpoint[256];
    FileSystemType fs_type;
    void* fs_private;
    struct MountPoint* next;
} MountPoint;

// VFS操作
typedef struct {
    // 挂载管理
    ErrorCode (*vfs_mount)(const char* device, const char* mountpoint, FileSystemType type);
    ErrorCode (*vfs_unmount)(const char* mountpoint);
    MountPoint* (*vfs_find_mount)(const char* path);
    
    // 路径解析
    ErrorCode (*vfs_resolve)(const char* path, char* resolved);
    ErrorCode (*vfs_split)(const char* path, char* dir, char* file);
    
    // 文件系统探测
    FileSystemType (*vfs_probe)(const char* device);
    
    // 缓存管理
    ErrorCode (*vfs_cache_init)(u32 size);
    ErrorCode (*vfs_cache_flush)(void);
} VirtualFileSystem;

// ======================
// 文件系统管理器
// ======================

typedef struct {
    // 模块管理
    ErrorCode (*register_fs)(FileSystemType type, FileSystemModuleExports* exports);
    ErrorCode (*unregister_fs)(FileSystemType type);
    FileSystemModuleExports* (*get_fs)(FileSystemType type);
    
    // 设备管理
    ErrorCode (*register_device)(const char* name, void* driver);
    ErrorCode (*unregister_device)(const char* name);
    
    // 性能监控
    void (*fs_stats)(FileSystemType type, void* stats);
    void (*io_stats)(void);
    
    // 调试支持
    ErrorCode (*fs_debug)(FileSystemType type, u32 command, void* param);
} FileSystemManager;

// ======================
// 公共API函数声明
// ======================

// 文件系统模块注册
ErrorCode fs_module_register(FileSystemType type, FileSystemModuleExports* exports);

// VFS初始化
ErrorCode vfs_init(void);

// 标准文件操作（通过VFS）
FileHandle* fs_open(const char* path, u32 mode);
ErrorCode fs_close(FileHandle* file);
u32 fs_read(FileHandle* file, void* buffer, u32 size);
u32 fs_write(FileHandle* file, const void* buffer, u32 size);

// 目录操作
ErrorCode fs_mkdir(const char* path);
ErrorCode fs_rmdir(const char* path);
DirEntry* fs_readdir(const char* path, u32* count);

// 文件系统管理
ErrorCode fs_mount(const char* device, const char* mountpoint, FileSystemType type);
ErrorCode fs_unmount(const char* mountpoint);
ErrorCode fs_format(const char* device, FileSystemType type);

#endif // __FS_MODULE_H__
