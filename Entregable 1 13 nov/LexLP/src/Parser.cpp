#include "Parser.h"

// ---------------------------------------------------------------------------
// Utilidades
// ---------------------------------------------------------------------------

// Cantidad de caracteres (no bytes) de un lexema UTF-8: sirve para ubicar "justo después" de un token.
static int longitudEnCaracteres(const std::string& s) {
    int n = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) ++n;
    }
    return n;
}

Parser::Parser(const std::vector<Token>& tokensDelLexer)
    : fin(TokenType::FIN_ARCHIVO, "", 1, 1) {
    // El parser solo ve tokens "con significado": los comentarios no forman parte de la gramática.
    for (const Token& t : tokensDelLexer) {
        if (t.tipo == TokenType::COMENT || t.tipo == TokenType::ERROR_LEXICO ||
            t.tipo == TokenType::FIN_ARCHIVO) {
            continue;
        }
        toks.push_back(t);
    }
    // El fin de archivo se ubica justo después del último token (para mensajes como "falta '}'").
    if (!toks.empty()) {
        const Token& u = toks.back();
        fin = Token(TokenType::FIN_ARCHIVO, "", u.linea, u.columna + longitudEnCaracteres(u.lexema));
    }
}

const Token& Parser::actual() const {
    return pos < toks.size() ? toks[pos] : fin;
}

const Token& Parser::mirar(size_t desplazamiento) const {
    size_t idx = pos + desplazamiento;
    return idx < toks.size() ? toks[idx] : fin;
}

const Token& Parser::anterior() const {
    if (pos == 0 || toks.empty()) return actual();
    return toks[pos - 1 < toks.size() ? pos - 1 : toks.size() - 1];
}

Token Parser::avanzar() {
    if (pos >= toks.size()) return fin;   // nunca se pasa del final
    return toks[pos++];
}

bool Parser::consumirSi(TokenType t) {
    if (es(t)) {
        avanzar();
        return true;
    }
    return false;
}

bool Parser::esTipoDato(TokenType t) {
    return t == TokenType::INT || t == TokenType::FLOAT ||
           t == TokenType::CHAR || t == TokenType::BOOLEAN;
}

bool Parser::esTipoRetorno(TokenType t) {
    return esTipoDato(t) || t == TokenType::VOID;
}

bool Parser::inicioDeSentencia(TokenType t) {
    return esTipoDato(t) || t == TokenType::ID || t == TokenType::RETURN ||
           t == TokenType::IF || t == TokenType::WHILE || t == TokenType::FOR ||
           t == TokenType::SCANF || t == TokenType::PRINTLN || t == TokenType::LLAVE_IZQ;
}

// LL(2): "int suma (" empieza una función; "int suma =" / "int suma ;" es una declaración.
bool Parser::esInicioFuncion() const {
    return esTipoRetorno(actual().tipo) &&
           (mirar(1).tipo == TokenType::ID || mirar(1).tipo == TokenType::MAIN) &&
           mirar(2).tipo == TokenType::PAR_IZQ;
}

NodoPtr Parser::nodoID(const Token& t) {
    // Se muestra el atributo del token tal como lo entrega el léxico: <ID, posicion en la tabla de simbolos>
    std::string valor = t.lexema;
    if (t.atributo >= 0) valor += "  <ID, " + std::to_string(t.atributo) + ">";
    return crearNodo("ID", valor, t.linea, t.columna);
}

// ---------------------------------------------------------------------------
// Reporte de errores
// ---------------------------------------------------------------------------

std::string Parser::describir(const Token& t) {
    if (t.tipo == TokenType::FIN_ARCHIVO) return "el fin del archivo";
    if (t.tipo == TokenType::ID) return "el identificador '" + t.lexema + "'";
    if (t.tipo == TokenType::NUM_INT || t.tipo == TokenType::NUM_DEC) return "el numero '" + t.lexema + "'";
    if (t.tipo == TokenType::TEXTO) return "el texto " + t.lexema;
    if (categoriaToken(t.tipo) == "Palabra reservada") return "la palabra reservada '" + t.lexema + "'";
    return "'" + t.lexema + "'";
}

