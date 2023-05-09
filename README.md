# Lab1
使用GNU Flex和GNU Bison编写的词法+语法分析器
- 完成了附加要求1.1：识别八进制数、十六进制数
- 完成了附加要求1.2：识别指数形式的浮点数
- 完成了附加要求1.3：识别单行注释、块注释（其中块注释的实现请参阅[Stack Overflow回答](https://stackoverflow.com/questions/2130097/difficulty-getting-c-style-comments-in-flex-lex)和[Flex Manual](http://westes.github.io/flex/manual/Start-Conditions.html)

# Lab2
完全使用C++类继承体系和智能指针实现的语义分析器
- 完成了附加要求2.3：结构体使用结构等价机制
### 原理简述
语义分析器在实验一中生成的语法树上进行独立的分析。`SemanticAnalyser`类的对象代表一个语义分析器的实例，它的`Analyse`方法就是语义分析的入口，将语法树的根节点传入即可完成语义分析。`SementicAnalyser`类定义了一系列以`Do`开头的对语法树上各个非终结符结点进行分析的方法，`Analyse`方法调用`DoExtDefList`方法对最顶层的`ExtDefList`结点进行分析，然后每个方法将根据自己的语法规则调用子结点的分析方法，逐级完成整个语法树的分析。各方法的具体说明请见`Lab2/bits/semantic_analyser.cpp`中的注释。  

在符号表的构建上，采用了类继承的方法来恰当地表示不同类型的符号。由于所有符号都有行号、名称这两个属性，因此创建了类`Symbol`，这就是所有符号的基类。进一步，符号可大体分为变量类别和结构体定义类别两种，而变量类别又可细分为算术类型、数组类型、函数类型和结构体类型，据此就可以创建一系列子类来描述不同类型的符号。`Lab2/bits/symbols/symbol.h`中有各个类的继承关系图。符号表分为两张，一张存储变量类型符号（即`symbol_table_`成员），另一张存储结构体定义类型的符号（即`struct_def_symbol_table_`成员），前者从字符串（符号名）映射到`std::shared_ptr<VariableSymbol>`，后者从字符串（结构体名）映射到`std::shared_ptr<StructDefSymbol>`。`std::shared_ptr<VariableSymbol>`是各变量类型父类的智能指针，可根据其中的`GetVariableSymbolType()`方法返回的具体类型将其`.get()`方法返回的指针转换为一个子类的指针。  

这些符号类也可以用来单独表示符号的类型或名称。例如，`SemanticAnalyser::DoSpecifier`的返回值只包含变量的类型（因为设计原则是每个非终结符结点的处理方法都只收集其下方结点的信息，而不应该接受父节点传入的额外信息，只有极少数例外），而`SemanticAnalyser::DoExtDecList`方法的返回值只包含变量名以及数组定义的信息，这两部分信息在其父结点`SemanticAnalyser::DoExtDef`处进行合并。

# Lab3
使用C++实现的中间代码生成器
- 完成了附加要求3.1：支持结构体类型变量、结构体类型参数
- 完成了附加要求3.2：支持一维数组参数、高维数组变量
- 支持全局变量的声明和使用，针对本人开发的[Web版IR虚拟机](https://ernestthepoet.github.io/ir-virtual-machine/)
### 原理简述
中间代码生成器同样在实验一中生成的语法树上独立运行，中间代码生成器类`IrGenerator`使用与实验二中`SemanticAnalyser`类相同的语法树分析基架。各方法的具体说明请见`Lab3/bits/ir_generator.cpp`中的注释。`InstructionGenerator`这个工具类用来生成一行指定类型的IR代码。  

`IrGenerator`类中处理非终结符结点的大部分方法都返回其生成的IR序列，但处理`Exp`（表达式）结点的方法`DoExp`较为特殊。一个表达式结点一定代表着某个值，我们需要将这个值返回给调用`DoExp`方法的其他结点；而得到这个值之前，可能还需要执行一系列前序IR指令。因此，我们设计了`ExpValue`类，这个类包含了计算表达式的准备IR序列（`preparation_sequence_`）、表达式的最终值（`final_value_`），以及表达式的类型（`source_type_`），`DoExp`方法返回的便是`ExpValue`类的智能指针。然而，对于使用高维数组的语句的翻译来说，仅有这三个属性还是不够的。因此，还设计了一个子类`ArrayElementExpValue`，包含高维数组当前所处的维数和高维数组本身的一些信息。  

除此之外，一个表达式的最终值可以有多种形式，我们将其分为以下三类：单变量形式（形如`var0`），可带前缀的单变量形式（形如`*var0`或`&var0`），以及非单变量形式（例如`var0 + var1`或`CALL fun`）。在有些情况下，必须使用无前缀的单变量形式，例如数组/结构体的基地址，由于它们要作为加法运算的一个操作数出现，因此它们必须是可带前缀的单个变量；有些情况下，则可以使用非单变量形式，这样可以省去一条赋值语句，有利于精简代码。因此，`DoExp`方法还接受额外的两个布尔参数`force_singular`和`singular_no_prefix`，前者指定是否强制生成单变量形式的最终值，在其为`true`时，后者进一步指定是否保证生成的单变量形式没有前缀。需要特别注意，如果被处理的表达式是一个左值，则绝对不能指定`singular_no_prefix`为`true`，否则得到的最终值就变成了另外一个变量。  

`DoExp`方法的另一个特殊之处在于，当处理的是一个非最终维度的数组索引（或者数组名本身），或者是一个结构体变量时，其返回值中的最终值是基地址，而不是该地址处的变量。数组/结构体类型的函数参数本身已经是一个地址，处理它们时，为了不错误地再给它们增加一个取地址符，`IrGenerator`类还有一个成员`is_address_symbol_`，它将符号名映射为一个布尔值，表示该符号是否是一个地址值。`DoExp`方法据此决定获取数组/结构体的基地址时是否要生成取地址符。
