// Бібліотеки
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

// Програма
int main() {

    // Відкриваємо файл
    ifstream numberFile("data.txt");

    // Змінна для читання рядків з файлу
    string numberReading;

    // Змінна яка зберігає добуток чисел
    // Тип "long long" використовується для збільшення максимального значення, яке можна зберігати в змінній до overflow (long long overflow),
    // тобто коли значення досягне більше максимального значення (для long long це 2^64 - 1), воно стане найменишим значенням (для long 
    // long - (2^64) )
    long long product = 1;

    // Перевіряємо чи файл нормально відкрився
    if (numberFile.is_open()) {
        // Для кожного рядка з файлу: перетворити рядок в число та помножити на загальний добуток
        while (getline(numberFile, numberReading)) {
            long long number; // Змінна яка зберігає поточне число
            
            // Перетворення рядка, зчитаного в файлу у число
            stringstream stream(numberReading);
            stream >> number;

            // Множення поточного числа на поточний добуток
            product *= number; // 
        }
    }

    // Закриваємо файл
    numberFile.close();

    // Вивід добутку в консоль
    cout << "Product of the numbers is: " << product << endl;

    return 0;
}