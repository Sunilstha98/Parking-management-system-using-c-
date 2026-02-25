#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime> //for time calculation
#include <iomanip>
#include <cstdlib>
#include <iomanip>
#include <cmath>
using namespace std;

struct Slot
{
    int id;
    int status; // 0 = empty, 1 = occupied
};

// main menu
void showMainMenu()
{
    cout << "\n=====================================\n";
    cout << "      PARKING MANAGEMENT SYSTEM\n";
    cout << "=====================================\n";
    cout << "1. Admin Login\n";
    cout << "2. Parking Operator Login\n";
    cout << "3. Exit\n";
    cout << "-------------------------------------\n";
    cout << "Enter your choice: ";
}

// login logic
bool login(string role)
{

    while (true)
    {

        string username, password;

        cout << "\nEnter Username: ";
        cin >> username;

        cout << "Enter Password: ";
        cin >> password;

        ifstream file("data/users.csv");
        string line;

        bool userFound = false;

        while (getline(file, line))
        {

            stringstream ss(line);
            string fileUser, filePass, fileRole;

            getline(ss, fileUser, ',');
            getline(ss, filePass, ',');
            getline(ss, fileRole, ',');

            if (username == fileUser && role == fileRole)
            {

                userFound = true;

                if (password == filePass)
                {
                    file.close();
                    cout << "\nLogin Successful!\n";
                    return true;
                }
                else
                {
                    cout << "\nInvalid Password! Try Again.\n";
                    break; // break file loop, retry login
                }
            }
        }

        file.close();

        if (!userFound)
        {
            cout << "\nInvalid Username! Try Again.\n";
        }
    }
}

// load slot from csv file
vector<Slot> loadSlots()
{
    vector<Slot> slots;
    ifstream file("data/parking_slots.csv");

    int id, status;
    char comma;

    while (file >> id >> comma >> status)
    {
        slots.push_back({id, status});
    }

    file.close();
    return slots;
}

// Display slots
void displaySlots()
{
    vector<Slot> slots = loadSlots();

    cout << "\n--- Parking Slots ---\n";

    for (auto &s : slots)
    {
        cout << "Slot " << s.id << " : ";
        if (s.status == 0)
            cout << "Empty\n";
        else
            cout << "Occupied\n";
    }
}

// save slot back to file
void saveSlots(vector<Slot> &slots)
{
    ofstream file("data/parking_slots.csv");

    for (auto &s : slots)
    {
        file << s.id << "," << s.status << "\n";
    }

    file.close();
}

// find available slot
int findAvailableSlot()
{
    vector<Slot> slots = loadSlots();

    for (auto &s : slots)
    {
        if (s.status == 0)
            return s.id;
    }

    return -1; // parking full
}

// Mark slot occupied
void occupySlot(int slotId)
{
    vector<Slot> slots = loadSlots();

    for (auto &s : slots)
    {
        if (s.id == slotId)
        {
            s.status = 1;
            break;
        }
    }

    saveSlots(slots);
}

// free slot
void freeSlot(int slotId)
{
    vector<Slot> slots = loadSlots();

    for (auto &s : slots)
    {
        if (s.id == slotId)
        {
            s.status = 0;
            break;
        }
    }

    saveSlots(slots);
}

// Time function
string getCurrentTime()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);

    stringstream ss;
    ss << 1900 + ltm->tm_year << "-"
       << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
       << setw(2) << setfill('0') << ltm->tm_mday << " "
       << setw(2) << setfill('0') << ltm->tm_hour << ":"
       << setw(2) << setfill('0') << ltm->tm_min << ":"
       << setw(2) << setfill('0') << ltm->tm_sec;

    return ss.str();
}

// generate token
int generateToken()
{
    return rand() % 90000 + 10000; // 5 digit token
}

