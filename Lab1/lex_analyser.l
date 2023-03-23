%{
    int chars=0;
    int words=0;
    int lines=0;
%}
letter [a-zA-Z]
%%
{letter}+ {words++;chars+=yyleng;}
[ ]+ {}
\n {lines++;}
. {printf("Unexpected char: %s\n",yytext);}
%%
int main(void){
    yylex();
    printf("%d %d %d\n",chars,words,lines);
    return 0;
}