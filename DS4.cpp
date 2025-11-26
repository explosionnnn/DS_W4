#include<iostream>

struct Order {
    int oid;
    int arrival;
    int duration;
    int timeout;
};

struct Orders {
    Order order;
    Orders *next;
};

class order_system {
    private:
        int chef_num;
        Queue *queues;
        Chef *chefs;
    public:
        order_system(int num) {
            num = chef_num;
            queues = nullptr;
            chefs = nullptr;
        }
        void BuildSystem() {
            queues = new Queue[chef_num];
            chefs = new Chef[chef_num];
        }

        ~order_system() {
            delete[] queues;
            delete[] chefs;
        }
};


class Queue{
    private:
        Orders *orders;
        Orders *head;
        Orders *tail;
    public:
        Queue() {
            orders = nullptr;
            head = nullptr;
            tail = nullptr;
        }

        void push(Order order) {
            Orders* newnode = new Orders{order, nullptr};
            if (orders == nullptr) {
                orders = newnode;
                head = newnode;
                tail = newnode;
            } else {
                tail -> next = newnode;
                tail = tail -> next;
            }
        }

        void pop() {
            if (head == nullptr) {
                return;
            }
            Orders* temp = head;
            if (head -> next == nullptr) {
                delete temp;
                orders = nullptr;
                head = nullptr;
                tail = nullptr;
                return;
            }  else {
                head = head -> next;
                delete temp;
            }
        }
};
class Chef {
    private:
        Order now;
    public:
        Chef() {
            now = {0, 0, 0, 0};
        }

};

