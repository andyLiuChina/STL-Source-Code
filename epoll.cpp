#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

using namespace std;

int sockprepare()
{
	int sock_t;

	sock_t = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in addr;
	
	bzero(&addr, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = 0;             //addr.sin_port = htons(44444);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(sock_t, (struct sockaddr*)&addr, sizeof(addr));

	struct sockaddr_in recv;
	socklen_t len = sizeof(recv);

	memset(&recv, 0, len);
	getsockname(sock_t, (struct sockaddr*)&recv, &len);
	cout << "bound port: " << ntohs(recv.sin_port) << endl;

	listen(sock_t, 3000);
	
	int flag = 1;
	setsockopt(sock_t, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	return sock_t;
}


int main()
{
	int sock_s = sockprepare();

	int epfd = epoll_create(32);
	
	struct epoll_event ev, events[32];
	ev.events = EPOLLIN;
	ev.data.fd = sock_s;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sock_s, &ev);

	int i, res = 0;
	int client_s;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	char buf[124] = {0};
	string str;

	while(1) {
		res = epoll_wait(epfd, events, 32, -1);

		for(i = 0; i < res; i++) {
			if(events[i].data.fd == sock_s) {  //new connection
				client_s = accept(sock_s, (struct sockaddr*)&client_addr, &len);
				printf("new connection, addr.ip: %s, addr.port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				write(client_s, "Welcome to my chat room\n", 24);

				ev.data.fd = client_s;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client_s, &ev);
			} else {
				recv(events[i].data.fd, buf, 124, 0);
				str = "You said: ";
				str += buf;
				send(events[i].data.fd, str.c_str(), strlen(str.c_str()), 0);
				memset(buf, 0, 123);
				str.clear();
			}
		}
	}

	return 0;
}
