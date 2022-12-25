#include <iostream>
#include <cstdlib>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <resolv.h>

//global vars
SSL_CTX *ctx;


using namespace std;

int initConnection(const char *domainName, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    //get ip from host name
    struct hostent *host;
    struct sockaddr_in addr;

    if ((host = gethostbyname(domainName)) == NULL) {
        printf("could not get host by name\n");
        return -1;
    }

    //setup connection info
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);

    printf("connecting to %s:%d...\n", inet_ntoa(addr.sin_addr), port);

    //make the connection
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("could not connect to server\n");
        close(sock);
        return -1;
    }

    return sock;
}

void initSSL() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}

SSL *sockToSSL(const char *domain, int port) {
    int server;
    SSL *SSLsock;
    const SSL_METHOD *method;
    
    //create new context
    method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    //open the connection and switch from raw socket to ssl socket
    server = initConnection(domain, port);
    SSLsock = SSL_new(ctx);
    SSL_set_fd(SSLsock, server);

    //confirm the connection
    if (SSL_connect(SSLsock) == -1) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return SSLsock;
}

int sendToSocket(SSL *SSLsock, const char *string) {
    if (SSL_write(SSLsock, string, strlen(string)) < 1) return -1;
    return 0;
}

int readFromSocket(SSL *SSLsock, char *string) {
    int bytesRead;
    char readBufferTemp[1024];

    //do {
        bytesRead = SSL_read(SSLsock, readBufferTemp, sizeof(readBufferTemp));
        strcat(string, readBufferTemp);
        //printf("string: %s\n", readBufferTemp);
        //readBufferTemp[0] = '\0';
    //} while (bytesRead > 0);

    return 0;
}

char *get_extip_address() {
    char *result;
    //pipe the output of dig to temp.txt and open that file
    FILE *fptr = fopen("temp.txt", "r");
    system("dig +short myip.opendns.com @resolver1.opendns.com > temp.txt");
    
    //get file size
    int size;
    fseek(fptr, 0L, SEEK_END);
    size = ftell(fptr);
    rewind(fptr);


    fread(result, size, 1, fptr);

    fclose(fptr);
    return result;
}


int main() {
    SSL *SSLsock;
    int sock;
    initSSL();

    if ((SSLsock = sockToSSL("api.cloudflare.com", 443)) == NULL) {
        printf("could not open ssl socket\n");
        return 1;
    }

    printf("connected to cloudflare server\n\n");

    char readBuffer[2048];
    char sendBuffer[4096] = {"GET / HTTP/1.1\r\n\r\n"};
    //PUT /client/v4/zones/ab4593b1d7b96846e08b4f5553657297/dns_records/3490891a976816a31c0560824b3b88f0 HTTP/1.1\r\n
    //Content-Type: application/json\r\nAuthorization: Bearer KJvlvmNzVz4EFHSO5tEKpAeYHTwaxCg0QtkgnCtM\r\n\r\n{\"type\":\"A\",\"name\":\"minigames.host\",\"content\":\"69.69.69.69\",\"ttl\":3600,\"proxied\":true}
    //printf("%s\n", sendBuffer);
    
    sendToSocket(SSLsock, sendBuffer);
    readFromSocket(SSLsock, readBuffer);
    printf("response: %s\n", readBuffer);

    SSL_free(SSLsock);
    SSL_CTX_free(ctx);
    return 0;
}

