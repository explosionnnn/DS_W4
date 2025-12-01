#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <sys/stat.h>
#include <algorithm>

struct Order {
    int oid;
    int arrival;
    int duration;
    int timeout;
    int seq;
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
    int seq;
};

struct TimeoutList {
    int oid;
    int cid;
    int delay;
    int departure;
    int seq;
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

};

class Chef {
    private:
        int free_time; //訂單結束時間
        int start_time; //訂單開始時間
        Order now;
    public:
        Chef() { free_time = 0; now = {0, 0, 0, 0}; start_time = 0;}

        bool IsFree(int current_time) {
        // 如果目前沒有任務 (now.oid == 0) 或者已經到達/超過空閒時間
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
            std::string header;
            std::getline(infile, header); // skip header
            int oid, arrival, duration, timeout;
            while(infile >> oid >> arrival >> duration >> timeout) {
                orders.push_back({oid, arrival, duration, timeout});
            }
            return orders;
        }

        static std::vector<Order> ReadSortedOrdersFromFile(int file_number) {
            std::vector<Order> orders;
            std::ifstream infile("sorted" + std::to_string(file_number) + ".txt");
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
            std::getline(infile, header);
            std::vector<Order> tmp;
            int oid, arrival, duration, timeout;
            while(infile >> oid >> arrival >> duration >> timeout) {
                // if (duration > 0 && arrival + duration <= timeout) {
                    tmp.push_back({oid, arrival, duration, timeout});
                // }
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

        static bool Exists(const std::string &name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

        static void WriteTimeoutListToFile(const std::vector<TimeoutList>& timeout_list,int file_number,std::string prefix) {
            std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
            outfile << "\t[Timeout List]\n";
            outfile << "\tOID\tCID\tDelay\tDeparture\n";
            for (size_t i=0;i<timeout_list.size();i++) {
                outfile << "[" << i+1 << "]\t" << timeout_list[i].oid << "\t";
                outfile << timeout_list[i].cid << "\t" << timeout_list[i].delay << "\t";
                outfile << timeout_list[i].departure << "\n";
            }
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
        std::vector<AbortList> abort_list_this_pass;
        std::vector<TimeoutList> timeout_list_this_pass;
        std::vector<Order> main_orders;
        int total_delay;
        int file_number;
    public:
        order_system(int num, int file_num) {
            chef_num = num;
            which_chef = 0;
            chefs = new Chef[chef_num];
            queues = new Queue[chef_num];
            total_delay = 0;
            file_number = file_num;
        }

        ~order_system() { delete[] chefs; delete[] queues; }

        void LoadOrders(const std::vector<Order>& orders) {
            main_orders = orders;
        }

        bool IsNextOrderIvalid() {
            if (!main_orders.empty()) {
                Order o = main_orders[0];
                return (o.duration <= 0 || o.arrival + o.duration > o.timeout);
            }
            return false;
        }

        void ShellSort(int n, std::vector<Order> target) {
            Order temp;
            for (int gap = n/2; gap > 0; gap /=2) {
                for (int i = gap; i < n; i++) {

                    temp.oid = target[i].oid;
                    int j = i;

                    while (j-gap >= 0 && target[j-gap].oid > temp.oid) {
                        target[j] = target[j-gap];
                        j -= gap;
                    }
                    target[j] = temp;
                }
            }
        }

        bool AllocateOrders() {
            if (main_orders.empty()) return false;
            int bestlen = 10000;
            int bestidx = -1;
            Order o = main_orders[0];

            for (int i = 0; i < chef_num; i++) {
                if (chefs[i].IsFree(clock.clk) && queues[i].IsEmpty()) {
                    chefs[i].DoThisOrder(o, clock.clk);
                    main_orders.erase(main_orders.begin());
                    return true;
                }
            }
            for (int i = 0; i < chef_num; i++) { //bug??
                if (queues[i].GetLength() < bestlen && !queues[i].QueueFull()) {
                    bestidx = i;
                    bestlen = queues[i].GetLength();
                }
            }
            if (bestidx != -1) {
                queues[bestidx].push(o);
                main_orders.erase(main_orders.begin());
                return true;
            }
            return false;
        }

        void AddOrder() {
            while (!main_orders.empty() && main_orders[0].arrival == clock.clk) {
                bool success = AllocateOrders();
                if (!success) {
                    SetAbort();
                }
            }
        }

        bool IsTimeout(const Order &o, int finish_time) {
            if (finish_time > o.timeout) {
                return true;
            }
            return false;
        }

        void SetAbort() { //第一種情況
            AbortList a = {main_orders[0].oid, 0, 0, main_orders[0].arrival};
            abort_list.push_back(a);
            main_orders.erase(main_orders.begin());
        }

        void SetAbort(const Order& o, int num) { //第二種情況
            //閒置時刻為結束時刻 == 現在時間
            AbortList a = {o.oid, num+1, clock.clk-o.arrival, clock.clk};
            abort_list_this_pass.push_back(a);
            queues[num].pop();
        }

        void ChefProcessOrders(int num) {
            while (!queues[num].IsEmpty() && chefs[num].GetFreeTime() <= clock.clk) {
                Order o = queues[num].GetHeadOrder();
                if (o.timeout < clock.clk) {
                    SetAbort(o, num);
                    continue;
                }
                else if (IsTimeout(o, clock.clk + o.duration)) {
                    chefs[num].DoThisOrder(o, clock.clk);
                    int delay = clock.clk - o.arrival;
                    TimeoutList t = {o.oid, num+1 , delay, clock.clk + o.duration};
                    timeout_list_this_pass.push_back(t);
                    queues[num].pop();
                    continue;
                }
                chefs[num].DoThisOrder(o, clock.clk);
                queues[num].pop();
            }
            if (chefs[num].IsFree(clock.clk) && queues[num].IsEmpty()) {
                chefs[num].SetFree();
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

        void SimulateQueues(int N) {
            std::string prefix;
            if(N==1) prefix="one";
            else if(N==2) prefix="two";
            else prefix="any";
            size_t idx = 0;
            int ignore_list = 0;
            int total_orders = main_orders.size();
            while (true) {
                if (main_orders.empty() && AllQueuesEmpty() && AllChefsFree()) {
                    break;
                }
                if (IsNextOrderIvalid()) {
                    main_orders.erase(main_orders.begin());
                    ignore_list++;
                    continue;
                }
                // next event = min( next arrival, next chef finish time (> clock) )
                int next_time = -1;
                if (!main_orders.empty()) {
                    next_time = main_orders[0].arrival;
                }
                for (int i = 0; i < chef_num; ++i) {
                    int ft = chefs[i].GetFreeTime();
                    if (ft > clock.clk) {
                        if (next_time == -1 || ft < next_time) next_time = ft;
                    }
                }
                if (next_time != -1 && clock.clk < next_time) {
                    clock.clk = next_time;
                }

                /* step 1: （untill chef  free_time > clock） */
                for (int i = 0; i < chef_num; ++i) {
                    ChefProcessOrders(i);
                }
                /* step 2: new order -> （arrival == clock）*/
                while (!main_orders.empty() && main_orders[0].arrival == clock.clk) {
                    if (IsNextOrderIvalid()) {
                        main_orders.erase(main_orders.begin());
                        ignore_list++;
                        continue;
                    }
                    bool success = AllocateOrders();
                    if (!success) {
                        SetAbort();
                    }
                }
                /*stable sort abort list and timeout list in this pass by CID (ascending)*/
                std::stable_sort(abort_list_this_pass.begin(), abort_list_this_pass.end(),
                [](const AbortList &a, const AbortList &b) {
                    if (a.cid != b.cid) return a.cid < b.cid;
                    return a.seq < b.seq; // tie-breaker 保持原序（通常 seq 是原插入順）
                });

                abort_list.insert(abort_list.end(), abort_list_this_pass.begin(), abort_list_this_pass.end());

// 同理處理 timeout
                std::stable_sort(timeout_list_this_pass.begin(), timeout_list_this_pass.end(),
                    [](const TimeoutList &a, const TimeoutList &b) {
                        if (a.cid != b.cid) return a.cid < b.cid;
                        return a.seq < b.seq;
                    });
                timeout_list.insert(timeout_list.end(), timeout_list_this_pass.begin(), timeout_list_this_pass.end());
                abort_list_this_pass.clear();
                timeout_list_this_pass.clear();
            }
            // 計算總延遲
            total_delay=0;
            for(auto a:abort_list) total_delay+=a.delay;
            for(auto t:timeout_list) total_delay+=t.delay;

            double failure_percentage = (abort_list.size()+timeout_list.size())*100.0/(total_orders-ignore_list);

            IOHandler::WriteAbortListToFile(abort_list,file_number,prefix);
            IOHandler::WriteTimeoutListToFile(timeout_list,file_number,prefix);
            std::ofstream outfile(prefix + std::to_string(file_number) + ".txt", std::ios::app);
            outfile << "[Total Delay]\n";
            outfile << total_delay << " min.\n";
            outfile << "[Failure Percentage]\n";
            outfile << std::fixed << std::setprecision(2) << failure_percentage << " %\n";
        }
};

class MenuSystem {
    private:
        Order* dyn_orders = nullptr;
        int dyn_count = 0;
        bool has_sorted_file = false;
        bool task2_done = false;
        int current_file_number = -1;

        void CommandSortFile() {
            std::string file_input;
            int file_number;
            std::cout << "\nInput a file number (e.g., 401, 402, 403, ...): ";
            std::cin >> file_input;
            try {
                file_number = std::stoi(file_input);
            } catch (...) {
                std::cout << "\n### input" << file_input << ".txt does not exist! ###\n\n";
                return;
            }
            auto t0 = std::chrono::high_resolution_clock::now();
            std::vector<Order> orders = IOHandler::ReadOrdersFromFile(file_number);
            auto t1 = std::chrono::high_resolution_clock::now();
            if (orders.empty()) {
                std::cout << "\n### input" << file_number << ".txt does not exist! ###\n\n";
                return;
            }

            std::cout << "\n\tOID\tArrival\tDuration\tTimeOut\n";
            int index = 1;
            for (auto &o : orders) {
                std::cout << "(" << index << ")"  << " \t" << o.oid << "\t" << o.arrival 
                        << "\t" << o.duration << "\t" << o.timeout << "\n";
                ++index;
            }

            auto t2 = std::chrono::high_resolution_clock::now();
            IOHandler::ShellSort(orders);
            auto t3 = std::chrono::high_resolution_clock::now();
            IOHandler::WriteSortedListToFile(orders, file_number);
            auto t4 = std::chrono::high_resolution_clock::now();

            std::cout << "\nReading data: " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << " us.\n\n";
            std::cout << "Sorting data: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() << " us.\n\n";
            std::cout << "Writing data: " << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() << " us.\n";

            has_sorted_file = true;
        }

        void CommandSimulateOneQueue() {
            std::string file_input;
            int file_number;
            std::cout << "\nInput a file number (e.g., 401, 402, 403, ...): ";
            std::cin >> file_input;
            try {
                file_number = std::stoi(file_input);
            } catch (...) {
                std::cout << "\n### sorted" << file_input << ".txt does not exist! ###\n";
                return;
            }
            current_file_number = file_number;
            std::string sorted_name = "sorted" + std::to_string(file_number) + ".txt";
            if (!IOHandler::Exists(sorted_name)) {
                std::cout << "\n### sorted" << file_number << ".txt does not exist! ###\n";
                return;
            }

            if (dyn_orders) { delete[] dyn_orders; dyn_orders = nullptr; dyn_count = 0; task2_done = false; }
            if (!IOHandler::ReadSortedToDynamic(dyn_orders, dyn_count, file_number)) {
                std::cout << "Failed to read sorted file into dynamic array.\n";
                return;
            }

            std::cout << "\n\tOID\tArrival\tDuration\tTimeOut\n";
            for (int i = 0; i < dyn_count; ++i) {
                Order &o = dyn_orders[i];
                std::cout << "(" << i+1 << ")" << " \t" << o.oid << "\t" << o.arrival 
                        << "\t" << o.duration << "\t" << o.timeout << "\n";
            }
            task2_done = true;

            std::vector<Order> ordersVec(dyn_orders, dyn_orders + dyn_count);
            order_system os(1, file_number);
            os.LoadOrders(ordersVec);
            os.SimulateQueues(1);
        }

        void CommandSimulateTwoQueues() {
            if (!task2_done) {
                std::cout << "\n### Execute command 2 first! ###\n";
                return;
            }
            std::vector<Order> ordersVec(dyn_orders, dyn_orders + dyn_count);
            order_system os(2, current_file_number);
            os.LoadOrders(ordersVec);
            os.SimulateQueues(2);
        }

        void CommandSimulateNQueues() {
            if (!task2_done) {
                std::cout << "\n### Execute command 2 first! ###\n";
                return;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::string s;
            int N = 0;
            while (true) {
                std::cout << "\nInput the number of queues: ";
                std::getline(std::cin, s);
                try {
                    N = std::stoi(s);
                    if (N > 0) break;
                } catch (...) { continue; }
            }
            std::vector<Order> ordersVec(dyn_orders, dyn_orders + dyn_count);
            order_system os(N, current_file_number);
            os.LoadOrders(ordersVec);
            os.SimulateQueues(N);
        }

    public:
        void Run() {
            while (true) {
                std::cout << "*** (^_^) Data Structure (^o^) ***\n";
                std::cout << "** Simulate FIFO Queues by SQF ***\n";
                std::cout << "* 0. Quit                        *\n";
                std::cout << "* 1. Sort a file                 *\n";
                std::cout << "* 2. Simulate one FIFO queue     *\n";
                std::cout << "* 3. Simulate two queues by SQF  *\n";
                std::cout << "* 4. Simulate some queues by SQF *\n";
                std::cout << "**********************************\n";
                std::cout << "Input a command(0, 1, 2, 3, 4): ";

                std::string choice_input;
                int choice;
                try {
                    std::cin >> choice_input;
                    choice = std::stoi(choice_input);
                } catch (...) {
                    break;
                }
                if (choice == 0) break;

                switch (choice) {
                    case 1: CommandSortFile(); break;
                    case 2: CommandSimulateOneQueue(); break;
                    case 3: CommandSimulateTwoQueues(); break;
                    case 4: CommandSimulateNQueues(); break;
                    default: std::cout << "\nCommand does not exist!\n\n"; break;
                }
                std::cout << "\n";
            }

            if (dyn_orders) delete[] dyn_orders;
        }
};

int main() {
    MenuSystem menu;
    menu.Run();
    return 0;
}