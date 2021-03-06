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
    //bool isPlayed = false;
    char prop = '*';
};

class Raya{
private:
    vector<vector<Casilla>> tablero;
public:
    int size;
    int num_jugada = 0; //En que jugada vamos?
    int max_jugadas = size * size; 
    void ImprimirTablero();
    bool InsertarJugada(char ficha, int x, int y);
    bool IsWin(int y, int x, char ficha);
    int VerificarEstadoJuego(int x, int y, char ficha);
    void ReiniciarTablero();
    Raya(int size){
        this->size = size;
        tablero.resize(size, vector<Casilla>(size));
    }
    ~Raya(){};
};

void Raya::ImprimirTablero(){
    int x = 0;
    string col = "  ";
    for (int y = 0; y < size; ++y)
        col+= to_string(y);
    cout << col << "\n";
    for (auto i = tablero.begin(); i != tablero.end(); ++i){
        cout << x << " ";
        x++;
        for( auto j = i->begin(); j != i->end(); ++j)           
            cout << j->prop ;          
        cout << "\n";
    }        
}

bool Raya::InsertarJugada(char ficha, int x, int y){
    if ((int)tablero[x][y].prop == (int)'*'){
        tablero[x][y].prop = ficha;
        return true;
    }
    else{
        return false;
    }
}

bool Raya::IsWin(int y,int x,char ficha){
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

int Raya::VerificarEstadoJuego(int x, int y, char ficha){
    if (IsWin(y, x, ficha)){
        return 1; //Hay ganador
    }
    else{
        if (num_jugada >= max_jugadas){
            //Hay empate
            return -1;
        }
        else{   //Juego en marcha
            num_jugada++;
            return 0;
        }       
    }
}

void Raya::ReiniciarTablero(){
    tablero.assign(size, vector<Casilla>(size));
    num_jugada = 0;
}


int size_juego = 3;
Raya TresRaya(size_juego);

void RecepcionMensaje(int SocketFD){
  int n;
  char buffer[256];
  int longitud = 0;
  for(;;){
    //Lectura Comando
    n = read(SocketFD,buffer,1);
    if (n < 0) perror("ERROR reading from socket");
    if (n == 1)
    {
      switch (buffer[0]) //LetraComando
      {
      case 'L':
        cout << "Perdiste";
        TresRaya.ReiniciarTablero();
        break;
      case 'W':
        cout << "Ganaste";
        TresRaya.ReiniciarTablero();
      case '=':
        cout << "Empate";
        TresRaya.ReiniciarTablero();
      case 'T':{
        n = read(SocketFD,buffer,1);
        if (n < 0) perror("ERROR reading from socket");
        cout << "Es tu turno " << buffer[0];
      }
      case 'A': //Verificar si es ficha ajena
      { //Se supone por ahora que nos llega siempre de forma correcta
        n = read(SocketFD,buffer,3); 
        //[0] es la ficha ajena a insertar
        //[1] [2] Estas son x y y por ahora de una cifra!!!
        if (n < 0) perror("ERROR reading from socket");
        
        TresRaya.InsertarJugada(buffer[0],(int)buffer[1]-48,(int)buffer[2]-48); //TODO
        TresRaya.ImprimirTablero();
      }
      }
      
    }
      //longitud = stoi(buffer);

      n = read(SocketFD,buffer,longitud);
      cout << buffer << "\n";
      bzero(buffer,256);
  }
}

string PadZeros(int number, int longitud)
{
  string num_letra = to_string(number);
  for (int i = 0; i < longitud - num_letra.length()+1; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}

string ProtocoloMensaje(string mensaje)
{
  //Encontrar primer espacio en blanco
  size_t blank = mensaje.find(" ");
  string nick = "";
  if (blank != string::npos)
  {
    //Encontro blank supuesto nickname
    nick = mensaje.substr(0,blank);
    nick = PadZeros(blank +1, 2) + nick;
  }
  else
  {
    //Es supuesto comando (incluir fin de cadena)
    return PadZeros(mensaje.length() + 1,2) + mensaje + '\0';  
  }
  //Generar nick + mensaje
  string msgtoChat = mensaje.substr(blank+1);
  msgtoChat = PadZeros(msgtoChat.length() +1, 3) + msgtoChat;

  return nick+ "+" + msgtoChat + '\0';  

}

void EnvioMensaje(int SocketFD){
  string msgToChat;
  char buffer[256];
  int n;
  for(;;){ 
    cin.clear(); 
    cout << "\nIngrese jugada: ";
    getline(cin, msgToChat);
    //Estoy considerando que client escribe ejm "O21" (ficha+x+y)
    if(TresRaya.InsertarJugada(msgToChat[0],(int)msgToChat[1]-48,(int)msgToChat[2]-48)==false)
      continue;
    n = write(SocketFD, msgToChat.c_str(), msgToChat.length());
    cout<<"Se envia al server "<<msgToChat<<endl;
    TresRaya.ImprimirTablero();
    
    bzero(buffer, 256);    
  }
}

int main(void){
  struct sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  string msgFromChat;
  int n;

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

  //Thread de Envio y Recepcion de Mensajes
  std::thread(EnvioMensaje, SocketFD).detach();
  std::thread(RecepcionMensaje, SocketFD).detach();
  //Mantener con vida los threads
  for(;;){}

  shutdown(SocketFD, SHUT_RDWR);
  close(SocketFD);
  return 0;
}