// vehicle entry
void vehicleEntry()
{
    vector<Slot> slots = loadSlots();

    int slotId = findAvailableSlot();

    if (slotId == -1)
    {
        cout << "\nParking Full!\n";
        return;
    }

    string vehicleNo, vehicleType;

    cout << "\nEnter Vehicle Number: ";
    cin >> vehicleNo;

    cout << "Enter Vehicle Type (bike/car/bus): ";
    cin >> vehicleType;

    int token = generateToken();
    string entryTime = getCurrentTime();

    // Save transaction
    ofstream file("data/transactions.csv", ios::app);

    file << token << ","
         << vehicleNo << ","
         << vehicleType << ","
         << slotId << ","
         << entryTime << ","
         << "NULL,"
         << "0\n";

    file.close();

    occupySlot(slotId);

    cout << "\n===== PARKING TOKEN =====\n";
    cout << "Token ID : " << token << endl;
    cout << "Slot ID  : " << slotId << endl;
    cout << "Entry    : " << entryTime << endl;
}

// get rate
double getRate(string vehicleType)
{
    ifstream file("data/rates.txt");

    string type;
    double rate;

    while (file >> type >> rate)
    {
        if (type == vehicleType)
        {
            file.close();
            return rate;
        }
    }

    file.close();
    return 0;
}

// calculate time difference
double calculateHours(string entryTime)
{
    tm tmEntry = {};
    stringstream ss(entryTime);

    ss >> get_time(&tmEntry, "%Y-%m-%d %H:%M:%S");

    time_t entry = mktime(&tmEntry);
    time_t now = time(0);

    double seconds = difftime(now, entry);

    return seconds / 3600.0; // convert to hours
}

// vehicle Exit
void vehicleExit()
{
    int tokenInput;
    cout << "\nEnter Token ID: ";
    cin >> tokenInput;

    ifstream file("data/transactions.csv");
    ofstream temp("data/temp.csv");

    string line;
    bool found = false;

    while (getline(file, line))
    {

        if (line.find(to_string(tokenInput)) != string::npos)
        {

            stringstream ss(line);

            string token, vehicleNo, vehicleType, slotId, entryTime, exitTime, amount;

            getline(ss, token, ',');
            getline(ss, vehicleNo, ',');
            getline(ss, vehicleType, ',');
            getline(ss, slotId, ',');
            getline(ss, entryTime, ',');
            getline(ss, exitTime, ',');
            getline(ss, amount, ',');

            double hours = calculateHours(entryTime);
            double rate = getRate(vehicleType);
            double total = ceil(hours) * rate; // charge per full hour

            string currentTime = getCurrentTime();

            temp << token << ","
                 << vehicleNo << ","
                 << vehicleType << ","
                 << slotId << ","
                 << entryTime << ","
                 << currentTime << ","
                 << total << "\n";

            freeSlot(stoi(slotId));

            cout << "\n===== BILL =====\n";
            cout << "Vehicle: " << vehicleNo << endl;
            cout << "Hours  : " << hours << endl;
            cout << "Amount : Rs. " << total << endl;
            cout << fixed << setprecision(2);
            cout << "Thanks for giving us chance to help you" << endl;

            found = true;
        }
        else
        {
            temp << line << "\n";
        }
    }

    file.close();
    temp.close();

    remove("data/transactions.csv");
    rename("data/temp.csv", "data/transactions.csv");

    if (!found)
    {
        cout << "Invalid Token!\n";
    }
}

// operator menu
void operatorMenu()
{

    int choice;

    while (true)
    {

        cout << "\n=====================================\n";
        cout << "      OPERATOR MENU\n";
        cout << "=====================================\n";
        cout << "1. View Parking Status\n";
        cout << "2. Check Available Slot\n";
        cout << "3. Vehicle Entry\n";
        cout << "4. Vehicle Exit\n";
        cout << "5. Back\n";
        cout << "Enter choice: ";

        cin >> choice;

        switch (choice)
        {

        case 1:
            displaySlots();
            break;

        case 2:
            cout << "Available Slot: " << findAvailableSlot() << endl;
            break;

        case 3:
            vehicleEntry();
            break;

        case 4:
            vehicleExit();
            break;

        case 5:
            return; //  Goes back to main menu

        default:
            cout << "Invalid Choice!\n";
        }
    }
}

