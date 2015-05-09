# Avaliação Continuada - Batalha Naval

## Especificação das necessidades de alteração no game:

### O jogo deverá conter:

* 5 navios de porta-avioes;
* 3 submarinos;
* 8 navios torpedeiros;
* 2 navios couraçados.

### Área ocupada e pontuação de cada equipamento de guerra:

* 5 navios de porta-avioes – 3 coordenadas consecutivas – 20 pontos;
* 3 submarinos – 3 coordenadas consecutivas – 35 pontos;
* 10 navios torpedeiros – 2 coordenadas consecutivas – 15 pontos;
* 3 navios couraçados – 5 coordenadas consecutivas – 50 pontos.

### Disposição dos equipamentos de guerra:

* As posições deverão ser escolhidas aleatoriamente pelo servidor (verificar se o equipamento poderá ser alocado a partir da primeira coordenada);
* O tabuleiro deverá ser uma matriz quadrada de ordem 100;
* Os equipamentos poderão estar distribuídos através de coordenadas da esquerda  para a direita e vice-versa; de cima para baixo e vice-versa.

### Especificações técnicas do jogo:

* Suporta até 50 jogadores simultaneamente e um mínimo de dois;
* Permite que cada jogador possa disparar até 20 tiros;
* Cada jogador deve ser executado em um computador independente;
* No equipamento que está executando o servidor nenhum jogador poderá jogar (a não ser em teste);
* A cada alvo abatido deverá ser somada e informada a pontuação ao jogador;
* Cada jogador deverá ser identificado e ser informado quantos disparos foram dados, qual a pontuação e quais equipamentos foram abatidos;
* A comunicação entre as aplicações que representam os jogadores e o servidor deverá ser realizada através do mecanismo de sockets, podendo ser escrito na linguagem Java ou **C**.

### Desenvolvimento do exercício:

* Equipe de 3 alunos;
* Data de testes: 22 e 29/05/2015 após as 20hs;
* Data de entrega: **29/05/2015 até às 20hs**, no laboratório;

## Pontuação:
Teste: será executado através de um robô cliente de teste que irá interagir com o módulo servidor para testar as funcionalidades solicitadas nos itens 1, 2, 3 e 4.

* Código funcionando entre 90 e 100% das funcionalidades solicitadas: 4 pontos;
* Código funcionando entre 60% e 90% das funcionalidades solicitadas: 3 pontos;
* Código funcionando com menos de  60% das funcionalidades solicitadas: 1,5 pontos;
* CÓPIA DE CÓDIGO ou não entrega ou não comparecimento: nenhuma pontuação.

## Referências:
Especificação e exemplos de código de um jogo de batalha naval para sistemas monousuários e não-distribuído:

* [Batalha Naval em Java](http://www.javaprogressivo.net/2012/09/jogo-batalha-naval-em-java.html)
* [Construindo jogo de batalha naval em matriz 10x15. Navios se cruzando.](http://www.guj.com.br/27593-construindo-jogo-de-batalha-naval-em-matriz-10x15-navios-se-cruzando)
* [Exemplo de jogo](http://www.guj.com.br/java/297085-batalha-naval---fiz-meu-jogo)

Exemplo de códigos em Java de sockets single thread e multi thread:

* [Conexão de multiplos sockets em um servidor](http://www.guj.com.br/23004-conexao-de-multiplos-sockets-em-um-servidor)
* [Java Socket: Entendendo a classe Socket e a ServerSocket em detalhes](http://www.devmedia.com.br/java-socket-entendendo-a-classe-socket-e-a-serversocket-em-detalhes/31894)
