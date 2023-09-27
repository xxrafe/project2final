#include <stdio.h>
#include <tgbot/tgbot.h>
#include <sqlite3.h>
#include <fstream>
#include <algorithm>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include <vector>
#include <iostream>
#include <vector>
#include <string>
using namespace boost::placeholders;
using namespace TgBot;
using namespace std;


/**
 * @brief Создает таблицу данных, если таковой не существует
 * @param s Путь к базе данных
*/
static int createDB(const char* s)
{
    sqlite3* DB;
    int exit = 0;

    exit = sqlite3_open(s, &DB);

    sqlite3_close(DB);

    return 0;
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


/**
 * @brief Создает таблицу внутри БД
 * @param s Путь к базе данных
*/
static int createTable(const char* s)
{
    sqlite3* DB;

    std::string sql = "CREATE TABLE IF NOT EXISTS INFO("
    "ID      INT  PRIMARY KEY, "
    "BALANCE INT  NOT NULL, "
    "ADDRESS TEXT NOT NULL );";

    try 
    {
        int exit = 0;
        exit = sqlite3_open(s, &DB);

        char* messaggeError;
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);

        if (exit != SQLITE_OK) {
            cerr << "Error Create Table" <<endl;
            sqlite3_free(messaggeError);
        }
        else
            std::cout << "Table created Successfully" << endl;
        sqlite3_close(DB);
    }
    catch (const exception & e)
    {
        cerr << e.what();
    }

    return 0;
}


/**
 * @brief Функция вставляет данные пользователя в базу данных
 * @param chatid ID пользователя
 * @param s Путь к базе данных
 * @param value Значение города
*/
static int insertData(int chatid, const char* s, std::string value)
{
    sqlite3* DB;
    char* messaggeError;

    int exit = sqlite3_open(s, &DB);
    // string sql("INSERT INTO INFO (" + data + ") VALUES('" + value + "');");
    string sql("INSERT INTO INFO (ID, BALANCE, ADDRESS) VALUES('" + std::to_string(chatid) + "', '10000', '" + value + "');");

    exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
    if (exit != SQLITE_OK) {
        cerr << "Error Insert" << endl;
        sqlite3_free(messaggeError);
    }
    else
        cout << "Records created Succesfully!" << endl;

    return 0;
}


/**
 * @brief Проверяет есть ли пользователь в базе данных
 * @param userId ID пользователя
*/
bool checkUserExists(int userId) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db);
    
    if (result != SQLITE_OK) {
        std::cout << "Ошибка открытия базы данных: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    std::string query = "SELECT COUNT(*) FROM INFO WHERE ID = " + std::to_string(userId) + ";";
    sqlite3_stmt* statement;
    
    result = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, nullptr);
    
    if (result != SQLITE_OK) {
        std::cout << "Ошибка выполнения запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(statement);
        sqlite3_close(db);
        return false;
    }
    
    bool userExists = false;
    
    if (sqlite3_step(statement) == SQLITE_ROW) {
        int count = sqlite3_column_int(statement, 0);
        userExists = (count > 0);
    }
    
    sqlite3_finalize(statement);
    sqlite3_close(db);
    
    return userExists;
}


/**
 * @brief Функция для вывода ошибки в случае если доступ к базе данных закрыт
*/
int executeQuery(const char* query, sqlite3* db, int(*callback)(void*, int, char**, char**), void* data) {
    char* errMsg = nullptr;
    int result = sqlite3_exec(db, query, callback, data, &errMsg);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
 
    return result;
}

/**
 * @brief Функция возвращает значение баланса пользователя из базы данных
 * @param userID ID пользователя
 * @param balanceChange Значение на которое изменится баланс (если число положительное, добавляет это значение к балансу, если отрицательное - отнимает)
*/
void changeBalanceByID(int userID, int balanceChange) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db);
 
    if (result != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных" << std::endl;
        return;
    }
    std::string query = "UPDATE INFO SET BALANCE = BALANCE + " + std::to_string(balanceChange) + " WHERE ID = " + std::to_string(userID);
 
    result = executeQuery(query.c_str(), db, nullptr, nullptr);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса" << std::endl;
    } else {
        std::cout << "Баланс пользователя с ID " << userID << " успешно изменен" << std::endl;
    }
 
    sqlite3_close(db);
}

 

/**
 * @brief Колбэк-функция для обработки результата запроса
*/
int balanceCallback(void* data, int argc, char** argv, char** columnNames) {
    if (argc > 0) {
        // Получаем значение balance
        std::string balance = argv[0] ? argv[0] : "0";
 
        // Преобразуем значение в число и сохраняем его в переменной, переданной через data
        int* balancePtr = static_cast<int*>(data);
        *balancePtr = std::stoi(balance);
    }
 
    return 0;
}

