#include <iostream>
#include <vector>
#include <string>

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
    void ImprimirTablero();
    void InsertarJugada(char ficha, int x, int y);
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

void Raya::InsertarJugada(char ficha, int x, int y)
{
    tablero[x][y].prop = ficha;
}


int main()
{
    int size = 3;
    int num_jugadores = 2;
    int turno = 0;
    string mensaje = "";
    Raya TresRaya(size);

    //max numero de jugadas = size * size
    for (int i = 0; i < size * size; ++i)
    {
        turno = i % num_jugadores;
        cout << "Turno jugador " << turno << "\n";
        cout << "Ingrese jugada";
        getline( cin, mensaje);
        TresRaya.InsertarJugada(mensaje[0], (int)(mensaje[1])-48, (int)mensaje[2]-48);
        TresRaya.ImprimirTablero();
    }
    return 0;
}

