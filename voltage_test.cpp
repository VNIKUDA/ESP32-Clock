// --------- ГОДИННИК ---------
//
// Що він може?
//    У годинника є три екрани: годинник, меню та налаштування.
//    Є будильник з сигналом. Можна налаштовувати час, дату, час будильника та
//    його стан (включений або виключений)
//
// Як вийти в меню?
//    Щоб потрапити на екран меню треба затиснути кнопку SET.
//    Звідти можна виконати наступні дії:
//      - Повернутись на годинник, тобто вийти з меню.
//      - Налаштувати поточний час
//      - Налаштувати поточну дату
//      - Налаштувати статус будильника
//      - Налаштувати час будильника
//    Між діями можна переміщатись на кнопки UP та DOWN.
//    Щоб підтвердити будь-яку дію в меню треба затиснути кнопку SET.
//    Після вибору дії вас перенесе не меню налаштування (або на годинник
//    якщо ви вибрали вийти).
//
// ----------------------------

// Бібліотеки
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

// Змінні для відстеження зміни часу
long currentTime, previousTime;

// Мілісекунд в секунді
#define MILLI_TO_SECOND 1000.0
#define MICRO_TO_SECOND 1000000.0

// Режим (годинник + будильник, вибір налаштування, меню налаштування)
int mode;

// Налаштування будильника
bool alarm_on = true;
bool alarm_playing = false;
int alarm_hours = 12, alarm_minutes = 0, alarm_seconds = 5;

// Налаштування поточного часу годинника
float hours = 12, minutes = 0, seconds = 0;

// Налаштування поточної дати
int date = 20, month = 4, year = 2025;

// Список днів в кожному місяці (для лютого йде перевірка в коді)
int monthDays[] = {
    31, // Січень
    28, // Лютий
    31, // Березень
    30, // Квітень
    31, // Травень
    30, // Червень
    31, // Липень
    31, // Серпень
    30, // Вересень
    31, // Жовтень
    30, // Листопад
    31  // Грудень
};

// Список налаштувань та дій для меню вибору налаштування
String options[] = {
    "Alarm Time", "Alarm Status", "Date", "Time", "Exit"};
int menu_option = 4; // Поточний вибір

// Список полів для кожного меню налаштування
float *timeFields[] = {&hours, &minutes, &seconds};
int *dateFields[] = {&date, &month, &year};
int *alarmFields[] = {&alarm_hours, &alarm_minutes, &alarm_seconds};

int fieldCount[] = {3, 1, 3, 3}; // Кількість полів на кожному меню налаштування
int current_field = 0;           // Поточне поле налаштування

// Поріг зарахування натиску кнопки на певну дію
#define MENU_ACTIVATION_THRESHOLD 1000 // Час для викликання SET MENU та підтвердження вибору\налаштувань
#define ACTION_THRESHOLD 60            // Час для зарахування натиску кнопки

// Клас кнопки (для легшості роботи з ними та зменшеню повторення коду)
class Button
{
private:
    int pin; // Цифровий порт, до якого підключена кнопка

    bool clicked = false; // Чи кнопка натиснута (тільк перший раз)
    bool hold = false;    // Чи кнопка затиснута
    bool pressed = false; // Чи кнопка натиснута (взагалі)
    int pressedTime = 0;  // Час з початку натиску кнопки в мілісекундах

    int action = LOW;       // Сигнал з цифрового порта
    int lastAction = LOW;   // Минулий сигнал з цифрового порта
    int lastActionTime = 0; // Час минулої зміни сигналу з цифрового порта

public:
    // Конструктор. Приймає цифровий порт, до якого приєднана кнопка
    Button(int pin)
    {
        this->pin = pin;
    };

    // Ініціалізація кнопки шляхом встановлення режиму роботи порта на вхід\зчитування
    void init()
    {
        pinMode(pin, INPUT);
    }

