# CMakeList.txt : 顶级 CMake 项目文件，执行全局配置
# 和此处的包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("FTP")

#############################################################
##independent codes
set(FTP_COMMON
	common/common.h
	common/common.cpp
)
#
set(FTP_UTILITY_FILE
	utility/file/file.h
	utility/file/file.cpp
)
#
set(FTP_UTILITY_NETWORK
	utility/network/network.h
	utility/network/network.cpp
)

#############################################################

set(FTP_PROTOCOL_RESOURCE_FTP_CMD
	protocol/resource/ftp_cmds/ftp_cmds.h
	protocol/resource/ftp_cmds/ftp_cmds.cpp

	${FTP_COMMON}
)
#
set(FTP_PROTOCOL_RESOURCE_FTP_CMD_BUFFER
	protocol/resource/ftp_cmd_buffer/ftp_cmd_buffer.h
	protocol/resource/ftp_cmd_buffer/ftp_cmd_buffer.cpp
)

#############################################################

set(FTP_PROTOCOL_AGETN_DTP_SERVER
	protocol/agent/ftp_dtp/ftp_dtp_server.h
	protocol/agent/ftp_dtp/ftp_dtp_server.cpp
	
	${FTP_UTILITY_FILE}
	${FTP_UTILITY_NETWORK}
)
#
set(FTP_PROTOCOL_AGETN_DTP_USER
	protocol/agent/ftp_dtp/ftp_dtp_user.h
	protocol/agent/ftp_dtp/ftp_dtp_user.cpp
	
	${FTP_UTILITY_FILE}
	${FTP_UTILITY_NETWORK}
)

#############################################################

set(FTP_PROTOCOL_AGENT_USER_PI
	protocol/agent/ftp_user_pi/ftp_user_pi.h
	protocol/agent/ftp_user_pi/ftp_user_pi.cpp

	${FTP_PROTOCOL_AGETN_DTP_USER}
	${FTP_PROTOCOL_RESOURCE_FTP_CMD}
	${FTP_UTILITY_NETWORK}
	${FTP_PROTOCOL_RESOURCE_FTP_CMD_BUFFER}
)
#
set(FTP_PROTOCOL_AGENT_SERVER_PI
	protocol/agent/ftp_server_pi/ftp_server_pi.h
	protocol/agent/ftp_server_pi/ftp_server_pi.cpp
	
	${FTP_PROTOCOL_AGETN_DTP_SERVER}
	${FTP_COMMON}
	${FTP_PROTOCOL_RESOURCE_FTP_CMD}
	${FTP_UTILITY_NETWORK}
	${FTP_PROTOCOL_RESOURCE_FTP_CMD_BUFFER}
)

#############################################################

add_executable(ftp_client launcher/ftp_client.cpp
	${FTP_PROTOCOL_AGENT_USER_PI}
)

add_executable(ftp_server launcher/ftp_server.cpp
	${FTP_PROTOCOL_AGENT_SERVER_PI}
)

#############################################################

add_executable(test_string_buffer test/test_string_buffer.cpp
	${FTP_PROTOCOL_RESOURCE_FTP_CMD_BUFFER}
)

add_executable(test_ftp_dtp_client test/test_ftp_dtp/test_ftp_dtp_client.cpp
	${FTP_PROTOCOL_AGETN_DTP_USER}
)

add_executable(test_ftp_dtp_server test/test_ftp_dtp/test_ftp_dtp_server.cpp
	${FTP_PROTOCOL_AGETN_DTP_SERVER}
)

#############################################################