/**
 * @brief Функция для получения значения balance пользователя по ID
 * @param userID ID пользователя
*/
int getBalanceByID(int userID) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db);
 
    if (result != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных" << std::endl;
        return 0;
    }
 
    std::string query = "SELECT balance FROM INFO WHERE ID = " + std::to_string(userID);
 
    int balance = 0;
    result = executeQuery(query.c_str(), db, balanceCallback, &balance);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса" << std::endl;
        balance = 0;
    }
 
    sqlite3_close(db);
    return balance;
}
 

/**
 * @brief Колбэк функция для обработки результата запроса
*/
int addressCallback(void* data, int argc, char** argv, char** columnNames) {
    if (argc > 0) {
        // Получаем значение ADDRESS
        std::string address = argv[0] ? argv[0] : "";
 
        // Сохраняем значение в переменной, переданной через data
        std::string* addressPtr = static_cast<std::string*>(data);
        *addressPtr = address;
    }
 
    return 0;
}

/**
 * @brief Функция для получения значения ADRESS по IP
 * @param userID ID пользователя
*/
std::string getAddressByID(int userID) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db);
 
    if (result != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных" << std::endl;
        return "";
    }
 
    std::string query = "SELECT ADDRESS FROM INFO WHERE ID = " + std::to_string(userID);
 
    std::string address = "";
    result = executeQuery(query.c_str(), db, addressCallback, &address);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса" << std::endl;
        address = "";
    }
 
    sqlite3_close(db);
    return address;
}
 

/**
 * @brief Функция для сортироваки массива строк по алфавиту
 * @param strings Массив из строк
*/
std::string sortStrings(const std::vector<std::string>& strings) {
    std::vector<std::string> sortedStrings = strings; // Создаем копию входного массива
 
    // Используем функцию std::sort для сортировки строк
    std::sort(sortedStrings.begin(), sortedStrings.end());
    std::string result = "";
    // Выводим отсортированные строки по одной на каждую
    for (const std::string& str : sortedStrings) {
        result += str;
    }
 
    return result;
}

/**
 * @brief Функция для сортировки товаров из txt файла
 * @param filepath Путь к базе данных
 * @param case1 Выбор между сортировкой по алфавиту/цене (0 - по алфавиту, 1 - по цене)
*/
std::string SortProducts(std::string filepath, int case1){
    std::string result = "";
    std::string finalresult = "";
    vector <std::string> product;
    vector <int> productprice;
    fstream fin;
    fin.open(filepath);
    std::string s1, s2;
    while (!fin.eof())
    {
        fin >> s1;
        fin >> s2;
        std::cout << s1 << " " << s2 << endl;
        product.push_back(s1);
        productprice.push_back(atoi(s2.c_str()));
    }
    fin.close();
    if (case1==1)
    {
        for (int i = 0; i < productprice.size(); i++) {
            for (int j = 0; j < productprice.size() - i; j++) {
                if (productprice[j] > productprice[j + 1]) {
                    // меняем элементы местами
                    int temp1 = productprice[j];
                    std::string temp2 = product[j];
                    product[j] = product[j+1];
                    product[j+1] = temp2;
                    productprice[j] = productprice[j + 1];
                    productprice[j + 1] = temp1;
                }
            }
        }
        for (int i = 0; i < product.size(); i++){
            result = result + product[i] + " - " + std::to_string(productprice[i]) + " RUB\n";
        }
        finalresult = result;
    }
    if (case1==0)
    {
        std::vector <std::string> output;
        
        for (int i = 0; i < product.size(); i++){
            result = product[i] + " - " + std::to_string(productprice[i]) + " RUB\n";
            output.push_back(result);
            cout << result;
        }
        result = sortStrings(output);
        finalresult = result;
    }
    return finalresult;
}

/**
 * @brief Функция для обработки команды /start
 * @param bot Бот
 * @param message Отправленное ообщение
*/
void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Добро пожаловать!\nКоманды:\n/balance - проверить баланс\n/products - посмотреть список товаров(отсортированные по алфавиту)\n/productsprice - посмотреть список товаров(отсортированные по цене)\n/pay - пополнить баланс на 1000 RUB\nДля покупки товара введите <купить (Товар)>\nДля поиска товара введите <поиск (Товар)>\n\nДля доставки товаров введите город проживания:");
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/") or (StringTools::startsWith(message->text, "/"))) {
            return;
        }
        int chatid = message->chat->id;
        if (!checkUserExists(chatid)){
            std::string city = message->text;
            int chatid = message->chat->id;
            insertData(chatid, "USERS.db", city);
            bot.getApi().sendMessage(message->chat->id, "Город в котором вы проживаете: " + city);
        }
        return;
    });
    return;
}
 

