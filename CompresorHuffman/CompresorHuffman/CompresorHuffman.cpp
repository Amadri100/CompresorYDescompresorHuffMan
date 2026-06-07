
#include <fstream> //Manejo de archivos
#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_map> //HashMap para guardar la frecuencia de los valores
#include <queue> //Incluye la priority queue 
using namespace std;
using namespace filesystem;


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

//=========================
//Aliases
//=========================

using colaHuffman = priority_queue<Arbol, vector<Arbol>, funcionComparar>; //Alias corto para el min-heap
using hashMapFrecuencia = unordered_map<char, int>; //Alias para el hashmap para acomular frecuencias
using hashMapCodigos = unordered_map<char, CodigoDeHuffman>;
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
int contarFrecuencias(hashMapFrecuencia& hashMap, ifstream& archivo) {
    int totalCaracteres = 0;
    char c;

    while (archivo.get(c)) { //Lee todos los caracteres y incrementa el contador respectivo para el simbolo.
        if (hashMap.contains(c))
            hashMap[c]++;
        else
            hashMap[c] = 1;
        totalCaracteres++;
    }
    archivo.clear();
    archivo.seekg(0); //regresa al inicio del archivo
    return totalCaracteres;
}

colaHuffman generarColaPrioridad(hashMapFrecuencia mapa) {
    colaHuffman minHeap;
    cout << mapa.size() << "\nA\n";
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
    Arbol nodo1 = nullptr;
    Arbol nodo2 = nullptr;
    Arbol raiz = nullptr; 
    if (minHeap.empty()) 
        return nullptr;

    while (minHeap.size() > 1) {
        Arbol nodo1 = popMinHeap(minHeap);
        Arbol nodo2 = popMinHeap(minHeap);
        minHeap.push(raizDe2Nodos(nodo1, nodo2));
    }

    return popMinHeap(minHeap);
}
void recorridoCodigosHuffman(Arbol arbolHuffman,hashMapCodigos& mapa,
    int codigo, int longitud) {
    if (arbolHuffman->caracter == caracterNoUsado) {
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

bool bitPrendido(int numero, int posicionBit) {
    int uno = 1;
    uno <<= posicionBit;
    return (numero & uno) == uno; 
}

void agregarNodoSegunCodigo(Arbol& arbol, int cantidadBits, int codigo, char caracter) {
    if (arbol != nullptr) {
        if (cantidadBits == 0) {
            arbol->caracter = caracter;
            return;
        }
        if (bitPrendido(codigo, cantidadBits - 1)) { //Codigo = 1 va la derecha
            //CantidadBits -1 = posicion del primer bit del codigo
            agregarNodoSegunCodigo(arbol->der, cantidadBits - 1, codigo, caracter);
        }
        else { // 0: izquierda
            agregarNodoSegunCodigo(arbol->izq, cantidadBits - 1, codigo, caracter);
        }
    }
    else {
        if (cantidadBits != 0) { //falta recorrido
            Arbol nodo = crearArbol(-1, caracterNoUsado);
            arbol = nodo;
            agregarNodoSegunCodigo(arbol, cantidadBits, codigo, caracter);
        }
        else { //Agrega el nodo final
            Arbol nodo = crearArbol(-1, caracter);
            arbol = nodo;
        }
    }
}
//fix para descomprimir
bool moverCodigoArbol(Arbol& raiz, int bit) {
    if (raiz == nullptr) return false;

    switch (bit) {
    case 1: raiz = raiz->der; break;
    case 0: raiz = raiz->izq; break;
    }

    if (raiz == nullptr) return false;
    return raiz->der == nullptr && raiz->izq == nullptr;
}

void convertirArchivoAVector(ifstream& archivo, vector<unsigned char>& datos) {
    char c;
    while (archivo.get(c)) {
        datos.push_back(c);
    }
    archivo.clear();
    archivo.seekg(0);; //Vuelve al inicio
}

//Nombre para cambiar la extension, origen para agregarElArchivo en la misma carpeta,
//El hashmap para los codigos
void escribirArchivoComprimido(string nombreArchivo, path origen, hashMapCodigos mapa, vector<unsigned char>& datos) {
    int cantidadDeCaracteres = datos.size();
    cout << "Datos a comprimir: " << cantidadDeCaracteres << endl;
    string binario = "";
    for (auto b : datos) {
        CodigoDeHuffman cod = mapa[(char)b];
        // Extraer bit por bit de izquierda a derecha segun la longitud
        for (int i = cod.longitud - 1; i >= 0; i--) {
            binario += ((cod.codigo >> i) & 1) ? '1' : '0'; // Operacion & para aislar cada bit
        }
    }

    int padding = (8 - (binario.size() & 7)) & 7; 
    binario.append(padding, '0');

    // Abrir archivo de salida .cmp en la misma carpeta de origen
    path rutaSalida = origen / (nombreArchivo + ".cmp");
    ofstream out(rutaSalida, ios::binary);
    if (!out.is_open()) {
        cout << "No se pudo crear el archivo comprimido\n";
        return;
    }



    int tam = (int)mapa.size();
    out.write(reinterpret_cast<char*>(&tam), sizeof(int)); //Cantidad de caracteres unicos
    out.write(reinterpret_cast<char*>(&cantidadDeCaracteres), sizeof(int)); //Total de caracteres a escribir

    //Estructura de los datos de cada caracter en el header: caracter, longitud ,codigo 
    for (auto& [caracter, codigo] : mapa) {
        out.put(caracter);
        out.write(reinterpret_cast<const char*>(&codigo.longitud), sizeof(int));
        out.write(reinterpret_cast<const char*>(&codigo.codigo),   sizeof(int));
        
    }

    // Escribir bits en grupos de 8 usando XOR ^ para construir cada byte
    for (size_t i = 0; i < binario.size(); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8; j++) {
            byte = (byte << 1) | (binario[i + j] == '1' ? 1 : 0);
        }
        out.put(static_cast<char>(byte));
    }

    out.close();
    cout << "Archivo comprimido en: " << rutaSalida.string() << endl;
}

void descomprimirArchivo(string nombreArchivo, path origen, Arbol& arbolRecreado, ifstream& archivo) {
   
    int caracteresIndividuales;
    int caracteresTotales;
    archivo.read(reinterpret_cast<char*>(&caracteresIndividuales), sizeof(int));
    archivo.read(reinterpret_cast<char*>(&caracteresTotales), sizeof(int));

    cout << "Simbolos unicos: " << caracteresIndividuales << endl;
    cout << "Simbolos total: " << caracteresTotales << endl;

    while (caracteresIndividuales != 0) {
        char caracter;
        int longitud;
        int codigo;
        archivo.read(reinterpret_cast<char*>(&caracter), sizeof(char));
        archivo.read(reinterpret_cast<char*>(&longitud), sizeof(int));
        archivo.read(reinterpret_cast<char*>(&codigo), sizeof(int));
        cout << "Caracter: [" << caracter << "] longitud: " << longitud << " codigo: " << codigo << endl;
        agregarNodoSegunCodigo(arbolRecreado, longitud, codigo, caracter);
        caracteresIndividuales--;
    }

    // Abrir archivo de salida .cmp en la misma carpeta de origen
    path rutaSalida = origen / (nombreArchivo + ".txt");
    ofstream out(rutaSalida, ios::binary);
    if (!out.is_open()) {
        cout << "No se pudo crear el archivo comprimido\n";
        return;
    }

    unsigned char caracterActual;
    Arbol copia = arbolRecreado;
    while (archivo.read(reinterpret_cast<char*>(&caracterActual), 1) && caracteresTotales != 0) {
        for (int i = 0; i < 8; i++) {
            if (caracteresTotales != 0) {
                if (bitPrendido(caracterActual << i, 7)) {
                    if (moverCodigoArbol(copia, 1)) {
                        out.put(copia->caracter);
                        copia = arbolRecreado;
                        caracteresTotales--;
                    }
                }  
                else {
                    if (moverCodigoArbol(copia, 0)) {
                        out.put(copia->caracter);
                        copia = arbolRecreado;
                        caracteresTotales--;
                    }
                }   
            }
        }
    }
    out.close();
    cout << "Archivo descomprimido en: " << rutaSalida.string() << endl;
}

int main()
{

  
    //Dirección:
    bool comprimir = false; //true: comprime false descomprime
    string rutaString = "archivo.cmp";
    
    path ruta(rutaString);
    path carpetaOrigen = ruta.parent_path();
    string nombre = ruta.stem().string();

    ifstream archivo(ruta, ios::binary);
    
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo\n";
        return 1;
    }

    if (comprimir) {
        hashMapFrecuencia frecuencias;
        contarFrecuencias(frecuencias, archivo);
        colaHuffman colaPrioridad = generarColaPrioridad(frecuencias);
        Arbol arbol = formarArbolHuffman(colaPrioridad);
        if (arbol == nullptr) { //Si no se puede formar un arbol da error.
            exit(1);
        }
        hashMapCodigos codigos;
        recorridoCodigosHuffman(arbol, codigos, 0, 0);
        vector<unsigned char> datos;
        convertirArchivoAVector(archivo, datos);
        escribirArchivoComprimido(nombre, carpetaOrigen, codigos, datos);
        delete arbol;
        archivo.close();
    }
    else {
        Arbol arbol = nullptr;
        descomprimirArchivo(nombre, carpetaOrigen, arbol, archivo);
        delete arbol;
    }

    
}