std::string Parser::nombreEsperado(TokenType t) {
    switch (t) {
        case TokenType::ID:       return "un identificador";
        case TokenType::NUM_INT:  return "un numero entero";
        case TokenType::COMPARACION: return "un operador de comparacion";
        default:                  return "'" + tokenTypeToString(t) + "'";
    }
}

void Parser::registrarEn(int linea, int columna, const std::string& encontrado,
                         const std::string& mensaje, bool permitirDuplicado) {
    // Si ya se reportó un error en este mismo token no se repite (evita cascadas tras recuperarse).
    if (!permitirDuplicado && pos == ultimoErrorPos) return;
    ultimoErrorPos = pos;
    errores.emplace_back(linea, columna, encontrado, mensaje);
}

void Parser::fallarEn(const Token& donde, const std::string& mensaje) {
    registrarEn(donde.linea, donde.columna,
                donde.tipo == TokenType::FIN_ARCHIVO ? "<fin de archivo>" : donde.lexema, mensaje);
    throw Abortar{};
}

void Parser::fallar(const std::string& mensaje) {
    fallarEn(actual(), mensaje);
}

// Consume el token esperado o reporta el error. 'contexto' completa el mensaje:
//   "Se esperaba ')' <contexto>, pero se encontro ..."
Token Parser::esperar(TokenType t, const std::string& contexto) {
    if (es(t)) return avanzar();
    fallar("Se esperaba " + nombreEsperado(t) + " " + contexto + ", pero se encontro " + describir(actual()));
}

// El ';' faltante se reporta justo DESPUÉS del último token válido (donde el programador lo olvidó),
// no en el token siguiente. Si lo que sigue es claramente otra sentencia en otra línea (o un '}'),
// se continúa como si el ';' existiera (recuperación local); si no, se salta (modo pánico).
void Parser::esperarPuntoComa(const std::string& contexto) {
    if (es(TokenType::PUNTO_COMA)) {
        avanzar();
        return;
    }
    const Token& ant = anterior();
    registrarEn(ant.linea, ant.columna + longitudEnCaracteres(ant.lexema),
                actual().tipo == TokenType::FIN_ARCHIVO ? "<fin de archivo>" : actual().lexema,
                "Se esperaba ';' " + contexto + " (se encontro " + describir(actual()) + ")");

    bool otraLinea = actual().linea > ant.linea && inicioDeSentencia(actual().tipo);
    if (otraLinea || es(TokenType::LLAVE_DER) || es(TokenType::FIN_ARCHIVO)) {
        return;   // se asume el ';' y se sigue
    }
    throw Abortar{};
}

// ---------------------------------------------------------------------------
// Recuperación de errores
// ---------------------------------------------------------------------------

// Dentro de un bloque: salta hasta justo después de un ';', o hasta un '}' / inicio de sentencia / inicio de función.
void Parser::sincronizarSentencia() {
    while (!es(TokenType::FIN_ARCHIVO)) {
        if (es(TokenType::PUNTO_COMA)) { avanzar(); return; }
        if (es(TokenType::LLAVE_DER)) return;
        if (inicioDeSentencia(actual().tipo) && actual().tipo != TokenType::ID) return;
        if (esInicioFuncion()) return;
        avanzar();
    }
}

// A nivel de programa: salta hasta el comienzo de la siguiente función.
void Parser::sincronizarFuncion() {
    while (!es(TokenType::FIN_ARCHIVO) && !esInicioFuncion()) {
        avanzar();
    }
}

// ---------------------------------------------------------------------------
// Punto de entrada
// ---------------------------------------------------------------------------

NodoPtr Parser::analizar() {
    pos = 0;
    tieneMain = false;
    ultimoErrorPos = static_cast<size_t>(-1);
    errores.clear();
    avisos.clear();
    return programa();
}

// ---------------------------------------------------------------------------
// Estructura del programa
// ---------------------------------------------------------------------------