// admin revenue report
void totalRevenue()
{
    ifstream file("data/transactions.csv");
    string line;

    double total = 0;

    while (getline(file, line))
    {

        stringstream ss(line);
        string token, vehicleNo, vehicleType, slotId, entryTime, exitTime, amount;

        getline(ss, token, ',');
        getline(ss, vehicleNo, ',');
        getline(ss, vehicleType, ',');
        getline(ss, slotId, ',');
        getline(ss, entryTime, ',');
        getline(ss, exitTime, ',');
        getline(ss, amount, ',');

        // Skip header row
        if (token == "token_id")
            continue;

        // Skip unfinished transactions
        if (amount == "0" || amount == "NULL")
            continue;

        try
        {
            total += stod(amount);
        }
        catch (...)
        {
            continue;
        }
    }

    cout << "\nTotal Revenue: Rs. " << total << endl;
}

// display rates
void displayRates()
{
    ifstream file("data/rates.txt");
    string type;
    double rate;

    cout << "\n--- Current Parking Rates ---\n";

    while (file >> type >> rate)
    {
        cout << type << " : Rs. " << rate << " per hour\n";
    }

    file.close();
}

// update raes
void updateRate()
{
    displayRates();

    string vehicleType;
    double newRate;

    cout << "\nEnter Vehicle Type to Update: ";
    cin >> vehicleType;

    cout << "Enter New Rate: ";
    cin >> newRate;

    ifstream file("data/rates.txt");
    ofstream temp("data/temp_rates.txt");

    string type;
    double rate;
    bool found = false;

    while (file >> type >> rate)
    {

        if (type == vehicleType)
        {
            temp << type << " " << newRate << "\n";
            found = true;
        }
        else
        {
            temp << type << " " << rate << "\n";
        }
    }
    return;

    file.close();
    temp.close();

    remove("data/rates.txt");
    rename("data/temp_rates.txt", "data/rates.txt");

    if (found)
    {
        cout << "\nRate Updated Successfully!\n";
        return;
    }
    else
    {
        cout << "\nVehicle Type Not Found!\n";
        return;
    }
}

// total vehicle parked
void totalVehicles()
{

    ifstream file("data/transactions.csv");
    string line;

    int count = -1;

    while (getline(file, line))
    {

        stringstream ss(line);
        string token;

        getline(ss, token, ',');

        if (token == "token_id")
            continue;

        count++;
    }

    file.close();

    cout << "\nTotal Vehicles Recorded: " << count << endl;
}

// search vehicle
void searchVehicle()
{

    string searchNumber;
    cout << "\nEnter Vehicle Number: ";
    cin >> searchNumber;

    ifstream file("data/transactions.csv");
    string line;

    bool found = false;

    while (getline(file, line))
    {

        stringstream ss(line);

        string token, vehicleNo, vehicleType, slotId, entryTime, exitTime, amount;

        getline(ss, token, ',');
        getline(ss, vehicleNo, ',');
        getline(ss, vehicleType, ',');
        getline(ss, slotId, ',');
        getline(ss, entryTime, ',');
        getline(ss, exitTime, ',');
        getline(ss, amount, ',');

        if (vehicleNo == searchNumber)
        {

            cout << "\n--- Vehicle Found ---\n";
            cout << "Token ID : " << token << endl;
            cout << "Type     : " << vehicleType << endl;
            cout << "Slot     : " << slotId << endl;
            cout << "Entry    : " << entryTime << endl;
            cout << "Exit     : " << exitTime << endl;
            cout << "Amount   : " << amount << endl;

            found = true;
        }
    }

    file.close();

    if (!found)
        cout << "Vehicle Not Found!\n";
}

// generate report
void reportMenu()
{

    int choice;

    while (true)
    {

        cout << "\n=====================================\n";
        cout << "         REPORT MENU\n";
        cout << "=====================================\n";
        cout << "1. Total Revenue\n";
        cout << "2. Total Vehicles Parked\n";
        cout << "3. Search Vehicle by Number\n";
        cout << "4. Print Overal report\n";
        cout << "5. Back\n";
        cout << "Enter choice: ";

        cin >> choice;

        switch (choice)
        {

        case 1:
            totalRevenue();
            break;

        case 2:
            totalVehicles();
            break;

        case 3:
            searchVehicle();
            break;

        case 4:
            totalRevenue();
            totalVehicles();

        case 5:
            return;

        default:
            cout << "Invalid Choice!\n";
        }
    }
}

