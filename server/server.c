#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client_t.h"
#include "session.h"

int main(int argc, char *argv[])
{
	int port; /* Server's port */
	int sockfd; /* Server's socket fd */
	int clisockfd; /* Client's socket fd */
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	pthread_t thread; /* Thread for new user */
	client_t *client; /* New client */
	size_t uid = 0; /* User's personal ID on this session */
	socklen_t clilen = sizeof(client_addr);
	
	/* Usage: ./server port_number */
	if (argc < 2)
	{
        fprintf(stderr,"ERROR, no port provided.\n");
        return 1;
    }
	
	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket.");
		return 1;
	}
	memset((char *) &server_addr, 0, sizeof(server_addr));
    port = atoi(argv[1]);
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
	
	/* Binding */
	if (bind(sockfd, (struct sockaddr *) &server_addr,
        sizeof(server_addr)) < 0)
		{
			perror("ERROR on binding.");
			return 1;
		}
		
	/* Listening */	
	if (listen(sockfd,5) < 0)
	{
		perror("ERROR on listening.");
		return 1;
	}
	
	puts("[SYSTEM_MSG]: Server has been started.");
	
	/* Accept clients */
	while(1)
	{
		clisockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clilen);
		if (clisockfd < 0)
		{
			perror("ERROR on accept");
			continue;
		}
		
		/* Check if MAX_CLIENTS is reached */
		if ((get_clients_num()+1) == MAX_CLIENTS){
			printf("[SYSTEM_MSG]: Can't accept %s (MAX_CLIENTS is reached).", inet_ntoa(client_addr.sin_addr));
			close(clisockfd);
			continue;
		}
		else
		{
			uid++;
			printf("[SYSTEM_MSG]: %s has been accepted.\n", inet_ntoa(client_addr.sin_addr));
		}
		
		/* Set up client's settings */
		client = (client_t*)malloc(sizeof(client_t));
		client->addr = client_addr;
		client->clientfd = clisockfd;
		client->uid = uid;
		sprintf(client->name, "%lu", uid);
		
		/* Add user to UserList and create new thread */
		add_user(client);
		if(pthread_create(&thread, NULL, &session, (void*) client))
		{
			close(clisockfd);
			delete_user(client->uid);
			free(client);
		}
	}
	return 0; /* Why not? :) */
}