#include "Dungeon2D.hpp"

Dungeon2D::Dungeon2D(int widthTiles, int heightTiles) 
                : width{widthTiles}, height{heightTiles}, root(nullptr){}
/**
 * @todo  método generate deverá estruturar a topologia da masmorra seguindo
 *  quatro fases lógicas essenciais, baseadas no conceito de Particionamento
 *  Binário do Espaço.
 * 
 *  Primeira fase, o controle de aleatoriedade. 
 *  O método deve utilizar a seed recebida como argumento para alimentar o motor 
 *  gerador de números pseudo-aleatórios da linguagem. O contrato exige que a mesma 
 *  semente resulte obrigatoriamente na mesma estrutura arquitetónica gerada, assegurando
 *  um comportamento estritamente determinístico.
 * 
 *  Segunda fase, a divisão espacial.
 *  O algoritmo instância o nó root e define a sua região abrangendo a totalidade da grelha da masmorra. 
 *  Em seguida, processa um corte na área do nó (aleatoriamente no eixo vertical ou horizontal), gerando dois novos
 *  nós menores inseridos nos ponteiros left e right. A operação é chamada de forma recursiva nestes novos nós,
 *  dividindo a área repetidamente até que os espaços resultantes fiquem demasiado pequenos para novos cortes.   
 * 
 *  Terceira fase, a perfuração de salas.
 *  Quando um nó para de ser cortado, ele torna-se uma folha na árvore (isLeaf retorna verdadeiro).
 *  É no interior da região destas folhas que o algoritmo aloca o retângulo de uma sala (room). 
 *  O contrato estabelece uma limitação física exata: o cálculo das dimensões aleatórias não pode permitir a criação de aposentos 
 *  menores do que 5 por 5 tiles. As salas devem estar contidas inteiramente dentro dos limites da região da sua respectiva folha.
 * 
 *  Quarta fase, a consolidação de rotas.
 *  O contrato exige um único espaço geográfico inteiramente conectado, assegurando que não existam salas inacessíveis. 
 *  O algoritmo deve retroceder na árvore analisando os nós pais para desenhar um corredor unindo as estruturas que pertencem ao seu
 *  nó filho esquerdo com as estruturas que pertencem ao seu nó filho direito. Todo o corredor gerado é instanciado usando o alias Corridor
 *  e armazenado na lista corridors. A regra estipulada exige que o retângulo destes corredores tenha rigorosamente 3 tiles de largura.   
 * 
 */
namespace{
    constexpr int MIN_NODE_SIZE = 7;
    
    void splitNode(std::unique_ptr<BSPNode>& node)
    {
        int w = node->region.right - node->region.left;
        int h = node->region.bottom - node->region.top;

        if(w < MIN_NODE_SIZE * 2 && h < MIN_NODE_SIZE * 2){
            return;
        }

        bool splitHorizontal;
        if(w > h && w >= MIN_NODE_SIZE * 2){
            splitHorizontal = false;
        }else if(h > w && h >= MIN_NODE_SIZE * 2){
            splitHorizontal = true;
        }else{
            splitHorizontal = (std::rand() % 2) == 0;
        }

        node->left  = std::make_unique<BSPNode>();
        node->right = std::make_unique<BSPNode>();

        if(splitHorizontal){
            int max_split = h - MIN_NODE_SIZE;
        }
    }
}

void Dungeon2D::generate(unsigned seed){
    srand(seed);
    
    root = std::make_unique<BSPNode>();
    root->region = Rect{0, 0, width, height};
    
    splitNode(root);
}

