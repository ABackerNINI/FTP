# CMakeList.txt : 顶级 CMake 项目文件，执行全局配置
# 和此处的包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("FTP")

#############################################################

set(FTP_COMMON
	common/common.h
	common/common.cpp
)

#############################################################

set(FTP_PROTOCOL_AGENT_CLIENT_PI
	protocol/agent/ftp_client/ftp_client_pi.h
	protocol/agent/ftp_client/ftp_client_pi.cpp
)

set(FTP_PROTOCOL_AGENT_SERVER_PI
	protocol/agent/ftp_server/ftp_server_pi.h
	protocol/agent/ftp_server/ftp_server_pi.cpp
)

##

set(FTP_PROTOCOL_AGETN_DTP
	protocol/agent/ftp_dtp/ftp_dtp.h
	protocol/agent/ftp_dtp/ftp_dtp.cpp
)

##

set(FTP_PROTOCOL_RESOURCE_FTP_CMD
	protocol/resource/ftp_cmds/ftp_cmds.h
	protocol/resource/ftp_cmds/ftp_cmds.cpp
)

set(FTP_PROTOCOL_RESOURCE
	${FTP_PROTOCOL_RESOURCE_FTP_CMD}
)

##

set(FTP_PROTOCOL_CLIENT
	${FTP_PROTOCOL_AGENT_CLIENT_PI}
	${FTP_PROTOCOL_AGETN_DTP}
	${FTP_PROTOCOL_RESOURCE}
)

set(FTP_PROTOCOL_SERVER
	${FTP_PROTOCOL_AGENT_SERVER_PI}
	${FTP_PROTOCOL_AGETN_DTP}
	${FTP_PROTOCOL_RESOURCE}
)

#############################################################

set(FTP_UTILITY_NETWORK
	utility/network/network.h
	utility/network/network.cpp
)

set(FTP_UTILITY_STRING_BUFFER
	utility/string_buffer/string_buffer.h
	utility/string_buffer/string_buffer.cpp
)

set(FTP_UTILITY
	${FTP_UTILITY_NETWORK}
	${FTP_UTILITY_STRING_BUFFER}
)

#############################################################

add_executable(ftp_client launcher/ftp_client.cpp
	${FTP_COMMON}
	${FTP_PROTOCOL_CLIENT}
	${FTP_UTILITY}
)

add_executable(ftp_server launcher/ftp_server.cpp
	${FTP_COMMON}
	${FTP_PROTOCOL_SERVER}
    ${FTP_UTILITY}
)

#############################################################

add_executable(test_string_buffer test/test_string_buffer.cpp
	${FTP_UTILITY_STRING_BUFFER}
)

#############################################################