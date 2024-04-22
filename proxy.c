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

// 주어진 URI를 호스트명, 포트, 경로로 파싱하는 함수
void parse_uri(char *uri, char *hostname, char *pathname, char *port) {
    char *ptr;
    ptr = strstr(uri, "://");
    if (ptr != NULL) { // "://" 문자열을 찾았을 경우
        sscanf(ptr + 3, "%[^:/]:%[^/]%s", hostname, port, pathname); // 호스트명, 포트, 경로를 추출하여 변수에 저장
    } else { // "://" 문자열을 찾지 못한 경우
        sscanf(uri, "%[^:/]:%[^/]%s", hostname, port, pathname); // URI 전체를 호스트명, 포트, 경로로 간주하여 추출
    }
}

// 클라이언트에게 에러 메시지를 전송하는 함수
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

    // 에러 메시지 생성
    sprintf(body, "<html><title>Proxy Error</title>"); // HTML 페이지의 시작 부분 작성
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body); // 페이지 바탕색 설정
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg); // 에러 번호와 짧은 메시지 추가
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause); // 에러 원인 추가
    sprintf(body, "%s<hr><em>The Proxy server</em>\r\n", body); // 서버 정보 추가

    // HTTP 응답 전송
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg); // HTTP 응답 라인 작성 (상태 코드와 메시지)
    Rio_writen(fd, buf, strlen(buf)); // 클라이언트에게 전송
    sprintf(buf, "Content-type: text/html\r\n"); // HTML 컨텐츠 타입 설정
    Rio_writen(fd, buf, strlen(buf)); // 클라이언트에게 전송
    sprintf(buf, "Content-length: %lu\r\n\r\n", strlen(body)); // HTML 본문의 길이 설정
    Rio_writen(fd, buf, strlen(buf)); // 클라이언트에게 전송
    Rio_writen(fd, body, strlen(body)); // HTML 본문을 클라이언트에게 전송
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
