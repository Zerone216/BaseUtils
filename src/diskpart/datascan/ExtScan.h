/****************************************************************************************************************************
*
*	Copyright (c) 1998-2019  XI'AN SAMING TECHNOLOGY Co., Ltd
*	西安三茗科技股份有限公司  版权所有 1998-2019 
*
*   PROPRIETARY RIGHTS of XI'AN SAMING TECHNOLOGY Co., Ltd are involved in  the subject matter of this material.       
*   All manufacturing, reproduction,use, and sales rights pertaining to this subject matter are governed bythe license            
*   agreement. The recipient of this software implicitly accepts the terms of the license.	
*
*   本软件文档资料是西安三茗科技股份有限公司的资产,任何人士阅读和使用本资料必须获得相应的书面
*   授权,承担保密责任和接受相应的法律约束. 								     
*
*----------------------------------------------------------------------------------------------------------------------------
*
*   文 件 名 ： ExtScan.h 
*   文件描述 ：  
*   创建日期 ：2019年8月15日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __EXTSCAN_H__
#define __EXTSCAN_H__

#include <common/baselib/baselib.h>
#include "../diskopt/diskrw.h"
#include "scanbase.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*---------------------------------外部变量说明---------------------------------*/





/*---------------------------------外部函数原型说明---------------------------------*/





/*---------------------------------内部函数原型说明---------------------------------*/





/*---------------------------------全局变量---------------------------------*/





/*---------------------------------模块级变量---------------------------------*/





/*---------------------------------常量定义---------------------------------*/





/*---------------------------------宏定义---------------------------------*/

/*
 * Feature set definitions
 */
#define EXT2_SB(sb)     (sb)
#define EXT2_HAS_COMPAT_FEATURE(sb,mask)    ( DWORD)((sb->s_feature_compat) & (mask) )
#define EXT2_HAS_RO_COMPAT_FEATURE(sb,mask) ( DWORD)((sb->s_feature_ro_compat) & (mask) )
#define EXT2_HAS_INCOMPAT_FEATURE(sb,mask)  ( DWORD)((sb->s_feature_incompat) & (mask) )

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC    0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES   0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL     0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR        0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INO      0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX       0x0020
#define EXT2_FEATURE_COMPAT_LAZY_BG         0x0040
#define EXT2_FEATURE_COMPAT_ANY             0xffffffff


#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER     0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE       0x0002
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE        0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM         0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK        0x0020
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE      0x0040
#define EXT2_FEATURE_RO_COMPAT_ANY              0xffffffff

#define EXT2_FEATURE_INCOMPAT_COMPRESSION       0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE          0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER           0x0004
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV       0x0008
#define EXT4_FEATURE_INCOMPAT_META_BG           0x0010
#define EXT3_FEATURE_INCOMPAT_EXTENTS           0x0040
#define EXT4_FEATURE_INCOMPAT_64BIT             0x0080
#define EXT4_FEATURE_INCOMPAT_MMP               0x0100
#define EXT2_FEATURE_INCOMPAT_ANY               0xffffffff

#define EXT2_BG_BLOCK_UNINIT    0x0002 /* Block bitmap not initialized */

#define EXT2_MIN_DESC_SIZE_64BIT 64
#define EXT2_MIN_DESC_SIZE 32
 
#pragma pack(1)

/*
 * Structure of the super block
 */