    // Оновлення данних про кнопку
    void update()
    {
        // Читання поточного сигналу з цифрового порта
        action = digitalRead(this->pin);

        // Якщо сигнал відрізняється від попереднього, то оновити час останої дії
        // кнопки та останього сигналу.
        if (action != lastAction)
        {
            lastAction = action;
            lastActionTime = currentTime;
        }

        // Якщо поточний сигнал високий (тобто кнопка затиснута) і час з останьої дії
        // є більшим за час для зарахування натиску
        if (action == HIGH && currentTime - lastActionTime >= ACTION_THRESHOLD)
        {

            // Якщо це вже друга обробка, то кажемо що кнопка вже затиснута, а не натиснута
            // перевіряти коли кнопка була натиснута, а не затиснута
            if (this->hold == false && this->clicked == true)
            {
                this->hold = true;
                this->clicked = false;
            }
            // Перевіряємо чи це перша обробка, для того щоб мати змогу
            if (this->pressed == false && this->clicked == false)
            {
                this->clicked = true;
                this->hold = false;
            }

            // Кажемо, що кнопка взагалі натиснута (не важливо чи затиснута чи натиснута вперше)
            this->pressed = true;
        }
        // Якщо ні, то кажемо що кнопка взагалі не натиснута і час натиску відповідно
        // дорівнює 0
        else
        {
            this->pressed = false;
            this->pressedTime = 0;
            this->hold = false;
            this->clicked = false;
        }

        // Якщо конпка взагалі натиснута, то відслідковуємо час скільки вона є натиснутою
        if (this->pressed)
        {
            pressedTime = currentTime - lastActionTime;
        }
    }

    // Методи-інтерфейс для отримання данних з кнопки

    bool getPressed()
    {
        return this->pressed;
    }

    int getPressedTime()
    {
        return this->pressedTime;
    }

    bool getClicked()
    {
        return this->clicked;
    }

    bool getHold()
    {
        return this->hold;
    }

    // Скидання стану кнопки, крім стану натиску взагалі. Використовується для
    // того, щоб, наприклад, коли користувач затиснув на вихід в меню його одразу не
    // перекинуло назад в меню (якщо стан затиску і час натиску залишиться такими ж,
    // тоді програма подумає що користувач спеціально затиснув кнопку на екрані годиника
    // і хоче в меню)
    void reset()
    {
        this->pressedTime = 0;
        this->hold = false;
        this->clicked = false;
    }
};

// ДисплеЇ
Adafruit_SSD1306 leftOled(128, 64, &Wire);
Adafruit_SSD1306 rightOled(128, 64, &Wire);

// Датчик температури
Adafruit_BMP280 bmp;

// Кнопки
Button upButton(0);
Button setButton(1);
Button downButton(2);

#define PIEZO 3 // Цифровий порт для пієзодинаміка

// Функція для відмальовки вертикальної стрілки
void drawVArrow(int x, int y, int w, int h, int thickness, int direction, Adafruit_SSD1306 *display, int color = WHITE)
{
    for (int i = 0; i < thickness; i++)
    {
        if (direction == 1)
        {
            display->drawLine(x, y + h + i, x + w / 2, y + i, color);
            display->drawLine(x + w / 2, y + i, x + w, y + h + i, color);
        }
        else
        {
            display->drawLine(x, y + i, x + w / 2, y + h + i, color);
            display->drawLine(x + w / 2, y + h + i, x + w, y + i, color);
        }
    }
}

// ЕКРАН ГОДИННИКА
// Оновлює час
void timeUpdate()
{
    if (seconds >= 60)
    {
        seconds = 0;
        minutes += 1;
    }
    if (minutes >= 60)
    {
        minutes = 0;
        hours += 1;
    }
}

// Оновлює дату
void dateUpdate()
{
    if (hours >= 24)
    {
        hours = 0;
        date += 1;
        if (month == 2)
        {
            monthDays[1] = (year % 4 == 0) ? 29 : 28;
        }
    }
    if (date > monthDays[month - 1])
    {
        date = 1;
        month += 1;
    }
    if (month > 12)
    {
        month = 1;
        year += 1;
    }
}

// Оновлення годиника в цілому
void clockUpdate()
{
    // Якщо кнопка меню затиснута, то перейти на екран меню
    if (setButton.getHold() && setButton.getPressedTime() >= MENU_ACTIVATION_THRESHOLD)
    {
        menu_option = 4;
        mode = 1;
        setButton.reset();
    }

    // Розрахунок скільки секунд пройшло з минулого оновлення
    float deltaTime = currentTime - previousTime;
    seconds += deltaTime / MILLI_TO_SECOND;

    // Оновлення даних годиника
    timeUpdate();
    dateUpdate();

    // Керування будильником
    // Якщо булильник грає та кнопка меню натиснута, то зупинити будильник
    if (setButton.getPressed() && alarm_playing)
    {
        alarm_playing = false;
        setButton.reset();
    }
    // Якщо час співпадає з часом будильника, то запустити будильник
    if ((int)seconds == alarm_seconds && minutes == alarm_minutes && hours == alarm_hours && alarm_on)
        alarm_playing = true;

    // Ввімкнення та вимкення пієзодинаміка для створення того самого
    // пікання :)
    if (alarm_playing && currentTime % 500 > 250)
        analogWrite(PIEZO, 150);
    else
        analogWrite(PIEZO, 0);

    previousTime = currentTime;
}