// Programa -> Funcion+          (debe existir la función main)
NodoPtr Parser::programa() {
    auto raiz = crearNodo("Programa", "", 1, 1);

    if (toks.empty()) {
        registrarEn(1, 1, "<fin de archivo>",
                    "El programa esta vacio: se esperaba al menos la funcion 'main'", true);
        return raiz;
    }

    while (!es(TokenType::FIN_ARCHIVO)) {
        size_t inicio = pos;
        try {
            raiz->agregar(funcion());
        } catch (const Abortar&) {
            sincronizarFuncion();
            if (pos == inicio) avanzar();   // garantiza progreso: nunca se queda en un ciclo infinito
        }
    }

    // Solo se informa si no hubo otros errores: si no, sería un error derivado del anterior
    // (p. ej. "main() {...}" sin tipo de retorno ya fue reportado).
    if (!tieneMain && errores.empty()) {
        registrarEn(fin.linea, fin.columna, "<fin de archivo>",
                    "El programa no tiene la funcion 'main' (ej: void main() { ... })", true);
    }
    return raiz;
}

// Funcion -> TipoRetorno (ID | main) '(' [Parametro {',' Parametro}] ')' Bloque
NodoPtr Parser::funcion() {
    auto nodo = crearNodo("Funcion", "", actual().linea, actual().columna);

    if (!esTipoRetorno(actual().tipo)) {
        fallar("Se esperaba el tipo de retorno de una funcion (int, float, char, boolean o void), pero se encontro " +
               describir(actual()) + " (en LP el programa es una secuencia de funciones)");
    }
    Token tipo = avanzar();
    nodo->agregar(crearNodo("Tipo", tipo.lexema, tipo.linea, tipo.columna));

    if (es(TokenType::MAIN)) {
        Token m = avanzar();
        tieneMain = true;
        nodo->agregar(crearNodo("Nombre", m.lexema, m.linea, m.columna));
    } else if (es(TokenType::ID)) {
        nodo->agregar(nodoID(avanzar()));
    } else {
        fallar("Se esperaba el nombre de la funcion despues del tipo de retorno, pero se encontro " +
               describir(actual()));
    }

    if (!es(TokenType::PAR_IZQ)) {
        std::string pista;
        if (es(TokenType::ASIGNACION) || es(TokenType::PUNTO_COMA) ||
            es(TokenType::COMA) || es(TokenType::CORCHETE_IZQ)) {
            pista = " (en LP las variables se declaran dentro de una funcion, no fuera)";
        }
        fallar("Se esperaba '(' despues del nombre de la funcion, pero se encontro " +
               describir(actual()) + pista);
    }
    avanzar();   // '('

    auto params = crearNodo("Parametros");
    if (!es(TokenType::PAR_DER)) {
        do {
            params->agregar(parametro());
        } while (consumirSi(TokenType::COMA));
    }
    esperar(TokenType::PAR_DER, "para cerrar la lista de parametros");
    nodo->agregar(std::move(params));

    nodo->agregar(bloque());
    return nodo;
}

// Parametro -> Tipo ID ['[' ']']
NodoPtr Parser::parametro() {
    auto nodo = crearNodo("Parametro", "", actual().linea, actual().columna);
    if (!esTipoDato(actual().tipo)) {
        fallar("Se esperaba el tipo de un parametro (int, float, char o boolean), pero se encontro " +
               describir(actual()));
    }
    Token tipo = avanzar();
    nodo->agregar(crearNodo("Tipo", tipo.lexema, tipo.linea, tipo.columna));

    if (!es(TokenType::ID)) {
        fallar("Se esperaba el nombre del parametro despues del tipo '" + tipo.lexema +
               "', pero se encontro " + describir(actual()));
    }
    nodo->agregar(nodoID(avanzar()));

    if (consumirSi(TokenType::CORCHETE_IZQ)) {
        esperar(TokenType::CORCHETE_DER, "en el parametro de tipo arreglo (ej: int a[])");
        nodo->agregar(crearNodo("Arreglo"));
    }
    return nodo;
}

