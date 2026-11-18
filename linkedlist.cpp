#include <bits/stdc++.h>
using namespace std;

struct Node {
    string cmd;
    Node* next;
    Node* prev;

    Node(string cmd) {
        this->cmd = cmd;
        next = nullptr;
        prev = nullptr;
    }
};

class LinkedList {
public:
    Node* head;
    Node* tail;
    Node* curnt;
    int cnt = 0;
    int LIMIT = 50;
    LinkedList() {
        head = nullptr;
        tail =nullptr;
          curnt = nullptr;
    }
    void append(string stuff) {
        Node* ncmd= new Node(stuff);
        if (head == nullptr) {
            head = ncmd;
            tail = ncmd;
        }
        else {
            tail->next = ncmd;
            ncmd->prev = tail;
            tail = ncmd;
        }

        cnt++;

        if (cnt > LIMIT) {
            Node* del = head;
            head = head->next;
            head->prev = nullptr;
            delete del;
            cnt--;
        }
    }

    void display() {
        Node* temp = head;

        while (temp != nullptr) {
            cout << temp->cmd << endl;
            temp = temp->next;
        }
    }
    void starthistory(){
    curnt = tail;
  }
  
};

int main() {
    LinkedList ll;

    ll.append("ls");
    ll.append("pwd");
    ll.append("git status");

    ll.display();
}
