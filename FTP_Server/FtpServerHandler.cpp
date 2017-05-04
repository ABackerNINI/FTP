#include "FtpServerHandler.h"

void FtpServerHandler::CommitHandler(network::SVR_SOCKET_CONTEXT * _SocketContext, network::ServerCallback _ServerCallback){
	
}

char *str = "HTTP/1.1 200 OK\r\n\r\nContent-Length:5\r\n\r\n12345\r\n";

void FtpServerHandler::ServerCallback(network::SVR_SOCKET_CONTEXT *_SocketContext, int _Ev, void *_Data) {
	switch (_Ev) {
	case network::SVREV_ACCEPTED:
		printf("Accepted\n");
		Client *_NewClient = new Client();
		_SocketContext->m_Extra = _NewClient;
		break;
	case network::SVREV_RECVD:
		printf("Recvd\n");
		network::Server::Send(_SocketContext->m_ClientSocket, str, strlen(str));
		break;
	case network::SVREV_SENT:
		printf("Sent\n");
		Sleep(5000);
		network::Server::Close(_SocketContext->m_ClientSocket);
		break;
	case network::SVREV_CLOSED:
		printf("Closed\n");
		break;
	case network::SVREV_COMMIT:
		printf("Commit\n");
		if (strcmp((const char *)_Data, "QUIT") == 0) {
			delete (Client *)_SocketContext->m_Extra;
		}
		break;
	default:
		printf("Default\n");
		break;
	}
}