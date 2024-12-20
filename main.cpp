#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>

using namespace std;

const int NUM_REINOS = 7;
const int NUM_CIDADES_POR_REINO = 20;
const int NUM_URNAS_POR_CIDADE = 5;
const int NUM_URNAS_TOTAL = NUM_REINOS * NUM_CIDADES_POR_REINO * NUM_URNAS_POR_CIDADE;

//Como temos 5 urnas por cidade e cada cidade tem entre 1000 e 30k habitantes, entao cada urna tem entre 200 e 6000 votos
const int MIN_VOTOS = 200;
const int MAX_VOTOS = 6000;
const int MIN_TEMPO = 2;
const int MAX_TEMPO = 120;

sem_t semaforo;
vector<int> votos(NUM_REINOS, 0);
bool votos_encerrados = false;     // indica quando todas as threads de una terminaram

int calcularVotos() {
    return MIN_VOTOS + (rand() % (MAX_VOTOS - MIN_VOTOS + 1));
}   // Simula a contagem de votos entre 6000 e 200 porque cada cidade tem 5 urnas e a cidade tem de 1000 a 30000 habitantes

int calcularTempoDeEspera() {
    return MIN_TEMPO + (rand() % (MAX_TEMPO - MIN_TEMPO + 1));
} // Gera tempo entre 2 e 120 segundos

void* urnaEletronica(void* arg) {
    int reino = *((int*)arg);
    int votos_na_urna = calcularVotos();
    int tempo_de_espera = calcularTempoDeEspera();

    sleep(tempo_de_espera);

    sem_wait(&semaforo);
    votos[reino] += votos_na_urna;
    sem_post(&semaforo);

    pthread_exit(NULL);
}

void* monitorVotos(void* arg) {
    while (!votos_encerrados) {
        sem_wait(&semaforo);
        cout << "\033[2J\033[H"; // Limpa a tela e move o cursor para o topo
        cout << "Contagem em tempo real dos votos:" << "\n";
        for (int i = 0; i < NUM_REINOS; ++i) {
            cout << "Reino " << i + 1 << ": " << votos[i] << " votos" << "\n";
        }
        sem_post(&semaforo);
        sleep(1); // Atualiza a cada 1 segundo
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_URNAS_TOTAL];
    pthread_t monitor_thread;
    int indices[NUM_URNAS_TOTAL];
    sem_init(&semaforo, 0, 1);

    // Cria a thread de monitoramento
    pthread_create(&monitor_thread, NULL, monitorVotos, NULL);

    // Cria as threads das urnas
    for (int i = 0; i < NUM_URNAS_TOTAL; ++i) {
        indices[i] = i % NUM_REINOS;
        pthread_create(&threads[i], NULL, urnaEletronica, (void*)&indices[i]);
    }

    // Aguarda todas as threads de urnas terminarem
    for (int i = 0; i < NUM_URNAS_TOTAL; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Define o encerramento do monitoramento
    votos_encerrados = true;
    pthread_join(monitor_thread, NULL);

    sem_destroy(&semaforo);

    // Exibe a contagem final
    cout << "\n" << "A votação terminou!" << "\n" << "\n";

    int soma_total_de_votos = 0;
    for (int i = 0; i < NUM_REINOS; ++i) {
        soma_total_de_votos += votos[i];
    }

    cout << "Soma total de votos: " << soma_total_de_votos << "\n";

    return 0;
}

// comando pra rodar: "g++ -pthread -o main main.cpp && ./main"