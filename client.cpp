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
    void ImprimirTablero();
    bool InsertarJugada(char ficha, int x, int y);
    bool win(int y,int x,char ficha);
    Raya(int size){
        this->size = size;
        tablero.resize(size, vector<Casilla>(size));
    }
    ~Raya(){};
};
/////////////////////////////////////////////////////////////////
void Raya::ImprimirTablero(){
    int x = 0;
    string col = "  ";
    for (int y = 0; y < size; ++y)
        col+= to_string(y);
    cout << col << "\n";
    for (auto i = tablero.begin(); i != tablero.end(); ++i){
        cout << x << " ";x++;
        for( auto j = i->begin(); j != i->end(); ++j)           
            cout << j->prop ;          
        cout << "\n";
    }        
}
/////////////////////////////////////////////////////////////////
bool Raya::InsertarJugada(char ficha, int x, int y){
  if(tablero[x][y].prop=='*'){
    tablero[x][y].prop = ficha;
    return true;
  }
  return false;
}
/////////////////////////////////////////////////////////////////
bool Raya::win(int y,int x,char ficha){
    int d=0,a=0;
    for(int i = 0; i < size; i++){
        if(tablero[y][i].prop ==ficha)d++;
        if(tablero[i][x].prop ==ficha)a++;
    }
    if (d==size || a==size ){return true;}
    d=0;a=0;
    if (x==y || (x+y)==size){
        for(int i = 0; i < size; i++){
            if(tablero[i][i].prop==ficha)d++;
            if(tablero[i][size-1-i].prop==ficha)a++;
        }
        if (d==size || a==size ){return true;}
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////////

string ficha="";
Raya mapa(3);
bool turno=1;
/*
string PadZeros(int number, int longitud){
  string num_letra = to_string(number);
  for (int i = 0; i < longitud - num_letra.length()+1; ++i)
    num_letra = "0" + num_letra;
  return num_letra;
}
*/
///////////////////////////////////////////////////////////////////////////////////
void ProtocoloMensaje(string mensaje){//Ax00TX //A:actualizacion //T:Turno
  mapa.InsertarJugada(mensaje[7],(int)mensaje[8]-48,(int)mensaje[9]-48);
  mapa.ImprimirTablero();
  if(mensaje[11]==ficha[0])
    turno=true;
}
///////////////////////////////////////////////////////////////////////////////////
void RecepcionMensaje(int SocketFD){
  int n;
  char buffer[256];
  int longitud=0;
  for(;;){
    //Lectura length de mensaje
    n = read(SocketFD,buffer,12);
    if (n < 0) perror("ERROR reading from socket");
    if(buffer[1]=='1'){//LOSE
      cout<<"Perdiste"<<endl;
    }
    if(buffer[3]=='1'){//win
      cout<<"Gano "<<buffer[11]<<endl;
    }
    if(buffer[5]=='1'){//=
      cout<<"Empate"<<endl;
    }
    ProtocoloMensaje(buffer);
    bzero(buffer,256);
  }
}
///////////////////////////////////////////////////////////////////////////////////
void EnvioMensaje(int SocketFD){
  string msgToChat="";
  char buffer[256];
  int n;
  //puede ser solo 0 o X
  for(;;){ 
    if(!turno)//si no es su turno no inserta jugada
      continue;
    cin.clear(); 
    cout << "\nIngrese jugada: ";
    getline(cin, msgToChat);
    if(!mapa.InsertarJugada(ficha[0],(int)msgToChat[0]-48,(int)msgToChat[1]-48));
      continue;
    msgToChat = ficha+msgToChat;
    n = write(SocketFD, msgToChat.c_str(), msgToChat.length());
    mapa.ImprimirTablero();
    
    turno=false;
    bzero(buffer, 256);     
  }
}
///////////////////////////////////////////////////////////////////////////////////
int main(void){
  struct sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  if (-1 == SocketFD){
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }
  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1100);
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
  if (0 > Res){
    perror("error: first parameter is not a valid address family");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  else if (0 == Res){
    perror("char string (second parameter does not contain valid ipaddress");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  bzero(buffer, 256);
  cout<<"Escoge ficha ";
  getline(cin,ficha);
  std::thread(EnvioMensaje, SocketFD).detach();
  std::thread(RecepcionMensaje, SocketFD).detach();
  
  for(;;){}
  shutdown(SocketFD, SHUT_RDWR);
  close(SocketFD);
  return 0;
}