// Bloque -> '{' Sentencia* '}'
NodoPtr Parser::bloque() {
    Token apertura = actual();
    if (!es(TokenType::LLAVE_IZQ)) {
        fallar("Se esperaba '{' para abrir el cuerpo, pero se encontro " + describir(actual()));
    }
    avanzar();
    auto nodo = crearNodo("Bloque", "", apertura.linea, apertura.columna);

    bool cortadoPorFuncion = false;
    while (!es(TokenType::LLAVE_DER) && !es(TokenType::FIN_ARCHIVO)) {
        // Si aparece el encabezado de otra función, casi seguro faltó cerrar este bloque con '}'
        // (o se intentó anidar una función, cosa que LP no permite).
        if (esInicioFuncion()) {
            cortadoPorFuncion = true;
            break;
        }

        size_t inicio = pos;
        try {
            nodo->agregar(sentencia());
        } catch (const Abortar&) {
            sincronizarSentencia();
            if (pos == inicio) avanzar();   // garantiza progreso
        }
    }

    if (es(TokenType::LLAVE_DER)) {
        avanzar();
    } else {
        registrarEn(actual().linea, actual().columna,
                    actual().tipo == TokenType::FIN_ARCHIVO ? "<fin de archivo>" : actual().lexema,
                    "Falta '}' para cerrar el bloque abierto en la linea " + std::to_string(apertura.linea) +
                    ", columna " + std::to_string(apertura.columna) +
                    (cortadoPorFuncion ? " (aqui empieza otra funcion; en LP las funciones no se anidan)" : ""));
    }
    return nodo;
}

// Sentencia -> Declaracion | Asignacion | LlamadaSentencia | Return | Bloque
//            | (Entregable 2: If | While | For | Scanf | Println)
NodoPtr Parser::sentencia() {
    const Token& t = actual();

    if (esTipoDato(t.tipo))                 return declaracion();
    if (t.tipo == TokenType::ID)            return asignacionOLlamada();
    if (t.tipo == TokenType::RETURN)        return retorno();
    if (t.tipo == TokenType::LLAVE_IZQ)     return bloque();

    switch (t.tipo) {
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::FOR:
        case TokenType::SCANF:
        case TokenType::PRINTLN:
            return sentenciaPendiente();
        case TokenType::ELSE:
            fallar("'else' sin un 'if' previo");
        case TokenType::VOID:
        case TokenType::MAIN:
            fallar("No se puede escribir " + describir(t) +
                   " dentro de un bloque (las funciones no se anidan; quiza falto cerrar con '}')");
        default:
            fallar("Se esperaba una sentencia (declaracion, asignacion o return), pero se encontro " +
                   describir(t));
    }
}

// ---------------------------------------------------------------------------
// Sentencias del Entregable 1
// ---------------------------------------------------------------------------

// Declaracion -> Tipo DeclVar {',' DeclVar} ';'
NodoPtr Parser::declaracion() {
    auto nodo = crearNodo("Declaracion", "", actual().linea, actual().columna);
    Token tipo = avanzar();
    nodo->agregar(crearNodo("Tipo", tipo.lexema, tipo.linea, tipo.columna));

    do {
        nodo->agregar(declaracionVariable());
    } while (consumirSi(TokenType::COMA));

    esperarPuntoComa("al final de la declaracion");
    return nodo;
}

// DeclVar -> ID ['[' NUM_INT ']'] ['=' Expresion]
NodoPtr Parser::declaracionVariable() {
    if (!es(TokenType::ID)) {
        fallar("Se esperaba un identificador en la declaracion, pero se encontro " + describir(actual()));
    }
    Token id = avanzar();
    auto nodo = crearNodo("Variable", "", id.linea, id.columna);
    nodo->agregar(nodoID(id));

    bool esArreglo = false;
    if (consumirSi(TokenType::CORCHETE_IZQ)) {
        if (!es(TokenType::NUM_INT)) {
            fallar("El tamano de un arreglo debe ser una constante entera (ej: int a[5]), pero se encontro " +
                   describir(actual()));
        }
        Token tam = avanzar();
        esperar(TokenType::CORCHETE_DER, "para cerrar el tamano del arreglo");
        nodo->agregar(crearNodo("Tamano", tam.lexema, tam.linea, tam.columna));
        esArreglo = true;
    }

    if (es(TokenType::ASIGNACION)) {
        Token igual = avanzar();
        if (esArreglo) {
            fallarEn(igual, "No se permite inicializar un arreglo en su declaracion (asigne cada elemento por separado)");
        }
        auto ini = crearNodo("Inicializacion", "", igual.linea, igual.columna);
        ini->agregar(expresion());
        nodo->agregar(std::move(ini));
    }
    return nodo;
}

