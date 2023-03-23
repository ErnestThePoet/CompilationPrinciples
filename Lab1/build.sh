mkdir build
mkdir generated
flex -o generated/lex_analyser.c lex_analyser.l && cmake . -Bbuild && cd build && make