/**
 * @brief Функция для обработки команды /balance
 * @param bot Бот
 * @param message Отправленное сообщение
*/
void handleBalanceCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    // Здесь необходимо получить баланс пользователя из базы данных и отправить его в сообщении
    int chatid = message->chat->id;
    std::string balance = std::to_string(getBalanceByID(chatid));
    bot.getApi().sendMessage(message->chat->id, "Ваш баланс: " + balance);
}

/**
 * @brief Функция для обработки команды /pay
 * @param bot Бот
 * @param message Отправленное сообщение
*/
void handlePayCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    int chatid = message->chat->id;
    changeBalanceByID(chatid, 1000);
    int balance = getBalanceByID(chatid);
    bot.getApi().sendMessage(message->chat->id, "Вы пополнили баланс на 1000 RUB\nВаш баланс: " + std::to_string(balance) + " RUB");
}
 
/**
 * @brief Функция для обработки команды /productsprice
 * @param bot Бот
 * @param message Отправленное сообщение
*/
void handleProductsPriceCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    std::string products = "";
    products = "Вот список товаров:\n" + SortProducts("products.txt", 1);
    bot.getApi().sendMessage(message->chat->id, products);
}

/**
 * @brief Функция для обработки команды /products
 * @param bot Бот
 * @param message Отправленное сообщение
*/
void handleProductsCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    std::string products = "";
    products = "Вот список товаров:\n" + SortProducts("products.txt", 0);
    bot.getApi().sendMessage(message->chat->id, products);
}
 
 

/**
 * @brief Функция для обработки текстовых сообщение (поиск и покупка товаров)
 * @param bot Бот
 * @param message Введенное сообщение
*/
void handleTextMessage(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    std::string text = message->text;
    std::string product = "";
    product = extractSearchItem(text);
    if (StringTools::startsWith(message->text, "/")){
        int a = 0;
    }
    else {
        if (product == "Неправильный формат команды!"){
            bot.getApi().sendMessage(message->chat->id, product);
        }
        else{
            if (StringTools::startsWith(message->text, "поиск")){
                product = extractSearchItem(text);
                if (product == "Неправильный формат команды!"){
                    bot.getApi().sendMessage(message->chat->id, product);
                }
                else
                {
                std::cout << product;
                fstream fin;
                fin.open("products.txt");
                std::string s1, s2;
                int ifFound = 0;
                while (!fin.eof() and product != s1)
                {
                    fin >> s1;
                    fin >> s2;
                    if (product == s1){
                        ifFound += 1;
                    }
                }
                if (ifFound == 0){
                    bot.getApi().sendMessage(message->chat->id, "Товар не найден!");
                } else {
                    bot.getApi().sendMessage(message->chat->id, "Товар: " + s1 + "\nцена товара: " + s2);
                }
                }
            }
            if (StringTools::startsWith(message->text, "купить")){
                std::cout << product;
                fstream fin;
                fin.open("products.txt");
                std::string s1, s2;
                int ifFound = 0;
                while (!fin.eof() and product != s1)
                {
                    fin >> s1;
                    fin >> s2;
                    if (product == s1){
                        ifFound += 1;
                    }
                }
                if (ifFound == 0){
                    bot.getApi().sendMessage(message->chat->id, "Товар не найден!");
                } else {
                    if ( -atoi(s2.c_str()) > getBalanceByID(message->chat->id)){
                        bot.getApi().sendMessage(message->chat->id, "Не достаточно средств на балансе!");
                        return;
                    }
                    changeBalanceByID(message->chat->id, -atoi(s2.c_str()));
                    bot.getApi().sendMessage(message->chat->id, "Товар успешно куплен! Товар доставят по адресу г. " + getAddressByID(message->chat->id));
                }
            }
        }
    }
}

int main(){
 
    // Токен бота
    TgBot::Bot bot("6134520934:AAFw27MZL3WP0y26extR9VDrjNqJFZC9L5o");
    
    //Работа с базами данных
    const char* dir = "USERS.db"; 
    sqlite3* DB;
    createDB(dir);
    createTable(dir);

    // Установка обработчиков команд
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) { handleStartCommand(bot, message); });
    bot.getEvents().onCommand("balance", [&bot](TgBot::Message::Ptr message) { handleBalanceCommand(bot, message); });
    bot.getEvents().onCommand("productsprice", [&bot](TgBot::Message::Ptr message) { handleProductsPriceCommand(bot, message); });
    bot.getEvents().onCommand("products", [&bot](TgBot::Message::Ptr message) { handleProductsCommand(bot, message); });
    bot.getEvents().onCommand("pay", [&bot](TgBot::Message::Ptr message) { handlePayCommand(bot, message); });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) { handleTextMessage(bot, message); });
 
    // Запуск бота
    try {
        bot.getApi().deleteWebhook();
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        std::cerr << "Telegram bot error: " << e.what() << std::endl;
    }
 
    return 0;
}
