CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
# ********************************** 添加项目名 **********************************
PROJECT(UTILS)
MESSAGE(STATUS "set project name: ${PROJECT_NAME}")

# ********************************** 添加源码文件（递归扫描目录及所有子目录下的源文件） **********************************
SET(SRC_LIST "")
#SET(INCLUDE_LIST "")

#自定义宏，对某一目录及其子目录递归遍历添加所有的源文件（注：此处不使用function是因为函数的变量都是局部变量，递归退出后数据丢失）
MACRO(AUX_SOURCE_DIRECTORY_RECURSE CUR_DIR)

	#LIST(APPEND INCLUDE_LIST ${CUR_DIR})          #追加到全局变量INCLUDE_LIST列表中 
	#MESSAGE(STATUS "Scanning ${CUR_DIR}")
	FILE(RELATIVE_PATH BASE_SRC_DIR ${PROJECT_SOURCE_DIR} ${CUR_DIR})  #相对于工程目录的相对路径
	MESSAGE(STATUS "Scanning ./${BASE_SRC_DIR}")
	
	SET(TEMP_SRC_LIST "")
	AUX_SOURCE_DIRECTORY(${BASE_SRC_DIR} TEMP_SRC_LIST)   #添加当前目录下的所有源文件
	LIST(APPEND SRC_LIST ${TEMP_SRC_LIST})          #追加到全局变量SRC_LIST列表中
	
	SET(FILE_LIST "")
	FILE(GLOB FILE_LIST ${CUR_DIR} ${CUR_DIR}/*)    #获取当前目录下的所有文件及文件夹
	
	FOREACH(CUR_FILE ${FILE_LIST})        #遍历文件列表
		IF(IS_DIRECTORY ${CUR_FILE})      #文件类型为目录
			IF((${CUR_FILE} STRLESS ${CUR_DIR}) OR (${CUR_FILE} STRGREATER ${CUR_DIR}))  #文件名与当前目录名不一样（. 表示当前目录，不过滤将会无限循环）
				
				AUX_SOURCE_DIRECTORY_RECURSE(${CUR_FILE})
			ENDIF((${CUR_FILE} STRLESS ${CUR_DIR}) OR (${CUR_FILE} STRGREATER ${CUR_DIR}))
		ENDIF(IS_DIRECTORY ${CUR_FILE})
	ENDFOREACH(CUR_FILE)
ENDMACRO(AUX_SOURCE_DIRECTORY_RECURSE)

#对扫描的目录初始化为绝对路径
MACRO(ADD_DIR_RECURSE DST_DIR)
	GET_FILENAME_COMPONENT(FULL_DST_PATH ${DST_DIR} REALPATH BASE_DIR ${PROJECT_SOURCE_DIR}/)  #修改成相对于工程目录的绝对路径
	AUX_SOURCE_DIRECTORY_RECURSE(${FULL_DST_PATH})
ENDMACRO(ADD_DIR_RECURSE)

#打印最终获取的源文件列表信息
FUNCTION(DISPLAY_SRC_FILE_LIST SRC_FILE_LIST)
	MESSAGE("     ╍╍┳╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍")
	FOREACH(FILE_NAME ${ARGV})
		GET_FILENAME_COMPONENT(FULL_FILE_PATH ${FILE_NAME} ABSOLUTE BASE_DIR ${PROJECT_SOURCE_DIR}/)
		FILE(RELATIVE_PATH BASE_PATH "${PROJECT_SOURCE_DIR}" "${FULL_FILE_PATH}")
		MESSAGE("       ┣╍➤ ./${BASE_PATH}")
	ENDFOREACH(FILE_NAME)
	MESSAGE("     ╍╍┻╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍╍")
	RETURN()
ENDFUNCTION(DISPLAY_SRC_FILE_LIST)

FUNCTION(DEPEND filelist)
MESSAGE("Searching for dependencies of all source files ...")
SET(BATCH "${CMAKE_BINARY_DIR}/dep.sh")
FILE(WRITE ${BATCH} "#!/bin/bash\n")
FOREACH(sfile ${ARGV})
	STRING(REGEX MATCH "^[a-zA-Z0-9\-_/]*\.c$" rfile ${sfile})
	IF(rfile)
		FILE(APPEND ${BATCH} "gcc -MM ../${rfile}\n")
	ENDIF(rfile)
ENDFOREACH(sfile)
EXECUTE_PROCESS(COMMAND bash ${BATCH})
FILE(REMOVE ${BATCH})
MESSAGE("Over!")
ENDFUNCTION(DEPEND)

ADD_DIR_RECURSE(src SRC_LIST)   #添加当前目录下的源文件（build目录及图片资源目录等不做处理）
#ADD_DIR_RECURSE(src/diskpart SRC_LIST)
LIST(LENGTH SRC_LIST SRC_NUM)  #获取源文件的数量
MESSAGE(STATUS "========================= Add total ${SRC_NUM} source files: =========================")
DISPLAY_SRC_FILE_LIST(${SRC_LIST})
#DEPEND(${SRC_LIST})

# ********************************** 添加库目录 **********************************
FIND_PACKAGE(PkgConfig REQUIRED)
# ============ gtk+-2.0 ============
PKG_CHECK_MODULES(GTK2 REQUIRED gtk+-2.0)
INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK2_LIBRARY_DIRS})
# ============ x11 ============
PKG_CHECK_MODULES(X11 REQUIRED x11)
INCLUDE_DIRECTORIES(${X11_INCLUDE_DIRS})
LINK_DIRECTORIES(${X11_LIBRARY_DIRS})
# ============ gthread-2.0 ============
PKG_CHECK_MODULES(GTHREAD REQUIRED gthread-2.0)
INCLUDE_DIRECTORIES(${GTHREAD_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTHREAD_LIBRARY_DIRS})
# ============ glib-2.0 ============
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)
INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})
LINK_DIRECTORIES(${GLIB_LIBRARY_DIRS})
# ============ nopoll ============
PKG_CHECK_MODULES(NOPOLL REQUIRED nopoll)
INCLUDE_DIRECTORIES(${NOPOLL_INCLUDE_DIRS})
LINK_DIRECTORIES(${NOPOLL_LIBRARY_DIRS})
# ============ openssl ============
PKG_CHECK_MODULES(OPENSSL REQUIRED openssl)
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIRS})
LINK_DIRECTORIES(${OPENSSL_LIBRARY_DIRS})
# ============ uuid ============
PKG_CHECK_MODULES(UUID REQUIRED uuid)
INCLUDE_DIRECTORIES(${UUID_INCLUDE_DIRS})
LINK_DIRECTORIES(${UUID_LIBRARY_DIRS})

# ********************************** 添加include目录 **********************************
#INCLUDE_DIRECTORIES(${INCLUDE_LIST})
#DISPLAY_SRC_FILE_LIST(${INCLUDE_LIST})
INCLUDE_DIRECTORIES(src)

# ********************************* 设置编译器类型 ********************************
SET(CMAKE_C_COMPILER gcc)
MESSAGE(STATUS "set COMPILER => ${CMAKE_C_COMPILER}")

# ********************************* 添加预编译选项 ********************************
ADD_COMPILE_OPTIONS(-g -Wall)
MESSAGE(STATUS "add compile optional: -g -Wall")

# ********************************* 添加预编译宏定义 ********************************
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
MESSAGE(STATUS "add definitions: _FILE_OFFSET_BITS=64")

# ********************************* 生成共享库 ********************************* 
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)  ##设置共享库/静态库输出路径
#ADD_LIBRARY(utils SHARED ${SRC_LIST})
#SET_TARGET_PROPERTIES(utils PROPERTIES VERSION 1.0 SOVERSION 1)
SET(EXTRA_LIBS ${EXTRA_LIBS} bz2 m rt curl pthread zstd lz4 lzma) ##设置链接依赖的库（静态库/动态库）
MESSAGE(STATUS "set extra libs: ${EXTRA_LIBS}")
#TARGET_LINK_LIBRARIES(utils ${EXTRA_LIBS} ${GTK2_LIBRARIES} ${GTHREAD_LIBRARIES} ${GLIB_LIBRARIES} ${X11_LIBRARIES} ${OPENSSL_LIBRARIES} ${NOPOLL_LIBRARIES} ${UUID_LIBRARIES}) 

# ********************************* 生成可执行文件 *********************************
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) ##设置可执行文件输出路径
#AUX_SOURCE_DIRECTORY(src SRCS)   #添加当前目录下的所有源文件
ADD_EXECUTABLE(main ${SRC_LIST})
#LINK_DIRECTORIES(lib) ##设置链接库的目录
#TARGET_LINK_LIBRARIES(main utils)##设置链接依赖的库（静态库/动态库）
TARGET_LINK_LIBRARIES(main ${EXTRA_LIBS} ${GTK2_LIBRARIES} ${GTHREAD_LIBRARIES} ${GLIB_LIBRARIES} ${X11_LIBRARIES} ${OPENSSL_LIBRARIES} ${NOPOLL_LIBRARIES} ${UUID_LIBRARIES}) 

# ********************************* 添加自定义命令1 (打包：pkg)********************************* 
SET(PKG_DEPENDS 
	./bintools 
	./picture 
	./library 
	./config
	./run
)

ADD_CUSTOM_TARGET(pkg
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMAND cp ./build/main ./main
)

ADD_CUSTOM_COMMAND(TARGET pkg
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMAND GZIP="-9"
	COMMAND tar -zcf netclone-`date +%y%m%d`.tgz --exclude=.svn ${PKG_DEPENDS}
	COMMAND rm -rf ./main
	COMMENT "start to execute target <pkg> ..."
)




