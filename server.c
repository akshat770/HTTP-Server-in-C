// Simple HTTP Server in C for Windows
// Handles GET, POST, PUT, DELETE requests
// Logs requests in index.html
// Beginner-friendly, easy to understand

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") 

#define PORT 8080
#define BUF_SIZE 5000 


void create_html_log(const char *method, const char *path, const char *body) {
    FILE *file = fopen("index.html", "w"); 
    if (!file) return;

    fprintf(file,
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <title>HTTP Request Log</title>\n"
        "  <style>\n"
        "    body { font-family: Arial; padding: 20px; background: #f5f5f5; }\n"
        "    .container { background: white; padding: 20px; border-radius: 8px; "
        "box-shadow: 0px 0px 10px rgba(0,0,0,0.1); }\n"
        "    h1 { color: #007bff; }\n"
        "  </style>\n"
        "</head>\n"
        "<body>\n"
        "<div class=\"container\">\n"
        "<h1>%s Request Received</h1>\n"
        "<p>Path: %s</p>\n",
        method, path
    );

    if (body && strlen(body) > 0)
        fprintf(file, "<p>Body: %s</p>\n", body);

    fprintf(file, "</div>\n</body>\n</html>");
    fclose(file);
}


void generate_response(const char *method, const char *path, const char *body, char *response) {
    char content[BUF_SIZE];

    
    snprintf(content, BUF_SIZE,
        "<html><body><h1>%s Request Received</h1><p>Path: %s</p>%s</body></html>",
        method, path, (body ? body : ""));

    
    snprintf(response, BUF_SIZE,
        "HTTP/1.0 200 OK\r\n"
        "Server: C-WebServer\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n"
        "%s",
        strlen(content), content);
}


void handle_client(SOCKET client_fd) {
    char buffer[BUF_SIZE];
    char response[BUF_SIZE];

    
    int bytes = recv(client_fd, buffer, BUF_SIZE - 1, 0);
    if (bytes <= 0) return;

    buffer[bytes] = '\0';

    
    char method[10], path[1024];
    sscanf(buffer, "%s %s", method, path);

    
    char *body = strstr(buffer, "\r\n\r\n");
    if (body) body += 4; 

    if (body) {
        
        char *end = body + strlen(body) - 1;
        while(end > body && (*end == '\r' || *end == '\n')) {
            *end = '\0';
            end--;
        }
    }

    
    create_html_log(method, path, body);

    
    if (strcmp(method,"GET")==0 || strcmp(method,"POST")==0 ||
        strcmp(method,"PUT")==0 || strcmp(method,"DELETE")==0) {
        generate_response(method, path, body, response);
    } else {
        
        snprintf(response, BUF_SIZE,
            "HTTP/1.0 405 Method Not Allowed\r\n"
            "Server: C-WebServer\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 58\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>405 Method Not Allowed</h1></body></html>");
    }

    
    send(client_fd, response, strlen(response), 0);
}

int main() {
    
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Winsock initialization failed\n");
        return 1;
    }

    
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);         
    server_addr.sin_addr.s_addr = INADDR_ANY;   

    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    
    if (listen(server_fd, 10) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("HTTP server is running on port %d\n", PORT);

    
    while (1) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);

        SOCKET client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected\n");

        
        handle_client(client_fd);

        
        closesocket(client_fd);
        printf("Client disconnected\n\n");
    }

    
    closesocket(server_fd);
    WSACleanup();
    return 0;
}