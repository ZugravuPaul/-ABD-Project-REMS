#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 4096
#define PORT 9097

char* extractTargetURL(const char* request) {
    char hostname[MAX_BUFFER_SIZE];
    sscanf(request, "%*[^\n]\n%[^\n]", &hostname);
    
    char* targetUrl = malloc(strlen(hostname)+1);
    strcpy(targetUrl, hostname+6);



    return targetUrl;
}

void handleClientRequest(int clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // read request
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
    char* request = strdup(buffer);

    printf("\n%s\n",request);
    char* targetUrl = extractTargetURL(request);
    targetUrl[strlen(targetUrl)-1]='\0';
    printf("\n%s\n",targetUrl);


    // close(clientSocket);
    // free(request);
    // goto jump;
    


    // Open a connection to the target server
    struct sockaddr_in targetAddr;
    memset(&targetAddr, 0, sizeof(targetAddr));
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(80);

    struct hostent* host = gethostbyname(targetUrl);
    
    if (host==NULL){
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
        close(clientSocket);
        free(request);
        free(targetUrl);
        return;
    }
    
    
    memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);
    char target_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &targetAddr.sin_addr, target_ip, sizeof(target_ip));  //debug hostname and ip address
    printf("\nHostName of %s is: %s\n", host->h_name ,target_ip);

    int targetSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (targetSocket == -1) {
    perror("Failed to create target socket");
    return ;
}
    connect(targetSocket, (struct sockaddr*)&targetAddr, sizeof(targetAddr));

    //forward the client's request
    write(targetSocket, request, strlen(request));

    //forward the target server's response 
    memset(buffer, 0, sizeof(buffer));
    while ((bytesRead = read(targetSocket, buffer, sizeof(buffer))) > 0) {
        write(clientSocket, buffer, bytesRead);

        //buffer[(int)bytesRead+1]='\0';
        printf("%.*s", (int)bytesRead, buffer); 
        memset(buffer, 0, sizeof(buffer));
    }

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
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);           //<<<<<<<========================proxy address
    //serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 


    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Failed to bind server socket");
        return 1;
    }

    // server bound IP address
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
        else {
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
        
        close(clientSocket);
        }
    }

    close(serverSocket);
    return 0;
}
