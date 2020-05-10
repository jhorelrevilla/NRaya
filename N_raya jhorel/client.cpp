//gcc -std=c++11 client.cpp -lpthread -o client
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
using namespace std;



struct Casilla{
    char prop = '*';
};
class Raya{
private:
    vector<vector<Casilla>> tablero;
public:
    int size;
    int num_jugada = 0;
    int max_jugadas = size * size; 
    Raya(int size){
        this->size = size;
        tablero.resize(size, vector<Casilla>(size));
    }
    void ImprimirTablero(){
        int x = 0;
        string top = "  ";
        for (int i = 0; i < size; i++){top+= to_string(i)+" ";}
        top+="\n";
        cout << top;
        for (auto i = tablero.begin(); i != tablero.end(); ++i){
            cout << x << " ";x++;
            for( auto j = i->begin(); j != i->end(); ++j)           
                cout << j->prop<<" ";          
            cout << "\n";
        } 
    }
    bool InsertarJugada(char ficha, int x, int y){
      tablero[x][y].prop = ficha;
    }
    bool IsWin(int y, int x, char ficha){
      int d=0,a=0;
      for(int i = 0; i < size; i++){
          if(tablero[y][i].prop ==ficha)d++;
          if(tablero[i][x].prop ==ficha)a++;
      }
      if (d==size || a==size ){return true;}
      d=0;a=0;
      if (x==y || (x+y)==size-1){
          for(int i = 0; i < size; i++){
              if(tablero[i][i].prop==ficha)d++;
              if(tablero[i][size-1-i].prop==ficha)a++;
          }
          if (d==size || a==size ){return true;}
      }
      return false;
    }
    int VerificarEstadoJuego(int x, int y, char ficha){
      if (IsWin(y, x, ficha)){
          return 1; //Hay ganador
      }
      else{
          if (num_jugada >= max_jugadas-1){
              //Hay empate
              return -1;
          }
          else{   //Juego en marcha
              num_jugada++;
              return 0;
          }       
      }
    }
    void ReiniciarTablero(){
      tablero.assign(size, vector<Casilla>(size));
      num_jugada = 0;
    }
    bool verificarPos(int x,int y){
      cout<<"x "<<x<<" y "<<y<<endl;
      if ((int)tablero[x][y].prop == (int)'*')
        return true;
      return false;
    }
    ~Raya(){};
};
////////////////////////////////////////////////////////////////////////////////////////
int size_juego = 3;
Raya *TresRaya;
string usuario;
////////////////////////////////////////////////////////////////////////////////////////
void RecepcionMensaje(int SocketFD){
  int n;
  char buffer[256];
  int longitud = 0;
  for(;;){
    n = read(SocketFD,buffer,1);
    if (n < 0) perror("ERROR reading from socket");
    if (n == 1){
      TresRaya->InsertarJugada(buffer[0],(int)(buffer[1]-48),(int)(buffer[2]-48));
      TresRaya->ImprimirTablero();
    }
    bzero(buffer,256);
  }
}
////////////////////////////////////////////////////////////////////////////////////////
void EnvioMensaje(int SocketFD){
  string msgToChat="";
  char buffer[256];
  int n;
  for(;;){
      cout << "\nIngrese posiciones: \n";
      cin>>msgToChat;
      cout<<"1"<<endl;
      
      if(msgToChat!="" && TresRaya->verificarPos((int)msgToChat[0]-48,(int)msgToChat[1]-48 )){
        cout<<"2"<<endl;
        msgToChat=usuario[0]+msgToChat;
        cout<<"envie al server"<<msgToChat<<endl;
        n = write(SocketFD, msgToChat.c_str(), msgToChat.length());
        bzero(buffer, 256);
      } 
  }
}
////////////////////////////////////////////////////////////////////////////////////////
int main(void){
  struct sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  int n;
  if (-1 == SocketFD){perror("cannot create socket");exit(EXIT_FAILURE);}
  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
  stSockAddr.sin_family = AF_INET;

  stSockAddr.sin_port = htons(1100);
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

  if (0 > Res){perror("error: first parameter is not a valid address family");close(SocketFD);exit(EXIT_FAILURE);}
  else if (0 == Res){perror("char string (second parameter does not contain valid ipaddress");close(SocketFD);exit(EXIT_FAILURE);}
  if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){perror("connect failed");close(SocketFD);exit(EXIT_FAILURE);}
  bzero(buffer, 256);
  // / / / / / / / / / / / / / / / / / / / / / /
  //cout<<"Escribe el tamanio del juego \n";
  //cin>>size_juego;
  TresRaya=new Raya(size_juego);
  cout<<"Escoje la ficha que te va a representar: ";
  getline(cin,usuario);
  string msg="T"+to_string(size_juego)+"U"+usuario;
  n=write(SocketFD, msg.c_str(), msg.length());//envia T(tamanio)U(ficha)
  // / / / / / / / / / / / / / / / / / / / / / / / / / / / / /
  std::thread(EnvioMensaje, SocketFD).detach();
  std::thread(RecepcionMensaje, SocketFD).detach();
  for(;;){}
  shutdown(SocketFD, SHUT_RDWR);
  close(SocketFD);
  return 0;
}
