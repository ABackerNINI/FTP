# CMakeList.txt : 顶级 CMake 项目文件，执行全局配置
# 和此处的包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("FTP")

set(FTP_CLIENT_PI
	ftp_client/FtpClient.h
	ftp_client/FtpClient.cpp
)

set(FTP_SERVER_PI
	ftp_server/FtpServer.h
	ftp_server/FtpServer.cpp
)

#############################################################

set(RESOURCE_COMMON
	resource/Common/Common.h
	resource/Common/Common.cpp
)

set(RESOURCE_FTP_CMD
	resource/FtpCmds/FtpCmds.h
	resource/FtpCmds/FtpCmds.cpp
)

set(RESOURCE
	${RESOURCE_COMMON}
	${RESOURCE_FTP_CMD}
)

#############################################################

set(UTILITY_NETWORK
	utility/Network/Network.h
	utility/Network/Network.cpp
)

set(UTILITY_STRING_BUFFER
	utility/StringBuffer/StringBuffer.h
	utility/StringBuffer/StringBuffer.cpp
)

set(UTILITY
	${UTILITY_NETWORK}
	${UTILITY_STRING_BUFFER}
)

#############################################################

add_executable(ftp_client ftp_client/ClientMain.cpp
	${FTP_CLIENT_PI}
	${RESOURCE}
	${UTILITY}
)

add_executable(ftp_server ftp_server/ServerMain.cpp
	${FTP_SERVER_PI}
	${RESOURCE}
    ${UTILITY}
)

# 包含子项目。
#add_subdirectory ("FTP")
