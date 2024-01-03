#include <bits/stdc++.h>

using namespace std;

const int NUM_EMPLOYEES_A = 2;
const int NUM_EMPLOYEES_B = 2;
const int NUM_EMPLOYEES_C = 3;
const int WORKING_HOURS = 7 * 60; // 7 hours in minutes
const int A_SERVICE_TIME = 3;
const int B_SERVICE_TIME = 10;
const int C_SERVICE_TIME = 15;

set<tuple<int, int, int, int, bool, int>> allCustomers;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

class ServiceEmployee
{
public:
    int number;
    bool isAvailable;
    int workingTime;
    int idleTime;

    ServiceEmployee(int number)
    {
        this->number = number;
        this->isAvailable = true;
        this->workingTime = 0;
        this->idleTime = 0;
    }
};

class Customer
{
public:
    int arrivalTime;
    int serviceTime;
    int serviceStartedTime;
    bool requiredService;
    int number;

    int waitTime;
    int employeeNumber;
    int timeWhenGetsA;

    // Default constructor
    Customer() {}

    // One argument constructor
    Customer(int arrivalTime, int serviceTime, int serviceStartedTime, bool requiredService, int number)
    {
        this->arrivalTime = arrivalTime;
        this->serviceTime = serviceTime;
        this->serviceStartedTime = serviceStartedTime;
        this->requiredService = requiredService;
        this->number = number;
        this->timeWhenGetsA = -1; // Initialize timeWhenGetsA to -1
    }
};

struct CustomerOverview
{
    int arrivalTime;
    int firstServiceTime;
    int firstServiceEmployeeNum;
    int secondServiceTime;
    int secondServiceEmployeeNum;
    string service;
};

class BankSimulation
{
private:
    queue<Customer> aWaitingQueue, aWorkingQueue; // A Queues
    queue<Customer> bWaitingQueue, bWorkingQueue; // B Queues
    queue<Customer> cWaitingQueue, cWorkingQueue; // C Queues

    // statistics
    int totalWaitingTime = 0; // for all customers
    int employeeID = -1;      // Default value if vectors are empty

    // Variables to store working and idle times for each employee
    vector<ServiceEmployee> aEmployees = {ServiceEmployee(0), ServiceEmployee(1)};
    vector<ServiceEmployee> bEmployees = {ServiceEmployee(0), ServiceEmployee(1)};
    vector<ServiceEmployee> cEmployees = {ServiceEmployee(0), ServiceEmployee(1), ServiceEmployee(2)};

    vector<Customer> aServedCustomers, bServedCustomers, cServedCustomers;
    vector<Customer> allservedCustomers;

    int aNumOfTotalWaitingCustomers = 0, bNumOfTotalWaitingCustomers = 0, cNumOfTotalWaitingCustomers = 0;
    int aMaxNumOfWaitingCustomers = 0, bMaxNumOfWaitingCustomers = 0, cMaxNumOfWaitingCustomers = 0;

    CustomerOverview customersOverview[700];

public:
    // Default constructor
    BankSimulation()
    {
    }

    // Generate a random integer in the range 0..9
    bool getServiceType()
    {                                                                 // 0 : B , 1: C
        int randomInteger = uniform_int_distribution<int>(0, 9)(rng); // 0..9
        if (randomInteger < 4)
            return 0; // 40% chance of B

        return 1; // 60% chance of C
    }

    int getNextCustomerDelay()
    {
        int randomInteger = uniform_int_distribution<int>(0, 9)(rng); // 0..9
        if (randomInteger < 1)
            return 1;
        else if (randomInteger < 3)
            return 2;
        else if (randomInteger < 7)
            return 3;
        else if (randomInteger < 9)
            return 4;

        return 5;
    }

    int getServiceProviderNumber(vector<ServiceEmployee> &employees)
    {
        vector<int> availableIndicies;
        for (int i = 0; i < employees.size(); i++)
        {
            if (employees[i].isAvailable)
            {
                availableIndicies.push_back(i);
            }
        }

        if (availableIndicies.empty())
        {
            // Handle the case when no employees are available
            return -1; // or any other suitable value
        }

        int randomIndex = uniform_int_distribution<int>(0, availableIndicies.size() - 1)(rng);
        int employeeIndex = availableIndicies[randomIndex];
        return employeeIndex;
    }