// add users
void addUser()
{

    string username, password, role;

    cout << "\nEnter New Username: ";
    cin >> username;

    cout << "Enter Password: ";
    cin >> password;

    cout << "Enter Role (admin/operator): ";
    cin >> role;

    // Check if username already exists
    ifstream checkFile("data/users.csv");
    string line;

    while (getline(checkFile, line))
    {

        stringstream ss(line);
        string fileUser;

        getline(ss, fileUser, ',');

        if (fileUser == username)
        {
            cout << "Username already exists!\n";
            checkFile.close();
            return;
        }
    }

    checkFile.close();

    ofstream file("data/users.csv", ios::app);
    file << username << "," << password << "," << role << "\n";
    file.close();

    cout << "User added successfully!\n";
}

// delete users
void deleteUser()
{

    string usernameToDelete;
    cout << "\nEnter Username to Delete: ";
    cin >> usernameToDelete;

    string adminPassword;
    cout << "Enter Admin Password for Confirmation: ";
    cin >> adminPassword;

    // Verify admin password
    ifstream verifyFile("data/users.csv");
    string line;
    bool adminVerified = false;

    while (getline(verifyFile, line))
    {

        stringstream ss(line);
        string fileUser, filePass, fileRole;

        getline(ss, fileUser, ',');
        getline(ss, filePass, ',');
        getline(ss, fileRole, ',');

        if (fileRole == "admin" && filePass == adminPassword)
        {
            adminVerified = true;
            break;
        }
    }

    verifyFile.close();

    if (!adminVerified)
    {
        cout << "Incorrect Admin Password! Deletion Cancelled.\n";
        return;
    }

    // Prevent deleting admin account itself
    if (usernameToDelete == "admin")
    {
        cout << "Default admin cannot be deleted!\n";
        return;
    }

    ifstream file("data/users.csv");
    ofstream temp("data/temp_users.csv");

    bool found = false;

    while (getline(file, line))
    {

        stringstream ss(line);
        string fileUser;

        getline(ss, fileUser, ',');

        if (fileUser == usernameToDelete)
        {
            found = true;
            continue; // skip writing → deletes user
        }

        temp << line << "\n";
    }

    file.close();
    temp.close();

    remove("data/users.csv");
    rename("data/temp_users.csv", "data/users.csv");

    if (found)
        cout << "User deleted successfully!\n";
    else
        cout << "User not found!\n";
}

// manage users
void manageUsers()
{

    int choice;

    while (true)
    {

        cout << "\n1. Add User\n";
        cout << "2. Delete User\n";
        cout << "3. Back\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {

        case 1:
            addUser();
            break;

        case 2:
            deleteUser();
            break;

        case 3:
            return; // 🔥 goes back to Admin Menu

        default:
            cout << "Invalid Choice!\n";
        }
    }
}

// admin menu
void adminMenu()
{

    int choice;

    while (true)
    {

        cout << "\n=====================================\n";
        cout << "         ADMIN MENU\n";
        cout << "=====================================\n";
        cout << "1. Manage Users\n";
        cout << "2. Manage Parking Rules\n";
        cout << "3. Manage Parking Rates\n";
        cout << "4. Generate Reports\n";
        cout << "5. Back\n";
        cout << "Enter choice: ";

        cin >> choice;

        switch (choice)
        {

        case 1:
            manageUsers();
            break;

        case 2:
            // parking rules
            break;

        case 3:
            updateRate();
            break;

        case 4:
            reportMenu();
            break;

        case 5:
            return;

        default:
            cout << "Invalid Choice!\n";
        }
    }
}

int main()
{
    int choice;
    srand(time(0));

    while (true)
    {
        showMainMenu();
        cin >> choice;

        switch (choice)
        {
        case 1:
            if (login("admin"))
            {
                adminMenu();
            }
            else
            {
                cout << "\nInvalid Credentials!\n";
            }
            break;

        case 2:
            if (login("operator"))
            {
                operatorMenu();
            }
            else
            {
                cout << "\nInvalid Credentials!\n";
            }

            break;

        case 3:
            cout << "\nExiting System...\n";
            return 0;

        default:
            cout << "\nInvalid Choice! Try Again.\n";
        }
    }

    return 0;
}
