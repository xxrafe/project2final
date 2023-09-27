#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tgboth.h"
#include <vector>
#include <string>

std::string sortStrings(const std::vector<std::string>& strings) {
    std::vector<std::string> sortedStrings = strings; // Создаем копию входного массива
 
    // Используем функцию std::sort для сортировки строк
    std::sort(sortedStrings.begin(), sortedStrings.end());
    std::string result = "";
    // Выводим отсортированные строки по одной на каждую
    for (const std::string& str : sortedStrings) {
        result += str + "\n";
    }
 
    return result;
}

std::string extractSearchItem(const std::string& input) {
    std::string searchItem;
 
    // Находим позиции открывающей и закрывающей скобок
    size_t openingBracketPos = input.find('(');
    size_t closingBracketPos = input.find(')');
 
    if (openingBracketPos != std::string::npos && closingBracketPos != std::string::npos) {
        // Извлекаем содержимое между скобками
        searchItem = input.substr(openingBracketPos + 1, closingBracketPos - openingBracketPos - 1);
    }
    else {
        return "Неправильный формат команды!";
    }
 
    return searchItem;
}
