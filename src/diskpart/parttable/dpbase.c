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
*   文 件 名 ： dpbase.c 
*   文件描述 ：  
*   创建日期 ：2019年8月16日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
#include <stdarg.h>	/* ANSI C header file */
#include <syslog.h> 	/* for syslog() */
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <linux/hdreg.h>
#include <getopt.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/io.h>

#include "dpbase.h"

int open_disk(char * devname, int rwMode)
{
	int diskfd = open(devname, rwMode);
	if(diskfd < 0 )
	{
		assert(0);
		eLog("open disk[%s] failed!", devname);
		return -1;
	}

	iLog("open disk[%s] succeed! diskfd=[%d]", devname, diskfd);
	return diskfd;
}

void close_disk(int * diskfd)
{
	if(*diskfd)
	{
		close(*diskfd);
		*diskfd = 0;
		return;
	}
	
	return;
}

int get_disk_guid(BYTE ptMode, int diskfd, BYTE * diskGuid)
{
	BYTE sector[SECTOR_SIZE];
	if(ptMode == DISK_PART_TABLE_GPT)
	{
		if(readwrite_hdisk_sector(DISK_READ, diskfd, 1, 1, sector) < 0) //读取1扇区
			return -1;
		
		GPT_HEADER * pGptHeader = (GPT_HEADER *)sector;
		memcpy(diskGuid, pGptHeader->hdr_guid, 16);
		iLog("diskGuid=[" FMT_GUID_LOWER "]", SNIF_GUID(diskGuid));
	}
	else if(ptMode == DISK_PART_TABLE_MBR)
	{
		if(readwrite_hdisk_sector(DISK_READ, diskfd, 0, 1, sector) < 0) //读取0扇区
			return -1;
		
		PROTECT_MBR_SECTOR * pMbrSec = (PROTECT_MBR_SECTOR *)sector;
		memcpy(diskGuid, &pMbrSec->diskUuid, 4);
		iLog("diskGuid=[" FMT_DOS_UUID_LOWER "]", SNIF_DOS_UUID(diskGuid));
	}
	else
		;
	
	return 0;
}

int get_local_disk_num()
{
	int disknum = 0;
	//char * result = get_cmd_result("cat /proc/partitions | awk '{print $4}' | grep -Eo '[s|h]d[a-z]$|nbd[0-9]+$|nvme[0-9]+n[0-9]+$' | wc -l");
	char * result = get_cmd_result("lsblk -lb | grep disk | awk '{print $1}' | wc -l");
	if(result == NULL)
		return disknum;
	
	disknum = atoi(result);
	free_cmd_result(&result);

	iLog("disknum=[%d]", disknum);
	return disknum;
}

int get_disk_devname_by_index(char * devName, int index)
{
	//char * result = get_cmd_result("cat /proc/partitions | awk '{print $4}' | grep -Eo '[s|h]d[a-z]$|nbd[0-9]+$|nvme[0-9]+n[0-9]+$' | sed -n '%d'p", index + 1);
	char * result = get_cmd_result("lsblk -lb | grep disk | awk '{print $1}' | sed -n '%d'p", index + 1);
	if(result == NULL)
		return -1;
	
	sprintf(devName, "/dev/%s", result);
	free_cmd_result(&result);

	iLog("devName=[%s]", devName);
	return 0;
}

BYTE get_disk_removable(char * devName)
{
	char * volume = devName + str_find_last_of((const char *)devName, '/') + 1; //去掉 /dev/
	char * result = get_cmd_result("cat /sys/block/%s/removable", volume);
	if(result == NULL)
		return -1;
	
	int removable = atoi(result);
	free_cmd_result(&result);

	iLog("removable=[%d]", removable);
	return removable;
}

int get_dev_node_info(char * devName, DEV_NODE_INFO * devNode)
{
	char * result = NULL;
	char * volume = devName + str_find_last_of((const char *)devName, '/') + 1; //去掉 /dev/
	
	result = get_cmd_result("cat /proc/partitions | grep -E '%s$' | awk '{print $1}'", volume);
	if(result == NULL)
		return -1;
	
	devNode->major = atoi(result);
	free_cmd_result(&result);
	iLog("major=[%d]", devNode->major);
	
	result = get_cmd_result("cat /proc/partitions | grep -E '%s$' | awk '{print $2}'", volume);
	if(result == NULL)
		return -1;
	
	devNode->minor = atoi(result);
	free_cmd_result(&result);
	iLog("minor=[%d]", devNode->minor);
	
	return 0;
}


