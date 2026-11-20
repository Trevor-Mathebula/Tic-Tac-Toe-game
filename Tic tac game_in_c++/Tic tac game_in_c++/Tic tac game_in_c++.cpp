// Tic Tac Toe - console game (second C++ game)
// Flow: welcome/loading screen -> menu -> create accounts or log in (players.csv)

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

const int roundMax = 10;
const string DB_FILE = "players.csv";   // columns: Name;Surname;Age;Gender;Password


//<------------------------- class Person -------------------------------->//
class Person
{
protected:
    string name;
    string surname;
    int age;
    char gender;
public:
    Person();
    void setPerson(string, string, int, char);
    string getName();
    string getSurname();
    int getAge();
    string getGender();
};

Person::Person()
{
    name = "";
    surname = "";
    age = 0;
    gender = ' ';
}

void Person::setPerson(string nm, string sn, int ag, char gen)
{
    name = (nm.length() > 0) ? nm : "XXX";
    surname = (sn.length() > 0) ? sn : "XXX";
    age = (ag > 0) ? ag : 1;
    gender = (toupper(gen) == 'M' || toupper(gen) == 'F') ? toupper(gen) : 'M';
}

string Person::getName() { return name; }
string Person::getSurname() { return surname; }
int    Person::getAge() { return age; }
string Person::getGender() { return (gender == 'M') ? "Male" : "Female"; }


//<------------------------- class Player -------------------------------->//
class Player : public Person
{
private:
    string userName;
    int score[roundMax];
    int gamesPlayed;
    int numOpponents;
    int rank;
    int totalScore;
    void calcTotal();

public:
    Player();
    void setPlayer(string, string, int, char, string, int[], int, int, int);
    string getUserName();
    int getScore(int);
    int getGamesPlayed();
    int getNumOpponents();
    int getRank();
    int getTotal();
    string findBest();
};

Player::Player()
{
    userName = " ";
    for (int i = 0; i < roundMax; i++)
    {
        score[i] = 0;
    }
    gamesPlayed = 0;
    numOpponents = 0;
    rank = 0;
    totalScore = 0;
}

void Player::setPlayer(string nm, string sn, int ag, char gen, string un,
    int sc[], int gp, int no, int rk)
{
    setPerson(nm, sn, ag, gen);
    userName = (un.length() > 0) ? un : "XXXX";
    for (int i = 0; i < roundMax; i++)
    {
        score[i] = sc[i];
    }
    gamesPlayed = gp;
    numOpponents = no;
    rank = rk;
    calcTotal();
}

string Player::getUserName() { return userName; }
int    Player::getScore(int ind) { return score[ind]; }
int    Player::getGamesPlayed() { return gamesPlayed; }
int    Player::getNumOpponents() { return numOpponents; }
int    Player::getRank() { return rank; }
int    Player::getTotal() { return totalScore; }

void Player::calcTotal()
{
    int sum = 0;
    for (int i = 0; i < roundMax; i++)
    {
        sum += score[i];
    }
    totalScore = sum;
}

string Player::findBest()
{
    return (rank == 1) ? userName : "";
}


//<------------------- string helpers (used by the login) ---------------->//

// Remove leading/trailing spaces, tabs and '\r' (Windows line endings)
string trim(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Trim and convert to lower case so comparisons ignore case
string normalise(const string& s)
{
    string result = trim(s);
    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return tolower(c); });
    return result;
}


//<------------------------- player database ----------------------------->//

// One row of players.csv (kept separate from the Player class above)
struct PlayerRecord
{
    string name;
    string surname;
    string age;
    string gender;
    string password;
};

// Read every player from the CSV. Returns false if the file can't be opened.
bool loadPlayers(const string& csvPath, vector<PlayerRecord>& players)
{
    ifstream file(csvPath);
    if (!file.is_open()) return false;

    string line;
    getline(file, line);  // skip the header row

    while (getline(file, line))
    {
        if (trim(line).empty()) continue;  // skip blank lines

        stringstream ss(line);
        PlayerRecord p;
        getline(ss, p.name, ';');
        getline(ss, p.surname, ';');
        getline(ss, p.age, ';');
        getline(ss, p.gender, ';');
        getline(ss, p.password);           // rest of the line

        p.password = trim(p.password);
        players.push_back(p);
    }
    return true;
}

// Ask for new players' details and add them to the CSV (existing players are kept)
void writeToFile(const string& fname)
{
    // write the header only when the file is new or empty
    bool needHeader;
    {
        ifstream check(fname);
        needHeader = !check.good() || check.peek() == EOF;
    }

    ofstream outfile(fname, ios::app);
    if (!outfile)
    {
        cout << fname << " could not be opened, database crashed" << endl;
        return;
    }
    if (needHeader)
    {
        outfile << "Name;Surname;Age;Gender;Password" << endl;
    }

    int numPlayers = 0;
    cout << "----------------------------------------------------------" << endl;
    cout << "                          GAME INFO" << endl;
    cout << "----------------------------------------------------------" << endl << endl;
    cout << "Please enter the number of players: ";
    cin >> numPlayers;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "----------------------------------------------------------" << endl;
    cout << "                          PERSONAL INFO" << endl;
    cout << "----------------------------------------------------------" << endl << endl;

    for (int i = 0; i < numPlayers; i++)
    {
        string name, surname, password;
        int age = 0;
        char gender = ' ';

        cout << "Please enter your name: ";
        getline(cin, name);
        cout << "Please enter your surname: ";
        getline(cin, surname);
        cout << "Please enter your age: ";
        cin >> age;
        cout << "Please enter your gender m/M/f/F: ";
        cin >> gender;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Please create your own password: ";
        getline(cin, password);
        cout << endl << endl;

        outfile << name << ";" << surname << ";" << age << ";"
            << gender << ";" << password << endl;
    }
}