// Відмальовка екрану годиника з будильником
void displayClock()
{
    if (!alarm_playing)
    {
        leftOled.setTextSize(4);
        rightOled.setTextSize(2);

        for (int i = 0; i < 2; i++)
        {
            leftOled.setCursor(2 + i, 16);
            leftOled.print((hours < 10) ? '0' + String((int)hours) : String((int)hours));
            leftOled.print(":");
            leftOled.print((minutes < 10) ? '0' + String((int)minutes) : String((int)minutes));
        }

        rightOled.setCursor(7, 8);
        rightOled.print((date < 10) ? '0' + String(date) : String(date));
        rightOled.print(".");
        rightOled.print((month < 10) ? '0' + String(month) : String(month));
        rightOled.print(".");
        rightOled.print(year);

        rightOled.drawFastHLine(0, 31, 128, WHITE);

        rightOled.setCursor(33, 41);
        rightOled.print(String(bmp.readTemperature()));
        rightOled.print("C");
    }
    else
    {
        leftOled.setTextSize(2);
        rightOled.setTextSize(2);

        leftOled.drawRect(4, 12, 120, 2, WHITE);
        leftOled.setCursor(33, 26);
        leftOled.print("ALARM");
        leftOled.drawRect(4, 52, 120, 2, WHITE);

        rightOled.setCursor(15, 26);
        rightOled.print((alarm_hours < 10) ? '0' + String(alarm_hours) : String(alarm_hours));
        rightOled.print(":");
        rightOled.print((alarm_minutes < 10) ? '0' + String(alarm_minutes) : String(alarm_minutes));
        rightOled.print(":");
        rightOled.print((alarm_seconds < 10) ? '0' + String(alarm_seconds) : String(alarm_seconds));
    }
}

// ЕКРАН МЕНЮ
// Оновлення та обробка кнопок в меню
void menuUpdate()
{
    // Піднятися вверх по списку дій
    if (upButton.getClicked() && menu_option < 4)
        menu_option++;

    // Спуститися вниз по списку дій
    if (downButton.getClicked() && menu_option > 0)
        menu_option--;

    // Якщо кнопка затиснута (тобто користувач підтвердив вибір)
    if (setButton.getHold() && setButton.getPressedTime() >= MENU_ACTIVATION_THRESHOLD)
    {
        // Скидаємо стан кнопки
        setButton.reset();

        // Якщо вибір був вийти, топ повертаємось на годинник
        if (menu_option == 4)
        {
            mode = 0;
        }
        // Якщо ні, то переходимо на екран налаштувань
        else
        {
            mode = 2;
            current_field = 0;
        }

        // Оновлення часу (потрібно, щоб годинник не збився поки він у меню)
        float deltaTime = currentTime - previousTime;
        float deltaSeconds = deltaTime / MILLI_TO_SECOND;
        while (deltaSeconds > 60)
        {
            float timeToMinute = 60.0 - seconds;
            deltaSeconds -= timeToMinute;
            seconds += timeToMinute;
            timeUpdate();
            dateUpdate();
        }
        seconds += deltaSeconds;

        if (seconds > 60)
        {
            deltaSeconds = seconds - 60;
            timeUpdate();
            dateUpdate();
            seconds += deltaSeconds;
        }
        timeUpdate();
        dateUpdate();

        previousTime = currentTime;
    }
}

// Відмалювати екран меню
void displayMenu()
{
    leftOled.setTextSize(2);
    rightOled.setTextSize(1);

    leftOled.drawRect(4, 12, 120, 2, WHITE);
    leftOled.setCursor(15, 26);
    leftOled.print("SET MENU");
    leftOled.drawRect(4, 52, 120, 2, WHITE);

    for (int option = 4; option >= 0; option--)
    {
        rightOled.setCursor(12, 5 + 8 * (4 - option) + 4 * (4 - option));
        rightOled.print(options[option]);
    }

    rightOled.drawRect(5, 2 + 8 * (4 - menu_option) + 4 * (4 - menu_option), 104, 12, WHITE);
}

