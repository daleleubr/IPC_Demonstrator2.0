#pragma once// JSON.h
#ifndef JSON_FORMATTER_H
#define JSON_FORMATTER_H

#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <ctime>

class JsonFormatter {
public:
    /**
     * Formata um evento como JSON
     * @param eventType Tipo do evento
     * @param details Detalhes do evento (chave-valor)
     * @return String formatada em JSON
     */
    static std::string formatEvent(const std::string& eventType,
        const std::map<std::string, std::string>& details = {});

    /**
     * Formata um erro como JSON
     * @param errorMessage Mensagem de erro
     * @param details Detalhes do erro (chave-valor)
     * @return String formatada em JSON
     */
    static std::string formatError(const std::string& errorMessage,
        const std::map<std::string, std::string>& details = {});

    /**
     * Obtém o timestamp atual formatado
     * @return String com timestamp no formato YYYY-MM-DD HH:MM:SS
     */
    static std::string getCurrentTimestamp();

    /**
     * Escapa caracteres especiais para strings JSON
     * @param input String original
     * @return String escapada
     */
    static std::string escapeJsonString(const std::string& input);
};

#endif // JSON_FORMATTER_H
