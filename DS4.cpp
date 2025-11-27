#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>

// ----------------- STRUCTS -----------------
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

struct SortedList {
    int oid;
    int arrival;
    int duration;
    int timeout;
};

struct AbortList {
    int oid;
    int cid; 
    int delay;
    int abort;
};

struct TimeoutList {
    int oid;
    int cid; 
    int delay;
    int departure;
};

// ----------------- QUEUE -----------------
class Queue {
    private:
        Orders *head;
        Orders *tail;
    public:
        Queue() { head = nullptr; tail = nullptr; }

        void push(Order order) {
            Orders* newnode = new Orders{order, nullptr};
            if (!head) {
                head = tail = newnode;
            } else {
                tail->next = newnode;
                tail = newnode;
            }
        }

        void pop() {
            if (!head) return;
            Orders* to_delete = head;
            head = head->next;
            delete to_delete;
            if (!head) tail = nullptr;
        }

        Order GetHeadOrder() {
            if (head) return head->order;
            return {0,0,0,0};
        }

        Orders* GetHead() { return head; }

        bool IsEmpty() { return head == nullptr; }

        bool QueueFull() { return GetLength() >= 3; }

        int GetLength() {
            Orders* cur = head;
            int len = 0;
            while (cur) { len++; cur = cur->next; }
            return len;
        }
    };

    // ----------------- CHEF -----------------
class Chef {
    private:
        int free_time;
        Order now;
    public:
        Chef() { free_time = 0; now = {0,0,0,0}; }

        bool IsFree(int current_time=0) { 
            return now.oid == 0 || free_time <= current_time; 
        }

        void ProcessOrder(Order o, int &delay, bool &isTimeout) {
            int start_time = std::max(free_time, o.arrival);
            delay = start_time - o.arrival;
            if (start_time + o.duration > o.timeout) {
                isTimeout = true;
            } else {
                isTimeout = false;
            }
            free_time = start_time + o.duration;
            now = o;
        }

        int GetFreeTime() { return free_time; }
};

// ----------------- CLOCK -----------------
class Clock {
public:
    int clk;
    Clock() { clk=0; }
    void Tick() { clk++; }
};

// ----------------- IO HANDLER -----------------
class IOHandler {
public:
    static std::vector<Order> ReadOrdersFromFile(int file_number) {
        std::vector<Order> orders;
        std::ifstream infile("input" + std::to_string(file_number) + ".txt");
        std::string header;
        std::getline(infile, header); // skip header
        int oid, arrival, duration, timeout;
        while(infile >> oid >> arrival >> duration >> timeout) {
            orders.push_back({oid, arrival, duration, timeout});
        }
        return orders;
    }

    static void WriteAbortListToFile(const std::vector<AbortList>& abort_list,int file_number,std::string prefix) {
        std::ofstream outfile(prefix + std::to_string(file_number) + ".txt");
        outfile << "\t[Abort List]\n";
        outfile << "\tOID\tCID\tDelay\tAbort\n";
        for(size_t i=0;i<abort_list.size();i++){
            outfile << "[" << i+1 << "]\t" << abort_list[i].oid << "\t" 
                    << abort_list[i].cid << "\t" << abort_list[i].delay << "\t" 
                    << abort_list[i].abort << "\n";
        }
    }

    static void WriteTimeoutListToFile(const std::vector<TimeoutList>& timeout_list,int file_number,std::string prefix) {
        std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
        outfile << "\t[Timeout List]\n";
        outfile << "\tOID\tCID\tDelay\tDeparture\n";
        for(size_t i=0;i<timeout_list.size();i++){
            outfile << "[" << i+1 << "]\t" << timeout_list[i].oid << "\t" 
                    << timeout_list[i].cid << "\t" << timeout_list[i].delay << "\t" 
                    << timeout_list[i].departure << "\n";
        }
    }

    static void WriteSortedListToFile(int total_delay,double failure_percentage,int file_number,std::string prefix) {
        std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
        outfile << "[Total Delay]\n" << total_delay << " min.\n";
        outfile << "[Failure Percentage]\n" << std::fixed << std::setprecision(2) << failure_percentage << " %\n";
    }

