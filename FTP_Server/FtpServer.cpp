#include "FtpServer.h"

void FtpServer::OnAccepted(network::SVR_SOCKET_CONTEXT * _SocketContext){
	_SocketContext->m_Extra = new Client();
}

void FtpServer::OnRecvd(network::SVR_SOCKET_CONTEXT * _SocketContext){
}

void FtpServer::OnSent(network::SVR_SOCKET_CONTEXT * _SocketContext){
}

void FtpServer::OnClosed(network::SVR_SOCKET_CONTEXT * _SocketContext){
}
