/* Programação de Alto Desempenho - 2020/2
    Beatriz de Camargo Castex Ferreira - 10728077
   Trabalho 2 - Paralelização da Aglomeração de Grafos
   Programa que calcula o coeficiênte de aglomeração de um grafo de forma
   paralela usando listas de adjacência e threads.

   Escolhi usar threads ao invés do openMP mais por curiosidade mesmo. Não
   tinha entendido direito então achei que usar no trabalho iria me ajudar.

   A paralelização reduziu o tempo de execução aproximadamente na metade,
   dependendo do tipo de grafo. Para o tipo ba não foi tão efetiva, infelizmente.

 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>

namespace chrono = std::chrono;

// Criamos o vetor onde ficarão os coeficientes
std::vector<double> coeficientes(10);

/* ------------- FUNÇÕES PARA CALCULAR COEFICIENTE  -----------*/


// Calcula quantos nódulos tem num grafo dada a lista de arestas e o tamanho da lista.
int graph_size(int tamanho, std::vector<std::vector<int> > entrada){
        int N = 0;
        for (int i = 0; i < tamanho; i++) {
                if ( entrada[i][0] > N ) { N = entrada[i][0];}
                if ( entrada[i][1] > N ) { N = entrada[i][1];}
        }
        return N;
}

// Monta uma lista de adjacencias dada uma lista de arestas, seu tamanho e o número de nódulos.
std::vector<std::vector<int> > build_adj_list(int tamanho, int num_nodes, std::vector<std::vector<int> > entrada){
        std::vector<std::vector<int> > l_adjacencia(num_nodes+1);

        for (int i = 0; i < tamanho; i++) {
                /* Eu sei que fazer push back não é necessário quando se sabe o tamanho mas n sei
                   como posso descobrir o número de vizinhos para ter uma tamnho pré definido para não
                   ter que fazer push back */
                l_adjacencia[entrada[i][0]].push_back(entrada[i][1]);
                l_adjacencia[entrada[i][1]].push_back(entrada[i][0]);
        }
        return l_adjacencia;
}


// Calcula uma lista de coeficientes de aglomeração dado o número de nódulos no grafo e uma lista de adjacencias
//void agl_coef(std::vector<double> coeficientes, std::vector<std::vector<int> > l_adjacencia, int start_node, int end_node){
void agl_coef(std::vector<std::vector<int> > l_adjacencia, int start_node, int end_node){

        // Escolher o nódulo
        int node = 0;
        for (int c = start_node; c <= end_node; c++) {
                node = c;

                // Ver quantos vizinhos o nódulo tem e quem são os vizinhos
                // Nesse caso os vizinhos já estão listados na lista de adjacência então só precisamos do grau
                int grau = l_adjacencia[node].size();
                double coef_aglomeracao;
                if (grau<2) {
                        coef_aglomeracao = 0;
                } else {
                        // usndo uma referência para a lista de vizinhos
                        std::vector<int> &vizinhos = l_adjacencia[node];

                        /* Testar os pares ignorando o número que veio antes e ele mesmo
                           por exemplo em um grafo de 3 nódulos testar 0 com 1,2,3
                           depois testar 1 com 2,3 e depois 2 com 3. (se possível fazer isso só com a lista
                           de vizinhos - ela tem q estar organizada e tem que arranjar um jeito de não
                           considerar o ítem que foi testado antes e ele mesmo)*/

                        double e_ligados = 0;
                        for (int i = 0; i < grau-1; i++) {
                                int start = i+1; // Começamos sempre no próximo para não testar com ele mesmo ou os anteriores.
                                for (int j = start; j < grau; j++) {
                                        if (std::find(l_adjacencia[vizinhos[i]].begin(), l_adjacencia[vizinhos[i]].end(), vizinhos[j]) != l_adjacencia[vizinhos[i]].end()) { //  Verifica se existe o número dentro da vizinhança
                                                e_ligados += 1;
                                        }
                                }
                        }

                        coef_aglomeracao = (2*e_ligados)/(grau*(grau-1));
                }
                coeficientes[node] = coef_aglomeracao;
        }

}