    int getNumAvailableEmployees(vector<ServiceEmployee> &employees)
    {
        int num = 0;
        for (auto &employee : employees)
        {
            num += employee.isAvailable;
        }
        return num;
    }

    bool customersExist()
    {
        return getNumAvailableEmployees(aEmployees) != NUM_EMPLOYEES_A ||
               getNumAvailableEmployees(bEmployees) != NUM_EMPLOYEES_B ||
               getNumAvailableEmployees(cEmployees) != NUM_EMPLOYEES_C;
    }

    void moveCustomerToNextQueue(int time, Customer &customer)
    {
        customersOverview[customer.number].service = customer.requiredService ? "C" : "A";
        customersOverview[customer.number].firstServiceTime = time;
        customersOverview[customer.number].firstServiceEmployeeNum = customer.employeeNumber;

        customer.serviceStartedTime = -1;
        customer.serviceTime = customer.requiredService ? C_SERVICE_TIME : B_SERVICE_TIME;
        queue<Customer> *workingQueue = customer.requiredService ? &cWorkingQueue : &bWorkingQueue;
        queue<Customer> *waitingQueue = customer.requiredService ? &cWaitingQueue : &bWaitingQueue;
        vector<ServiceEmployee> *employees = customer.requiredService ? &cEmployees : &bEmployees;

        allCustomers.insert(make_tuple(
            customer.number,
            customer.arrivalTime,
            customer.timeWhenGetsA,
            customer.employeeNumber,
            customer.requiredService,
            1));

        customer.arrivalTime = time;
        if (getNumAvailableEmployees(*employees))
        {
            // There are employee(s) available
            int employeeNumber = getServiceProviderNumber(*employees);
            customer.employeeNumber = employeeNumber;
            employees->at(employeeNumber).isAvailable = 0;
            customer.serviceStartedTime = time;
            workingQueue->push(customer);
        }
        else
        {
            // There aren't any employees available
            waitingQueue->push(customer);
        }
        if (!customer.requiredService && customer.timeWhenGetsA == -1)
        {
            customer.timeWhenGetsA = time; // Set the time when the customer gets A
        }
    }

    void processQueue(int time, queue<Customer> &waitingQueue, queue<Customer> &workingQueue, vector<ServiceEmployee> &employees, vector<Customer> &servedCustomers, bool shouldMoveToNextQueue)
    {
        if (!workingQueue.empty())
        {

            Customer customer = workingQueue.front();
            customer.timeWhenGetsA = customer.arrivalTime + customer.waitTime + 3;
            if (time - customer.serviceStartedTime == customer.serviceTime)
            {
                workingQueue.pop();
                servedCustomers.push_back(customer);
                allservedCustomers.push_back(customer);
                employees[customer.employeeNumber]
                    .isAvailable = 1;

                // Only when processing A queue
                if (shouldMoveToNextQueue)
                {
                    moveCustomerToNextQueue(time, customer);
                }
                else
                {
                    customersOverview[customer.number].secondServiceTime = time;
                    customersOverview[customer.number].secondServiceEmployeeNum = customer.employeeNumber;
                }

                if (!waitingQueue.empty())
                {
                    Customer customer = waitingQueue.front();
                    waitingQueue.pop();
                    int employeeNumer = getServiceProviderNumber(employees);
                    employees[employeeNumer].isAvailable = 0;
                    customer.serviceStartedTime = time;
                    customer.employeeNumber = employeeNumer;
                    customer.waitTime = customer.serviceStartedTime - customer.arrivalTime;
                    totalWaitingTime += customer.waitTime;
                    workingQueue.push(customer);
                }
            }
        }
    }

    void processQueues(int time)
    {
        processQueue(time, cWaitingQueue, cWorkingQueue, cEmployees, cServedCustomers, false);
        processQueue(time, bWaitingQueue, bWorkingQueue, bEmployees, bServedCustomers, false);
        processQueue(time, aWaitingQueue, aWorkingQueue, aEmployees, aServedCustomers, true);
    }

