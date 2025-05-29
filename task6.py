# Бібліотеки
import numpy as np

# Читаємо файл
with open("data.txt", "r") as file:
    numbers_reading = file.readlines() # Дані з файлу

# Через бібліотеку numpy
# Переводимо дані з файлу в числа та створюємо numpy масив
numbers = [int(number) for number in numbers_reading]
numbers = np.array(numbers, dtype=np.longlong)

# Виводимо добуток всіх чисел в масиві
print(f"Numpy product of the numbers: {numbers.prod()}")

# Або якщо чистим python
# Переводимо дані з файлу в числа
numbers = [int(number) for number in numbers_reading]
product = 1 # Змінна, в яку буде записаний добуток чисел

# Множемо кожне число
for number in numbers:
    product *= number

# Виводимо добуток всіх чисел
print(f"Python product of the numbers: {product}")