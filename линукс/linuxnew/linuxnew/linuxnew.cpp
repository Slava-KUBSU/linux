#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

// Кроссплатформенные заголовки
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(a) _mkdir(a)
#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <dirent.h>
#define MKDIR(a) mkdir(a, 0755)
#endif

using namespace std;

// Константы
const string HISTORY_PATH = ".kubsh_history";
const string USERS_DIR = "users";

// Вспомогательные функции
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Пункт 9: Обработчик сигнала
void sighup_handler(int sig) {
    cout << "Configuration reloaded" << endl;
}

// Пункт 11: VFS для пользователей
void setup_users_vfs() {
    string home_dir;

#ifdef _WIN32
    const char* home_dir_c = getenv("USERPROFILE");
#else
    const char* home_dir_c = getenv("HOME");
#endif

    if (!home_dir_c) {
        cerr << "Cannot get home directory" << endl;
        return;
    }

    home_dir = home_dir_c;

#ifdef _WIN32
    string users_path = home_dir + "\\" + USERS_DIR;
#else
    string users_path = home_dir + "/" + USERS_DIR;
#endif

    // Создаем каталог если не существует
    if (MKDIR(users_path.c_str()) != 0) {
        if (errno != EEXIST) {
            cerr << "Cannot create directory " << users_path << endl;
            return;
        }
    }

    cout << "Users VFS created at: " << users_path << endl;
}

// Функция для получения информации о диске
void get_disk_info(const string& device) {
#ifdef _WIN32
    if (device.find("sd") != string::npos || device.find("/dev/") != string::npos) {
        cout << "Linux device names not supported in Windows" << endl;
        cout << "Use drive letters like C:, D:, etc." << endl;
        return;
    }

    string cmd = "wmic logicaldisk where \"DeviceID='" + device + "'\" get Size,FreeSpace,FileSystem,VolumeName";
#else
    string cmd = "fdisk -l " + device + " 2>/dev/null || echo 'Cannot get disk info for " + device + "'";
#endif

    system(cmd.c_str());
}

// Главная функция
int main() {
    string line;

    // Пункт 9: Установка обработчика сигнала
#ifndef _WIN32
    signal(SIGHUP, sighup_handler);
#endif

    // Пункт 11: Инициализация VFS
    setup_users_vfs();

    cout << "=== kubsh (minimal shell) ===" << endl;
    cout << "Commands: echo, \\q (exit), history, \\e, \\x, \\l" << endl << endl;

#ifdef _WIN32
    cout << "Examples for Windows:" << endl;
    cout << "  echo Hello World" << endl;
    cout << "  \\e PATH" << endl;
    cout << "  \\x dir" << endl;
    cout << "  \\l C:" << endl << endl;
#else
    cout << "Examples for Linux:" << endl;
    cout << "  echo Hello World" << endl;
    cout << "  \\e PATH" << endl;
    cout << "  \\x ls" << endl;
    cout << "  \\l /dev/sda" << endl << endl;
#endif

    // Пункт 2: Главный цикл
    while (true) {
        cout << "kubsh> ";

        // Пункт 2: Выход по Ctrl+D (Linux) или Ctrl+Z (Windows)
        if (!getline(cin, line)) {
#ifdef _WIN32
            cout << endl << "Exit (Ctrl+Z)" << endl;
#else
            cout << endl << "Exit (Ctrl+D)" << endl;
#endif
            break;
        }

        // Пункт 1: Печать введенной строки (игнорируем пустые)
        if (line.empty()) continue;

        // Пункт 4: Сохранение в историю
        ofstream hist(HISTORY_PATH.c_str(), ios::app);
        if (hist.is_open()) {
            hist << line << endl;
            hist.close();
        }

        // Пункт 3: Команда для выхода
        if (line == "\\q") {
            cout << "Exit by command \\q" << endl;
            break;
        }

        // Пункт 5: Команда echo
        if (line.find("echo ") == 0) {
            string text = line.substr(5);
            cout << text << endl;
            continue;
        }

        // Дополнительная команда: history
        if (line == "history") {
            ifstream in(HISTORY_PATH.c_str());
            string hline;
            int i = 1;
            if (in.is_open()) {
                while (getline(in, hline)) {
                    cout << i++ << "  " << hline << endl;
                }
                in.close();
            }
            else {
                cout << "Command history is empty" << endl;
            }
            continue;
        }

        // Пункт 7: Вывод переменной окружения
        if (line.find("\\e ") == 0) {
            string var_name = line.substr(3);
            char* var_value = getenv(var_name.c_str());

            if (var_value) {
#ifdef _WIN32
                vector<string> values = split(var_value, ';');  // В Windows разделитель ;
#else
                vector<string> values = split(var_value, ':');  // В Linux разделитель :
#endif
                for (size_t i = 0; i < values.size(); i++) {
                    cout << values[i] << endl;
                }
            }
            else {
                cout << "Environment variable not found: " << var_name << endl;
            }
            continue;
        }

        // Пункт 8: Выполнение бинарника
        if (line.find("\\x ") == 0) {
            string command = line.substr(3);
            int result = system(command.c_str());
            if (result != 0) {
                cout << "Command finished with error code: " << result << endl;
            }
            continue;
        }

        // Пункт 10: Информация о разделах диска
        if (line.find("\\l ") == 0) {
            string device = line.substr(3);
            get_disk_info(device);
            continue;
        }

        // Пункт 6: Проверка введенной команды
        cout << "Unknown command: " << line << endl;
    }

    cout << "History saved to: " << HISTORY_PATH << endl;
    return 0;
}