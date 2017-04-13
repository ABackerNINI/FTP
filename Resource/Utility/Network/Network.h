#include <stdio.h>

namespace network {
	struct config_server {
		int port;
		int max_connect;
	};

	typedef void (*callback_server)(int ev, void *data);

	class Server {
	public:
		bool start(config_server *cs,callback_server callback);
		bool stop();
	private:
		bool _start(int port,int max_connect);
		void _stop(SOCKET sockid);
	private:
		SOCKET sockid;
	};

	class Client {
	public:
		bool send();
	private:

	};
}