    static void ShellSort(std::vector<Order> &orders) {
        int n = orders.size();
        for(int gap=n/2; gap>0; gap/=2){
            for(int i=gap; i<n; i++){
                Order temp = orders[i];
                int j=i;
                while(j>=gap && (orders[j-gap].arrival>temp.arrival || 
                    (orders[j-gap].arrival==temp.arrival && orders[j-gap].oid>temp.oid))){
                    orders[j] = orders[j-gap];
                    j-=gap;
                }
                orders[j]=temp;
            }
        }
    }
};

// ----------------- ORDER SYSTEM -----------------
class order_system {
    private:
        int chef_num;
        Chef* chefs;
        Queue* queues;
        Clock clock;
        std::vector<AbortList> abort_list;
        std::vector<TimeoutList> timeout_list;
        std::vector<Order> main_orders;
        int total_delay;
    public:
        order_system(int num) {
            chef_num = num;
            chefs = new Chef[chef_num];
            queues = new Queue[chef_num];
            total_delay = 0;
        }
        
        ~order_system() { delete[] chefs; delete[] queues; }

        void LoadOrders(const std::vector<Order>& orders) {
            main_orders = orders;
        }

        void SimulateQueues(int N) {
            std::string prefix;
            if(N==1) prefix="one";
            else if(N==2) prefix="two";
            else prefix="any";

            size_t idx = 0;
            int total_orders = main_orders.size();
            while(idx < total_orders){
                // 將到達時間的訂單加入最短佇列或閒置廚師
                for(int i=0;i<N;i++){
                    if(queues[i].GetLength() < 3){
                        int delay;
                        bool isTimeout;
                        chefs[i].ProcessOrder(main_orders[idx], delay, isTimeout);
                        if(chefs[i].GetFreeTime() > main_orders[idx].timeout || isTimeout){
                            TimeoutList t = {main_orders[idx].oid,i+1,delay,chefs[i].GetFreeTime()};
                            timeout_list.push_back(t);
                        }
                        queues[i].push(main_orders[idx]);
                        idx++;
                        if(idx>=total_orders) break;
                    } else {
                        AbortList a = {main_orders[idx].oid,0,0,main_orders[idx].arrival};
                        abort_list.push_back(a);
                        idx++;
                    }
                }
            }

            // 計算總延遲
            total_delay=0;
            for(auto a:abort_list) total_delay+=a.delay;
            for(auto t:timeout_list) total_delay+=t.delay;

            double failure_percentage = (abort_list.size()+timeout_list.size())*100.0/total_orders;

            IOHandler::WriteAbortListToFile(abort_list,401,prefix);
            IOHandler::WriteTimeoutListToFile(timeout_list,401,prefix);
            IOHandler::WriteSortedListToFile(total_delay,failure_percentage,401,prefix);
        }
};

// ----------------- MAIN -----------------
int main() {
    while(true) {
        std::cout << "*** (^_^) Data Structure (^o^) ***\n";
        std::cout << "** Simulate FIFO Queues by SQF ***\n";
        std::cout << "* 0. Quit                        *\n";
        std::cout << "* 1. Sort a file                 *\n";
        std::cout << "* 2. Simulate one FIFO queue     *\n";
        std::cout << "* 3. Simulate two queues by SQF  *\n";
        std::cout << "* 4. Simulate some queues by SQF *\n";
        std::cout << "**********************************\n";
        std::cout << "Input a command(0, 1, 2, 3, 4): ";

        int choice;
        std::cin >> choice;
        if(choice==0) break;

        int file_number;
        std::cout << "Enter file number (e.g., 401): ";
        std::cin >> file_number;
        std::vector<Order> orders = IOHandler::ReadOrdersFromFile(file_number);

        if(choice>=1){
            IOHandler::ShellSort(orders);
        }

        switch(choice) {
            case 1:
                std::cout << "Orders sorted by Arrival and OID.\n";
                break;
            case 2: {
                order_system os(1);
                os.LoadOrders(orders);
                os.SimulateQueues(1);
                break;
            }
            case 3: {
                order_system os(2);
                os.LoadOrders(orders);
                os.SimulateQueues(2);
                break;
            }
            case 4: {
                int N;
                std::cout << "Enter number of chefs/queues: ";
                std::cin >> N;
                order_system os(N);
                os.LoadOrders(orders);
                os.SimulateQueues(N);
                break;
            }
            default:
                std::cout << "Invalid command!\n";
        }
    }
    std::cout << "Program terminated.\n";
    return 0;
}

