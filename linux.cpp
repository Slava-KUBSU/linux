
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
            cout << "����� ctrl+d";
            break;
        }

        if(line=='/q') 
        {
          cout << "����� �� ������� /q";
           break;
        }

        cout << line << endl;

    }
    return 0;
}


