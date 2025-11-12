A parte o1 e o2 está no arquivo compilador.cpp.
A parte do pre processador está no arquivo pre.cpp

Alunos:
Erick Taira 222011525  |  Lucca Magalhães 222011552  |  Leandro Beloti Kornelius 211020900
LINUX UBUNTU

compilar:

g++ compilador.cpp -o compilador

g++ -o preprocessor pre.cpp Preprocessor.cpp

executar parte pré-processador:
./preprocessor dados.asm

executar parte o1:
./compilador.o dados.pre o1

executar parte o2:
./compilador.o dados.pre o2

executar ambos o1 e o2:
./compilador.o dados.pre all