int get_disk_serial_num(char * devName, char * serialNum, int bufLen)
{
	char * volume = devName + str_find_last_of((const char *)devName, '/') + 1; //去掉 /dev/
	char * result = get_cmd_result("udevadm info --path=/sys/class/block/%s | grep ID_SERIAL= | cut -d= -f2", volume);
	if(result == NULL)
		return 0;
	
	strncpy(serialNum, result, bufLen);
	free_cmd_result(&result);
	
	iLog("serialNum=[%s]", serialNum);
	return 0;
}

BYTE get_disk_interface_type(char * devName)
{
	char * volume = devName + str_find_last_of((const char *)devName, '/') + 1; //去掉 /dev/
	
	//判断移动硬盘。
	char * result = get_cmd_result("udevadm info --path=/sys/class/block/%s | grep ID_BUS= | cut -d= -f2", volume);
	if(result == NULL)
		return DISK_INTERFACE_UNKNOWN;

	char interfaceType[16];
	strncpy(interfaceType, result, sizeof(interfaceType));
	free_cmd_result(&result);
	
	iLog("interfaceType=[%s]", interfaceType);

	if(strncasecmp(interfaceType, "ata", strlen(interfaceType)) == 0)
		return DISK_INTERFACE_ATA;
	else if(strncasecmp(interfaceType, "m2", strlen(interfaceType)) == 0)
		return DISK_INTERFACE_M2;
	else if(strncasecmp(interfaceType, "scsi", strlen(interfaceType)) == 0)
		return DISK_INTERFACE_SCSI;
	else if(strncasecmp(interfaceType, "usb", strlen(interfaceType)) == 0)
		return DISK_INTERFACE_USB;
	else
		return DISK_INTERFACE_UNKNOWN;
}

BYTE get_disk_parttable_mode(char * devName)
{
	//char * volume = devName + str_find_last_of((const char *)devName, '/') + 1; //去掉 /dev/
	//char * result = get_cmd_result("udevadm info --path=/sys/class/block/%s | grep ID_PART_TABLE_TYPE= | cut -d= -f2", volume);
	
	char * result = get_cmd_result("blkid -p %s | xargs -n1 | grep -E '^PTTYPE=' | cut -d= -f2", devName);
	if(result == NULL)
		return DISK_PART_TABLE_UNKNOWN;
	
	char partTableMode[16];
	strncpy(partTableMode, result, sizeof(partTableMode));
	free_cmd_result(&result);
	
	iLog("partTableMode=[%s]", partTableMode);

	if(strncasecmp(partTableMode, "dos", strlen(partTableMode)) == 0)
		return DISK_PART_TABLE_MBR;
	else if(strncasecmp(partTableMode, "gpt", strlen(partTableMode)) == 0)
		return DISK_PART_TABLE_GPT;
	else
		return DISK_PART_TABLE_UNKNOWN;
}

//获得chs参数
DISK_CHS get_disk_geometry(char *devName, int diskfd)
{
	struct hd_geometry hg;
	memset(&hg, 0x00, sizeof(struct hd_geometry));

	DISK_CHS diskChs;
	memset(&diskChs, 0x00, sizeof(DISK_CHS));
	
	DDWORD size64 = 0;
	if(ioctl(diskfd, BLKGETSIZE64, &size64))
	{
		assert(0);
		eLog("Disk[%s]: ioctl BLKGETSIZE64 error!", devName);
		return diskChs;
	}
	
	diskChs.size = size64 / SECTOR_SIZE;
	
	if(ioctl(diskfd, HDIO_GETGEO, &hg))//块设备
	{
		assert(0);
		eLog("Disk[%s]: ioctl HDIO_GETGEO error!", devName);
		return diskChs;
	}
	
	diskChs.cylinders = hg.cylinders;
	diskChs.heads = hg.heads;
	diskChs.sectors = hg.sectors;
	diskChs.start = hg.start;
	diskChs.cylinderSize = diskChs.heads * diskChs.sectors;
	
	iLog("C.H.S=[%d,%d,%d]", diskChs.cylinders, diskChs.heads, diskChs.sectors);
	
	return diskChs;
}


int get_part_dev_name(DDWORD beginSec, char * diskDevName, char * partDevName, int bufLen)
{
	char * result = get_cmd_result("fdisk -l %s | grep -E '^%s' | grep -E ' %llu ' | awk '{print $1}'", diskDevName, diskDevName, beginSec);
	if(result == NULL)
		return -1;
	
	strncpy(partDevName, result, bufLen);
	free_cmd_result(&result);
	
	iLog("partDevName=[%s]", partDevName);
	return 0;
}

