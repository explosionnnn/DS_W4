// DS1HW4_11__11327121__11327155
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <sys/stat.h> // for stat
#include <cmath>
#include <limits>
#include <climits>
#include <algorithm>

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

struct DoneList {
    int oid;
    int cid;
    int delay;
    int departure;
};

struct ExecutingList {
    int which_chef;
    Order order;
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

        void Clear() {
            while (head) pop();
        }

        ~Queue() {
            Clear();
        }
    };

// ----------------- CHEF -----------------
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
            int start_t = std::max(free_time, o.arrival);
            delay = start_t - o.arrival;
            if (start_t + o.duration > o.timeout) {
                isTimeout = true;
            } else {
                isTimeout = false;
            }
            free_time = start_t + o.duration;
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
        if (!infile.is_open()) {
            std::cerr << "Cannot open input file input" << file_number << ".txt\n";
            return orders;
        }
        std::string header;
        std::getline(infile, header); // skip header
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

    static void WriteAbortListToFile(const std::vector<AbortList>& abort_list,int file_number,const std::string &prefix) {
        std::ofstream outfile(prefix + std::to_string(file_number) + ".txt");
        outfile << "\t[Abort List]\n";
        outfile << "\tOID\tCID\tDelay\tAbort\n";
        for(size_t i=0;i<abort_list.size();i++){
            outfile << "[" << i+1 << "]\t" << abort_list[i].oid << "\t"
                    << abort_list[i].cid << "\t" << abort_list[i].delay << "\t"
                    << abort_list[i].abort << "\n";
        }
    }

    static void WriteTimeoutListToFile(const std::vector<TimeoutList>& timeout_list,int file_number,const std::string &prefix) {
        std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
        outfile << "\t[Timeout List]\n";
        outfile << "\tOID\tCID\tDelay\tDeparture\n";
        for(size_t i=0;i<timeout_list.size();i++){
            outfile << "[" << i+1 << "]\t" << timeout_list[i].oid << "\t"
                    << timeout_list[i].cid << "\t" << timeout_list[i].delay << "\t"
                    << timeout_list[i].departure << "\n";
        }
    }

    static void WriteDoneListToFile(const std::vector<DoneList>& done_list,int file_number,const std::string &prefix) {
        std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
        outfile << "\t[Done List]\n";
        outfile << "\tOID\tCID\tDelay\tDeparture\n";
        for(size_t i=0;i<done_list.size();i++){
            outfile << "[" << i+1 << "]\t" << done_list[i].oid << "\t"
                    << done_list[i].cid << "\t" << done_list[i].delay << "\t"
                    << done_list[i].departure << "\n";
        }
    }

    static void WriteSummaryToFile(int total_delay,double failure_percentage,int file_number,const std::string &prefix) {
        std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
        outfile << "\t[Total Delay]\n";
        outfile << total_delay << " min.\n";
        outfile << "\t[Failure Percentage]\n";
        double fp_rounded = std::round(failure_percentage * 100.0) / 100.0;
        outfile << std::fixed << std::setprecision(2) << fp_rounded << " %\n";
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
        for(int gap = n/2; gap > 0; gap /= 2) {
            for (int i = gap; i < n; ++i) {
                Order temp = orders[i];
                int j = i;
                while (j >= gap && (orders[j-gap].arrival > temp.arrival ||
                      (orders[j-gap].arrival == temp.arrival && orders[j-gap].oid > temp.oid))) {
                    orders[j] = orders[j-gap];
                    j -= gap;
                }
                orders[j] = temp;
            }
        }
    }

    static bool Exists(const std::string &name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }
};

// ----------------- ORDER SYSTEM (修改 SimulateQueues 以逐時刻列印 debug 訊息) -----------------
class order_system {
    private:
        int chef_num;
        Chef* chefs;
        Queue* queues;
        Clock clock;
        std::vector<AbortList> abort_list;
        std::vector<TimeoutList> timeout_list;
        std::vector<DoneList> done_list;
        std::vector<Order> main_orders;
        int total_delay;
        int file_number;
        int total_orders_count;
    public:
        order_system(int num, int file_no): chef_num(num), file_number(file_no) {
            chefs = new Chef[chef_num];
            queues = new Queue[chef_num];
            total_delay = 0;
        }

        ~order_system() { delete[] chefs; delete[] queues; }

        void LoadOrders(const std::vector<Order>& orders) {
            main_orders = orders;
        }

        bool IsTimeout(const Order &o, int finish_time) {
            return finish_time > o.timeout;
        }

        void SetAbort_FirstCase(const Order& o) { // 第一種：未進入佇列就被取消（CID=0，delay=0，abort=arrival）
            AbortList a = {o.oid, 0, 0, o.arrival};
            abort_list.push_back(a);
        }

