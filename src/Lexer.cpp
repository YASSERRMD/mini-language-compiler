#include "Lexer.hpp"
#include <cctype>
#include <cstring>
#include <unordered_map>

namespace minilang {

// Keyword lookup table
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"let", TokenType::LET},
    {"fn", TokenType::FN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"print", TokenType::PRINT},
};

Lexer::Lexer(std::string source) : m_source(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    m_tokens.clear();
    m_start = 0;
    m_current = 0;
    m_line = 1;
    m_column = 1;

    while (!isAtEnd()) {
        m_start = m_current;
        Token token = scanToken();
        if (token.type != TokenType::ERROR) {
            m_tokens.push_back(token);
        }
    }

    m_tokens.push_back(makeToken(TokenType::EOF_TOKEN));
    return m_tokens;
}

Token Lexer::nextToken() {
    if (isAtEnd()) {
        return makeToken(TokenType::EOF_TOKEN);
    }

    m_start = m_current;
    return scanToken();
}

bool Lexer::match(char expected) {
    if (isAtEnd() || m_source[m_current] != expected) {
        return false;
    }
    m_current++;
    m_column++;
    return true;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                m_line++;
                m_column = 1;
                advance();
                break;
            default:
                return;
        }
    }
}

void Lexer::skipComment() {
    // Line comments: // to end of line
    if (peek() == '/' && peekNext() == '/') {
        while (!isAtEnd() && peek() != '\n') {
            advance();
        }
    }
}

Token Lexer::makeToken(TokenType type) const {
    return Token(type, m_source.substr(m_start, m_current - m_start), m_line, m_column);
}

Token Lexer::makeToken(TokenType type, double literal) const {
    return Token(type, m_source.substr(m_start, m_current - m_start), m_line, m_column, literal);
}

Token Lexer::makeToken(TokenType type, std::string literal) const {
    return Token(type, m_source.substr(m_start, m_current - m_start), m_line, m_column, std::move(literal));
}

Token Lexer::makeToken(TokenType type, bool literal) const {
    return Token(type, m_source.substr(m_start, m_current - m_start), m_line, m_column, literal);
}

Token Lexer::errorToken(const std::string& message) const {
    Token token;
    token.type = TokenType::ERROR;
    token.lexeme = message;
    token.line = m_line;
    token.column = m_column;
    return token;
}

Token Lexer::scanToken() {
    skipWhitespace();
    skipComment();
    skipWhitespace();

    m_start = m_current;

    if (isAtEnd()) {
        return makeToken(TokenType::EOF_TOKEN);
    }

    char c = advance();

    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return scanIdentifier();
    }

    // Numbers
    if (std::isdigit(c)) {
        return scanNumber();
    }

    // Strings
    if (c == '"') {
        return scanString();
    }

    // Operators and delimiters
    switch (c) {
        case '(':
            return makeToken(TokenType::LPAREN);
        case ')':
            return makeToken(TokenType::RPAREN);
        case '{':
            return makeToken(TokenType::LBRACE);
        case '}':
            return makeToken(TokenType::RBRACE);
        case ',':
            return makeToken(TokenType::COMMA);
        case ';':
            return makeToken(TokenType::SEMICOLON);
        case '+':
            return makeToken(TokenType::PLUS);
        case '-':
            return makeToken(TokenType::MINUS);
        case '*':
            return makeToken(TokenType::STAR);
        case '/':
            return makeToken(TokenType::SLASH);
        case '%':
            return makeToken(TokenType::PERCENT);
        case '=':
            if (match('=')) {
                return makeToken(TokenType::EQUAL_EQUAL);
            }
            return makeToken(TokenType::EQUAL);
        case '!':
            if (match('=')) {
                return makeToken(TokenType::BANG_EQUAL);
            }
            return errorToken("Unexpected '!' without '='");
        case '<':
            if (match('=')) {
                return makeToken(TokenType::LESS_EQUAL);
            }
            return makeToken(TokenType::LESS);
        case '>':
            if (match('=')) {
                return makeToken(TokenType::GREATER_EQUAL);
            }
            return makeToken(TokenType::GREATER);
        default:
            return errorToken(std::string("Unexpected character: ") + c);
    }
}

Token Lexer::scanIdentifier() {
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }

    std::string text = m_source.substr(m_start, m_current - m_start);
    auto it = KEYWORDS.find(text);
    if (it != KEYWORDS.end()) {
        return makeToken(it->second);
    }
    return makeToken(TokenType::IDENTIFIER);
}

Token Lexer::scanNumber() {
    while (std::isdigit(peek())) {
        advance();
    }

    // Handle decimal point
    if (peek() == '.' && std::isdigit(peekNext())) {
        advance(); // consume '.'
        while (std::isdigit(peek())) {
            advance();
        }
    }

    std::string numStr = m_source.substr(m_start, m_current - m_start);
    double value = std::stod(numStr);
    return makeToken(TokenType::NUMBER, value);
}

Token Lexer::scanString() {
    std::string value;

    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            m_line++;
            m_column = 1;
        }
        value += advance();
    }

    if (isAtEnd()) {
        return errorToken("Unterminated string");
    }

    advance(); // consume closing '"'
    return makeToken(TokenType::STRING, value);
}

} // namespace minilang
