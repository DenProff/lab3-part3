#include <iostream>
#include <set>
#include <string>
#include <windows.h>
#include <stack>
using namespace std;

// Структура узла дерева
struct TreeNode
{
    string info; // Операция или операнд
    TreeNode* left;
    TreeNode* right;
    TreeNode(string value) : info(value), left(nullptr), right(nullptr) {}
};

// Глобальные переменные
string expressionString; // Строка с выражением
int position = 0; // Номер текущего символа
char scanSymbol; // Текущий символ
set<char> Digit{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }; // Множество цифр
set<char> NoNullDigit{ '1', '2', '3', '4', '5', '6', '7', '8', '9' }; // Множество цифр без нуля
set<char> Letter{}; // Множество заглавных и строчных букв латинского алфавита (заполняется в начале работы программы)

TreeNode* Expression();

// Функция для создания нового узла
TreeNode* CreateNode(string val)
{
    return new TreeNode(val);
}

void ClearTree(TreeNode*& root) //Очистка дерева
{
    if (root == nullptr) //Дерево пустое
        return;

    ClearTree(root->left); //Рекурсивно удаляем левое поддерево

    ClearTree(root->right); //Рекурсивно удаляем правое поддерево

    delete root; //Удаляем текущий узел
    root = nullptr; //Обнуляем указатель
}

void NextSymbol() // Смещение на следующий символ строки
{
    if (position < expressionString.length() - 1)
    {
        position += 1;
        scanSymbol = expressionString[position];
    }
    else
        scanSymbol = '\0'; // Конец строки
}

// Проверка на принадлежность множеству цифр без нуля
bool CheckNoNullDigit()
{
    return (NoNullDigit.count(scanSymbol));
}

// Проверка на принадлежность множеству цифр
bool CheckDigit()
{
    return (Digit.count(scanSymbol));
}

// Проверка на принадлежность множеству букв
bool CheckLetter()
{
    return (Letter.count(scanSymbol));
}

// Функция для обработки переменной
TreeNode* Variable()
{
    string var;
    if (scanSymbol == '_')
    {
        var += scanSymbol;
        while (scanSymbol == '_')
        {
            var += scanSymbol;
            NextSymbol();
        }
        if (!CheckLetter()) // Недопустимое именование переменной
            throw runtime_error("Ошибка! Неверное именование переменной");
    }
    do
    {
        var += scanSymbol;
        NextSymbol();
    } while (CheckLetter() || CheckDigit() || scanSymbol == '_');

    return CreateNode(var);
}

// Функция для обработки числа
TreeNode* Number()
{
    string num;
    if (CheckDigit()) // Если первый символ цифра
    {
        if (scanSymbol == '0')
        {
            num += scanSymbol;
            NextSymbol();
            if (CheckDigit()) // Проверка на наличие ведущих нулей
                throw runtime_error("Ошибка! Ведущие нули у чисел недопустимы");
        }
        else
        {
            do
            {
                num += scanSymbol;
                NextSymbol();
            } while (CheckDigit());
        }

        if (scanSymbol == '.' || scanSymbol == ',')
        {
            num += scanSymbol;
            NextSymbol();
            if (CheckDigit())
            {
                do
                {
                    num += scanSymbol;
                    NextSymbol();
                } while (CheckDigit());
            }
        }
    }
    else // Если первый символ является разделителем
    {
        num += scanSymbol;
        NextSymbol();
        if (CheckDigit())
        {
            do
            {
                num += scanSymbol;
                NextSymbol();
            } while (CheckDigit());
        }
        else
            throw runtime_error("Ошибка! После знака разделителя в виде '.' или ',' должна быть исключительно цифра");
    }

    // Проверка на то, не заканчивается ли число буквой (недопустимо)
    if (CheckLetter())
        throw runtime_error("Ошибка! Число не может заканчиваться буквой");

    return CreateNode(num);
}

// Функция для обработки множителя
TreeNode* Factor()
{
    // Множитель начинается с символа отрицания
    if (scanSymbol == '!')
    {
        string operation = { scanSymbol };
        NextSymbol();
        TreeNode* node = CreateNode(operation);
        node->left = Factor();
        return node;
    }

    // Наличие знака множителя
    if (scanSymbol == '+' || scanSymbol == '-')
    {
        string operation = { scanSymbol };
        NextSymbol();
        TreeNode* node = CreateNode(operation);
        node->left = Factor();
        return node;
    }

    // Первый символ множителя соответствует числу
    if (CheckDigit() || scanSymbol == '.' || scanSymbol == ',')
        return Number();

    // Первый символ множителя соответствует переменной
    if (CheckLetter() || scanSymbol == '_')
        return Variable();

    // Первый символ множителя соответствует выражению
    if (scanSymbol == '(')
    {
        NextSymbol();
        TreeNode* node = Expression();
        if (scanSymbol == ')')
            NextSymbol();
        else
            throw runtime_error("Ошибка! Отсутствует закрывающая скобка");
        return node;
    }

    // Множитель не распознан
    throw runtime_error("Ошибка! Множитель не распознается - неправильный символ");
}

