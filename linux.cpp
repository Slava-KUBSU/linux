
#include <iostream>
#include <string>
using namespace std;
int main()
{
    string line;

    while (true)
    {
        cout << "kubsh";

        if (!getline(cin, line))
        {
            cout << "выход ctrl+d";
            break;
        }

        if(line=='/q') 
        {
          cout << "выход по команде /q";
           break;
        }

        cout << line << endl;

    }
    return 0;
}


