// DS1HW4_11__11327121__11327155
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <sys/stat.h> // for stat
#include <cmath>

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

struct ExecutingList {
    int which_chef;
    Order order;
};

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

        void Clear() {
            while (head) pop();
        }

        ~Queue() {
            Clear();
        }
    };

class Chef {
    private:
        int free_time; //訂單結束時間
        int start_time; //訂單開始時間
        Order now;
    public:
        Chef() { free_time = 0; now = {0, 0, 0, 0}; start_time = 0;}

        bool IsFree(int current_time=0) {
            return now.oid == 0 || free_time <= current_time;
        }

        void ProcessOrder(Order o, int &delay, bool &isTimeout) {
            int st = std::max(free_time, o.arrival);
            delay = st - o.arrival;
            if (st + o.duration > o.timeout) {
                isTimeout = true;
            } else {
                isTimeout = false;
            }
            free_time = st + o.duration;
            now = o;
        }

        int GetFreeTime() { return free_time; }

        void DoThisOrder(const Order& o, int clk) {
            now.oid = o.oid;
            now.arrival = o.arrival;
            now.duration = o.duration;
            now.timeout = o.timeout;
            free_time = clk + now.duration;
            start_time = clk;
        }

        int GetFinishtime() {
            return free_time;
        }

        int GetTimeOut() {
            return now.timeout;
        }

        int GetStartTime() {
            return start_time;
        }
 
        int FreeTime() {
            return free_time;
        }

        Order GetOrder() {
            return now;
        }

        void SetFree() { free_time = 0; now = {0, 0, 0, 0}; start_time = 0;}
};

class Clock {
public:
    int clk;
    Clock() { clk=0; }
    void Tick() { clk++; }
};

class IOHandler {
public:
    static std::vector<Order> ReadOrdersFromFile(int file_number) {
        std::vector<Order> orders;
        std::ifstream infile("input" + std::to_string(file_number) + ".txt");
        if (!infile.is_open()) {
            std::cerr << "Cannot open input file input" << file_number << ".txt\n";
            return orders;
        }
        std::string header;
        std::getline(infile, header); 
        int oid, arrival, duration, timeout;
        while(infile >> oid >> arrival >> duration >> timeout) {
            orders.push_back({oid, arrival, duration, timeout});
        }
        return orders;
    }