// Asignacion -> Variable '=' Expresion ';'
// LlamadaSentencia -> Llamada ';'
NodoPtr Parser::asignacionOLlamada() {
    const Token inicio = actual();

    // LL(2): "ID (" es una llamada a función usada como sentencia.
    if (mirar(1).tipo == TokenType::PAR_IZQ) {
        auto nodo = crearNodo("LlamadaSentencia", "", inicio.linea, inicio.columna);
        nodo->agregar(llamada());
        esperarPuntoComa("al final de la llamada");
        return nodo;
    }

    auto nodo = crearNodo("Asignacion", "", inicio.linea, inicio.columna);
    nodo->agregar(variable());

    if (!es(TokenType::ASIGNACION)) {
        std::string pista;
        if (actual().tipo == TokenType::COMPARACION && actual().lexema == "==") {
            pista = " (para asignar se usa '=', '==' es una comparacion)";
        }
        fallar("Se esperaba '=' en la asignacion a '" + inicio.lexema + "', pero se encontro " +
               describir(actual()) + pista);
    }
    avanzar();   // '='

    nodo->agregar(expresion());
    esperarPuntoComa("al final de la asignacion");
    return nodo;
}

// Return -> 'return' [Expresion] ';'
NodoPtr Parser::retorno() {
    Token r = avanzar();
    auto nodo = crearNodo("Return", "", r.linea, r.columna);
    if (!es(TokenType::PUNTO_COMA) && !es(TokenType::LLAVE_DER)) {
        nodo->agregar(expresion());
    }
    esperarPuntoComa("despues de 'return'");
    return nodo;
}

// ---------------------------------------------------------------------------
// Entregable 2: se reconocen y se omiten (con aviso) para poder analizar programas completos
// ---------------------------------------------------------------------------

NodoPtr Parser::sentenciaPendiente() {
    Token t = actual();
    avisos.emplace_back(t.linea, t.columna,
                        "La sentencia '" + t.lexema + "' corresponde al Entregable 2: se omite (su sintaxis aun no se verifica)");

    if (t.tipo == TokenType::SCANF || t.tipo == TokenType::PRINTLN) {
        avanzar();
        omitirHastaPuntoComa();
    } else {
        omitirControl();
    }
    return crearNodo("Pendiente", t.lexema + " (Entregable 2)", t.linea, t.columna);
}

// if / while / for:  palabra '(' ... ')' sentencia  [else sentencia]
void Parser::omitirControl() {
    TokenType clase = actual().tipo;
    avanzar();
    if (es(TokenType::PAR_IZQ)) omitirAgrupado(TokenType::PAR_IZQ, TokenType::PAR_DER);
    omitirSentencia();
    if (clase == TokenType::IF) {
        while (es(TokenType::ELSE)) {
            avanzar();
            omitirSentencia();
        }
    }
}

void Parser::omitirSentencia() {
    TokenType t = actual().tipo;
    if (t == TokenType::LLAVE_IZQ) {
        omitirAgrupado(TokenType::LLAVE_IZQ, TokenType::LLAVE_DER);
    } else if (t == TokenType::IF || t == TokenType::WHILE || t == TokenType::FOR) {
        omitirControl();
    } else {
        omitirHastaPuntoComa();
    }
}

// Salta un grupo balanceado: empieza en 'abre' y termina justo después de su 'cierra' correspondiente.
void Parser::omitirAgrupado(TokenType abre, TokenType cierra) {
    int profundidad = 0;
    while (!es(TokenType::FIN_ARCHIVO)) {
        if (es(abre)) ++profundidad;
        else if (es(cierra)) --profundidad;
        avanzar();
        if (profundidad == 0) break;
    }
}

void Parser::omitirHastaPuntoComa() {
    while (!es(TokenType::FIN_ARCHIVO) && !es(TokenType::PUNTO_COMA) && !es(TokenType::LLAVE_DER)) {
        avanzar();
    }
    consumirSi(TokenType::PUNTO_COMA);
}

// ---------------------------------------------------------------------------
// Expresiones
//
// La gramática se escribe por NIVELES de precedencia (de menor a mayor) y con ITERACIÓN en lugar de
// recursión por la izquierda; así es LL(1) y los operadores quedan asociativos a la izquierda:
//
//   Expresion -> ExprOr
//   ExprOr    -> ExprAnd  { '||' ExprAnd }
//   ExprAnd   -> ExprComp { '&&' ExprComp }
//   ExprComp  -> ExprSuma { COMP ExprSuma }
//   ExprSuma  -> ExprMult { ('+' | '-') ExprMult }
//   ExprMult  -> ExprUna  { ('*' | '/' | '%') ExprUna }
//   ExprUna   -> ('!' | '-') ExprUna | Primario
//   Primario  -> NUM_INT | NUM_DEC | TEXTO | Llamada | Variable | '(' Expresion ')'
// ---------------------------------------------------------------------------

