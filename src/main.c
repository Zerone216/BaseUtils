/****************************************************************************************************************************
*
*   文 件 名 ： main.c 
*   文件描述 ：  
*   创建日期 ：2019年5月13日
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
#include <locale.h>
#include <wchar.h>

#include <common/common.h>
#include <diskpart/diskpart.h>
#include <network/network.h>

int test_UCS(char * testStr)
{
	int allocLen = 1024;
	char * utf8Str = str_dup(testStr, allocLen);//
	
	int utf8Len = strlen(utf8Str);
	iLog("len=[%d]", utf8Len);
	iLog("utf8Str:[%s]", utf8Str);
	dump_mem_to_file(P_BYTE(utf8Str), utf8Len, "UNICODE-UTF8", FALSE);
	
	////////////////////////////sconv///////////////////////////////////
	iLog("sizeof(wchar)=[%d]", sizeof(wchar));
	int uniLen = utf8Len + 1;
	wchar * wcharStr = (wchar *)z_malloc(uniLen * sizeof(wchar));
	sconv_utf8_to_unicode(utf8Str, strlen(utf8Str), wcharStr, uniLen);
	dump_mem_to_file(P_BYTE(wcharStr), uniLen, "UNICODE-SCONV", FALSE);
	sconv_unicode_to_utf8(wcharStr, uniLen, utf8Str, allocLen);
	iLog("unicode_to_utf8:[%s]", utf8Str);
	z_free(&wcharStr);
	
	int unicodeLen = 0;
	//////////////////////////////UCS-2/////////////////////////////////
	iLog("sizeof(WORD)=[%d]", sizeof(WORD));
	WORD * unicodeStr = (WORD *)z_malloc(utf8Len * sizeof(WORD));
	unicodeLen = StrUtf8ToUnicode(utf8Str, utf8Len, UCS_2, (void *)unicodeStr, utf8Len);
	iLog("unicodeLen=[%d]", unicodeLen);
	iLog("UcsType=[%d]", GetUnicodeUcsType((void *)unicodeStr));
	iLog("GetUnicodeLen=[%d]", GetUnicodeLen((void *)unicodeStr));
	
	dump_mem_to_file(P_BYTE(unicodeStr), unicodeLen * sizeof(WORD), "UNICODE-UCS2", FALSE);
	utf8Len = StrUnicodeToUtf8((void *)unicodeStr, unicodeLen, utf8Str, allocLen);
	iLog("utf8Len=[%d]", utf8Len);
	iLog("UnicodeToUtf8:[%s]", utf8Str);
	z_free(&unicodeStr);
	
	//////////////////////////////UCS-4/////////////////////////////////
	iLog("sizeof(wchar_t)=[%d]", sizeof(wchar_t));
	int ucs4Len = utf8Len + 1;
	iLog("ucs4Len=[%d]", ucs4Len);
	wchar_t * wchartStr = (wchar_t *)z_malloc(ucs4Len * sizeof(wchar_t));
	unicodeLen = StrUtf8ToUnicode(utf8Str, utf8Len, UCS_4, (void *)wchartStr, ucs4Len);
	iLog("unicodeLen=[%d]", unicodeLen);
	iLog("wcslen(wchartStr)=[%d]", wcslen(wchartStr));
	dump_mem_to_file(P_BYTE(wchartStr), ucs4Len * sizeof(wchar_t) , "UNICODE-UCS4", FALSE);
	iLog("UcsType=[%d]", GetUnicodeUcsType((void *)wchartStr));
	iLog("GetUnicodeLen=[%d]", GetUnicodeLen((void *)wchartStr));
	
	utf8Len = StrUnicodeToUtf8((void *)wchartStr, unicodeLen, utf8Str, allocLen);
	iLog("utf8Len=[%d]", utf8Len);
	iLog("UnicodeToUtf8:[%s]", utf8Str);
	
	//////////////////////////////WCHAR/////////////////////////////////
	wchar_t * wch = wcs_dup(wchartStr, 512);
	iLog("wcslen(wch)=[%d]", wcslen(wch));
	dump_mem_to_file(P_BYTE(wch), wcslen(wch) * sizeof(wchar_t), "UNICODE-WCS", FALSE);
	utf8Len = StrUnicodeToUtf8((DWORD *)wch, unicodeLen, utf8Str, allocLen);
	iLog("utf8Len=[%d]", utf8Len);
	iLog("UnicodeToUtf8:[%s]", utf8Str);
	wcs_free(&wch);
	z_free(&wchartStr);
	str_free(&utf8Str);
	return 0;
}

int test_openssl(char * fileName)
{
	int dLen = 0;
	BYTE * data = file_dump(fileName, &dLen);
	if(data)
	{
		md_t mdOutput;
		iLog("MD4[%s]: " FMT_GUID_UPPER, fileName, SNIF_GUID(P_BYTE(md_sum(MD5_CKSUM, data, dLen, mdOutput))));
		iLog("MD5[%s]: " FMT_GUID_UPPER, fileName, SNIF_GUID(P_BYTE(md_sum(MD4_CKSUM, data, dLen, mdOutput))));

		sha_t shaOutput;
		iLog("SHA[%s]: " FMT_GUID_UPPER, fileName, SNIF_GUID(P_BYTE(sha_sum(SHA_CKSUM, data, dLen, shaOutput))));
		iLog("SHA1[%s]: " FMT_GUID_UPPER, fileName, SNIF_GUID(P_BYTE(sha_sum(SHA1_CKSUM, data, dLen, shaOutput))));
		
		safe_free(&data);
	}
	
	return 0;
}

int test_uuidgen()
{
	uuid_t uuid;
	generate_uuid(uuid, MODE_UUID_GEN_RANDOM);
	char * guid = unparse_uuid(uuid, CASE_UUID_PARSE_UPPER);
	iLog("guid=[%s]", guid);
	str_free(&guid);
	return 0;
}


int test_timeTools(char * timeExp1, char * timeExp2)
{
	struct tm timeStart = GetTimeInfoFromExpress(timeExp1);
	struct tm timeEnd = GetTimeInfoFromExpress(timeExp2);
	iLog("TimePass=[%llu]", CalcTimePassByAssignPeriod(timeStart, timeEnd));
	return 0;
}

int test_regex_match(char * srcString, char * parten)
{
	MATCH_RESULT * matchStrs = regex_match_all_substr(srcString, parten);
	if(matchStrs)
	{
		show_substr(matchStrs);
		destroy_substr(&matchStrs);
	}

	return 0;
}

int test_netcard_init()
{
	LOCAL_NETCARD_INFO * localNetcard = local_netcard_info_init();
	if(localNetcard)
	{
		local_netcard_info_display(localNetcard);
		local_netcard_info_release(&localNetcard);
	}
	
	return 0;
}

int test_harddisk_init(int diskIndex, int partIndex)
{
	LOCAL_DISK_INFO * localDisk = local_disk_info_init();
	if(localDisk)
	{
		DEF_DISK_INFO * pDiskInfo = &localDisk->diskInfo[diskIndex];
		DEF_PART_INFO *pPartInfo = &pDiskInfo->partInfo[partIndex];
		PART_BITMAP_INFO * pPartBitmapInfo = data_scan_part_bitmap_info_init(pDiskInfo->diskfd, pPartInfo->fileSys, pPartInfo->beginSec, pPartInfo->sectorNum, TRUE);
		data_scan_part_bitmap_info_destory(&pPartBitmapInfo);
		
		local_disk_info_release(&localDisk);
	}
	
	return 0;
}

int test_Ntfs_part(char * DevName, DDWORD partBegin, DDWORD partSize, char * searchFile, char * newFile, int searchDeepth)
{
	NTFS_PART_INFO * pNtfsPartInfo = Ntfs_get_part_info(DevName, partBegin, partSize);
	if(pNtfsPartInfo)
	{
		return Ntfs_scan_file_in_direction(searchFile, pNtfsPartInfo, searchDeepth);
		
		NTFS_FILE_DATA_INFO * pNtfsFileDataInfo = Ntfs_get_file_data_info(searchFile, pNtfsPartInfo);
		if(pNtfsFileDataInfo)
		{
			Ntfs_read_file_from_data_info(newFile, pNtfsFileDataInfo);
			
			/*
			///////////////////////////////////////////////////////////////////////////
			BYTE * dataBuf = (BYTE *) z_malloc(pNtfsFileDataInfo->fileAllocSize);
			if(dataBuf)
			{
				Ntfs_read_file_by_data_info(pNtfsFileDataInfo, dataBuf, pNtfsFileDataInfo->fileAllocSize);
				dump_mem_to_file(dataBuf, pNtfsFileDataInfo->fileRealSize, argv[4], FALSE);
				
				int validClusterNum = Ntfs_analyse_bitmap(dataBuf, pNtfsFileDataInfo->fileRealSize);
				iLog("validClusterNum=[%d / %d]", validClusterNum, pNtfsFileDataInfo->fileRealSize * 8);
				iLog("Using2: [%.04lf%%]", (double)(validClusterNum * 100.0 / (pNtfsFileDataInfo->fileRealSize * 8)));
				
				//iLog("Cluster[%d] used=[%d]", atoi(argv[4]), Ntfs_judge_cluster_if_used(dataBuf, pNtfsFileDataInfo->fileRealSize, atoi(argv[4])));
				
				z_free(&dataBuf);
			}
			///////////////////////////////////////////////////////////////////////////
			*/
			
			Ntfs_release_file_data_info(&pNtfsFileDataInfo);
		}
		
		Ntfs_release_part_info(&pNtfsPartInfo);
	}

	return 0;
}

