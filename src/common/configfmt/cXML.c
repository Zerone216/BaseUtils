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
*   文 件 名 ： cXML.c 
*   文件描述 ：  
*   创建日期 ：2019年8月27日
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

//#include <libxml/parser.h>
//#include <libxml/tree.h>

#include "cXML.h"

#if 0
#define XML_CHAR(s) (xmlChar *)(s)

xmlDocPtr cXML_ReadFile(const char *filename, const char *encoding, int options){return xmlReadFile(filename, encoding, options);}
int cXML_SaveFile(const char *filename, xmlDocPtr cur){return xmlSaveFile(filename, cur);}
xmlDocPtr cXML_CreateNewDoc(char * xmlVersion){return xmlNewDoc(XML_CHAR(xmlVersion));}
void cXML_FreeDoc(xmlDocPtr doc){xmlFreeDoc(doc);}

xmlNodePtr cXML_CreateNewNode(xmlNsPtr ns, const char * nodeName){return xmlNewNode(ns, XML_CHAR(nodeName));}
xmlNodePtr cXML_SetDocRootElem(xmlDocPtr doc, xmlNodePtr root){return xmlDocSetRootElement(doc,root);}
xmlNodePtr cXML_AddChildNode(xmlNodePtr parent, xmlNodePtr child){ return  xmlAddChild(parent, child);}



xmlNodePtr cXML_CreateNewNode()
{
    xmlChar *result = NULL;
    int size = 0;
    xmlDocPtr doc = xmlNewDoc(XML_BAD_CAST("1.0"));  //定义文档和节点指针
 	
    xmlNodePtr root_node = xmlNewNode(NULL,BAD_CAST "root");    
    xmlDocSetRootElement(doc,root_node);        //设置根节点
    
    //在根节点中直接创建节点
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode1", BAD_CAST "newNode1 content");
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode2", BAD_CAST "newNode2 content");
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode3", BAD_CAST "newNode3 content");
    
    //创建一个节点，设置其内容和属性，然后加入根结点
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST "node2");
    xmlNodePtr content = xmlNewText(BAD_CAST "NODE CONTENT");
    xmlAddChild(root_node,node);
    xmlAddChild(node,content);
    xmlNewProp(node,BAD_CAST "attribute",BAD_CAST "yes");
    
    //创建一个儿子和孙子节点
    node = xmlNewNode(NULL, BAD_CAST "son");
    xmlAddChild(root_node,node);
    xmlNodePtr grandson = xmlNewNode(NULL, BAD_CAST "grandson");
    xmlAddChild(node,grandson);
    //xmlAddChild(grandson, xmlNewText(BAD_CAST "This is a grandson node"));
    xmlNodePtr congson = xmlNewNode(NULL, BAD_CAST "congson");
    xmlAddChild(grandson,congson);
    
    //存储xml文档
    //xmlKeepBlanksDefault(0);
    xmlDocDumpFormatMemoryEnc(doc, &result, &size, "UTF-8", 1);
	
    int nRel = xmlSaveFile("CreateXml.xml",doc);
    if (nRel != -1)
    {
        printf("一个xml文档被创建，写入%d个字节\n", nRel);
    }
	
    //释放文档内节点动态申请的内存
    xmlFreeDoc(doc);
    return 1;
}

#endif