// Функция для обработки слагаемого
TreeNode* Addend()
{
    TreeNode* node = Factor();
    while (scanSymbol == '*' || scanSymbol == '/')
    {
        string operation = { scanSymbol };
        NextSymbol();
        TreeNode* newNode = CreateNode(operation);
        newNode->left = node;
        newNode->right = Factor();
        node = newNode;
    }
    return node;
}

// Функция для обработки операции сравнения
TreeNode* Comparison()
{
    TreeNode* node = Addend();
    while (scanSymbol == '+' || scanSymbol == '-')
    {
        string operation = { scanSymbol };
        NextSymbol();
        TreeNode* newNode = CreateNode(operation);
        newNode->left = node;
        newNode->right = Addend();
        node = newNode;
    }
    return node;
}

// Функция для обработки операции равенства
TreeNode* Equality()
{
    TreeNode* node = Comparison();
    while (scanSymbol == '<' || scanSymbol == '>')
    {
        string operation = { scanSymbol };
        NextSymbol();
        if (scanSymbol == '=')
        {
            operation += scanSymbol;
            NextSymbol();
        }
        TreeNode* newNode = CreateNode(operation);
        newNode->left = node;
        newNode->right = Comparison();
        node = newNode;
    }
    return node;
}

// Функция для обработки логического множителя
TreeNode* LogicalFactor()
{
    TreeNode* node = Equality();
    while (scanSymbol == '=' || scanSymbol == '!')
    {
        string operation = { scanSymbol };

        char temp = scanSymbol;
        NextSymbol();
        if (temp == '!' && scanSymbol != '=')
            throw runtime_error("Ошибка! Недопустимый знак операции равенства");
        if (temp == '!')
        {
            operation += scanSymbol;
            NextSymbol();
        }

        TreeNode* newNode = CreateNode(operation);
        newNode->left = node;
        newNode->right = Equality();
        node = newNode;
    }
    return node;
}

// Функция для обработки логического слагаемого
TreeNode* LogicalAddend()
{
    TreeNode* node = LogicalFactor();
    while (scanSymbol == '&')
    {
        string operation = { scanSymbol };
        NextSymbol();
        TreeNode* newNode = CreateNode(operation);
        newNode->left = node;
        newNode->right = LogicalFactor();
        node = newNode;
    }
    return node;
}

// Функция для обработки выражения
TreeNode* Expression()
{
    TreeNode* node = LogicalAddend();
    while (scanSymbol == '|')
    {
        string operation = { scanSymbol };
        NextSymbol();
        TreeNode* newNode = CreateNode(operation);
        newNode->left = node;
        newNode->right = LogicalAddend();
        node = newNode;
    }
    return node;
}

// Функция для вывода дерева в одну строку (Обратная Польская Запись)
void PrintExpression(TreeNode* root)
{
    if (root != nullptr)
    {

        // Обходим левое поддерево
        PrintExpression(root->left);

        // Обходим правое поддерево
        PrintExpression(root->right);

        // Выводим текущий оператор или операнд
        cout << root->info << " ";
    }
}

int main()
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    // Заполнение множества заглавными буквами
    for (int i = 0; i < 26; ++i)
        Letter.insert('A' + i);

    // Заполнение множества строчными буквами
    for (int i = 26; i < 52; ++i)
        Letter.insert(tolower('A' + i - 26));

    // Выражение для обработки
    expressionString = { "a+b" };
    position = 0;
    scanSymbol = expressionString[position];

    TreeNode* root = nullptr;
    try
    {
        root = Expression();
        // Дошли до конца строки
        if (scanSymbol == '\0')
        {
            cout << "Выражение синтаксически верно" << endl;
            cout << "\nОбратная польская запись: ";
            PrintExpression(root);
            cout << endl;
        }
        else
        {
            // Остановились на закрывающей скобке => отсутствует открывающая
            if (scanSymbol == ')')
                throw runtime_error("Ошибка! Отсутствует открывающая скобка");

            // Не дошли до конца строки => лишние символы
            else
            {
                cout << "В выражении присутствуют лишние символы"
                    << "\nОшибка возникла при разборе символа '"
                    << expressionString[position] << "' на позиции " << position + 1 << endl;
            }
        }
    }
    catch (const exception& ex) // При разборе возникла ошибка
    {
        if (root != nullptr)
            ClearTree(root);
        cerr << ex.what() << "\nОшибка возникла при разборе символа '"
            << expressionString[position] << "' на позиции " << position + 1 << endl;
    }
}