    void outputCustomer(ofstream &file, const vector<Customer> &customers, const string &serviceType = "")
    {
        file << left << setw(15) << "CustomerNumber" << setw(15) << "ArrivalTime" << setw(20) << "ServiceStartTime" << setw(15) << "WaitTime" << setw(15) << "EmployeeNumber";
        if (!serviceType.empty())
        {
            file << setw(15) << "ServiceType";
        }
        file << endl;

        for (const auto &customer : customers)
        {
            file << left << setw(15) << customer.number << setw(15) << customer.arrivalTime << setw(20)
                 << customer.arrivalTime + customer.waitTime << setw(15) << customer.waitTime << setw(15) << customer.employeeNumber;
            if (!serviceType.empty())
            {
                file << setw(15) << serviceType;
            }
            file << endl;
        }
    }
    void outputEmployeeTimes(const vector<ServiceEmployee> &employees, const string &filename)
    {
        ofstream file(filename);

        file << left << setw(15) << "EmployeeNumber" << setw(20) << "WorkingTime" << setw(20) << "IdleTime" << endl;

        for (auto &employee : employees)
        {
            file << left << setw(15) << employee.number << setw(20) << employee.workingTime << setw(20) << employee.idleTime << endl;
        }

        file.close();
    }

    void outputEmployeeTimes()
    {
        outputEmployeeTimes(aEmployees, "employee_times_A.txt");
        outputEmployeeTimes(bEmployees, "employee_times_B.txt");
        outputEmployeeTimes(cEmployees, "employee_times_C.txt");
    }

    void checkEmployeesWorkStatus()
    {
        for (auto &employee : aEmployees)
        {
            employee.idleTime += employee.isAvailable;
            employee.workingTime += !employee.isAvailable;
        }
        for (auto &employee : bEmployees)
        {
            employee.idleTime += employee.isAvailable;
            employee.workingTime += !employee.isAvailable;
        }
        for (auto &employee : cEmployees)
        {
            employee.idleTime += employee.isAvailable;
            employee.workingTime += !employee.isAvailable;
        }
    }

    void checkNumOfWaitingCustomer()
    {
        aNumOfTotalWaitingCustomers += aWaitingQueue.size();
        bNumOfTotalWaitingCustomers += bWaitingQueue.size();
        cNumOfTotalWaitingCustomers += cWaitingQueue.size();
        aMaxNumOfWaitingCustomers = max(aMaxNumOfWaitingCustomers, (int)aWaitingQueue.size());
        bMaxNumOfWaitingCustomers = max(bMaxNumOfWaitingCustomers, (int)bWaitingQueue.size());
        cMaxNumOfWaitingCustomers = max(cMaxNumOfWaitingCustomers, (int)cWaitingQueue.size());
    }

