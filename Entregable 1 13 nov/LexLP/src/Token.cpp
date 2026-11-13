#include "Token.h"

std::string tokenTypeToString(TokenType tipo) {
    switch (tipo) {
        case TokenType::NUM_INT:       return "NUM_INT";
        case TokenType::NUM_DEC:       return "NUM_DEC";
        case TokenType::ID:            return "ID";
        case TokenType::TEXTO:         return "TEXTO";
        case TokenType::INT:           return "INT";
        case TokenType::FLOAT:         return "FLOAT";
        case TokenType::CHAR:          return "CHAR";
        case TokenType::BOOLEAN:       return "BOOLEAN";
        case TokenType::VOID:          return "VOID";
        case TokenType::IF:            return "IF";
        case TokenType::ELSE:          return "ELSE";
        case TokenType::FOR:           return "FOR";
        case TokenType::WHILE:         return "WHILE";
        case TokenType::SCANF:         return "SCANF";
        case TokenType::PRINTLN:       return "PRINTLN";
        case TokenType::MAIN:          return "MAIN";
        case TokenType::RETURN:        return "RETURN";
        case TokenType::ASIGNACION:    return "=";
        case TokenType::SUMA:          return "+";
        case TokenType::RESTA:         return "-";
        case TokenType::MULT:          return "*";
        case TokenType::DIV:           return "/";
        case TokenType::MODULO:        return "%";
        case TokenType::AND:           return "&&";
        case TokenType::OR:            return "||";
        case TokenType::NOT:           return "!";
        case TokenType::COMPARACION:   return "COMP";
        case TokenType::PAR_IZQ:       return "(";
        case TokenType::PAR_DER:       return ")";
        case TokenType::CORCHETE_IZQ:  return "[";
        case TokenType::CORCHETE_DER:  return "]";
        case TokenType::LLAVE_IZQ:     return "{";
        case TokenType::LLAVE_DER:     return "}";
        case TokenType::COMA:          return ",";
        case TokenType::PUNTO_COMA:    return ";";
        case TokenType::COMENT:        return "COMENT";
        case TokenType::ERROR_LEXICO:  return "ERROR_LEXICO";
        case TokenType::FIN_ARCHIVO:   return "FIN_ARCHIVO";
    }
    return "DESCONOCIDO";
}

std::string categoriaToken(TokenType tipo) {
    switch (tipo) {
        case TokenType::NUM_INT:       return "Numero entero";
        case TokenType::NUM_DEC:       return "Numero decimal";
        case TokenType::ID:            return "Identificador";
        case TokenType::TEXTO:         return "Constante de texto";
        case TokenType::INT:
        case TokenType::FLOAT:
        case TokenType::CHAR:
        case TokenType::BOOLEAN:
        case TokenType::VOID:
        case TokenType::IF:
        case TokenType::ELSE:
        case TokenType::FOR:
        case TokenType::WHILE:
        case TokenType::SCANF:
        case TokenType::PRINTLN:
        case TokenType::MAIN:
        case TokenType::RETURN:        return "Palabra reservada";
        case TokenType::ASIGNACION:    return "Operador de asignacion";
        case TokenType::SUMA:
        case TokenType::RESTA:
        case TokenType::MULT:
        case TokenType::DIV:
        case TokenType::MODULO:        return "Operador aritmetico";
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::NOT:           return "Operador logico";
        case TokenType::COMPARACION:   return "Operador de comparacion/relacional";
        case TokenType::PAR_IZQ:
        case TokenType::PAR_DER:
        case TokenType::CORCHETE_IZQ:
        case TokenType::CORCHETE_DER:
        case TokenType::LLAVE_IZQ:
        case TokenType::LLAVE_DER:
        case TokenType::COMA:
        case TokenType::PUNTO_COMA:    return "Simbolo especial";
        case TokenType::COMENT:        return "Comentario";
        case TokenType::ERROR_LEXICO:  return "Error lexico";
        case TokenType::FIN_ARCHIVO:   return "Fin de archivo";
    }
    return "Desconocido";
}

std::string Token::toString() const {
    // Los identificadores llevan su posición en la tabla de símbolos como atributo.
    if (tipo == TokenType::ID && atributo >= 0) {
        return "<ID, " + std::to_string(atributo) + ">";
    }
    // Los operadores de comparación llevan como atributo el operador concreto.
    if (tipo == TokenType::COMPARACION) {
        return "<COMP, " + lexema + ">";
    }
    return "<" + tokenTypeToString(tipo) + ">";
}