    static bool ReadSortedToDynamic(Order *&arr, int &n, int file_number) {
        std::string fname = "sorted" + std::to_string(file_number) + ".txt";
        std::ifstream infile(fname);
        if (!infile.is_open()) {
            return false;
        }
        std::string header;
        std::getline(infile, header); // skip header
        std::vector<Order> tmp;
        int oid, arrival, duration, timeout;
        while(infile >> oid >> arrival >> duration >> timeout) {
            tmp.push_back({oid, arrival, duration, timeout});
        }
        n = (int)tmp.size();
        if (n == 0) {
            arr = nullptr;
            return true;
        }
        arr = new Order[n];
        for (int i = 0; i < n; ++i) arr[i] = tmp[i];
        return true;
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

    static void WriteSortedListToFile(const std::vector<Order>& orders, int file_number) {
        std::ofstream outfile("sorted" + std::to_string(file_number) + ".txt");
        outfile << "OID\tArrival\tDuration\tTimeOut\n";
        for (auto &o : orders) {
            outfile << o.oid << "\t"
                << o.arrival << "\t"
                << o.duration << "\t"
                << o.timeout << "\n";
        }
    }

    static void ShellSort(std::vector<Order> &orders) {
        int n = (int)orders.size();
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

    static bool Exists(const std::string &name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }
};

class order_system {
    private:
        int chef_num;
        int which_chef;
        Chef* chefs;
        Queue* queues;
        Clock clock;
        std::vector<AbortList> abort_list;
        std::vector<TimeoutList> timeout_list;
        Order* orders; 
        int orders_count;
        int current_index; 
        int total_delay;
    public:
        order_system(int num) {
            chef_num = num;
            which_chef = 0;
            chefs = new Chef[chef_num];
            queues = new Queue[chef_num];
            orders = nullptr;
            orders_count = 0;
            current_index = 0;
            total_delay = 0;
        }

        ~order_system() { delete[] chefs; delete[] queues; }

        void LoadOrdersFromDynamicArray(Order* arr, int count) {
            orders = arr;
            orders_count = count;
            current_index = 0;
        }

        bool HasRemainingOrders() {
            if (current_index < orders_count) return true;
            for (int i = 0; i < chef_num; ++i) {
                if (!queues[i].IsEmpty()) return true;
            }
            for (int i = 0; i < chef_num; ++i) {
                if (!chefs[i].IsFree(clock.clk)) return true;
            }
            return false;
        }

        bool AllocateOrdersForOne(Order &o) {
            if (chef_num != 1) return false;
            if (chefs[0].IsFree() && queues[0].IsEmpty()) {
                chefs[0].DoThisOrder(o, clock.clk);
                return true;
            }
            if (!queues[0].QueueFull()) {
                queues[0].push(o);
                return true;
            }
            return false;
        }

        bool AllocateOrdersSQF(Order &o) {
            int idle_cnt = 0;
            int last_idle_idx = -1;
            for (int i = 0; i < chef_num; ++i) {
                if (chefs[i].IsFree(clock.clk) && queues[i].IsEmpty()) {
                    idle_cnt++;
                    last_idle_idx = i;
                }
            }
            if (idle_cnt == 1) {
                chefs[last_idle_idx].DoThisOrder(o, clock.clk);
                return true;
            }
            if (idle_cnt > 1) {
                int pick = -1;
                for (int i = 0; i < chef_num; ++i) {
                    if (chefs[i].IsFree(clock.clk) && queues[i].IsEmpty()) { pick = i; break; }
                }
                if (pick != -1) {
                    chefs[pick].DoThisOrder(o, clock.clk);
                    return true;
                }
            }
            int bestlen = 100000;
            int bestidx = -1;
            for (int i = 0; i < chef_num; ++i) {
                int len = queues[i].GetLength();
                if (len < bestlen && !queues[i].QueueFull()) {
                    bestlen = len;
                    bestidx = i;
                } else if (len == bestlen && len < 100000 && !queues[i].QueueFull()) {
                    // tie -> smallest id already preserved because we iterate increasing i
                }
            }
            if (bestidx != -1) {
                queues[bestidx].push(o);
                return true;
            }
            return false;
        }

        bool IsTimeout(const Order &o, int finish_time) {
            return finish_time > o.timeout;
        }

        void SetAbort_FirstCase(const Order& o) { 
            AbortList a = {o.oid, 0, 0, o.arrival};
            abort_list.push_back(a);
        }

        void SetAbort_FromQueue(Order& o, int cid) { 
            AbortList a = {o.oid, cid+1, clock.clk - o.arrival, clock.clk};
            abort_list.push_back(a);
        }

        void CheckFinish(int num) {
            int finish_time = chefs[num].GetFinishtime();
            int start_time = chefs[num].GetStartTime();
            Order o = chefs[num].GetOrder();
            if (finish_time == clock.clk) {
                if (IsTimeout(o, finish_time)) {
                    int delay = start_time - o.arrival;
                    TimeoutList t = {o.oid, num+1 , delay, finish_time};
                    timeout_list.push_back(t);
                }
                chefs[num].SetFree();
            }
        }

        void GetNextOrder(int num) {
            Order o;
            while (!queues[num].IsEmpty()) {
                o = queues[num].GetHeadOrder();
                if (o.timeout < chefs[num].GetFreeTime()) {
                    SetAbort_FromQueue(o, num);
                    queues[num].pop();
                } else {
                    chefs[num].DoThisOrder(o, clock.clk);
                    queues[num].pop();
                    break;
                }
            }
        }

        bool AllQueuesEmpty() {
            for (int i = 0; i < chef_num; ++i) {
                if (!queues[i].IsEmpty()) return false;
            }
            return true;
        }

        bool AllChefsFree() {
            for (int i = 0; i < chef_num; ++i) {
                if (!chefs[i].IsFree(clock.clk)) return false;
            }
            return true;
        }

        void SimulateQueues(int N, int file_number, bool from_sorted_already_loaded) {
            std::string prefix;
            if(N==1) prefix="one";
            else if(N==2) prefix="two";
            else prefix="any";
            if (!orders || orders_count == 0) {
                std::cerr << "No sorted orders loaded to simulate.\n";
                return;
            }
            int total_orders = orders_count;
            while (true) {
                clock.Tick();
                for (int i = 0; i < chef_num; i++) {
                    if (!chefs[i].IsFree(clock.clk)) {
                        CheckFinish(i);
                    }
                }
                for (int i = 0; i < chef_num; i++) {
                    if (chefs[i].IsFree(clock.clk)) {
                        GetNextOrder(i);
                    }
                }
                while (current_index < orders_count && orders[current_index].arrival == clock.clk) {
                    Order o = orders[current_index];
                    bool allocated = false;
                    if (chef_num == 1) {
                        allocated = AllocateOrdersForOne(o);
                        if (!allocated) {
                            SetAbort_FirstCase(o);
                        }
                    } else {
                        allocated = AllocateOrdersSQF(o);
                        if (!allocated) {
                            AbortList a = {o.oid, 0, 0, o.arrival};
                            abort_list.push_back(a);
                        }
                    }
                    current_index++; 
                }
                if (current_index >= orders_count && AllQueuesEmpty() && AllChefsFree()) {
                    break;
                }
            }
            total_delay = 0;
            for(auto &a:abort_list) total_delay += a.delay;
            for(auto &t:timeout_list) total_delay += t.delay;
            double failure_percentage = 0.0;
            if (total_orders > 0) {
                failure_percentage = ((double)(abort_list.size() + timeout_list.size())) * 100.0 / total_orders;
            }
            IOHandler::WriteAbortListToFile(abort_list,file_number,prefix);
            IOHandler::WriteTimeoutListToFile(timeout_list,file_number,prefix);
            std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
            outfile << "\t[Total Delay]\n";
            outfile << total_delay << " min.\n";
            outfile << "\t[Failure Percentage]\n";
            double fp_rounded = std::round(failure_percentage * 100.0) / 100.0;
            outfile << std::fixed << std::setprecision(2) << fp_rounded << " %\n";
            outfile.close();
            std::cout << "Simulation finished. Output written to " << prefix << file_number << ".txt\n";
            std::cout << "Total Delay: " << total_delay << " min. Failure %: " << std::fixed << std::setprecision(2) << fp_rounded << " %\n";
        }
};

int main() {
    Order* dyn_orders = nullptr; 
    int dyn_count = 0;
    bool has_sorted_file = false; 
    bool task2_done = false; 
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
        if (!(std::cin >> choice)) break;
        if(choice==0) break;
        int file_number;
        if(choice==1) {
            std::cout << "Enter file number (e.g., 401): ";
            std::cin >> file_number;
            std::vector<Order> orders = IOHandler::ReadOrdersFromFile(file_number);
            if (orders.empty()) {
                std::cerr << "No orders read from input" << file_number << ".txt or file not found.\n";
                continue;
            }
            std::cout << "原始 orders:\n";
            for (auto &o : orders) {
                std::cout << o.oid << "\t" << o.arrival << "\t" << o.duration << "\t" << o.timeout << "\n";
            }
            auto t0 = std::chrono::high_resolution_clock::now();
            IOHandler::ShellSort(orders);
            auto t1 = std::chrono::high_resolution_clock::now();
            auto t2 = std::chrono::high_resolution_clock::now();
            IOHandler::WriteSortedListToFile(orders, file_number);
            auto t3 = std::chrono::high_resolution_clock::now();
            long long read_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count(); 
            long long sort_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            long long write_us = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
            std::cout << "Sorting done and written to sorted" << file_number << ".txt\n";
            std::cout << "Sort time (us): " << sort_us << "\n";
            std::cout << "Write time (us): " << write_us << "\n";
            has_sorted_file = true;
        }
        else if(choice==2) {
            std::cout << "Enter file number (e.g., 401): ";
            std::cin >> file_number;
            std::string sorted_name = "sorted" + std::to_string(file_number) + ".txt";
            if (!IOHandler::Exists(sorted_name)) {
                std::cerr << "Error: " << sorted_name << " does not exist. Please run task 1 first to create sorted file.\n";
                continue;
            }
            if (dyn_orders) { delete[] dyn_orders; dyn_orders = nullptr; dyn_count = 0; task2_done = false; }
            bool ok = IOHandler::ReadSortedToDynamic(dyn_orders, dyn_count, file_number);
            if (!ok) {
                std::cerr << "Failed to read sorted file into dynamic array.\n";
                continue;
            }
            std::cout << "Loaded " << dyn_count << " orders from " << sorted_name << " into dynamic array.\n";
            task2_done = true;
            order_system os(1);
            os.LoadOrdersFromDynamicArray(dyn_orders, dyn_count);
            os.SimulateQueues(1, file_number, true);
        }
        else if(choice==3) {
            if (!task2_done) {
                std::cerr << "Error: You must run task 2 first to load sorted file into memory. Task 3 aborted.\n";
                continue;
            }
            order_system os(2);
            os.LoadOrdersFromDynamicArray(dyn_orders, dyn_count);
            os.SimulateQueues(2, file_number, true);
        }
        else if(choice==4) {
            if (!task2_done) {
                std::cerr << "Error: You must run task 2 first to load sorted file into memory. Task 4 aborted.\n";
                continue;
            }
            int N;
            std::cout << "Enter number of chefs/queues: ";
            std::cin >> N;
            if (N <= 0) {
                std::cerr << "Invalid number of chefs.\n";
                continue;
            }
            order_system os(N);
            os.LoadOrdersFromDynamicArray(dyn_orders, dyn_count);
            os.SimulateQueues(N, file_number, true);
        }
        else {
            std::cout << "Invalid command!\n";
        }
    }
    if (dyn_orders) delete[] dyn_orders;
    std::cout << "Program terminated.\n";
    return 0;
}
