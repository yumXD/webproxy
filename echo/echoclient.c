#include "csapp.h"

int main(int argc, char **argv) // argc: 입력받은 인자의 수 argv: 입력받은 인자들의 배열
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) { // 파일 실행 시 인자를 제대로 넘겨주지 않았을 경우
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]); // 안내 메세지를 출력하고,
        exit(0); // 어플리케이션 종료
    }
    host = argv[1]; // 전달해준 첫 번째 인자를 host에 저장
    port = argv[2]; // 전달해준 첫 번째 인자를 port에 저장

    clientfd = Open_clientfd(host, port); // 소켓 인터페이스 핸들링을 도와주는 Open_clientfd 함수를 호출하여 서버와 연결하고, 리턴받은 소켓 식별자를 clientfd에 저장
    Rio_readinitb(&rio, clientfd); // rio 구조체를 초기화하고, rio를 통해 파일 디스크립터 clientfd에 대한 읽기 작업을 수행할 수 있도록 설정

    while (Fgets(buf, MAXLINE, stdin) != NULL) { // 반복하여 유저에게서 받은 입력을 buf에 저장하는데, 입력이 끊기거나 오류가 발생한다면 반복문을 종료
        Rio_writen(clientfd, buf, strlen(buf)); // 파일 디스크립터를 통해 buf에 저장된 데이터를 서버로 전송
        Rio_readlineb(&rio, buf, MAXLINE); // rio 구조체를 통해 파일 디스크립터에서 한 줄의 문자열을 읽어와 buf에 저장, MAXLINE은 버퍼의 최대 크기
        Fputs(buf, stdout); // buf에 저장된 문자열을 표준 출력 stdout에 출력해줌
    }
    Close(clientfd); // 파일 디스크립터를 닫아서 클라이언트의 연결을 종료하고 사용한 리소스를 반환해줌
    exit(0);
}