// Patrón común de los niveles binarios:  operando { operador operando }
NodoPtr Parser::nivelBinario(NodoPtr (Parser::*operando)(), bool (*esOperador)(TokenType)) {
    NodoPtr izq = (this->*operando)();
    while (esOperador(actual().tipo)) {
        Token op = avanzar();
        auto nodo = crearNodo("Op", op.lexema, op.linea, op.columna);
        nodo->agregar(std::move(izq));
        nodo->agregar((this->*operando)());
        izq = std::move(nodo);
    }
    return izq;
}

NodoPtr Parser::expresion() { return exprOr(); }

NodoPtr Parser::exprOr() {
    return nivelBinario(&Parser::exprAnd, [](TokenType t) { return t == TokenType::OR; });
}

NodoPtr Parser::exprAnd() {
    return nivelBinario(&Parser::exprComp, [](TokenType t) { return t == TokenType::AND; });
}

NodoPtr Parser::exprComp() {
    return nivelBinario(&Parser::exprSuma, [](TokenType t) { return t == TokenType::COMPARACION; });
}

NodoPtr Parser::exprSuma() {
    return nivelBinario(&Parser::exprMult,
                        [](TokenType t) { return t == TokenType::SUMA || t == TokenType::RESTA; });
}

NodoPtr Parser::exprMult() {
    return nivelBinario(&Parser::exprUnaria, [](TokenType t) {
        return t == TokenType::MULT || t == TokenType::DIV || t == TokenType::MODULO;
    });
}

NodoPtr Parser::exprUnaria() {
    if (es(TokenType::NOT) || es(TokenType::RESTA)) {
        Token op = avanzar();
        auto nodo = crearNodo("OpUnario", op.lexema, op.linea, op.columna);
        nodo->agregar(exprUnaria());
        return nodo;
    }
    return primario();
}

NodoPtr Parser::primario() {
    const Token& t = actual();
    switch (t.tipo) {
        case TokenType::NUM_INT:
        case TokenType::NUM_DEC:
        case TokenType::TEXTO: {
            Token lit = avanzar();
            return crearNodo(tokenTypeToString(lit.tipo), lit.lexema, lit.linea, lit.columna);
        }
        case TokenType::ID:
            if (mirar(1).tipo == TokenType::PAR_IZQ) return llamada();
            return variable();
        case TokenType::PAR_IZQ: {
            avanzar();
            NodoPtr interna = expresion();
            esperar(TokenType::PAR_DER, "para cerrar el parentesis de la expresion");
            return interna;   // los paréntesis solo agrupan: el árbol ya refleja la precedencia
        }
        default:
            fallar("Se esperaba una expresion (numero, texto, identificador, '(' , '-' o '!'), pero se encontro " +
                   describir(t));
    }
}

// Variable -> ID | ID '[' Expresion ']'
NodoPtr Parser::variable() {
    Token id = esperar(TokenType::ID, "(nombre de variable)");
    if (es(TokenType::CORCHETE_IZQ)) {
        avanzar();
        auto nodo = crearNodo("Indice", "", id.linea, id.columna);
        nodo->agregar(nodoID(id));
        nodo->agregar(expresion());
        esperar(TokenType::CORCHETE_DER, "para cerrar el indice del arreglo");
        return nodo;
    }
    return nodoID(id);
}

// Llamada -> ID '(' [Expresion {',' Expresion}] ')'
NodoPtr Parser::llamada() {
    Token id = avanzar();
    avanzar();   // '(' (ya verificado con lookahead)
    auto nodo = crearNodo("Llamada", "", id.linea, id.columna);
    nodo->agregar(nodoID(id));

    auto args = crearNodo("Argumentos");
    if (!es(TokenType::PAR_DER)) {
        do {
            args->agregar(expresion());
        } while (consumirSi(TokenType::COMA));
    }
    esperar(TokenType::PAR_DER, "para cerrar los argumentos de la llamada");
    nodo->agregar(std::move(args));
    return nodo;
}
