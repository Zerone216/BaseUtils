/****************************************************************************************************************************
*
*   文 件 名 ： NtfsScan.c 
*   文件描述 ：  
*   创建日期 ：2019年8月15日
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
#include <getopt.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/io.h>

#include "NtfsScan.h"

//在调用ClusterVal前，请先调用ntfs_scan_init，之后再调用ntfs_scan_end
int my_wcstombs(char *dest, const wchar_t *wcsrc, size_t n)
{
    int i;
    BYTE* src = ( BYTE* ) wcsrc;
    for(i = 0; i < n ;i ++)
    {
        *dest = *(src);
        src += 2;
        dest ++;
    }
	
    return i;
}

//每次临时缓冲使用完后都需要释放
void ntfs_free_ptr_mem(BYTE ** m_pptr)
{
	z_free(m_pptr);
}

//成功返回0
DWORD  ntfs_get_mft_head (NTFS_DATA_SCAN_INFO * pDataScanInfo, LPMFTHEAD  pmftHead, ULONGLONG ullAddr)
{
    BYTE buffer[SECTOR_SIZE];
    DWORD dwRes = readwrite_hdisk_sector(DISK_READ, \
										pDataScanInfo->diskfd, \
										pDataScanInfo->partBeginSec + ullAddr, \
										1, \
										buffer);
	if(dwRes == -1)
		return RET_READDISK_ERROR;
	
    memcpy(pmftHead, buffer, sizeof(MFTHEAD));
    return NOERROR;
}

//根据Fixup字节检查记录是否合法
//修正最后两个字节
DWORD  ntfs_check_mft_fixup(LPBYTE pMft, ULONG lcbMft)
{
    int i = 0;
    LPMFTHEAD pMftHead = (LPMFTHEAD)pMft;
	
    //判断 MFT记录的分配空间是否合法；
    if(lcbMft < (ULONG)((pMftHead->usFixupNum - 1) * SECTOR_SIZE))
        return RET_UNKNOWN_ERROR;
	
    LPWORD pwFix = (LPWORD)(pMft + pMftHead->usFixupOffset);
    for(i = 1; i < pMftHead->usFixupNum; i++ )
    {
        LPWORD ptrFix = (LPWORD)(pMft + (i * SECTOR_SIZE) -sizeof(WORD));
        if(pwFix[0] != ptrFix[0])
            return RET_UNKNOWN_ERROR;
		
        ptrFix[0] = pwFix[i];//修正每个扇区最后两个字节
    }
	
    return NOERROR;
}

DWORD  ntfs_get_mft_file_runs(NTFS_DATA_SCAN_INFO * pDataScanInfo, MFTHEAD * mftHead, LPBYTE pRuns)
{
    pDataScanInfo->m_pptr = (BYTE *)z_malloc(pDataScanInfo->m_uMFTSize);
    if(NULL == pDataScanInfo->m_pptr)
        return RET_BUFFER_ALLOC_FAIL;
	
    USHORT usRead = (USHORT)pDataScanInfo->m_uMFTSize / SECTOR_SIZE;
    ULONGLONG ullAddr = pDataScanInfo->m_ntfsBoot.ullMFTAddr * pDataScanInfo->m_ntfsBoot.SecPerClr;
    DWORD dwRes = readwrite_hdisk_sector(DISK_READ, \
								pDataScanInfo->diskfd, \
								pDataScanInfo->partBeginSec + (DWORD)ullAddr, \
								(DWORD)usRead, \
								pDataScanInfo->m_pptr);
    if ( dwRes < 0)
    {
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return RET_READDISK_ERROR;
    }
	
    dwRes = ntfs_check_mft_fixup(pDataScanInfo->m_pptr, pDataScanInfo->m_uMFTSize);
    if(NOERROR != dwRes)
    {
    	iLog("ntfs_check_mft_fixup failed!");
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return dwRes;
    }
	
    LPMFTATTR pHead = (LPMFTATTR)((DWORD)pDataScanInfo->m_pptr + (DWORD)mftHead->usAttrOffset);
    while ( $DATA != pHead->dwAttrType && \
			0 != pHead->dwAttrType && -1L != pHead->dwAttrType)
        pHead=(LPMFTATTR)((DWORD)(pHead) + (DWORD)pHead->usAttrSize);   //查找数据属性
	
	if ( $DATA != pHead->dwAttrType || FALSE == pHead->bISResident )
    {
    	iLog("Not found $DATA file!");
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr); //没有找到或文件是常驻属性，没有Run
        return RET_UNKNOWN_ERROR;
    }
	
    int cbSize = pHead->usAttrSize - pHead->unAttrib.NonResidAttr.usNrDataOffset;
	memcpy((LPSTR)pRuns, (LPSTR)((DWORD)pHead + (DWORD)pHead->unAttrib.NonResidAttr.usNrDataOffset), cbSize);
	
    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr); 
    return NOERROR;
}

//得到 m_ullRootAddr 
DWORD  ntfs_search_root_directory(NTFS_DATA_SCAN_INFO * pDataScanInfo)
{
	int i = 0;
    MFTHEAD mftHead;
    ULONGLONG ulSize = 0;
	LONGLONG llStAddr = 0;
	BYTE pmftRuns[MAX_RUNS] = {0};
	
	ULONGLONG ullAddr = pDataScanInfo->m_ntfsBoot.ullMFTAddr * pDataScanInfo->m_ntfsBoot.SecPerClr;
    memset(&mftHead, 0 , sizeof (MFTHEAD));
	
    DWORD dwRes = ntfs_get_mft_head(pDataScanInfo, &mftHead, ullAddr);
    if(NOERROR != dwRes)
	{
		iLog("ntfs_get_mft_head failed!");
    	return dwRes;
	}
	
    pDataScanInfo->m_uMFTSize = mftHead.ulMFTAllocSize;
    if(pDataScanInfo->m_uMFTSize < SECTOR_SIZE || pDataScanInfo->m_uMFTSize % SECTOR_SIZE != 0)
		return RET_UNKNOWN_ERROR;
	
    dwRes = ntfs_get_mft_file_runs(pDataScanInfo, &mftHead, pmftRuns);
    if(NOERROR != dwRes)
    {
		iLog("ntfs_get_mft_file_runs failed!");
		return dwRes;
    }
	
    ntfs_make_member_form_runs(pDataScanInfo, pmftRuns, &llStAddr, &ulSize); // the .dot dir. alway is front, so need't search Runs;

    llStAddr = pDataScanInfo->m_ntfsBoot.ullMFTAddr * pDataScanInfo->m_ntfsBoot.SecPerClr;
    pDataScanInfo->m_pptr = (BYTE *)z_malloc(pDataScanInfo->m_uMFTSize);
    if(NULL == pDataScanInfo->m_pptr)
        return RET_BUFFER_ALLOC_FAIL;
	
    int iStep = pDataScanInfo->m_uMFTSize / SECTOR_SIZE;
    TCHAR szName[MAX_PATH];
	
    for(i = 0; i < ulSize; i += iStep)
    {
        dwRes = readwrite_hdisk_sector(DISK_READ, \
									pDataScanInfo->diskfd, \
									pDataScanInfo->partBeginSec + (DWORD)llStAddr, \
									(DWORD)iStep, \
									pDataScanInfo->m_pptr);
        if(dwRes == -1)
        {
            ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
            return RET_READDISK_ERROR;
        }
		        
        ntfs_get_name_from_mft_attr(pDataScanInfo->m_pptr, szName, MAX_PATH);
		
        if(0 == strcmp(".", (const char*)szName))
        {
            pDataScanInfo->m_ullRootAddr = llStAddr;
            break;
        }
		
        llStAddr += iStep;
    }
	
    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
    return NOERROR;
}

DWORD ntfs_get_name_from_mft_attr(LPBYTE pMft, LPSTR pszName, UINT uNameSize)
{
    LPMFTHEAD pHead = (LPMFTHEAD) pMft ;
    LPMFTATTR pAttr = (LPMFTATTR)( (DWORD)pMft + (DWORD)pHead->usAttrOffset );

    while($NAME != pAttr->dwAttrType && \
			0 != pAttr->dwAttrType && -1L != pAttr->dwAttrType)
		pAttr = (LPMFTATTR)((DWORD) pAttr + (DWORD)pAttr->usAttrSize);
	
    if($NAME != pAttr->dwAttrType || \
		FALSE != pAttr->bISResident)
    {
        return RET_UNKNOWN_ERROR;
    }
	
    LP$FILENAME pfName = (LP$FILENAME)((DWORD)pAttr + (DWORD)pAttr->unAttrib.ResidAttr.usRDataOffset);
    my_wcstombs((char *)pszName,pfName->pwChar,pfName->bNameLen);
	
    if ( uNameSize > pfName->bNameLen )
        pszName[pfName->bNameLen] = '\0';
    else
        pszName[uNameSize - 1] = '\0';

    return NOERROR;
}


//注意 llStAddr是有符号的,得到一个Run的信息
//返回run的起始簇数和run占用扇区数
DWORD ntfs_make_member_form_runs(NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pRuns, LONGLONG *llStAddr, ULONGLONG *ulSize)
{
    UINT ucbSize = LOB ( pRuns[0] );
    UINT ucbAddr = HIB ( pRuns[0] );
    pRuns++ ;
	
    switch ( ucbSize )
    {
    case 1:
        *ulSize = (ULONGLONG ) *((LPBYTE) pRuns);
        pRuns += 1;
        break;
    case 2:
        *ulSize = (ULONGLONG ) *((USHORT*) pRuns);
        pRuns += 2;
        break;
    case 3:
        *ulSize = (ULONGLONG ) *((ULONG*) pRuns);
        *ulSize &= 0xFFFFFF;
        pRuns += 3;
        break;
    case 4:
        *ulSize = (ULONGLONG ) *((ULONG*) pRuns);
        pRuns += 4;
        break;
    case 5:
        *ulSize = ((ULONGLONG ) *pRuns);
        *ulSize &= 0xFFFFFFFFFFLL;
        pRuns += 5;
        break;
    case 6:
        *ulSize = ((ULONGLONG ) *pRuns);
        *ulSize &= 0xFFFFFFFFFFFFLL;
        pRuns += 6;
        break;
    case 7:
        *ulSize = ((ULONGLONG ) *pRuns);
        *ulSize &= 0xFFFFFFFFFFFFFFLL;
        pRuns += 7;
        break;
    case 8:
        *ulSize = ((ULONGLONG ) *pRuns);
        pRuns += 8;
        break;
    default:
        *ulSize = 0;
        pRuns += ucbSize;
    }

    switch (ucbAddr)
    {
    case 1:
        *llStAddr = (LONGLONG ) *((char*) pRuns);
        break;
    case 2:
        *llStAddr = (LONGLONG ) *(( short*) pRuns);
        break;
    case 3:
        {
            long lAddr = *((unsigned long*) pRuns);
            if(0 != (lAddr & 0x800000))
                *llStAddr = lAddr |= 0xFF000000;
            else
                *llStAddr = lAddr &= 0xFFFFFF;
        }
        break;
    case 4:
        *llStAddr = (LONGLONG ) *((long*) pRuns);
        break;
    case 5:
        {
            __int64 lAddr = *((ULONGLONG *) pRuns);
            if(0 != (lAddr & 0x8000000000LL))
                *llStAddr = lAddr |= 0xFFFFFF0000000000LL;
            else
                *llStAddr = lAddr &= 0xFFFFFFFFFFLL;
        }
    case 6:
        {
            __int64 lAddr = *((ULONGLONG *) pRuns);
            if(0 != (lAddr & 0x800000000000LL))
                *llStAddr = lAddr |= 0xFFFF000000000000LL;
            else
                *llStAddr = lAddr &= 0xFFFFFFFFFFFFLL;
        }
    case 7:
        {
            __int64 lAddr = *((ULONGLONG *) pRuns);
            if(0 != ( lAddr & 0x80000000000000LL))
                *llStAddr = lAddr |= 0xFF00000000000000LL;
            else
                *llStAddr = lAddr &= 0xFFFFFFFFFFFFFFLL;
        }
    case 8:
        *llStAddr = (LONGLONG ) *(( __int64*) pRuns);
        break;
    default:
        *llStAddr = 0;
        pRuns += ucbAddr;
        break;
    }
	
    *ulSize *= pDataScanInfo->m_ntfsBoot.SecPerClr;
	return NOERROR;
}

//得到文件的运行
DWORD ntfs_get_file_runs(LPBYTE pMft, LPBYTE pRuns)
{
    LPMFTHEAD pHead = (LPMFTHEAD)pMft;
    LPMFTATTR pAttr = (LPMFTATTR)((DWORD)pMft + (DWORD)pHead->usAttrOffset);
	
    while($DATA != pAttr->dwAttrType && \
			0 != pAttr->dwAttrType && -1L != pAttr->dwAttrType)
        pAttr = (LPMFTATTR)((DWORD)pAttr + (DWORD)pAttr->usAttrSize);
	
    if($DATA != pAttr->dwAttrType ||\
		FALSE == pAttr->bISResident)
    {
        *pRuns = '\0';                //找不到索引分配属性，或属性是常驻的
        return RET_UNKNOWN_ERROR;
    }

    int cbSize = pAttr->usAttrSize - pAttr->unAttrib.NonResidAttr.usNrDataOffset;
    memcpy((LPSTR)pRuns, (LPSTR)pAttr + pAttr->unAttrib.NonResidAttr.usNrDataOffset, cbSize);
	
    return NOERROR;
}


//得到目录的运行
DWORD ntfs_get_direction_runs(LPBYTE pMft, LPBYTE pRuns)
{
    LPMFTHEAD pHead = (LPMFTHEAD)pMft;
    if(FALSE == (pHead->wResident & 0x2 ))
    {
        *pRuns = '\0';
        return NON_DIR;
    }

    LPMFTATTR pAttr = (LPMFTATTR)((DWORD)pMft + (DWORD)pHead->usAttrOffset);
    while ( $INDEX_ALLOC != pAttr->dwAttrType && \
                0 != pAttr->dwAttrType && -1L != pAttr->dwAttrType)
        pAttr = (LPMFTATTR)((DWORD)pAttr + (DWORD)pAttr->usAttrSize);
	
    if($INDEX_ALLOC != pAttr->dwAttrType ||\
		FALSE == pAttr->bISResident )
    {
        *pRuns = '\0';                //找不到索引分配属性，或属性是常驻的
        return EMPTY_DIR;
    }
	
    int cbSize = pAttr->usAttrSize -pAttr->unAttrib.NonResidAttr.usNrDataOffset;
    memcpy((LPSTR)pRuns, (LPSTR)pAttr + pAttr->unAttrib.NonResidAttr.usNrDataOffset, cbSize);
	
    return NOERROR;
}

//得到所有run的占用的字节数
ULONGLONG  ntfs_get_runs_all_size(NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pRuns, UINT ucbRuns)
{
    int                 iseek;
    __int64             llstAddr; //non use
    ULONGLONG   ullSize, ullAllSize ;
	
    pRuns[ucbRuns -1] = '\0';
    ullAllSize = 0;
	
    while(0 != pRuns[0])
    {
        iseek = HIB(pRuns[0]) + LOB(pRuns[0]) + 1;
        ntfs_make_member_form_runs(pDataScanInfo, pRuns, &llstAddr, &ullSize);
		
        ullAllSize += ullSize;
        pRuns += iseek;
    }
	
    return ullAllSize * pDataScanInfo->m_ntfsBoot.SecInByte;
}

//将目录所有运行的数据读到内存,需要修正一下Fixup位置
DWORD ntfs_read_direction_runs_to_mem(NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pRuns)
{
    int iseek;
    __int64 llstAddr, llAddr = 0;
    ULONGLONG ullSize;
    DWORD dwRes;
	BYTE * pMpptr = pDataScanInfo->m_pptr;
	
    while(0 != pRuns[0])
    {
        iseek = HIB(pRuns[0]) + LOB(pRuns[0]) + 1;
        ntfs_make_member_form_runs(pDataScanInfo, pRuns, &llstAddr, &ullSize);
        llAddr += (llstAddr * pDataScanInfo->m_ntfsBoot.SecPerClr);
		
        dwRes=readwrite_hdisk_sector(DISK_READ,\
									pDataScanInfo->diskfd, \
									pDataScanInfo->partBeginSec + (DWORD)llAddr, \
									(WORD)ullSize, \
									pMpptr);
        if(dwRes == -1)
            return RET_READDISK_ERROR;
		
        dwRes = ntfs_check_mft_fixup(pMpptr, (WORD)ullSize * SECTOR_SIZE);   //用这个函数修正INDX中的FIXUP位置，效果一样
        if(NOERROR != dwRes)
        {           
            return dwRes;
        }
		
        pRuns += iseek;
        pMpptr += (ullSize * SECTOR_SIZE);
    }
	
    return NOERROR;
}

//从根目录查找文件的Run,不递归查找
DWORD ntfs_search_file_from_root(TCHAR* findname, NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE  pFileRuns)
{
    LPINDX          pIndx;
    LPINDXATTR      pIndxAttr;
    TCHAR           szName[MAX_PATH];    
    UINT uAlcSize;   //uAlcSize 目录总共占用的字节数  
    DWORD dwres=0;
    BYTE    pRuns[MAX_RUNS];

    pDataScanInfo->m_pptr = (BYTE *)z_malloc( pDataScanInfo->m_uMFTSize);
    if(NULL == pDataScanInfo->m_pptr)
        return RET_BUFFER_ALLOC_FAIL;
	
    dwres=readwrite_hdisk_sector(DISK_READ, \
    							pDataScanInfo->diskfd, \
    							pDataScanInfo->partBeginSec + pDataScanInfo->m_ullRootAddr, \
    							(DWORD)pDataScanInfo->m_uMFTSize/SECTOR_SIZE, \
    							pDataScanInfo->m_pptr);
    if(dwres == -1)
    {
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return RET_READDISK_ERROR;
    }
	
    dwres = ntfs_check_mft_fixup(pDataScanInfo->m_pptr, pDataScanInfo->m_uMFTSize);  //修正一下记录
    if( NOERROR != dwres )
    {
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return dwres;
    }
	
    ntfs_get_direction_runs(pDataScanInfo->m_pptr, pRuns);
    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
	
    ULONGLONG ullAllSize = ntfs_get_runs_all_size(pDataScanInfo, pRuns, MAX_RUNS);
    pDataScanInfo->m_pptr = (BYTE *)z_malloc((int)ullAllSize);
    if(NULL == pDataScanInfo->m_pptr)
        return RET_BUFFER_ALLOC_FAIL;
	
    dwres = ntfs_read_direction_runs_to_mem(pDataScanInfo, pRuns);
    if(NOERROR != dwres)
    {
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return dwres;
    }
	
	//dump_mem_to_file(pDataScanInfo->m_pptr, ullAllSize, "INDEX_DATA_SCAN", FALSE);
	
    pIndx = (LPINDX)pDataScanInfo->m_pptr;
    uAlcSize = (UINT)ullAllSize;
	
    //得到第一个索引项的位置,加上0x18才是实际的结构头大小
    uAlcSize -= (pIndx->dwAllocSize + 0x18);  
    pIndxAttr = (LPINDXATTR)(pDataScanInfo->m_pptr + (0x18 + pIndx->wHeadSize));
	
    if ( 'I' != pIndx->bDirID [0] ||\
	         'N' != pIndx->bDirID [1] ||\
	         'D' != pIndx->bDirID [2] ||\
	         'X' != pIndx->bDirID [3] )
    {
    	iLog("bDirID=[%c%c%c%c], not INDX!", pIndx->bDirID[0], pIndx->bDirID[1], pIndx->bDirID[2], pIndx->bDirID[3]);
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return RET_UNKNOWN_ERROR;
    }
	
    while(TRUE)
    {
        if(((ULONG)pIndxAttr -(ULONG)pIndx) >= pIndx->dwUseSize)     //索引项不在这个索引记录中了
        {
            if ( 0 == uAlcSize )//
                break;
            else
            {                                         
                pIndx = (LPINDX)((LPSTR)pIndx + (pIndx->dwAllocSize + 0x18));  //得到下一个索引记录的地址
                if( 'I' != pIndx->bDirID [0] ||\
			                     'N' != pIndx->bDirID [1] ||\
			                     'D' != pIndx->bDirID [2] ||\
			                     'X' != pIndx->bDirID [3] )
                {
                    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
                    return RET_UNKNOWN_ERROR;
                }

                if((pIndx->dwAllocSize + 0x18) > uAlcSize)
                {   
                    if(pIndx->dwUseSize > uAlcSize)
                       pIndx->dwUseSize = uAlcSize;
					
                    uAlcSize = 0;
                }
                else
                   uAlcSize -= (pIndx->dwAllocSize + 0x18);
            	
                pIndxAttr = (LPINDXATTR)((LPSTR)pIndx + (0x18 + pIndx->wHeadSize));    //得到第一个索引项地址
            }
        }
		
        if(pIndxAttr->dwMFTIndx < 0) //支持查找$BITMAP文件
        {
            pIndxAttr = (LPINDXATTR)((LPSTR)pIndxAttr + pIndxAttr->wcbSize);
            continue;
        }
        else
        {
            if(FALSE == (UNICODE_NAME & pIndxAttr->bFileNSpace))  //只显示UNICODE文件名，不显示DOS等文件名
            {
                pIndxAttr = (LPINDXATTR)((LPSTR)pIndxAttr + pIndxAttr->wcbSize);
                continue;
            }
          	
            int iChar = my_wcstombs((char *)szName,pIndxAttr->wzFileName,pIndxAttr->bFileNameLen);
            if(iChar == -1)
                assert(0);
            szName[iChar] = '\0';//以前得到的文件名不正确是因为没有对Fixup字节进行修正
			
            if(strncasecmp((const char*)szName, (const char*)findname, sizeof(findname)) == 0)
            {
            	iLog("szName=[%s]", szName);
                BYTE * pmft= (BYTE *)z_malloc(pDataScanInfo->m_uMFTSize);
                dwres = ntfs_get_mft_record(pDataScanInfo, pmft,pIndxAttr->dwMFTIndx, (char *)szName);
                if(dwres!=NOERROR)
                {
                    eLog("NTFS_GetMFTRecord index %d error!", pIndxAttr->dwMFTIndx);
                    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
                    z_free(&pmft);
                    return RET_UNKNOWN_ERROR;
                }
                
                dwres = ntfs_get_file_runs(pmft, pFileRuns);
                if(dwres!=NOERROR)
                {
                    eLog("NTFS_GetFileRuns error!");
                    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
                    z_free(&pmft);
                    assert(0);
                    return RET_UNKNOWN_ERROR;
                }

                ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
                z_free(&pmft);
                return NOERROR;
                break;
            }                       
            pIndxAttr = (LPINDXATTR)((LPSTR)pIndxAttr + pIndxAttr->wcbSize);
        }
    }

	eLog("Can't find file!");
    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
   
    return RET_UNKNOWN_ERROR;
}

//列出根目录的所有文件
DWORD ntfs_list_root_directory(NTFS_DATA_SCAN_INFO * pDataScanInfo)
{
    LPINDX          pIndx;
    LPINDXATTR      pIndxAttr;
    TCHAR           szName[MAX_PATH];    
    UINT uAlcSize;   //uAlcSize 目录总共占用的字节数  
    DWORD           dwres=0;
    BYTE    pRuns[MAX_RUNS];

    pDataScanInfo->m_pptr = (BYTE *)z_malloc(pDataScanInfo->m_uMFTSize);
    if(NULL == pDataScanInfo->m_pptr)
        return RET_BUFFER_ALLOC_FAIL;
	
    dwres=readwrite_hdisk_sector(DISK_READ, \
    							pDataScanInfo->diskfd, \
    							pDataScanInfo->partBeginSec + pDataScanInfo->m_ullRootAddr,\
    							pDataScanInfo->m_uMFTSize / SECTOR_SIZE, \
    							pDataScanInfo->m_pptr);
    if(dwres == -1)
    {
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return RET_READDISK_ERROR;
    }
	
    ntfs_get_direction_runs(pDataScanInfo->m_pptr,pRuns);
    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);

    ULONGLONG ullAllSize = ntfs_get_runs_all_size(pDataScanInfo, pRuns, MAX_RUNS); 
    pDataScanInfo->m_pptr = (BYTE *)z_malloc((int)ullAllSize);
    if(NULL == pDataScanInfo->m_pptr)
        return RET_BUFFER_ALLOC_FAIL;
	
    dwres = ntfs_read_direction_runs_to_mem(pDataScanInfo, pRuns);
    if ( NOERROR != dwres )
    {
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return dwres;
    }
	
    pIndx = (LPINDX)pDataScanInfo->m_pptr;
    uAlcSize = (UINT)ullAllSize;

    //得到第一个索引项的位置,加上0x18才是实际的结构头大小
    uAlcSize -= (pIndx->dwAllocSize + 0x18);  
    pIndxAttr = (LPINDXATTR)(pDataScanInfo->m_pptr + (0x18 + pIndx->wHeadSize));

    if('I' != pIndx->bDirID [0] ||\
	         'N' != pIndx->bDirID [1] ||\
	         'D' != pIndx->bDirID [2] ||\
	         'X' != pIndx->bDirID [3])
    {
        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
        return RET_UNKNOWN_ERROR;
    }
	
    while(TRUE)
    {
        if((ULONG)pIndxAttr -(ULONG)pIndx >= pIndx->dwUseSize)     //索引项不在这个索引记录中了
        {
            if(0 == uAlcSize)          //
                break;       
            else
            {                                         
                pIndx = (LPINDX )((LPSTR)pIndx + (pIndx->dwAllocSize + 0x18));  //得到下一个索引记录的地址
                if ( 'I' != pIndx->bDirID [0] ||\
			                     'N' != pIndx->bDirID [1] ||\
			                     'D' != pIndx->bDirID [2] ||\
			                     'X' != pIndx->bDirID [3])
                {
                    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
                    return RET_UNKNOWN_ERROR;
                }

                if((pIndx->dwAllocSize + 0x18) > uAlcSize)
                {   
                    if(pIndx->dwUseSize>uAlcSize)
                       pIndx->dwUseSize=uAlcSize;
					
                    uAlcSize =0;            
                }
                else
                   uAlcSize -= (pIndx->dwAllocSize + 0x18);
            
                pIndxAttr = (LPINDXATTR)((LPSTR)pIndx + (0x18 + pIndx->wHeadSize)); //得到第一个索引项地址
            }
        }

        if(pIndxAttr->dwMFTIndx < 12) //不列出元文件。共有16个元文件，但有四个在目录中
        {
            pIndxAttr = (LPINDXATTR)((LPSTR)pIndxAttr + pIndxAttr->wcbSize);
            continue;
        }
        else
        {
            if(FALSE == (UNICODE_NAME & pIndxAttr->bFileNSpace))  //只显示UNICODE文件名，不显示DOS等文件名
            {
                pIndxAttr = (LPINDXATTR)((LPSTR)pIndxAttr + pIndxAttr->wcbSize);
                continue;
            }
			
            int iChar = my_wcstombs((char *)szName,pIndxAttr->wzFileName, pIndxAttr->bFileNameLen);
            if(iChar == -1)
                assert(0);
			
            szName[iChar] = '\0';   //得到的文件名不一定正确，只在MFT中的文件名才是一定正确的
            pIndxAttr = (LPINDXATTR)((LPSTR)pIndxAttr + pIndxAttr->wcbSize);
			
			iLog("Find File %s. MFT pos 0X%X.",szName, pIndxAttr->dwMFTIndx);
        }
    }
    
    ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
    return NOERROR;
}

//根据MFT参考号获得MFT记录和文件名,因为在目录中的文件名可能有误
DWORD ntfs_get_mft_record (NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pMft, DWORD mftindex ,char* szName )
{
	BYTE mftRuns[MAX_RUNS];
	MFTHEAD      mftHead;
	DWORD        dwAllSize=0;
	LONGLONG     llStAddr = 0;
	ULONGLONG    ulSize = 0;
	int iseek = 0;
    DWORD  oppindex=0;
	
	LPBYTE pmftRuns = (LPBYTE)mftRuns;
	ULONGLONG ullAddr = pDataScanInfo->m_ntfsBoot.ullMFTAddr * pDataScanInfo->m_ntfsBoot.SecPerClr ;
	DWORD dwRes = ntfs_get_mft_head(pDataScanInfo, &mftHead, ullAddr);
	if(NOERROR != dwRes)
		return dwRes;
	
   	dwRes = ntfs_get_mft_file_runs(pDataScanInfo, &mftHead, pmftRuns);
    if(NOERROR != dwRes)
		return dwRes;
	
    pmftRuns[MAX_RUNS -1] = '\0';
	
    pDataScanInfo->m_pptr = (BYTE *)z_malloc(pDataScanInfo->m_uMFTSize);
    if(NULL == pDataScanInfo->m_pptr)
    {
        return RET_BUFFER_ALLOC_FAIL;
    }
	
    ullAddr=0;
    while(0 != pmftRuns[0])
	{
	    iseek = HIB ( pmftRuns[0] ) + LOB ( pmftRuns[0] ) + 1;
	    ntfs_make_member_form_runs(pDataScanInfo, pmftRuns, &llStAddr, &ulSize);
		
	    dwAllSize += (DWORD)ulSize;
	    ullAddr += (llStAddr * pDataScanInfo->m_ntfsBoot.SecPerClr);
	    if(dwAllSize/ (pDataScanInfo->m_uMFTSize/ SECTOR_SIZE) >= mftindex)       //扇区数/每个MFT记录占有的扇区数
	    {
	        oppindex=mftindex-oppindex;
	        ullAddr=ullAddr+oppindex * (pDataScanInfo->m_uMFTSize/SECTOR_SIZE);
			
	        dwRes=readwrite_hdisk_sector(DISK_READ, \
									pDataScanInfo->diskfd, \
									pDataScanInfo->partBeginSec + (DWORD)ullAddr, \
									(WORD)pDataScanInfo->m_uMFTSize/SECTOR_SIZE, \
									pDataScanInfo->m_pptr);
	        if(dwRes == -1)
	        {
	            ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
	            return RET_READDISK_ERROR;
	        }
	        
	        dwRes = ntfs_check_mft_fixup(pDataScanInfo->m_pptr, pDataScanInfo->m_uMFTSize);     //修正一下MFT记录
	        if(NOERROR != dwRes)
	        {
	            ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
	            return dwRes;
	        }
			
	        memcpy(pMft, pDataScanInfo->m_pptr, pDataScanInfo->m_uMFTSize);
	        ntfs_get_name_from_mft_attr(pDataScanInfo->m_pptr, (BYTE *)szName, MAX_PATH);
			
	        ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
	        return NOERROR;

	        break;
	    }
		
	    oppindex = dwAllSize / (pDataScanInfo->m_uMFTSize/ SECTOR_SIZE);
	    pmftRuns += iseek;
	}
 	
	ntfs_free_ptr_mem(&pDataScanInfo->m_pptr);
	return RET_UNKNOWN_ERROR;
}

WORD  ntfs_get_root_file_pos(char* filename, NTFS_DATA_SCAN_INFO * pDataScanInfo, DWORD* startsec)
{
    BYTE FileRuns[MAX_RUNS];
    LONGLONG llStAddr=0;
    ULONGLONG ulSize, ullAddr;
	int iseek = 0;

	LPBYTE pFileRuns=(LPBYTE)FileRuns;
    if(ntfs_search_file_from_root((BYTE *)filename, pDataScanInfo, pFileRuns) != NOERROR)
    {
        eLog("NTFS_SearchFileFromRoot %s error!", filename);
        return FALSE;
    }
	
    pFileRuns[MAX_RUNS -1] = '\0';
	
    ntfs_make_member_form_runs(pDataScanInfo, pFileRuns, &llStAddr, &ulSize);
    *startsec = (DWORD)llStAddr* pDataScanInfo->m_ntfsBoot.SecPerClr;//没有0簇和1簇
    *startsec += pDataScanInfo->partBeginSec;

	pDataScanInfo->ntfsBmNum = 0;
    NTFS_BITMAP_INFO * p = pDataScanInfo->ntfsBitmapInfo;
	memset(p, 0x00, sizeof(NTFS_BITMAP_INFO) * 512);
	
    ullAddr = 0;
	
    while(0 != pFileRuns[0])
    {
        iseek = HIB(pFileRuns[0]) + LOB(pFileRuns[0]) + 1;

		ntfs_make_member_form_runs(pDataScanInfo, pFileRuns, &llStAddr, &ulSize);

		ullAddr += llStAddr ;
		p->m_BitmapPos = (DWORD)ullAddr;       //起始LCN超出4个字节时，直接将高位去掉即可。
		p->m_BitmapSize = (DWORD)ulSize;
		p ++;

		pDataScanInfo->ntfsBmNum ++;
		
		if(p -pDataScanInfo->ntfsBitmapInfo >= 512 * sizeof(NTFS_BITMAP_INFO))
		{
			eLog("%s has too many fragments!",filename);
			return FALSE;
		}
		
		pFileRuns += iseek;
    }
 
    return TRUE;
}

static int get_bitmp_index(NTFS_BITMAP_INFO * ntfsBitmapInfo, DWORD sector, DWORD * left)
{
	int index = -1;
	*left = sector;
	int i = 0;
	for(i = 0; i < 512; i ++)
	{
		if(*left <= ntfsBitmapInfo[i].m_BitmapSize)
		{
			index = i;
			break;
		}
		
		*left -= ntfsBitmapInfo[i].m_BitmapSize;	
	}
	
	return index;
}

int ntfs_get_clusterval(NTFS_DATA_SCAN_INFO * pDataScanInfo, DWORD number)
{
	DWORD sector = number / NTFS_CLUSTER_SIZE;
	DWORD whichByte =(number % NTFS_CLUSTER_SIZE) / NTFS_SECTORS_PER_CLUSTER;
	DWORD whichBit= (number % NTFS_CLUSTER_SIZE) % NTFS_SECTORS_PER_CLUSTER;
	DWORD relative = 0;
	
	int index = get_bitmp_index(pDataScanInfo->ntfsBitmapInfo, sector, &relative);
	if(index == -1)
		return -1;
	
	if (sector != pDataScanInfo->m_SecInBuf)
	{
		//读某个分区中的扇区(扇区位置,扇区个数,缓冲区)
		DDWORD readSector = pDataScanInfo->partBeginSec + pDataScanInfo->ntfsBitmapInfo[index].m_BitmapPos * pDataScanInfo->m_secpercluster + relative;
		readwrite_hdisk_sector(DISK_READ, \
							pDataScanInfo->diskfd, \
							readSector, \
							1, \
							pDataScanInfo->sectorBuf);
		pDataScanInfo->m_SecInBuf = sector;
	}
	
	BYTE BitShift[] = { 1, 2, 4, 8, 16, 32, 64, 128};
	BYTE val = pDataScanInfo->sectorBuf[whichByte] & BitShift[whichBit];
	if(val)
		return 1;
	else
		return 0;
}

int ntfs_datascan_initial(NTFS_DATA_SCAN_INFO * pDataScanInfo)
{
	if(pDataScanInfo == NULL)
		return FALSE;
	
    BYTE buffer[SECTOR_SIZE];
	if(readwrite_hdisk_sector(DISK_READ, pDataScanInfo->diskfd, pDataScanInfo->partBeginSec + 0, 1, buffer) < 0)
		return FALSE;
	
    memset(&pDataScanInfo->m_ntfsBoot, 0, sizeof ( NTFS_BOOT_SECTOR ));
    memcpy ( &pDataScanInfo->m_ntfsBoot, buffer, sizeof ( NTFS_BOOT_SECTOR));
	
    if (( 'N' != pDataScanInfo->m_ntfsBoot.OemID[0]) || \
		( 'T' != pDataScanInfo->m_ntfsBoot.OemID[1]) || \
        ( 'F' != pDataScanInfo->m_ntfsBoot.OemID[2]) ||\
        ( 'S' != pDataScanInfo->m_ntfsBoot.OemID[3]))
	{
		iLog("OemID=[%c%c%c%c], Not NTFS!", pDataScanInfo->m_ntfsBoot.OemID[0], pDataScanInfo->m_ntfsBoot.OemID[1], pDataScanInfo->m_ntfsBoot.OemID[2], pDataScanInfo->m_ntfsBoot.OemID[3]);
    	return FALSE;
    }
	
    if(ntfs_search_root_directory(pDataScanInfo) != NOERROR)
    {
		iLog("ntfs_search_root_directory failed!");
    	return FALSE;
    }
	
    DWORD bitmapstartsec = 0;
    if(ntfs_get_root_file_pos("$BITMAP",pDataScanInfo, &bitmapstartsec) == FALSE)
    {
		iLog("ntfs_get_root_file_pos failed!");
    	return FALSE;
    }
	
	iLog("pDataScanInfo->ntfsBmNum=[%d]", pDataScanInfo->ntfsBmNum);
	int i = 0;
	for(i = 0; i < pDataScanInfo->ntfsBmNum; i ++)
		iLog("BITMAP[%d]: -->[%d + %d]", i, pDataScanInfo->ntfsBitmapInfo[i].m_BitmapPos, pDataScanInfo->ntfsBitmapInfo[i].m_BitmapSize);
	
    pDataScanInfo->m_secpercluster = pDataScanInfo->m_ntfsBoot.SecPerClr;
    pDataScanInfo->m_clusternum = (DWORD)pDataScanInfo->m_ntfsBoot.ullSectorsOfParti/pDataScanInfo->m_secpercluster;
    pDataScanInfo->m_datastart = 0;
	
    return TRUE;
}


NTFS_DATA_SCAN_INFO * ntfs_datascan_init(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec)
{
	NTFS_DATA_SCAN_INFO * pDataScanInfo = (NTFS_DATA_SCAN_INFO *)z_malloc(sizeof(NTFS_DATA_SCAN_INFO));
	if(pDataScanInfo == NULL)
		return NULL;
	
	memset(pDataScanInfo, 0x00, sizeof(NTFS_DATA_SCAN_INFO));
	pDataScanInfo->diskfd = diskfd;
	pDataScanInfo->partBeginSec = partBeginSec;
	pDataScanInfo->partTotalSec = partTotalSec;
	pDataScanInfo->m_SecInBuf = 0xffffffff; //FAT表缓冲
	
	if( ntfs_datascan_initial(pDataScanInfo) == FALSE)
	{
		ntfs_datascan_end(&pDataScanInfo);
		return NULL;
	}

	return pDataScanInfo;	
}

void ntfs_datascan_end(NTFS_DATA_SCAN_INFO ** pDataScanInfo)
{
	if((*pDataScanInfo)->m_pptr)
		z_free(&((*pDataScanInfo)->m_pptr));
	
	z_free(pDataScanInfo);
}

static int ntfs_mark_bitmap(BYTE * pBitmap, int diskfd, DDWORD bitmapBeginSec, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	iLog("Ntfs part scan: mark bitmap...");
	NTFS_DATA_SCAN_INFO * pDataScanInfo = ntfs_datascan_init( diskfd, partBeginSec, partTotalSec);
	if(pDataScanInfo)
	{
		int i = 0;
		iLog("m_clusternum=[%d]", pDataScanInfo->m_clusternum);
		for(i = 0; i < pDataScanInfo->m_clusternum; i ++)
	        if (ntfs_get_clusterval(pDataScanInfo, i) == 1)
	            mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec + pDataScanInfo->m_secpercluster * i, pDataScanInfo->m_secpercluster);
		
		ntfs_datascan_end(&pDataScanInfo);
    }
	else
		mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, partTotalSec);
	
	return 0;
}

BYTE * ntfs_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	iLog("Ntfs part scan: generate part bitmap...");
	BYTE * pBitmap = (BYTE *) z_malloc(bitmapSize);
	if(pBitmap == NULL)
		return NULL;
	
	ntfs_mark_bitmap(pBitmap, diskfd, 0, partBeginSec, partTotalSec, bitmapSize);
	return pBitmap;
}

int ntfs_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	return ntfs_mark_bitmap(pBitmap, diskfd, partBeginSec, partBeginSec, partTotalSec, bitmapSize);
}
