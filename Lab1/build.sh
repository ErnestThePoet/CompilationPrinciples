mkdir build
GENERATED_DIR=generated
mkdir $GENERATED_DIR
flex -o $GENERATED_DIR/lex_analyser.c --header-file=$GENERATED_DIR/lex_analyser.h lex_analyser.l \
&& bison -o $GENERATED_DIR/parser.c -H$GENERATED_DIR/parser.h parser.y \
&& cmake . -Bbuild \
&& cd build \
&& make