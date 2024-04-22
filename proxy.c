#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void doit(int fd);

void parse_uri(char *uri, char *hostname, char *pathname, char *port);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
        "Firefox/10.0.3\r\n";

// 프록시 서버의 핵심 동작을 담당하는 함수
void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio_client, rio_server;

    // 클라이언트로부터 HTTP 요청을 읽어들임
    Rio_readinitb(&rio_client, fd);
    Rio_readlineb(&rio_client, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    // 메소드가 GET이 아니면 오류를 발생시킴
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented", "Proxy does not implement this method");
        return;
    }

    // 서버 호스트명과 포트를 추출
    char hostname[MAXLINE], pathname[MAXLINE], port[MAXLINE];
    parse_uri(uri, hostname, pathname, port);

    // 서버로 연결
    int serverfd = Open_clientfd(hostname, port);
    if (serverfd < 0) {
        clienterror(fd, hostname, "404", "Not found", "Proxy couldn't connect to the server");
        return;
    }

    // 서버에 HTTP 요청 전송
    Rio_readinitb(&rio_server, serverfd);
    Rio_writen(serverfd, buf, strlen(buf));

    // 요청 헤더와 본문을 서버로 전송
    while (Rio_readlineb(&rio_client, buf, MAXLINE) > 0) {
        if (strcmp(buf, "\r\n") == 0) break; // 헤더 끝 확인
        Rio_writen(serverfd, buf, strlen(buf));
    }

    // 서버로부터 응답을 받아 클라이언트에 전송
    while (Rio_readlineb(&rio_server, buf, MAXLINE) > 0) {
        Rio_writen(fd, buf, strlen(buf));
    }

    // 연결 종료
    Close(serverfd);
}

void parse_uri(char *uri, char *hostname, char *pathname, char *port) {
    
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {

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
