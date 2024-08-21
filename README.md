### Trabalho final de FCG
### Bob-omb em: dire, dire FCG

O trabalho foi desenvolvido por Léo Hardt e Laura Ramos para a disciplina de fundamentos de computação gráfica da UFRGS. A dupla se dividiu da seguinte forma para a criação do trabalho:
Léo: gerenciador de shaders, camera look at, 
Laura: logging, model do bob-omb, textura, camera livre

### Ferramentas

Não foi feito o uso de ferramentas como ChatGPT, apenas o uso de códigos fornecidos nos laboratórios anteriores da disciplina e tutoriais da internet.

### Descrição do processo de desenvolvimento
Model matrix: 

Interação com o usuário: descrição no manual do jogo, feita usando o teclado



Camera look-at: implementada seguindo o personagem principal.
Camera livre: implementada quando alterada pelo usuário, se move com os botões de seta.

Malha poligonal complexa: foram colocados blocos que interagem com o bob-omb, tanto no campo de visão de entrada, quanto na parte superior do ambiente.

Interação entre objetos: foram feitas colisões cubo-cubo em todos os objetos, incluindo o Bob-omb, mesmo ele sendo redondo.

Iluminação de Blinn-Phong: os objetos foram todos iluminados usando blinn-phong.

Mapeamento de textura: uilizada em todos os objetos. Para o chão a textura se repete multiplas vezes para que a imagem não fique esticada.

Curva de Bézier: utilizada em um cubo que se meve utilizando a curva de Bézier e sua inversa, ou seja, faz um caminha de ida e o mesmo para voltar.

Animação de movimento baseado no tempo: esse tópico é utilizado para mover os pés do personagem (bob-omb), que se mexem quando ele está caminhando pelo ambiente.

### Imagens


### Manual de Jogos

As teclas 'A', 'W', 'D' e 'espace' controlam o bob-omb cada uma tendo as seguintes funções:
A: move para a direita;
W: move para frente;
D: move para a esquerda;
espace: pula.

A camera look at do Bob-omb pode ser movimentada usando o botão esquerdo do mouse.

Para alterar para camera livre deve-se clicar 'Ctrl+1', escrever freeCamera e clicar 'enter'. Para alterar a posição da camera as setas são utilizadas.
Para trocar o FPS deve-se clicar 'Ctrl+1', escrever targetFPS, escolher o FPS desejado e clicar 'enter'.
Tanto para alterar a camera quando o FPS é possível sair do comando clicando "Ctrl+2".

### Compilação e execução

Para compilar basta executar o MakeFile.
Para executar basta o comando make run.