// Cria os threads e separa os nódulos para terem suas listas de adjaência calculadas por cada thread
void parallel_agl_coef(int num_nodes, std::vector<std::vector<int> > l_adjacencia, int num_threads){
        coeficientes.resize(num_nodes+1);
        // Criamos um vetor com todos os threads de que iremos precisar
        std::vector<std::thread> threads;
        // Dividimos o grafo para que cada thread trabalhe numa parte igual de nódulos
        int nodes_per_spread = (num_nodes+1) / num_threads;
        // Definimos o começo e o fim da lista de nódulos que cada thread vai usar.
        int end_node = nodes_per_spread-1;
        int start_node = 0;
        // Criamos cada thread e chamamos a função que calcula os coeficientes angulares
        for (int i = 0; i < num_threads; i++) {

                if (i == (num_threads-1)) {
                        if (((num_nodes+1)%num_threads) != 0) {
                                end_node += (num_nodes+1)%num_threads;
                        }
                }

                threads.emplace_back(agl_coef, l_adjacencia, start_node, end_node);
                start_node += nodes_per_spread;
                end_node += nodes_per_spread;

        }
        // Juntamos os threads
        for(auto& t : threads) {
                t.join();
        }
}



int main(int, char *argv[]) {

        /* ------------- ARQUIVO DE ENTRADA -----------*/

        // Lendo o arquivo
        std::ifstream arquivo(argv[1]);
        if (!arquivo.is_open()) {
                std::cerr << "Error opening file " << argv[1] << std::endl;
                return 2;
        }
        std::vector<std::vector<int> > entrada;
        int x;
        int y;
        // Existem entradas a serem lidas
        while(!arquivo.eof()) {
                std::vector<int> aux;
                // Lemos os dados de uma linha
                // Precisamos que a primeira entrada seja válida (não vazia)
                if (arquivo.good()) {

                        arquivo >> x;
                        arquivo >> y;

                        aux.push_back(x);
                        aux.push_back(y);
                }
                entrada.push_back(aux);
        }
        // todas as entradas foram lidas
        int tamanho = entrada.size() - 1;

        /* ------------- COEF. DE AGLOMERAÇÃO -----------*/


        // Descobrindo o tamanho do grafo:
        int num_nodes = graph_size(tamanho, entrada);

        // Montando lista de adjacencias:
        std::vector<std::vector<int> > l_adjacencia = build_adj_list(tamanho, num_nodes, entrada);

        // Iniciando o cronômetro
        auto t1 = chrono::high_resolution_clock::now();

        /* Eu escolhi 6 threads porque no meu computador (mac late 2010, com i7 2.8 GHz)
           foi o mais rápido para arquivos ws n 20000 k 1000 (2.6 s para 4 threads,
           2.7s  para 3, 2.8s para 5, 2.5s para 6 e 2,7 para 8 threads ).
           Foi mais rápido com números pares de threads tbm porque o
           número de nódulos é par. Mais do que 6 nódulos o tempo de execução voltou a aumentar.
           Acho que depende muito também do tipo de grafo. Para grafos ba 4 threads foi melhor*/
        parallel_agl_coef(num_nodes, l_adjacencia, 6);

        // Parando o cronômetro e calculando diferença
        auto t2 = chrono::high_resolution_clock::now();
        auto dt = chrono::duration_cast<chrono::microseconds>(t2 - t1);

        // Escreve resultados do cronômetro.
        std::cout << "Time passed: " << dt.count() / 1e6 << std::endl;

        /* ------------- ARQUIVO DE SAÍDA -----------*/

        // Editamos o nome do arquivo
        std::string nome_arquivo;
        nome_arquivo.append(argv[1]);
        std::string apagar = ".edgelist";
        std::string::size_type it = nome_arquivo.find(apagar);
        if (it != std::string::npos) {
                nome_arquivo.erase(it, apagar.length());
                std::string ext = ".cls";
                nome_arquivo += ext;
        }

        // Montar arquivo:
        std::ofstream saida;
        saida.open(nome_arquivo);
        saida.precision(6);
        int t = coeficientes.size();
        for (int i = 0; i < t; i++) {
                saida << std::fixed << coeficientes[i] << '\n';
        }

        return 0;
}
