#include <iostream>
#include <cstdlib>
#include <string>

#include <stdio.h>
#include <stdlib.h>
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
    method = SSLv23_client_method();
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
    if (SSL_write(SSLsock, string, strlen(string)) != 0) return -1;
    return 0;
}

int readFromSocket(SSL *SSLsock, char *string) {
    if (SSL_read(SSLsock, string, sizeof(string)) != 0) return -1;
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
    initSSL();
    SSLsock = sockToSSL("api.cloudflare.com", 443);

    if ((SSLsock = sockToSSL("api.cloudflare.com", 443)) == NULL) {
        printf("could not open ssl socket\n");
        return 1;
    }

    printf("connected to cloudflare server\n\n");

    char buffer[1024];
    SSL_write(SSLsock, "hi", 2);
    int bytes = SSL_read(SSLsock, buffer, 1024);
    printf("bytes read: %d\n", bytes);
    printf("\nresp: %s\n", buffer);

	// string env_var = getenv("TOKEN");
    // if (env_var.empty()) {
    //     cerr << "[ERROR] No such variable found!" << endl;
    //     exit(EXIT_FAILURE);
    // }
    //printf("y: %s\n", get_extip_address());
    /*char *extip = get_extip_address();
    string strip = extip;
    
    cout << strip << endl;*/

    //free all connections
    SSL_free(SSLsock);
    SSL_CTX_free(ctx);
    return 0;
}

