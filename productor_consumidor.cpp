#include <vector>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <cstring>
#include <signal.h>
#include <chrono>

using namespace std;

class ProductorConsumidor {
    vector<int> cola;
    int in;
    int out;
    int actual, capacidad;
    FILE* log_file;
public:
    ProductorConsumidor(int capa, FILE* log)
    : cola(capa), in(0), out(0), actual(0), capacidad(capa), log_file(log) {}

    void duplicar() {
        vector<int> new_cola(capacidad*2);
        for (int i = 0; i < actual; ++i) {
            new_cola[i] = cola[(out + i) % capacidad];
        }
        cola = move(new_cola);
        in = actual;
        out = 0;
        capacidad = capacidad*2;
        fprintf(log_file, "Se duplicó el tamaño de la cola a: %d\n", capacidad);
    }

    void reducir() {
        int new_capacidad = capacidad / 2;
        if (new_capacidad < 1) return;
        vector<int> new_cola(new_capacidad);

        for (int i = 0; i < actual; ++i) {
            new_cola[i] = cola[(out + i) % capacidad];
        }
        cola = move(new_cola);
        in = actual;
        out = 0;
        capacidad = new_capacidad;

        fprintf(log_file, "Se redujo el tamaño de la cola a: %d\n", capacidad);
    }

    void enqueue(int item) {
        if (actual == capacidad) {
            duplicar();
        }
        cola[in] = item;
        in = (in + 1) % capacidad;
        actual++;

        fprintf(log_file, "Productor agregó el elemento: %d\n", item);
    }

    int dequeue() {
        if (actual == 0) {
            throw runtime_error("La cola está vacía");
        }

        int item = cola[out];
        out = (out + 1) % capacidad;
        actual--;
        if (actual > 0 && actual <= capacidad / 4) {
            reducir();
        }

        fprintf(log_file, "Consumidor extrajo el elemento: %d\n", item);

        return item;
    }

    bool isEmpty() {
        return actual == 0;
    }
};

class Monitor {
    ProductorConsumidor cola;
    int num_productores;
    int num_consumidores;
    int tiempo_espera;
    pthread_mutex_t mtx;
    pthread_cond_t cv_productores;
    pthread_cond_t cv_consumidores;
    int productores_restantes; 
    bool todos_productores_terminaron; //indicador todos los productores han terminado
    FILE* log_file; 




public:
    Monitor(int capa, int p, int c, int t, FILE* log) 
    : cola(capa, log), num_productores(p), num_consumidores(c), tiempo_espera(t), log_file(log),  //pasar log a cola, cola atiende log
      productores_restantes(p), todos_productores_terminaron(false) {
        pthread_mutex_init(&mtx, nullptr);
        pthread_cond_init(&cv_productores, nullptr);
        pthread_cond_init(&cv_consumidores, nullptr);
    }

    void productor(int id) {
        pthread_mutex_lock(&mtx);

        cola.enqueue(id);
        cout << "Productor " << id << " agregó el elemento " << id << endl;

    
        productores_restantes--;

      
        if (productores_restantes == 0) {
            todos_productores_terminaron = true;
            pthread_cond_broadcast(&cv_consumidores);  // despierta a consumidores
        }

        
        while (!todos_productores_terminaron) {
            pthread_cond_wait(&cv_productores, &mtx);  
        }

        pthread_mutex_unlock(&mtx);
    }

    void consumidor(int id) {

        pthread_mutex_lock(&mtx);  

        // espera por productores, en caso de llegar antes que un productor, se queda esperando.
        while (!todos_productores_terminaron) {
            cout << "Consumidor " << id << " esperando" << endl;
            pthread_cond_wait(&cv_consumidores, &mtx); 
        }

        try {
            int item = cola.dequeue();
            cout << "Consumidor " << id << " consumió el elemento " << item << endl;

        } catch (const runtime_error& e) {
            
            cout << "Consumidor " << id << " espera: " << e.what() << endl;

            auto start_time = std::chrono::steady_clock::now();
            //verificar constantemente
            while (true) {
                // tiempo
                auto end_time = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed_seconds = end_time - start_time;

                //salir. tiempo superado
                if (elapsed_seconds.count() >= tiempo_espera) {
                    
                    break; 
                }
            }
            cout << "Consumidor " << id << " cancelando por no haber nada en cola" << endl;

            pthread_mutex_unlock(&mtx);
            pthread_cancel(pthread_self()); 

            return;  
        }
        pthread_cond_broadcast(&cv_productores);
        pthread_mutex_unlock(&mtx);
    }


    void run() {
        vector<pthread_t> hilos_productores(num_productores);
        vector<pthread_t> hilos_consumidores(num_consumidores);

        //crear threads
        for (int i = 0; i < num_productores; ++i) {
            pthread_create(&hilos_productores[i], nullptr, 
                           (void* (*)(void*)) &Monitor::productor, this);
        }
        for (int i = 0; i < num_consumidores; ++i) {
            pthread_create(&hilos_consumidores[i], nullptr, 
                           (void* (*)(void*)) &Monitor::consumidor, this);
        }

        //espera
        for (int i = 0; i < num_productores; ++i) {
            pthread_join(hilos_productores[i], nullptr);
        }
        for (int i = 0; i < num_consumidores; ++i) {
            pthread_join(hilos_consumidores[i], nullptr);
        }
    }
};




int main(int argc, char *argv[]) {
    FILE* log_file = fopen("log.txt", "w");
    if (!log_file) {
        cerr << "No se pudo abrir el archivo de log" << endl;
        return 1;
    }

    // por defecto
    int num_productores = 10, num_consumidores = 5, tamaño_inicial = 50, tiempo_espera = 1;
    //parseo
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) num_productores = atoi(argv[++i]);
        if (strcmp(argv[i], "-c") == 0) num_consumidores = atoi(argv[++i]);
        if (strcmp(argv[i], "-s") == 0) tamaño_inicial = atoi(argv[++i]);
        if (strcmp(argv[i], "-t") == 0) tiempo_espera = atoi(argv[++i]);
    }

    Monitor monitor(tamaño_inicial, num_productores, num_consumidores, tiempo_espera, log_file);
    //ejecuta los hilos.
    monitor.run();


    fclose(log_file);

    return 0;
}