        void SetAbort_FromQueue(const Order& o, int num) { // 從佇列取出時發現逾時
            AbortList a = {o.oid, num+1, clock.clk - o.arrival, clock.clk};
            abort_list.push_back(a);
        }

        void CheckFinish(int num) {
            int finish_time = chefs[num].GetFinishtime();
            int start_time = chefs[num].GetStartTime();
            Order o = chefs[num].GetOrder();
            if (finish_time == clock.clk) {
                int delay = start_time - o.arrival;
                if (IsTimeout(o, finish_time)) {
                    TimeoutList t = {o.oid, num+1 , delay, finish_time};
                    timeout_list.push_back(t);
                } else {
                    DoneList d = {o.oid, num+1, delay, finish_time};
                    done_list.push_back(d);
                }
                chefs[num].SetFree();
            }
        }

        void GetNextOrder(int num) {
            Order o;
            while (!queues[num].IsEmpty()) {
                o = queues[num].GetHeadOrder();
                // 若這筆從佇列取出時已無法在 timeout 前完成 -> 把他 Abort（delay = now - arrival）
                if (o.timeout < chefs[num].GetFreeTime()) {
                    AbortList a = {o.oid, num+1, clock.clk - o.arrival, clock.clk};
                    abort_list.push_back(a);
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

        bool AllocateOrdersSQF(const Order &o) {
            // 精簡版 SQF，盡量與你原本邏輯一致（閒置直接做、否則放最短佇列）
            int idle_cnt = 0;
            int pick_idle_idx = -1;
            for (int i = 0; i < chef_num; ++i) {
                if (chefs[i].IsFree(clock.clk) && queues[i].IsEmpty()) {
                    idle_cnt++;
                    if (pick_idle_idx == -1) pick_idle_idx = i;
                }
            }
            if (idle_cnt == 1) {
                chefs[pick_idle_idx].DoThisOrder(o, clock.clk);
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
            int bestlen = std::numeric_limits<int>::max();
            int bestidx = -1;
            for (int i = 0; i < chef_num; ++i) {
                int len = queues[i].GetLength();
                if (len < bestlen && !queues[i].QueueFull()) {
                    bestlen = len;
                    bestidx = i;
                }
            }
            if (bestidx != -1) {
                queues[bestidx].push(o);
                return true;
            }
            return false;
        }

        void SimulateQueues(int N) {
            std::string prefix;
            if (N == 1) prefix = "one";
            else if (N == 2) prefix = "two";
            else prefix = "any";

            int total_orders = (int)main_orders.size();

            // 確保 main_orders 已按照 arrival, oid 排好
            std::sort(main_orders.begin(), main_orders.end(), [](const Order &a, const Order &b) {
                if (a.arrival != b.arrival) return a.arrival < b.arrival;
                return a.oid < b.oid;
            });

            while (true) {
                // ----------------------------
                // Step 1: 處理完成/逾時訂單
                // ----------------------------
                for (int i = 0; i < chef_num; ++i) {
                    if (!chefs[i].IsFree() && chefs[i].GetFinishtime() == clock.clk) {
                        Order o = chefs[i].GetOrder();
                        int start_time = chefs[i].GetStartTime();
                        int finish_time = chefs[i].GetFinishtime();
                        int delay = start_time - o.arrival;

                        if (finish_time > o.timeout) {
                            TimeoutList t = { o.oid, i + 1, delay, finish_time };
                            timeout_list.push_back(t);
                        } else {
                            DoneList d = { o.oid, i + 1, delay, finish_time };
                            done_list.push_back(d);
                        }
                        chefs[i].SetFree();
                    }
                }

                // ----------------------------
                // Step 2: arrival (分派新訂單)
                // ----------------------------
                while (!main_orders.empty() && main_orders.front().arrival == clock.clk) {
                    Order o = main_orders.front();

                    std::vector<int> idle_empty;
                    for (int i = 0; i < chef_num; ++i) {
                        if (chefs[i].IsFree(clock.clk) && queues[i].IsEmpty()) idle_empty.push_back(i);
                    }

                    if (!idle_empty.empty()) {
                        // Case 1 & 2: 閒置廚師，選編號最小
                        int idx = *std::min_element(idle_empty.begin(), idle_empty.end());
                        chefs[idx].DoThisOrder(o, clock.clk);
                    } else {
                        // Case 3: 最短佇列
                        int best_len = INT_MAX;
                        int best_idx = -1;
                        for (int i = 0; i < chef_num; ++i) {
                            if (!queues[i].QueueFull()) {
                                int len = queues[i].GetLength();
                                if (len < best_len || (len == best_len && i < best_idx)) {
                                    best_len = len;
                                    best_idx = i;
                                }
                            }
                        }
                        if (best_idx != -1) {
                            queues[best_idx].push(o);
                        } else {
                            // Case 4: 所有隊列滿 -> Abort
                            AbortList a = { o.oid, 0, 0, o.arrival };
                            abort_list.push_back(a);
                        }
                    }
                    main_orders.erase(main_orders.begin());
                }

                // ----------------------------
                // Step 3: 閒置廚師取隊列訂單（改成全局最短隊列優先）
                // ----------------------------
                while (true) {
                    int best_idx = -1;
                    int min_len = INT_MAX;
                    for (int i = 0; i < chef_num; ++i) {
                        if (!queues[i].IsEmpty() && (chefs[i].IsFree(clock.clk) || chefs[i].FreeTime() <= clock.clk)) {
                            int len = queues[i].GetLength();
                            if (len < min_len || (len == min_len && i < best_idx)) {
                                min_len = len;
                                best_idx = i;
                            }
                        }
                    }
                    if (best_idx == -1) break; // 沒有可以處理的隊列

                    Order head = queues[best_idx].GetHeadOrder();
                    int start_time = std::max(chefs[best_idx].FreeTime(), clock.clk);

                    if (head.timeout < start_time) {
                        // 超時 -> Abort
                        AbortList a = { head.oid, best_idx + 1, start_time - head.arrival, start_time };
                        abort_list.push_back(a);
                        queues[best_idx].pop();
                    } else {
                        chefs[best_idx].DoThisOrder(head, clock.clk);
                        queues[best_idx].pop();
                    }
                }

                // ----------------------------
                // Step 4: 檢查結束條件
                // ----------------------------
                bool allQempty = true;
                for (int i = 0; i < chef_num; ++i)
                    if (!queues[i].IsEmpty()) { allQempty = false; break; }

                bool allChefsFree = true;
                for (int i = 0; i < chef_num; ++i)
                    if (!chefs[i].IsFree(clock.clk)) { allChefsFree = false; break; }

                if (main_orders.empty() && allQempty && allChefsFree) break;

                clock.Tick();
            }

            // Summary
            int total_delay = 0;
            for (auto &a : abort_list) total_delay += a.delay;
            for (auto &t : timeout_list) total_delay += t.delay;

            double failure_percentage = 0.0;
            if (total_orders > 0)
                failure_percentage = ((double)(abort_list.size() + timeout_list.size())) * 100.0 / total_orders;

            IOHandler::WriteAbortListToFile(abort_list, file_number, prefix);
            IOHandler::WriteTimeoutListToFile(timeout_list, file_number, prefix);
            IOHandler::WriteSummaryToFile(total_delay, failure_percentage, file_number, prefix);
        }


};

// ----------------- MAIN (保持你的選單與 I/O 邏輯) -----------------
int main() {
    Order* dyn_orders = nullptr;
    int dyn_count = 0;
    bool task2_done = false;
    int last_file_number = 0;

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
            last_file_number = file_number;
            std::vector<Order> orders = IOHandler::ReadOrdersFromFile(file_number);
            if (orders.empty()) {
                std::cerr << "No orders read from input" << file_number << ".txt or file not found.\n";
                continue;
            }
            std::cout << "原始 orders:\n";
            for (auto &o : orders) {
                std::cout << o.oid << "\t" << o.arrival << "\t" << o.duration << "\t" << o.timeout << "\n";
            }
            IOHandler::ShellSort(orders);
            IOHandler::WriteSortedListToFile(orders, file_number);
            std::cout << "Sorting done and written to sorted" << file_number << ".txt\n";
        }
        else if(choice==2) {
            std::cout << "Enter file number (e.g., 401): ";
            std::cin >> file_number;
            last_file_number = file_number;
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
            std::vector<Order> ordersVec;
            for (int i = 0; i < dyn_count; ++i) ordersVec.push_back(dyn_orders[i]);
            order_system os(1, file_number);
            os.LoadOrders(ordersVec);
            os.SimulateQueues(1);
        }
        else if(choice==3) {
            if (!task2_done) {
                std::cerr << "Error: You must run task 2 first to load sorted file into memory. Task 3 aborted.\n";
                continue;
            }
            std::vector<Order> ordersVec;
            for (int i = 0; i < dyn_count; ++i) ordersVec.push_back(dyn_orders[i]);
            order_system os(2, last_file_number);
            os.LoadOrders(ordersVec);
            os.SimulateQueues(2);
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
            std::vector<Order> ordersVec;
            for (int i = 0; i < dyn_count; ++i) ordersVec.push_back(dyn_orders[i]);
            order_system os(N, last_file_number);
            os.LoadOrders(ordersVec);
            os.SimulateQueues(N);
        }
        else {
            std::cout << "Invalid command!\n";
        }
    }

    if (dyn_orders) delete[] dyn_orders;
    std::cout << "Program terminated.\n";
    return 0;
}
