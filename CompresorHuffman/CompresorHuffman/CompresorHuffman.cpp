
#include <fstream> //Manejo de archivos
#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_map> //HashMap para guardar la frecuencia de los valores
#include <queue> //Incluye la priority queue 
using namespace std;
using namespace filesystem;
using colaHuffman = priority_queue<Arbol, vector<Arbol>, funcionComparar>; //Alias corto para el min-heap
using hashMapFrecuencia = unordered_map<char, int>; //Alias para el hashmap para acomular frecuencias
using hashMapCodigos = unordered_map<char, CodigoDeHuffman>;

char caracterNoUsado = '\0'; //Valor por defecto = caracter null

typedef struct CodigoDeHuffman 
{
    int codigo; //Codigo que se le asigna (unicamente los bits que se aplican)
    int longitud; //En bits
};

typedef struct NodoHuffman
{
    int frecuencias;
    char caracter;
    NodoHuffman* izq;
    NodoHuffman* der;

} *Arbol;

//Define la comparación que utiliza la min-heap
typedef struct funcionComparar {
    bool operator()(Arbol a, Arbol b) {
        return a->frecuencias > b->frecuencias;
    }
};

//Inicializa un nodo con caracter y frecuencias
Arbol crearArbol(int frecuencia, char caracter) {
    Arbol arbol = new NodoHuffman;
    arbol->caracter = caracter;
    arbol->frecuencias = frecuencia;
    arbol->izq = nullptr;
    arbol->der = nullptr;
    return arbol;
}
Arbol raizDe2Nodos(Arbol nodo1, Arbol nodo2) {
    Arbol raiz = crearArbol(nodo1->frecuencias + nodo2->frecuencias, caracterNoUsado);
    raiz->izq = nodo1;
    raiz->der = nodo2;
    return raiz;
}


void borrarArbol(Arbol& raiz) {
    if (raiz != nullptr) {
        borrarArbol(raiz->izq);
        borrarArbol(raiz->der);
        delete(raiz);
    }
}

//Cuenta las frecuencias de todas las 
int contarFrecuencias(hashMapFrecuencia& hashMap, ifstream archivo) {
    int totalCaracteres = 0;
    char c;

    while (archivo.get(c)) { //Lee todos los caracteres y incrementa el contador respectivo para el simbolo.
        if (hashMap.contains(c))
            hashMap[c]++;
        else
            hashMap[c] = 1;
        totalCaracteres++;
    }
    
    return totalCaracteres;
}

colaHuffman generarColaPrioridad(hashMapFrecuencia mapa) {
    colaHuffman minHeap;
    for (const auto& [caracter, frecuencia] : mapa) { //Recorre todos los elementos y los inserta en el minMap
        Arbol nodo = crearArbol(frecuencia, caracter);
        minHeap.push(nodo);
    }
    return minHeap;
}
//Acorta el codigo para hacer pop pues hay que hacer top y luego pop.
inline Arbol popMinHeap(colaHuffman& minHeap) {
    Arbol nodo = minHeap.top();
    minHeap.pop();
    return nodo;
}

Arbol formarArbolHuffman(colaHuffman& minHeap) {
    Arbol nodo1; 
    Arbol nodo2; 
    Arbol raiz; 
    while (minHeap.size()  != 1) {
        if (minHeap.size() < 1)
            return NULL;
        nodo1 = popMinHeap(minHeap);
        nodo2 = popMinHeap(minHeap);
        raiz = raizDe2Nodos(nodo1, nodo2);
        minHeap.push(raiz);
    }
}
void recorridoCodigosHuffman(Arbol arbolHuffman,hashMapCodigos& mapa,
    int codigo, int longitud) {
    if (arbolHuffman->caracter != caracterNoUsado) {
        recorridoCodigosHuffman(arbolHuffman->izq, mapa, codigo << 1, longitud + 1);
        recorridoCodigosHuffman(arbolHuffman->der, mapa, (codigo << 1) + 1, longitud + 1);
    }
    else {
        CodigoDeHuffman codigos;
        codigos.codigo = codigo;
        codigos.longitud = longitud;
        mapa[arbolHuffman->caracter] = codigos;
    } 
}
//Nombre para cambiar la extension, origen para agregarElArchivo en la misma carpeta,
//El hashmap para los codigos
void escribirArchivoComprimido(string nombreArchivo, path origen, hashMapCodigos mapa) {

}


int main()
{

    //Dirección:
    string rutaString = "ArchivoAComprimir.txt";

    path ruta(rutaString);
    path carpetaOrigen = ruta.parent_path();
    string nombre = ruta.stem().string();

    ifstream archivo(ruta);

    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo\n";
    }


    std::cout << "Hello World!\n";
}