int test_fat32_part(char * devName, char * searchFile, char * newFile)
{

	DDWORD filesize = get_file_size(devName);
	return Fat32_read_file_from_part(devName, 0, filesize / 512, searchFile, newFile);
}

int test_MacAddrSting2Num(char * macAddr, BYTE * MAC, int bufLen)
{
	if(macAddr == NULL || strlen(macAddr) == 0 || bufLen < 6)
		return -1;

	int numArr[6];
	if(sscanf(macAddr, "%x%*[-:]%x%*[-:]%x%*[-:]%x%*[-:]%x%*[-:]%x", &numArr[0], &numArr[1], &numArr[2], &numArr[3], &numArr[4], &numArr[5]) != 6)
		return -1;
	
	MAC[0] = numArr[0];
	MAC[1] = numArr[1];
	MAC[2] = numArr[2];
	MAC[3] = numArr[3];
	MAC[4] = numArr[4];
	MAC[5] = numArr[5];
	
	return 0;
}

int main(int argc, char ** argv)
{
	//signal(SIGSEGV, (void *)debug_backtrace);
	
	set_log_file_write_flag(FALSE);
	
	BYTE mac[6];
	return test_MacAddrSting2Num(argv[1], mac, sizeof(mac));
	
	//return test_uuidgen();
	//return test_openssl(argv[1]);
	//return test_UCS(argv[1]);
	//return test_timeTools(argv[1] , argv[2]);
	//return test_regex_match(argv[1] , argv[2]);
	
	//return test_netcard_init();
	//return test_harddisk_init(atoi(argv[1]), atoi(argv[2]));
	
	//return test_Ntfs_part(argv[1], atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], atoi(argv[6]));
	
	return test_fat32_part(argv[1], argv[2], argv[3]);
	
	
	return 0;
}

