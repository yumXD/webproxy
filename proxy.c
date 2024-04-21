#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void doit(int fd);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
        "Firefox/10.0.3\r\n";

void doit(int fd) {
    
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]); // 지정된 포트에서 수신 소켓을 생성
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA * ) & clientaddr,
                        &clientlen); // 클라이언트의 연결을 수락
        Getnameinfo((SA * ) & clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                    0); // 클라이언트의 호스트 이름과 포트 번호를 파악
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd); // 클라이언트 요청 처리
        Close(connfd); // 연결 소켓 닫기
    }
}
