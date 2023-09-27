#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tgboth.h"
#include <vector>
#include <string>

// Тестирование функции sortStrings
 
// Тест на пустой вектор строк. Ожидается пустая строка.
TEST(SortStringsTest, EmptyVector) {
    std::vector<std::string> input;
    std::string result = sortStrings(input);
    EXPECT_EQ(result, "");
}
 
// Тест на сортировку вектора строк с одной строкой. Ожидается та же строка.
TEST(SortStringsTest, SingleString) {
    std::vector<std::string> input = {"Hello"};
    std::string result = sortStrings(input);
    EXPECT_EQ(result, "Hello\n");
}
 
// Тест на сортировку вектора строк с несколькими строками.
TEST(SortStringsTest, MultipleStrings) {
    std::vector<std::string> input = {"Banana", "Apple", "Cherry"};
    std::string result = sortStrings(input);
    EXPECT_EQ(result, "Apple\nBanana\nCherry\n");
}

// Тест на сортировку вектора строк с пустыми строками.
TEST(SortStringsTest, EmptyStrings) {
    std::vector<std::string> input = {"", "Zebra", "", "Ant"};
    std::string result = sortStrings(input);
    EXPECT_EQ(result, "\n\nAnt\nZebra\n");
}

// Тестирование функции extractSearchItem

// Тест на правильный формат ввода. Ожидается извлечение содержимого между скобками.
TEST(ExtractSearchItemTest, ValidInput) {
    std::string input = "Купить(SearchItem)";
    std::string result = extractSearchItem(input);
    EXPECT_EQ(result, "SearchItem");
}

// Ожидается извлечение содержимого между скобками.
TEST(ExtractSearchItemTest, JustBracket) {
    std::string input = "(SearchItem)";
    std::string result = extractSearchItem(input);
    EXPECT_EQ(result, "SearchItem");
}

// Тест на правильный формат ввода c числами. Ожидается извлечение содержимого между скобками.
TEST(ExtractSearchItemTest, NumbersInput) {
    std::string input = "Купить(SearchItem1337)";
    std::string result = extractSearchItem(input);
    EXPECT_EQ(result, "SearchItem1337");
}

// Тест на отсутствие закрывающей скобки. Ожидается сообщение об ошибке.
TEST(ExtractSearchItemTest, MissingClosingBracket) {
    std::string input = "Купить(SearchItem";
    std::string result = extractSearchItem(input);
    EXPECT_EQ(result, "Неправильный формат команды!");
}

// Тест на отсутствие открывающей скобки. Ожидается сообщение об ошибке.
TEST(ExtractSearchItemTest, MissingOpeningBracket) {
    std::string input = "КупитьSearchItem)";
    std::string result = extractSearchItem(input);
    EXPECT_EQ(result, "Неправильный формат команды!");
}

// Тест на отсутствие скобок. Ожидается сообщение об ошибке.
TEST(ExtractSearchItemTest, MissingBothBrackets) {
    std::string input = "КупитьSearchItem";
    std::string result = extractSearchItem(input);
    EXPECT_EQ(result, "Неправильный формат команды!");
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  
  return RUN_ALL_TESTS();
}