//<------------------------------- login --------------------------------->//
/*
  Asks for name, surname, age and gender, finds the matching player in the
  CSV, then asks for the password.
    - name, surname, age and gender are NOT case sensitive
    - password IS case sensitive
  Returns true on success; the matched player is copied into loggedIn.
*/
bool authenticatePlayer(const string& csvPath, PlayerRecord& loggedIn, int maxAttempts = 3)
{
    vector<PlayerRecord> players;
    if (!loadPlayers(csvPath, players))
    {
        cout << "Could not find '" << csvPath << "'. Create an account first.\n";
        return false;
    }

    // 1. Collect the details
    string name, surname, age, gender;
    cout << "Enter your name: ";
    getline(cin, name);
    cout << "Enter your surname: ";
    getline(cin, surname);
    cout << "Enter your age: ";
    getline(cin, age);
    cout << "Enter your gender (m/f): ";
    getline(cin, gender);

    // 2. Find the player (details are case-insensitive)
    const PlayerRecord* found = nullptr;
    for (const PlayerRecord& p : players)
    {
        if (normalise(p.name) == normalise(name) &&
            normalise(p.surname) == normalise(surname) &&
            normalise(p.age) == normalise(age) &&
            normalise(p.gender) == normalise(gender))
        {
            found = &p;
            break;
        }
    }

    if (found == nullptr)
    {
        cout << "Player not recognised.\n";
        return false;
    }

    // 3. Ask for the password (case sensitive, exact match)
    for (int attempt = 1; attempt <= maxAttempts; attempt++)
    {
        string password;
        cout << "Enter your password: ";
        getline(cin, password);

        if (password == found->password)
        {
            cout << "Welcome back, " << trim(found->name) << " "
                << trim(found->surname) << "!\n";
            loggedIn = *found;
            return true;
        }

        int remaining = maxAttempts - attempt;
        if (remaining > 0)
            cout << "Wrong password. " << remaining << " attempt(s) left.\n";
    }

    cout << "Too many wrong attempts. Access denied.\n";
    return false;
}


//<------------------------- welcome / loading screen -------------------->//
// Loading bar moves in random bursts and takes 10 seconds in total.
void showWelcomeScreen()
{
    const int steps = 50;        // bar width / number of random steps
    const int totalMs = 10000;   // total loading time: 10 seconds

    // random pause per step, scaled so the pauses add up to exactly 10 s
    mt19937 rng(random_device{}());
    uniform_real_distribution<double> dist(0.1, 1.0);
    vector<double> weights(steps);
    double sum = 0;
    for (double& w : weights)
    {
        w = dist(rng);
        sum += w;
    }

    system("cls");
    cout << "\n";
    cout << "  =====================================\n";
    cout << "  |                                   |\n";
    cout << "  |     WELCOME TO  TIC  TAC  TOE     |\n";
    cout << "  |                                   |\n";
    cout << "  |           X | O | X               |\n";
    cout << "  |          ---+---+---              |\n";
    cout << "  |           O | X | O               |\n";
    cout << "  |          ---+---+---              |\n";
    cout << "  |           X | O | X               |\n";
    cout << "  |                                   |\n";
    cout << "  =====================================\n\n";
    cout << "  Loading...\n\n";

    for (int i = 0; i < steps; i++)
    {
        int pause = static_cast<int>(weights[i] / sum * totalMs);
        this_thread::sleep_for(chrono::milliseconds(pause));

        int filled = i + 1;
        cout << "\r  [";
        for (int j = 0; j < steps; j++)
            cout << (j < filled ? '#' : ' ');
        cout << "] " << filled * 100 / steps << "%" << flush;
    }

    system("cls");
}
//<---------Building the tic tac game----------------------------->//


//<--------------------------------- main -------------------------------->//
int main()
{
    showWelcomeScreen();

    int choice = 0;
    cout << "1. Create account(s)\n2. Log in\nChoose: ";
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    system("cls");

    if (choice == 1)
    {
        writeToFile(DB_FILE);
        cout << "Accounts saved. Run the game again to log in.\n";
    }
    else
    {
        PlayerRecord player;
        if (authenticatePlayer(DB_FILE, player))
        {
            cout << "Login successful.\n";
            // TODO: start the tic tac toe game here
        }
        else
        {
            cout << "Login failed.\n";
        }
    }
    return 0;
}


/*
  To do:
   2. store login data in the spreadsheet (players.csv)   - done
   3. game recognises the player                          - done
   4. password checker                                    - done
   5. search the database for a player                    - done
   6. allow many players to use the game                  
   7. detect duplicate accounts
   8. use time in the game
   9. rank players against each other
  10. build the tic tac toe game itself
  11. count the opponents a player has faced
  12. calculate a player's win ratio
  13. user friendly GUI
*/