int get_part_label_name(char * partDevName, char * partLabel, int bufLen)
{
	//char * result = get_cmd_result("blkid -p %s | xargs -n1 | grep -E '^LABEL=' | cut -d= -f2", partDevName);
	
	char * volume = partDevName + str_find_last_of((const char *)partDevName, '/') + 1; //去掉 /dev/
	char * result = get_cmd_result("udevadm info --path=/sys/class/block/%s | grep ID_FS_LABEL= | cut -d= -f2", volume);
	if(result == NULL)
		return 0;
	
	strncpy(partLabel, result, bufLen);
	free_cmd_result(&result);
	
	iLog("partLabel=[%s]", partLabel);
	return 0;
}

int get_mbr_part_guid(char * partDevName, BYTE * partGuid, int bufLen)
{
	//char * volume = partDevName + str_find_last_of((const char *)partDevName, '/') + 1; //去掉 /dev/
	//char * result = get_cmd_result("udevadm info --path=/sys/class/block/%s | grep ID_PART_ENTRY_UUID= | cut -d= -f2", volume);
	
	char * result = get_cmd_result("blkid -p %s | xargs -n1 | grep -E '^PART_ENTRY_UUID=' | cut -d= -f2", partDevName);
	if(result == NULL)
		return 0;
	
	strncpy((char*)partGuid, result, strlen(result));
	free_cmd_result(&result);
	
	char * pStr = byte2string(partGuid, bufLen);
	iLog("partGuid=[%s]", pStr);
	str_free(&pStr);
	
	return 0;
}

int get_mbr_part_type_guid(char * partDevName, BYTE * partType, int bufLen)
{
	//char * volume = partDevName + str_find_last_of((const char *)partDevName, '/') + 1; //去掉 /dev/
	//char * result = get_cmd_result("udevadm info --path=/sys/class/block/%s | grep ID_FS_UUID= | cut -d= -f2", volume);

	char * result = get_cmd_result("blkid -p %s | xargs -n1 | grep -E '^UUID=' | cut -d= -f2", partDevName);
	if(result == NULL)
		return 0;
	
	strncpy((char*)partType, result, strlen(result));
	free_cmd_result(&result);

	char * pStr = byte2string(partType, bufLen);
	iLog("typeGuid=[%s]", pStr);
	str_free(&pStr);
	
	return 0;
}

BYTE get_part_filesys(char * partDevName)
{
	char * result = get_cmd_result("blkid -p %s | xargs -n1 | grep -E '^TYPE=' | cut -d= -f2", partDevName);
	if(result == NULL)
		return PT_FS_UNFORMAT;
	
	char fileSys[32];
	strncpy(fileSys, result, sizeof(fileSys));
	free_cmd_result(&result);
	iLog("fileSys=[%s]", fileSys);
	
	if(strncasecmp(fileSys, "ntfs", strlen("ntfs")) == 0) //ntfs
	{
		return PT_FS_NTFS;
	}
	else if(strncasecmp(fileSys, "vfat", strlen("vfat")) == 0) //fat32 or fat16
	{
		//char * result1 = get_cmd_result("udevadm info --path=/sys/class/block/%s | grep ID_FS_VERSION= | cut -d= -f2", volume);
		char * result1 = get_cmd_result("blkid -p %s | xargs -n1 | grep -E '^VERSION=' | cut -d= -f2", partDevName);
		if(result1 == NULL)
			return PT_FS_UNFORMAT;
		
		char fileSysVer[32];
		strncpy(fileSysVer, result1, sizeof(fileSysVer));
		free_cmd_result(&result1);
		iLog("fileSysVer=[%s]", fileSysVer);
		if(strncasecmp(fileSysVer, "FAT32", strlen("FAT32")) == 0) //FAT32
			return PT_FS_FAT32;
		else if(strncasecmp(fileSysVer, "FAT16", strlen("FAT16")) == 0) //FAT16
			return PT_FS_FAT16;
		else
			return PT_FS_UNKNOWN;
	}
	else if(strncasecmp(fileSys, "ext2", strlen("ext2")) == 0) //ext2
	{
		return PT_FS_LINUX_EXT2;
	}
	else if(strncasecmp(fileSys, "ext3", strlen("ext3")) == 0) //ext3
	{
		return PT_FS_LINUX_EXT3;
	}
	else if(strncasecmp(fileSys, "ext4", strlen("ext4")) == 0) //ext4
	{
		return PT_FS_LINUX_EXT4;
	}
	else
		return PT_FS_UNKNOWN;
}
