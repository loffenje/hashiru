#ifndef LEXER_H
#define LEXER_H

struct Lexer {
    std::string m_source;
    int m_current = 0;
    int m_start = 0;

    Lexer(const std::string &source): m_source {source} {}

    char advance();
    char peek();
    void skip_whitespace();
    std::string next_token();
    bool is_end() const;
};

char Lexer::advance() {
    return m_source[m_current++];
}

char Lexer::peek() {
    return m_source[m_current];
}

bool Lexer::is_end() const {
    return m_current >= m_source.size();
}

void Lexer::skip_whitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
            case '\v': {
                while (isspace(peek())) {
                    advance();
                }
                break;
            }
            default: return;
        }
    }
}

std::string Lexer::next_token() {
    skip_whitespace();

    m_start = m_current;

    char c = advance();

    if (isdigit(c)) {
        while (isdigit(peek())) {
            advance();
        }

        std::string token = m_source.substr(m_start, m_current - m_start);
        for (int i = 0; i < token.size(); i++) {
            token[i] = toupper(token[i]);
        }
        return token;
    }
    if (isalpha(c)) {
        while (isalnum(peek())) {
            advance();
        }

        while (isalpha(peek())) {
            advance();
        }

        std::string token = m_source.substr(m_start, m_current - m_start);
        for (int i = 0; i < token.size(); i++) {
            token[i] = toupper(token[i]);
        }
        return token;
    }

    return std::string {c};
}

#endif