    void simulate()
    {
        int time = 0, customerDelay = 0, customerNumber = 1;
        while (time < WORKING_HOURS || customersExist())
        {
            if (time < WORKING_HOURS && !customerDelay)
            {
                // New customer arrived
                bool serviceType = getServiceType();
                Customer customer;
                customersOverview[customerNumber].arrivalTime = time;

                // All A employees are busy
                if (!getNumAvailableEmployees(aEmployees))
                {
                    customer = Customer(time, A_SERVICE_TIME, -1, serviceType, customerNumber++);
                    aWaitingQueue.push(customer);
                }
                else
                {
                    int employeeNumer = getServiceProviderNumber(aEmployees);
                    aEmployees[employeeNumer].isAvailable = 0;
                    customer = Customer(time, A_SERVICE_TIME, time, serviceType, customerNumber++);
                    customer.employeeNumber = employeeNumer;
                    aWorkingQueue.push(customer);
                }

                customerDelay = getNextCustomerDelay();
            }

            processQueues(time);
            checkEmployeesWorkStatus();
            checkNumOfWaitingCustomer();

            time++;
            customerDelay--;
        }

        ofstream fileA("events_A.txt");
        ofstream fileB("events_B.txt");
        ofstream fileC("events_C.txt");
        ofstream fileStats("Stats.txt");
        ofstream fileAll("events_All.txt", ios::app); // Open in append mode

        // Include column titles for fileA, fileB, fileC, and fileAll

        // Include column titles for fileA
        fileA << left << setw(15) << "CustomerNumber" << setw(15) << "ArrivalTime" << setw(20) << "ServiceStartTime" << setw(15) << "WaitTime" << setw(15) << "EmployeeNumber" << setw(15) << "NextServiceType" << endl;

        for (const auto &customer : aServedCustomers)
        {
            fileA << left << setw(15) << customer.number << setw(15) << customer.arrivalTime << setw(20)
                  << customer.arrivalTime + customer.waitTime << setw(15) << customer.waitTime << setw(15) << customer.employeeNumber << setw(15) << (customer.requiredService ? "C" : "B") << endl;
        }

        outputCustomer(fileB, bServedCustomers, "B");
        outputCustomer(fileC, cServedCustomers, "C");

        fileA.close();
        fileB.close();
        fileC.close();

        // Close fileAll before reusing the name
        fileAll.close();

        // Open fileAll for writing in append mode
        ofstream fileAllWrite("events_All.txt", ios::trunc);

        // Include column titles for fileAll (only if it's a new file)
        if (fileAllWrite.tellp() == 0)
        {
            fileAllWrite << left << setw(15) << "CustomerNumber" << setw(15) << "ArrivalTime" << setw(15) << "CustomerEndsA" << setw(15) << "AEmployeeID" << setw(15) << "B or C" << setw(15) << "B|C employeeID" << setw(15) << "LeaveTime" << endl;
        }

        // Write your original data (allCustomers) to fileAllWrite
        for (int i = 1; i < customerNumber; i++)
        {
            fileAllWrite << left << setw(15) << i << setw(15) << customersOverview[i].arrivalTime << setw(15) << customersOverview[i].firstServiceTime << setw(15) << customersOverview[i].firstServiceEmployeeNum << setw(15) << customersOverview[i].service << setw(15) << customersOverview[i].secondServiceEmployeeNum << setw(15) << customersOverview[i].secondServiceTime << endl;
        }
        fileAllWrite.close();

        // Output additional information if needed

        cout << "Number of served customers: " << customerNumber - 1 << endl;

        cout << "Average wait is about: " << totalWaitingTime / (customerNumber - 1) << " minutes" << endl;

        double aAvgWaiting = aNumOfTotalWaitingCustomers / aServedCustomers.size();
        double bAvgWaiting = bNumOfTotalWaitingCustomers / bServedCustomers.size();
        double cAvgWaiting = cNumOfTotalWaitingCustomers / cServedCustomers.size();

        int totalAvgWaitingTime = (aAvgWaiting + bAvgWaiting + cAvgWaiting) / 3;
        cout << "Average number of waiting customers in a certain time: " << totalAvgWaitingTime << "\n\n";

        cout << "Maximum number of A waiting customers: " << aMaxNumOfWaitingCustomers << endl;
        cout << "Maximum number of B waiting customers: " << bMaxNumOfWaitingCustomers << endl;
        cout << "Maximum number of C waiting customers: " << cMaxNumOfWaitingCustomers << endl;

        int maxNumOfWaitingCustomers = aMaxNumOfWaitingCustomers + bMaxNumOfWaitingCustomers + cMaxNumOfWaitingCustomers;
        cout << "Maximum number of all waiting customers: " << maxNumOfWaitingCustomers << endl;
        cout << "\n";
        fileStats << left << setw(15) << "Number of served customers: " << customerNumber - 1 << endl;
        fileStats << left << setw(15) << "Average wait is about: " << totalWaitingTime / (customerNumber - 1) << " minutes" << endl;
        fileStats << left << setw(15) << "Average number of waiting customers in a certain time: " << totalAvgWaitingTime << endl;
        fileStats << left << setw(15) << "Maximum number of A waiting customers: " << aMaxNumOfWaitingCustomers << endl;
        fileStats << left << setw(15) << "Maximum number of B waiting customers: " << bMaxNumOfWaitingCustomers << endl;
        fileStats << left << setw(15) << "Maximum number of C waiting customers: " << cMaxNumOfWaitingCustomers << endl;
        fileStats << left << setw(15) << "Maximum number of all waiting customers: " << maxNumOfWaitingCustomers << endl;
        fileStats.close();

        outputEmployeeTimes();
    }
};

int main()
{
    BankSimulation BankSim;
    BankSim.simulate();
    return 0;
}