struct ext2_super_block {
    DWORD   s_inodes_count;     /* Inodes count */
    DWORD   s_blocks_count;     /* Blocks count */
    DWORD   s_r_blocks_count;   /* Reserved blocks count */
    DWORD   s_free_blocks_count;    /* Free blocks count */
    DWORD   s_free_inodes_count;    /* Free inodes count */
    DWORD   s_first_data_block; /* First Data Block */
    DWORD   s_log_block_size;   /* Block size */
    DWORD   s_log_cluster_size; /* Allocation cluster size */
    DWORD   s_blocks_per_group; /* # Blocks per group */
    DWORD   s_clusters_per_group;   /* # Fragments per group */
    DWORD   s_inodes_per_group; /* # Inodes per group */
    DWORD   s_mtime;        /* Mount time */
    DWORD   s_wtime;        /* Write time */
    WORD    s_mnt_count;        /* Mount count */
    int16_t s_max_mnt_count;    /* Maximal mount count */
    WORD    s_magic;        /* Magic signature */
    WORD    s_state;        /* File system state */
    WORD    s_errors;       /* Behaviour when detecting errors */
    WORD    s_minor_rev_level;  /* minor revision level */
    DWORD   s_lastcheck;        /* time of last check */
    DWORD   s_checkinterval;    /* max. time between checks */
    DWORD   s_creator_os;       /* OS */
    DWORD   s_rev_level;        /* Revision level */
    WORD    s_def_resuid;       /* Default uid for reserved blocks */
    WORD    s_def_resgid;       /* Default gid for reserved blocks */
    /*
     * These fields are for EXT2_DYNAMIC_REV superblocks only.
     *
     * Note: the difference between the compatible feature set and
     * the incompatible feature set is that if there is a bit set
     * in the incompatible feature set that the kernel doesn't
     * know about, it should refuse to mount the filesystem.
     *
     * e2fsck's requirements are more strict; if it doesn't know
     * about a feature in either the compatible or incompatible
     * feature set, it must abort and not try to meddle with
     * things it doesn't understand...
     */
    DWORD   s_first_ino;        /* First non-reserved inode */
    WORD    s_inode_size;       /* size of inode structure */
    WORD    s_block_group_nr;   /* block group # of this superblock */
    DWORD   s_feature_compat;   /* compatible feature set */
    DWORD   s_feature_incompat; /* incompatible feature set */
    DWORD   s_feature_ro_compat;    /* readonly-compatible feature set */
    BYTE    s_uuid[16];     /* 128-bit uuid for volume */
    char    s_volume_name[16];  /* volume name */
    char    s_last_mounted[64]; /* directory where last mounted */
    DWORD   s_algorithm_usage_bitmap; /* For compression */
    /*
     * Performance hints.  Directory preallocation should only
     * happen if the EXT2_FEATURE_COMPAT_DIR_PREALLOC flag is on.
     */
    BYTE    s_prealloc_blocks;  /* Nr of blocks to try to preallocate*/
    BYTE    s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
    WORD    s_reserved_gdt_blocks;  /* Per group table for online growth */
    /*
     * Journaling support valid if EXT2_FEATURE_COMPAT_HAS_JOURNAL set.
     */
    BYTE    s_journal_uuid[16]; /* uuid of journal superblock */
    DWORD   s_journal_inum;     /* inode number of journal file */
    DWORD   s_journal_dev;      /* device number of journal file */
    DWORD   s_last_orphan;      /* start of list of inodes to delete */
    DWORD   s_hash_seed[4];     /* HTREE hash seed */
    BYTE    s_def_hash_version; /* Default hash version to use */
    BYTE    s_jnl_backup_type;  /* Default type of journal backup */
    WORD    s_desc_size;        /* Group desc. size: INCOMPAT_64BIT */
    DWORD   s_default_mount_opts;
    DWORD   s_first_meta_bg;    /* First metablock group */
    DWORD   s_mkfs_time;        /* When the filesystem was created */
    DWORD   s_jnl_blocks[17];   /* Backup of the journal inode */
    DWORD   s_blocks_count_hi;  /* Blocks count high 32bits */
    DWORD   s_r_blocks_count_hi;    /* Reserved blocks count high 32 bits*/
    DWORD   s_free_blocks_hi;   /* Free blocks count */
    WORD    s_min_extra_isize;  /* All inodes have at least # bytes */
    WORD    s_want_extra_isize;     /* New inodes should reserve # bytes */
    DWORD   s_flags;        /* Miscellaneous flags */
    WORD   s_raid_stride;       /* RAID stride */
    WORD   s_mmp_update_interval;  /* # seconds to wait in MMP checking */
    ULONGLONG   s_mmp_block;            /* Block for multi-mount protection */
    DWORD   s_raid_stripe_width;    /* blocks on all data disks (N*stride)*/
    BYTE    s_log_groups_per_flex;  /* FLEX_BG group size */
    BYTE    s_reserved_char_pad;
    WORD    s_reserved_pad;     /* Padding to next 32bits */
    ULONGLONG   s_kbytes_written;   /* nr of lifetime kilobytes written */
    DWORD   s_snapshot_inum;    /* Inode number of active snapshot */
    DWORD   s_snapshot_id;      /* sequential ID of active snapshot */
    ULONGLONG   s_snapshot_r_blocks_count; /* reserved blocks for active
                          snapshot's future use */
    DWORD   s_snapshot_list;    /* inode number of the head of the on-disk snapshot list */
//#define EXT4_S_ERR_START ext4_offsetof(struct ext2_super_block, s_error_count)
    DWORD   s_error_count;      /* number of fs errors */
    DWORD   s_first_error_time; /* first time an error happened */
    DWORD   s_first_error_ino;  /* inode involved in first error */
    ULONGLONG   s_first_error_block;    /* block involved of first error */
    BYTE    s_first_error_func[32]; /* function where the error happened */
    DWORD   s_first_error_line; /* line number where error happened */
    DWORD   s_last_error_time;  /* most recent time of an error */
    DWORD   s_last_error_ino;   /* inode involved in last error */
    DWORD   s_last_error_line;  /* line number where error happened */
    ULONGLONG   s_last_error_block; /* block involved of last error */
    BYTE    s_last_error_func[32];  /* function where the error happened */
//#define EXT4_S_ERR_END ext4_offsetof(struct ext2_super_block, s_mount_opts)
    BYTE    s_mount_opts[64];
    DWORD   s_usr_quota_inum;   /* inode number of user quota file */
    DWORD   s_grp_quota_inum;   /* inode number of group quota file */
    DWORD   s_overhead_blocks;  /* overhead blocks/clusters in fs */
    DWORD   s_reserved[108];        /* Padding to the end of the block */
    DWORD   s_checksum;     /* crc32c(superblock) */
};


