#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <stdexcept>

class PageTable
{
private:
    int numMarcos; // almacena el numero de marcos

public:
    PageTable(int numMarcos) : numMarcos(numMarcos) {}

    bool FIFO(std::list<int> &marcos, std::unordered_map<int, std::list<int>::iterator> &pageMap, int page, int &fallos)
    {
        if (pageMap.find(page) != pageMap.end())
        {
            return false;
        }
        // si los marcos estan llenos se elimina la pag mas antigua
        if (marcos.size() == numMarcos)
        {
            int oldPage = marcos.front();
            marcos.pop_front();
            pageMap.erase(oldPage);
        }
        marcos.push_back(page);
        pageMap[page] = --marcos.end();
        fallos++;
        return true;
    }

    bool LRU(std::list<int> &marcos, std::unordered_map<int, std::list<int>::iterator> &pageMap, int page, int &fallos)
    {
        if (pageMap.find(page) != pageMap.end())
        {
            marcos.erase(pageMap[page]);
            marcos.push_front(page);
            pageMap[page] = marcos.begin();
            return false;
        }
        if (marcos.size() == numMarcos)
        {
            int oldPage = marcos.back();
            marcos.pop_back();
            pageMap.erase(oldPage);
        }
        marcos.push_front(page);
        pageMap[page] = marcos.begin();
        fallos++;
        return true;
    }

    bool RS(std::vector<std::pair<int, bool>> &RSmarcos, std::unordered_map<int, int> &pageMap, int &pointer, int page, int &fallos)
    {
        if (pageMap.find(page) != pageMap.end())
        {
            // encuentra la pagina y actualiza el bit de referencia.
            RSmarcos[pageMap[page]].second = true;
            return false;
        }

        fallos++;

        // reemplaza la pagina
        while (true)
        {
            if (!RSmarcos[pointer].second)
            {
                // elimina la pagina reemplazada del hash.
                pageMap.erase(RSmarcos[pointer].first);

                // reemplaza la pagina en el vector y actualiza el hash.
                RSmarcos[pointer] = {page, true};
                pageMap[page] = pointer;

                pointer = (pointer + 1) % RSmarcos.size();
                return true;
            }
            // limpia el bit de referencia y avanza.
            RSmarcos[pointer].second = false;
            pointer = (pointer + 1) % RSmarcos.size();
        }
    }

    bool OPTIMO(const std::vector<int> &referencia, int currentIndex, int page, int numMarcos,
                std::list<int> &marcos, std::unordered_map<int, std::list<int>::iterator> &pageMap, int &fallos)
    {
        // verifica si la pagina ya esta en la memoria
        if (pageMap.find(page) != pageMap.end())
        {
            return false; // pagina en la memoria, no hay fallo
        }

        fallos++;
        // si los marcos estan llenos, reemplazamos una pagina
        if (marcos.size() == numMarcos)
        {
            int farthest = currentIndex;
            int pageToRemove = -1;

            // busca la pagina que no se usara por mas tiempo
            for (const auto &marco : marcos)
            {
                auto it = std::find(referencia.begin() + currentIndex + 1, referencia.end(), marco); // busca la proxima aparicion (desde la posicion actual hasta el final)

                if (it == referencia.end()) // pagina que no se usara mas
                {
                    pageToRemove = marco;
                    break;
                }

                if (it - referencia.begin() > farthest) // compara la pos de la proxima aparicion con el valor de mas lejano
                {
                    farthest = it - referencia.begin();
                    pageToRemove = marco;
                }
            }
            marcos.erase(pageMap[pageToRemove]);
            pageMap.erase(pageToRemove);
        }
        // inserta la nueva pagina en los marcos
        marcos.push_back(page);
        pageMap[page] = --marcos.end(); // actualiza la posicion de la pagina en la tabla hash

        return true;
    }
};

