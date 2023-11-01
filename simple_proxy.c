#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 4096
#define PORT 9097

char* extractTargetUrl(const char* request) {
    char firstLine[MAX_BUFFER_SIZE];
    sscanf(request, "%[^\n]", firstLine);
    printf("\n%s\n",firstLine);

    //size_t start, end;
    //sscanf(firstLine, "GET %lu%*s%lu", &start, &end);
    char* token;
    token=strtok(firstLine, " ");
    token=strtok(NULL, " ");
    
    //printf("\n%lu\n",start);
    //printf("\n%lu\n",end);

    char* targetUrl = malloc(strlen(token)+1);
    //strncpy(targetUrl, firstLine + start, end - start);
    strcpy(targetUrl, token);
    targetUrl[strlen(token)] = '\0';
    strncpy(targetUrl,targetUrl+7,strlen(targetUrl)-2);

    return targetUrl;
}

void handleClientRequest(int clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Read the client's request
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
    char* request = strdup(buffer);
    //printf("\n%s\n",request);

    // Extract the target URL from the request
    char* targetUrl = extractTargetUrl(request);
    printf("\n%s\n",targetUrl);
   

    // Open a connection to the target server
    struct sockaddr_in targetAddr;
    memset(&targetAddr, 0, sizeof(targetAddr));
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(80);

    struct hostent* host = gethostbyname(targetUrl);
    if ((host=gethostbyname(buffer))==NULL){

        switch (h_errno){
        case HOST_NOT_FOUND:
        printf("Host not found. Error %i\n", h_errno);
        break;
        case TRY_AGAIN:
        printf("Non-Authoritative. Host not found. Error %i\n", h_errno);
        break;
        case NO_DATA:
        printf("Valid name, no data record of requested type. Error %i\n", h_errno);
        break;
        case NO_RECOVERY:
        printf("Non recoverable error. Error %i\n", h_errno);
        break;
    
        }
    }

    else
        printf("\nIP address of %s is: ", host->h_name );
    
    printf("%s",host->h_name);
    printf("%s",host->h_addr_list[0]);
    
    memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);

    int targetSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (targetSocket == -1) {
    perror("Failed to create target socket");
    return ;
}
    connect(targetSocket, (struct sockaddr*)&targetAddr, sizeof(targetAddr));

    // Forward the client's request to the target server
    write(targetSocket, request, strlen(request));

    // Forward the target server's response to the client
    memset(buffer, 0, sizeof(buffer));
    while ((bytesRead = read(targetSocket, buffer, sizeof(buffer))) > 0) {
        write(clientSocket, buffer, bytesRead);
        memset(buffer, 0, sizeof(buffer));
    }

    // Close the sockets
    close(targetSocket);
    close(clientSocket);

    free(request);
    free(targetUrl);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Failed to create server socket");
        return 1;
    }

    struct sockaddr_in serverAddr;  
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    //serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);           //<<<<<<<========================proxy address
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 


    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Failed to bind server socket");
        return 1;
    }

    // Print the bound IP address
    struct sockaddr_in bound_addr;
    socklen_t bound_len = sizeof(bound_addr);
    if (getsockname(serverSocket, (struct sockaddr *)&bound_addr, &bound_len) == 0) {
        char bound_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &bound_addr.sin_addr, bound_ip, sizeof(bound_ip));
        printf("Proxy server bound to IP address: %s\n", bound_ip);
    } else {
        perror("Error getting bound IP address");
    }

    if (listen(serverSocket, 10) == -1) {
        perror("Failed to listen on server socket");
        return 1;
    }

    printf("Now serving at :%d\n", PORT);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Failed to accept client connection");
            continue;
        }
        
         pid_t pid = fork();
         if (pid == -1) {
             perror("Failed to fork");
             continue;
         }

         if (pid == 0) {
             // Child process
             close(serverSocket);
             handleClientRequest(clientSocket);
            return 0;
        }

        //Parent process
        close(clientSocket);
    }

    close(serverSocket);
    return 0;
}
