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

struct Casilla
{
    //bool isPlayed = false;
    char prop = '*';
};

class Raya
{
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
    Raya(int size)
    {
        this->size = size;
        tablero.resize(size, vector<Casilla>(size));
    }
    ~Raya(){};
};

void Raya::ImprimirTablero()
{
    int x = 0;
    string col = "  ";
    for (int y = 0; y < size; ++y)
        col+= to_string(y);
    cout << col << "\n";
    for (auto i = tablero.begin(); i != tablero.end(); ++i)
    {
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

//////////////////////////////////////////////////////////////////////////
std::vector<pair<char, int>> LISTA_CLIENTES;
//Un mapa inverso, otra opcion es un Bimap
std::map<int,char> R_LISTA;

// Variables del Juego
int TURNO = 0;
int size_juego = 3;
int num_jugadores = 2;
Raya TresRaya(size_juego);
///////////

string PadZeros(int number, int longitud){
  string num_letra = to_string(number);
  for (int i = 0; i < longitud - num_letra.length()+1; ++i)
    num_letra = "0" + num_letra;
  return num_letra;
}

void IniciarJuego(){
  TresRaya.ReiniciarTablero();
  TURNO = 0;

}

void EnviarMensaje(int socket_client, string mensaje){
  //mensaje = PadZeros(mensaje.length() + 1, 3) + mensaje + "\0";
  int n = write(socket_client, mensaje.c_str(), mensaje.length());
  if (n < 0) perror("ERROR writing to socket");
}

int VerificarJugador(char comando, int socket_client){
  if (comando == 'C') //Es un mensaje de Chat
    return 1;
  else{
    if ( LISTA_CLIENTES[TURNO].second == socket_client)  
      return 0; //Le corresponde jugar
    else
      return -1; //No le corresponde  
  }
}

void BroadCast(string mensaje, int excepcion = -1){
  for (auto &i : LISTA_CLIENTES){
    if (i.second != excepcion){
      EnviarMensaje(i.second, mensaje);
    }
  }
}

void SiguienteTurno(){
  TURNO = TresRaya.num_jugada % num_jugadores;
  int socket_client = LISTA_CLIENTES[TURNO].second;
  EnviarMensaje(socket_client, "T"+ LISTA_CLIENTES[TURNO].first);
}

void VerificarEstado(int socket_client, char ficha,  int x, int y){
  switch (TresRaya.VerificarEstadoJuego(x, y, ficha)){
  case 0: //Juego en marcha
    SiguienteTurno(); 
    break;
  
  case 1: //Hay ganador
    EnviarMensaje(socket_client, "W"); //A ganador
    BroadCast("L", socket_client); //A resto de jugadores
    //TresRaya.ReiniciarTablero();
    IniciarJuego();
    break;

  case 2: //Hay empate
    BroadCast("="); //A todos jugadores
    //TresRaya.ReiniciarTablero();
    IniciarJuego();
    break;
  }
}

void Process_Client_Thread(int socket_client){
  string msgToChat;
  char buffer[256];
  int n;
  char comando;
  int longitud = 0;
  //int id_jugador;
  do{
      if (LISTA_CLIENTES.size() < 2)
        continue;

      //Lectura comando
      n = read(socket_client,buffer,1);     

      if (n < 0)
        perror("ERROR reading from socket");
      else if (n == 1){ 
        comando = buffer[0];

        switch (VerificarJugador(comando, socket_client)){
        case 0: //Es turno de jugador
        {
          
          n = read(socket_client, buffer, 2);
          cout<<"Caso 0"<<endl;
          char x = buffer[0];
          char y = buffer[1];
          //En este caso comando = ficha
          
          if (TresRaya.InsertarJugada( comando, (int)x-48, (int)y-48)){
            //Jugada Legal
            string jugada ="A";
            jugada+=comando;
            jugada+=x;jugada+=y;
            cout<<"jugada: "<<jugada<<endl;
            BroadCast(jugada, socket_client);
            VerificarEstado(socket_client, comando, (int)x-48 ,(int)y-48 );
          }
          else{
            //JugadaIlegal Mensaje de Error
            EnviarMensaje(socket_client, "C08Invalida");          
          }       
          bzero(buffer,256);
          break;
        }
        case 1: //Es un mensaje de chat
          cout<<"Caso 1"<<endl;
          BroadCast("Mensaje", socket_client); //TODO
          
          break;

        case -1:
        cout<<"Caso -1"<<endl;
          EnviarMensaje(socket_client, "C08Invalida"); //Mensaje de Error
          
          break;
        }
      }

    } while (true);

    shutdown(socket_client, SHUT_RDWR);
    close(socket_client); 
}




int main(void)
{
  struct sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  
  if(-1 == SocketFD)
  {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1100);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
  {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if(-1 == listen(SocketFD, 10))
  {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  //LISTA_CLIENTES.insert(std::pair<string,int>("lista",9999));

  int last_client = -1;
  for(;;)
  {
    int ClientSD = accept(SocketFD, NULL, NULL);

    if(0 > ClientSD)
    {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else
    {
      if (ClientSD != last_client && LISTA_CLIENTES.size() == 0)
      {
        LISTA_CLIENTES.push_back(std::pair<char,int>('O',(int)ClientSD));
        last_client = ClientSD;
      }
      else if (ClientSD != last_client && LISTA_CLIENTES.size() == 1)
      {
        LISTA_CLIENTES.push_back(std::pair<char,int>('X',(int)ClientSD));
        last_client = ClientSD;
      }     
    }
    
    

    //Verificar al menos dos jugadores

    std::thread(Process_Client_Thread, ClientSD).detach();

    if (TresRaya.num_jugada == 0)
    {
      IniciarJuego();
    }

  } 
  
  close(SocketFD);
  return 0;
}