/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
    DWORD   bg_block_bitmap;    /* Blocks bitmap block */
    DWORD   bg_inode_bitmap;    /* Inodes bitmap block */
    DWORD   bg_inode_table;     /* Inodes table block */
    WORD    bg_free_blocks_count;   /* Free blocks count */
    WORD    bg_free_inodes_count;   /* Free inodes count */
    WORD    bg_used_dirs_count; /* Directories count */
    WORD    bg_flags;
    DWORD   bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
    WORD    bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
    WORD    bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
    WORD    bg_itable_unused;   /* Unused inodes count */
    WORD    bg_checksum;        /* crc16(s_uuid+grouo_num+group_desc)*/
};
/*
 * Structure of a blocks group descriptor
 */
struct ext4_group_desc
{
    DWORD   bg_block_bitmap;    /* Blocks bitmap block */
    DWORD   bg_inode_bitmap;    /* Inodes bitmap block */
    DWORD   bg_inode_table;     /* Inodes table block */
    WORD    bg_free_blocks_count;   /* Free blocks count */
    WORD    bg_free_inodes_count;   /* Free inodes count */
    WORD    bg_used_dirs_count; /* Directories count */
    WORD    bg_flags;       /* EXT4_BG_flags (INODE_UNINIT, etc) */
    DWORD   bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
    WORD    bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
    WORD    bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
    WORD    bg_itable_unused;   /* Unused inodes count */
    WORD    bg_checksum;        /* crc16(sb_uuid+group+desc) */
    DWORD   bg_block_bitmap_hi; /* Blocks bitmap block MSB */
    DWORD   bg_inode_bitmap_hi; /* Inodes bitmap block MSB */
    DWORD   bg_inode_table_hi;  /* Inodes table block MSB */
    WORD    bg_free_blocks_count_hi;/* Free blocks count MSB */
    WORD    bg_free_inodes_count_hi;/* Free inodes count MSB */
    WORD    bg_used_dirs_count_hi;  /* Directories count MSB */
    WORD    bg_itable_unused_hi;    /* Unused inodes count MSB */
    DWORD   bg_exclude_bitmap_hi;   /* Exclude bitmap block MSB */
    WORD    bg_block_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+bitmap) MSB */
    WORD    bg_inode_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+bitmap) MSB */
    DWORD   bg_reserved;
};

typedef struct _EXT_DATA_SCAN_INFO_
{
	int diskfd;
	DDWORD partBeginSec;
	DDWORD partTotalSec;
	BYTE sectorBuf[SECTOR_SIZE * 2];                     //读BITMAP表时用的缓冲
	char partName[32];
    DWORD block_size_byte;      //块大小 1024 2048 4096字节
    DWORD block_size_sector;    //2 4 8
    DWORD group_count;          //组总数
    DWORD s_first_data_block;   //第一个数据块所在块
    DWORD blocks_per_group;     //一组有多少个块
    DWORD blocks_last_group;    //最后一组有多少个块
    DWORD group_desc_begsec;    //组描述符起始扇区，相对于分区起始扇区
    DWORD group_begsec;         //第0组起始扇区，相对于分区起始扇区 
    WORD  csum_flag;
    DWORD s_desc_size;
}EXT_DATA_SCAN_INFO, *pEXT_DATA_SCAN_INFO;


#pragma pack()

 int ext2_get_group_desc(EXT_DATA_SCAN_INFO * pDataScanInfo, int groupNum, struct ext2_group_desc *ext2GroupDesc);
 int ext2_get_val_by_bgflags(struct ext2_group_desc ext2_gdp, WORD bgFlag);

 int ext2_datascan_info_display(EXT_DATA_SCAN_INFO * pDataScanInfo);

 EXT_DATA_SCAN_INFO * ext2_datascan_init(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec);
 int ext2_get_block_bitmap(EXT_DATA_SCAN_INFO * pDataScanInfo, int groupNum, BYTE * buff, int bufSize);
 void ext2_datascan_end(EXT_DATA_SCAN_INFO ** pDataScanInfo);

 BYTE * ext_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize);
 int ext_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __EXTSCAN_H__ */
