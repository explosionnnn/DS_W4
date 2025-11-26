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

class Clock {
    public:
        int clk;
        Clock() {
            clk = 0;
        }
};

class order_system {
    private:
        Clock clock;
        Queue main_queue; // 共同queue
        int chef_num;
        int which_chef; //現在要哪個廚師
        Queue *queues; //每個廚一個queue
        Chef *chefs;
    public:
        order_system(int num) {
            chef_num =num;
            which_chef = 0;
            queues = nullptr;
            chefs = nullptr;
            Queue main_queue;
        }
        void BuildSystem() {
            queues = new Queue[chef_num];
            chefs = new Chef[chef_num];
        }

        ~order_system() {
            delete[] queues;
            delete[] chefs;
        }

        bool OnlyOneChefFree() {
            which_chef = 0;
            int count = 0;
            while (which_chef < chef_num) {
                if(chefs[which_chef].IsFree()) {
                    count++;
                }
                which_chef++;
            }
            if (count == 1) {
                return true;
            }
            return false;
        }


        bool AllocateOrders() {
            which_chef = 0;
            Order o = main_queue.GetHeadOrder();
            if (OnlyOneChefFree()) {
                queues[which_chef].push(o);
                main_queue.pop();
                return true;
            }
            while (which_chef < chef_num) {
                if (chefs[which_chef].IsFree()) {
                    queues[which_chef].push(o);
                    main_queue.pop();
                    return true;
                }
                which_chef++;
            }
            which_chef = 0;
            while (which_chef < chef_num) {
                if (!queues[which_chef].QueueFull()) {
                    queues[which_chef].push(o);
                    main_queue.pop();
                    return true;
                } else {
                    which_chef++;
                }
            }
            return false;
        }

        void AddOrder() {
            while (main_queue.GetHeadOrder().arrival == clock.clk) {
                bool success = AllocateOrders();

                if (!success) {
                    break;
                }
                if (main_queue.IsEmpty()) {
                    break;
                }
            }
        }

        void Simulate() {
            while (true) {
                clock.clk++;
                if (!main_queue.IsEmpty()) {
                    AddOrder();
                }


            }
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
            delete temp;
            if (head == nullptr) {
                orders = nullptr;
                tail = nullptr;
            }  else {
                head = head -> next;
            }
            return;
        }

        Orders* GetHead() {
            return head;
        }

        Order GetHeadOrder() {
            return head -> order;
        }

        int GetLength() {
            Orders *cur = head;
            int len = 0;
            while (cur != nullptr) {
                len++;
                cur = cur -> next;
            }
            return len;
        }

        bool QueueFull() {
            if (GetLength() == 3) {
                return true;
            } else {
                return false;
            }
        }

        bool IsEmpty() {
            if (head == nullptr) {
                return true;
            }
            return false;
        }
};
class Chef {
    private:
        Order now;
    public:
        Chef() {
            now = {0, 0, 0, 0};
        }
        bool IsFree() {
            if (now.oid = 0) {
                return true;
            }
            return false;
        }

};

