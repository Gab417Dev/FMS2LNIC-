#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
	// Listening socket creation
	int listen_fd = socket(
		AF_INET,	 // IPv4 internet protocol
		SOCK_STREAM, // two-way connection-based streams
		0
	);
	if (listen_fd < 0)
	{
		perror("socket");
		return 1;
	}

	// Setting "SO_REUSEADDR"
	int opt = 1;
	int res = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (res < 0)
	{
		perror("setsockopt");
		return 1;
	}

	// Setting up the socket address
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345); // port
	addr.sin_addr.s_addr = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr); // ip address

	// Bind the socket with the address
	res = bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (res < 0)
	{
		perror("bind");
		return 1;
	}

	// Listen for connection
	int backlog = 16;
	res = listen(listen_fd, backlog);
	if (res < 0)
	{
		perror("listen");
		return 1;
	}

	while (true)
	{
		// Wait for a client
		sockaddr_in client_addr{};
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
		if (client_fd < 0)
		{
			perror("accept");
			continue;
		}

		char ip[INET_ADDRSTRLEN];
		const char* ip_str = inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
		if (ip_str == nullptr)
		{
			perror("inet_ntop");
			ip_str = "unknown";
		}
		unsigned short client_port = ntohs(client_addr.sin_port);

		std::printf("Client connected: %s:%hu\n", ip_str, client_port);

		// echo loop
		while (true)
		{
			// Read
			char buf[4096];
			ssize_t n = read(client_fd, buf, sizeof(buf));

			// Close internal loop if client is closed or if an error occurs
			if (n == 0)
				break;
			if (n < 0)
			{
				perror("read");
				break;
			}

			// Sent without getting killed by SIGPIPE
			send(client_fd, buf, n, MSG_NOSIGNAL);
		}

		// Closing the connection with the client
		std::printf("Client disconnected: %s:%hu\n", ip_str, client_port);
		close(client_fd);
	}
}