// ЕКРАН НАЛАШТУВАННЯ
// Оновлення екрану налаштувань та обробка кнопок
void actionMenuUpdate()
{
    // Повисити дату, хвилини, місяць, тощо.
    if (upButton.getClicked() || upButton.getHold() && upButton.getPressedTime() > MILLI_TO_SECOND)
    {
        if (menu_option == 3)
            (*timeFields[current_field])++;
        else if (menu_option == 2)
            (*dateFields[current_field])++;
        else if (menu_option == 1)
            alarm_on = (alarm_on) ? false : true;
        else if (menu_option == 0)
            (*alarmFields[current_field])++;
    }

    // Понизити дату, хвилини, місяць, тощо.
    if (downButton.getClicked() || downButton.getHold() && downButton.getPressedTime() > MILLI_TO_SECOND)
    {
        if (menu_option == 3)
            (*timeFields[current_field])--;
        else if (menu_option == 2)
            (*dateFields[current_field])--;
        else if (menu_option == 1)
            alarm_on = (alarm_on) ? false : true;
        else if (menu_option == 0)
            (*alarmFields[current_field])--;
    }

    // Переключення між полям (якщо я, наприклад, зараз на годинах, то після натисику
    // буду на хвилинах і тд). Переключення цеклічне, тобто коли досягнуто останнє поле
    // перше поле стає поточним полем.
    if (setButton.getClicked())
    {
        current_field++;
        if (current_field >= fieldCount[menu_option])
        {
            current_field = 0;
        }
    }

    // Якщо кнопку SET затиснуто, то користувач підтверджує налаштування та можна переходити
    // Назад на екран меню
    if (setButton.getHold() && setButton.getPressedTime() >= MENU_ACTIVATION_THRESHOLD)
    {
        mode = 1;
        setButton.reset();
    }

    // Ціклічне поводження кожного поля (якщо хвилина 59, то натиснувши на кнопку
    // UP хвилина стане на занчення 00, і так для всіх полів)
    switch (menu_option)
    {
    case 3:
        if (mode == 1)
            previousTime = currentTime;

        if (seconds < 0)
            seconds = 59;
        else if (seconds >= 60)
            seconds = 0;

        if (minutes < 0)
            minutes = 59;
        else if (minutes >= 60)
            minutes = 0;

        if (hours < 0)
            hours = 23;
        else if (hours >= 24)
            hours = 0;
        break;
    case 2:
        if (month == 2)
            monthDays[1] = (year % 4 == 0) ? 29 : 28;
        else if (month < 1)
            month = 12;
        else if (month > 12)
            month = 1;

        if (date > monthDays[month - 1])
            date = 1;
        else if (date < 1)
            date = monthDays[month - 1];
        break;
    case 0:
        if (alarm_seconds < 0)
            alarm_seconds = 59;
        else if (alarm_seconds >= 60)
            alarm_seconds = 0;

        if (alarm_minutes < 0)
            alarm_minutes = 59;
        else if (alarm_minutes >= 60)
            alarm_minutes = 0;

        if (alarm_hours < 0)
            alarm_hours = 23;
        else if (alarm_hours >= 24)
            alarm_hours = 0;
        break;
    }
}

