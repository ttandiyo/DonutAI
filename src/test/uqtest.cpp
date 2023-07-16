#include "../UniqueQueue.h"
#include <iostream>

using namespace std;

int main(int argc, char const *argv[]) {
    UniqueQueue<int> q;
    cout << "Size: " << q.size() << endl;
    q.append(2);
    q.append(3);
    q.prepend(1);
    int item = q.pop();
    q.prepend(item);

    for (int i = 0; i < q.size(); ++i) {
        cout << q[i] << endl;
    }
    cout << "Size: " << q.size() << endl;

    while (q.size()) {
        cout << q.pop() << endl;
    }
    cout << "Size: " << q.size() << endl;

    return 0;
}