std::vector<int> archivoReferencia(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        throw std::runtime_error("No se pudo abrir el archivo.");
    }
    std::vector<int> referencia;
    int value;
    while (file >> value)
    {
        referencia.push_back(value);
    }
    return referencia;
}

void printStatus(const std::string &algoritmo, int page, const std::list<int> &marcos, int fallos)
{
    std::cout << "Algoritmo: " << algoritmo << " | Referencia: " << page << " | Memoria: ";
    for (const auto &marco : marcos)
        std::cout << marco << " ";
    std::cout << "| Fallos: " << fallos << "\n";
}

void runFIFO(PageTable &pageTable, const std::vector<int> &referencia, int numMarcos)
{
    std::list<int> marcos;
    std::unordered_map<int, std::list<int>::iterator> pageMap;
    int fallos = 0;

    for (const auto &page : referencia)
    {
        pageTable.FIFO(marcos, pageMap, page, fallos);
        printStatus("FIFO", page, marcos, fallos);
    }
}

void runLRU(PageTable &pageTable, const std::vector<int> &referencia, int numMarcos)
{
    std::list<int> marcos;
    std::unordered_map<int, std::list<int>::iterator> pageMap;
    int fallos = 0;

    for (const auto &page : referencia)
    {
        pageTable.LRU(marcos, pageMap, page, fallos);
        printStatus("LRU", page, marcos, fallos);
    }
}

void runRS(PageTable &pageTable, const std::vector<int> &referencia, int numMarcos)
{
    std::vector<std::pair<int, bool>> RSmarcos(numMarcos, {-1, false});
    std::unordered_map<int, int> pageMap; // tabla hash para rastrear las posiciones de las paginas
    int pointer = 0;
    int fallos = 0;

    for (const auto &page : referencia)
    {
        pageTable.RS(RSmarcos, pageMap, pointer, page, fallos); // llama con el numero correcto de argumentos
        std::cout << "Algoritmo: RS | Referencia: " << page << " | Memoria: ";
        for (const auto &marco : RSmarcos)
            std::cout << marco.first << "(" << marco.second << ") ";
        std::cout << "| Fallos: " << fallos << "\n";
    }
}

void runOPTIMO(PageTable &pageTable, const std::vector<int> &referencia, int numMarcos)
{
    std::list<int> marcos;                                     // lista enlazada para los marcos
    std::unordered_map<int, std::list<int>::iterator> pageMap; // tabla hash para las paginas
    int fallos = 0;

    for (size_t i = 0; i < referencia.size(); ++i)
    {
        pageTable.OPTIMO(referencia, i, referencia[i], numMarcos, marcos, pageMap, fallos);
        printStatus("OPTIMO", referencia[i], marcos, fallos);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        std::cerr << "Uso: ./simulador -m <num_marcos> -a <algoritmo> -f <archivo>\n";
        return 1;
    }

    int numMarcos = std::stoi(argv[2]);
    std::string algoritmo = argv[4];
    std::string filename = argv[6];

    std::vector<int> referencia;
    try
    {
        referencia = archivoReferencia(filename);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    PageTable pageTable(numMarcos);

    if (algoritmo == "All")
    {
        runFIFO(pageTable, referencia, numMarcos);
        runLRU(pageTable, referencia, numMarcos);
        runRS(pageTable, referencia, numMarcos);
        runOPTIMO(pageTable, referencia, numMarcos);
    }
    else if (algoritmo == "FIFO")
    {
        runFIFO(pageTable, referencia, numMarcos);
    }
    else if (algoritmo == "LRU")
    {
        runLRU(pageTable, referencia, numMarcos);
    }
    else if (algoritmo == "RS")
    {
        runRS(pageTable, referencia, numMarcos);
    }
    else if (algoritmo == "OPTIMO")
    {
        runOPTIMO(pageTable, referencia, numMarcos);
    }
    else
    {
        std::cerr << "Algoritmo desconocido: " << algoritmo << "\n";
        return 1;
    }

    return 0;
}
