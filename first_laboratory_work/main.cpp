#include <iostream>

class Calculator {
public:
    int add(int a, int b) {
        return a + b;
    }
};

int main() {
    Calculator calc;
    int value1, value2;
    std::cout << "Введите первое значение: ";
    std::cin >> value1;
    std::cout << "Введите второе значение: ";
    std::cin >> value2;

    int result = calc.add(value1, value2);
    std::cout << "Сумма " << value1 << " и " << value2 << " равна " << result << std::endl;

    return 0;
}