// Відмальовування екрану налаштувань
void displayActionMenu()
{
    switch (menu_option)
    {
    case 3:
        leftOled.setTextSize(2);
        rightOled.setTextSize(2);

        leftOled.drawRect(4, 12, 120, 2, WHITE);
        leftOled.setCursor(40, 26);
        leftOled.print("TIME");
        leftOled.drawRect(4, 52, 120, 2, WHITE);

        rightOled.setCursor(15, 26);
        rightOled.print((hours < 10) ? '0' + String((int)hours) : String((int)hours));
        rightOled.print(":");
        rightOled.print((minutes < 10) ? '0' + String((int)minutes) : String((int)minutes));
        rightOled.print(":");
        rightOled.print((seconds < 10) ? '0' + String((int)seconds) : String((int)seconds));

        drawVArrow(19 + 36.3 * current_field, 10, 13, 8, 3, 1, &rightOled);
        drawVArrow(19 + 36.3 * current_field, 45, 13, 8, 3, -1, &rightOled);

        break;

    case 2:
        leftOled.setTextSize(2);
        rightOled.setTextSize(2);

        leftOled.drawRect(4, 12, 120, 2, WHITE);
        leftOled.setCursor(40, 26);
        leftOled.print("DATE");
        leftOled.drawRect(4, 52, 120, 2, WHITE);

        rightOled.setCursor(7, 26);
        rightOled.print((date < 10) ? '0' + String(date) : String(date));
        rightOled.print(".");
        rightOled.print((month < 10) ? '0' + String(month) : String(month));
        rightOled.print(".");
        rightOled.print(year);

        if (current_field == 0)
        {
            drawVArrow(10, 10, 13, 8, 3, 1, &rightOled);
            drawVArrow(10, 45, 13, 8, 3, -1, &rightOled);
        }
        else if (current_field == 1)
        {
            drawVArrow(46, 10, 13, 8, 3, 1, &rightOled);
            drawVArrow(46, 45, 13, 8, 3, -1, &rightOled);
        }
        else
        {
            drawVArrow(95, 10, 13, 8, 3, 1, &rightOled);
            drawVArrow(95, 45, 13, 8, 3, -1, &rightOled);
        }

        break;

    case 1:
        leftOled.setTextSize(2);
        rightOled.setTextSize(2);

        leftOled.drawRect(4, 12, 120, 2, WHITE);
        leftOled.setCursor(33, 26);
        leftOled.print("ALARM");
        leftOled.drawRect(4, 52, 120, 2, WHITE);

        if (alarm_on) {
            rightOled.setTextColor(BLACK);
            rightOled.fillRect(9, 20, 33, 24, WHITE);
            rightOled.setCursor(14, 26);
            rightOled.print("ON");

            rightOled.setTextColor(WHITE);
            rightOled.setCursor(69, 26);
            rightOled.print("OFF");
        } else {
            rightOled.setTextColor(WHITE);
            rightOled.setCursor(14, 26);
            rightOled.print("ON");
            
            rightOled.setTextColor(BLACK);
            rightOled.fillRect(64, 20, 42, 24, WHITE);
            rightOled.setCursor(69, 26);
            rightOled.print("OFF");
        }

        rightOled.setTextColor(WHITE);
        break;

    case 0:
        leftOled.setTextSize(2);
        rightOled.setTextSize(2);

        leftOled.drawRect(4, 12, 120, 2, WHITE);
        leftOled.setCursor(5, 26);
        leftOled.print("ALARM TIME");
        leftOled.drawRect(4, 52, 120, 2, WHITE);

        rightOled.setCursor(15, 26);
        rightOled.print((alarm_hours < 10) ? '0' + String(alarm_hours) : String(alarm_hours));
        rightOled.print(":");
        rightOled.print((alarm_minutes < 10) ? '0' + String(alarm_minutes) : String(alarm_minutes));
        rightOled.print(":");
        rightOled.print((alarm_seconds < 10) ? '0' + String(alarm_seconds) : String(alarm_seconds));

        drawVArrow(19 + 36.3 * current_field, 10, 13, 8, 3, 1, &rightOled);
        drawVArrow(19 + 36.3 * current_field, 45, 13, 8, 3, -1, &rightOled);

        break;
    }
}

// Setup. Налаштування цифрових портів, ініціалізація усіх об'єктів та встановлення
// початкових параметрів роботи
void setup()
{
    esp_sleep_enable_timer_wakeup(0.2 * MICRO_TO_SECOND);
    pinMode(LED_BUILTIN, INPUT);
    pinMode(21, OUTPUT);
    digitalWrite(21, LOW);

    pinMode(PIEZO, OUTPUT);

    setButton.init();
    upButton.init();
    downButton.init();

    Wire.begin(8, 10);

    bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

    bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL,
        Adafruit_BMP280::SAMPLING_X16,
        Adafruit_BMP280::SAMPLING_NONE,
        Adafruit_BMP280::FILTER_OFF,
        Adafruit_BMP280::STANDBY_MS_4000);

    leftOled.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    rightOled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    leftOled.ssd1306_command(SSD1306_SETCONTRAST);
    leftOled.ssd1306_command(1);
    rightOled.ssd1306_command(SSD1306_SETCONTRAST);
    rightOled.ssd1306_command(1);

    leftOled.setTextSize(1);
    leftOled.setTextColor(WHITE);
    leftOled.cp437(true);
    rightOled.setTextSize(4);
    rightOled.setTextColor(WHITE);
    rightOled.cp437(true);

    currentTime = 0;
    previousTime = 0;

    mode = 0;
}

// Цикл програми
void loop()
{
    leftOled.clearDisplay();
    leftOled.setCursor(0, 0);

    float voltage = ((analogRead(4) / 4095.0) * 3.3) - 0.3;

    leftOled.printf("Voltage: %f\n", voltage);
    leftOled.display();
    delay(1000);
}