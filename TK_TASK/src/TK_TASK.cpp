#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

//Вывод сообщения об ошибке
void ERR_MESS(char object[]) {
	printf("%s failed with error #%d\n", object, errno);
}

//Перевод int в char*
static char buf[13];
char *itoa(int i){
	char *ptr = buf + sizeof(buf) - 1;
	unsigned int u;
	u = i;
	*ptr = 0;
	do
		*--ptr = '0' + (u % 10);
	while(u/=10);
	return ptr;
}

int main(int argc, char *argv[]) {
	int port_num = 9934; 											// порт для подключения
	int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 	// сокет для соединения
	if (listen_sock == -1) {
		ERR_MESS((char*)"Creating of socket");
		return(1);
	}
	sockaddr_in sa; 								// структура для бинда сервера
	memset((char*)&sa, 0, sizeof(sa));				// зачищаем выделенную память
	sa.sin_family = AF_INET; 						// тип
	sa.sin_port = htons(port_num); 					// порт
	sa.sin_addr.s_addr = inet_addr("127.0.0.1"); 	// свой же IP
	const int on = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	if (bind(listen_sock, (sockaddr*)&sa, sizeof(sa)) == -1) {
		ERR_MESS((char*)"Bind");
		return(1);
	}
	if (listen(listen_sock, SOMAXCONN) == -1) {				//ждём
		ERR_MESS((char*)"Listen");
		return(1);
	}

	int connection_socket = accept(listen_sock, NULL, NULL); //соединяемся
	if (connection_socket == -1) {
		ERR_MESS((char*)"Accept");
		return(1);
	}
	while(1){
		char read_buf[1] = {0}; // буффер для чтения, хоть и долго
								// зато можно большие сообщения
		int recv_bytes; 		// количество получаемых байтов
		int s=0, e=0, e_changed = -1, s_changed = -1;
		do {
			recv_bytes = recv(connection_socket, read_buf,1, MSG_WAITALL); // получаем
			if (recv_bytes > 0) {
				if (read_buf[0] == 'S' and s_changed== -1) s_changed = 1;
				if (read_buf[0] == 'E'and e_changed== -1) e_changed = 1;
				if (e_changed==-1) e ++;
				if (s_changed==-1) s ++;
			}
			else if (recv_bytes == 0){
				printf("Connection closed\n");
				return(0);
			}
			else {
				ERR_MESS((char*)"Recv");
				return(1);
			}
		} while (read_buf[0] != '\r');	// пока в конце не переход на новую строку
		int k = e - s - 1;				// считаем кол-во символов подстроки
		if(e_changed==-1 or s_changed==-1 or k<1) {
			char message[]="There is no subsequence between S and E\n";
			int send_bytes = send(connection_socket,message , sizeof(message), 0); //возвращаем результат
					if (send_bytes == -1) {
						ERR_MESS((char*)"Send");
						return(1);
					}
		}

		else{
			const char* res=itoa(k);				// переводим в текст
			char message[123]="Number of characters : ";
			strncat(message,res,strlen(res));
			strncat(message,"\n ",1);
			int send_bytes = send(connection_socket, message, strlen(message), 0); //возвращаем результат
			if (send_bytes == -1) {
				ERR_MESS((char*)"Send");
				return(1);
			}
		}
	}
	return(1);
}
