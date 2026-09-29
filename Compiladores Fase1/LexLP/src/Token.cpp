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
        case TokenType::ERROR_LEXICO:  return "ERROR_LEXICO";
        case TokenType::FIN_ARCHIVO:   return "FIN_ARCHIVO";
    }
    return "DESCONOCIDO";
}

std::string Token::toString() const {
    // Los identificadores llevan su posición en la tabla de símbolos como atributo.
    if (tipo == TokenType::ID && atributo >= 0) {
        return "<ID, " + std::to_string(atributo) + ">";
    }
    return "<" + tokenTypeToString(tipo) + ">";
}
