# CMakeList.txt : 顶级 CMake 项目文件，执行全局配置
# 和此处的包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("FTP")

set(FTP_CLIENT_PI
	ftp_client/ftp_client.h
	ftp_client/ftp_client.cpp
)

set(FTP_SERVER_PI
	ftp_server/ftp_server.h
	ftp_server/ftp_server.cpp
)

#############################################################

set(RESOURCE_COMMON
	resource/common/common.h
	resource/common/common.cpp
)

set(RESOURCE_FTP_CMD
	resource/ftp_cmds/ftp_cmds.h
	resource/ftp_cmds/ftp_cmds.cpp
)

set(RESOURCE
	${RESOURCE_COMMON}
	${RESOURCE_FTP_CMD}
)

#############################################################

set(UTILITY_NETWORK
	utility/network/network.h
	utility/network/network.cpp
)

set(UTILITY_STRING_BUFFER
	utility/string_buffer/string_buffer.h
	utility/string_buffer/string_buffer.cpp
)

set(UTILITY
	${UTILITY_NETWORK}
	${UTILITY_STRING_BUFFER}
)

#############################################################

add_executable(ftp_client ftp_client/main_client.cpp
	${FTP_CLIENT_PI}
	${RESOURCE}
	${UTILITY}
)

add_executable(ftp_server ftp_server/main_server.cpp
	${FTP_SERVER_PI}
	${RESOURCE}
    ${UTILITY}
)

#############################################################

add_executable(test_string_buffer test/test_string_buffer.cpp
	${UTILITY_STRING_BUFFER}
)

#############################################################