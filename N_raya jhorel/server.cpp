//g++ -std=c++11 server.cpp -lpthread -o server
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <vector>
using namespace std;

//////////////////////////////////////////////////////////////////////////
std::vector<pair<char, int>> LISTA_CLIENTES;
vector<pair<char,int>>::iterator Turno;
int tamanio=0;
bool start_game(){
  if (LISTA_CLIENTES.size()==tamanio-1){
    return true;
  }
  return false;
}
void EnviarMensaje(int socket_client, string mensaje){
  int n = write(socket_client, mensaje.c_str(), mensaje.length());
  if (n < 0) perror("ERROR writing to socket");
}
void BroadCast(string mensaje){
  cout<<"mensaje a todos "<<mensaje<<endl;
  for (auto &i : LISTA_CLIENTES){
    EnviarMensaje(i.second, mensaje);
  }
}
void Proceso_Thread(int socket_client){
  string msgToChat;
  char buffer[256];
  int n;
  int longitud = 0;
  do{ 
    n = read(socket_client,buffer,1);     
    if (n < 0) perror("ERROR reading from socket");
    if(n==1){
      BroadCast(buffer);
    }
    bzero(buffer, 256);
  } while (true);
  shutdown(socket_client, SHUT_RDWR);
  close(socket_client); 
}

int main(void){
  struct sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(-1 == SocketFD){perror("can not create socket");exit(EXIT_FAILURE);}
  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1100);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;
  if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){perror("error bind failed");close(SocketFD);exit(EXIT_FAILURE);}
  if(-1 == listen(SocketFD, 10)){perror("error listen failed");close(SocketFD);exit(EXIT_FAILURE);}

  for(;;){
    int ClientSD = accept(SocketFD, NULL, NULL);//ClientSD es el numero del socket
      if(0 > ClientSD){perror("error accept failed");close(SocketFD);exit(EXIT_FAILURE);}
      else{
          char buffer[4];
          int n=read(ClientSD,buffer,4);
          tamanio=(int)(buffer[1])-48;
          //tabla de tamanio del tablero -1
          LISTA_CLIENTES.push_back(std::pair<char,int>(buffer[3],(int)ClientSD));
          cout<<"Inserto cliente "<<buffer[3]<<(int)ClientSD<<endl;
          cout<<"empieza juego "<<start_game()<<endl;
          if(LISTA_CLIENTES.size()==tamanio-1){;
            Turno=LISTA_CLIENTES.begin();
            string pri(1,Turno->first);
            BroadCast(pri+"");
          }
          std::thread(Proceso_Thread, ClientSD).detach();
          Turno++;
      }
  }   
  close(SocketFD);
  return 0;
}
