#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <sqlite3.h>
#include <fstream>
#include <limits>
#include <iomanip>

using namespace std;

// helper: split comma-separated trimmed tokens
vector<string> split_and_trim(const string &s) {
    vector<string> out;
    string tmp;
    stringstream ss(s);
    while (getline(ss, tmp, ',')) {
        // trim
        size_t start = tmp.find_first_not_of(" \t\n\r");
        size_t end = tmp.find_last_not_of(" \t\n\r");
        if (start != string::npos && end != string::npos) out.push_back(tmp.substr(start, end - start + 1));
        else if (start != string::npos) out.push_back(tmp.substr(start));
    }
    return out;
}

bool execute_sql(sqlite3* db, const string& sql) {
    char *err = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) {
            cerr << "SQL error: " << err << "\n";
            sqlite3_free(err);
        }
        return false;
    }
    return true;
}

bool apply_sql_file(sqlite3* db, const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) return false;
    string sql((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    return execute_sql(db, sql);
}

// Normalize symptom string to lowercase
string to_lower_copy(const string &s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

// Load all known symptoms into map<string, symptom_id>
map<string,int> load_symptoms(sqlite3* db) {
    map<string,int> res;
    const char* q = "SELECT symptom_id, symptom_name FROM Symptoms";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) return res;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        if (name) res[to_lower_copy((const char*)name)] = id;
    }
    sqlite3_finalize(stmt);
    return res;
}

// Structure to hold disease info
struct Disease {
    int id;
    string name;
    string specialization;
    vector<int> symptom_ids; // symptom ids for the disease
};

vector<Disease> load_diseases_with_symptoms(sqlite3* db) {
    // Query to get disease_id, disease_name, specialization
    const char* q = "SELECT disease_id, disease_name, specialization FROM Diseases";
    sqlite3_stmt* stmt;
    vector<Disease> diseases;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) return diseases;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Disease d;
        d.id = sqlite3_column_int(stmt, 0);
        const unsigned char* nm = sqlite3_column_text(stmt,1);
        const unsigned char* sp = sqlite3_column_text(stmt,2);
        d.name = nm ? (const char*)nm : "";
        d.specialization = sp ? (const char*)sp : "";
        diseases.push_back(d);
    }
    sqlite3_finalize(stmt);

    // Load symptoms per disease
    const char* q2 = "SELECT symptom_id FROM DiseaseSymptoms WHERE disease_id = ?";
    sqlite3_stmt* stmt2;
    if (sqlite3_prepare_v2(db, q2, -1, &stmt2, nullptr) != SQLITE_OK) return diseases;
    for (auto &d : diseases) {
        sqlite3_reset(stmt2);
        sqlite3_bind_int(stmt2, 1, d.id);
        while (sqlite3_step(stmt2) == SQLITE_ROW) {
            int sid = sqlite3_column_int(stmt2, 0);
            d.symptom_ids.push_back(sid);
        }
    }
    sqlite3_finalize(stmt2);
    return diseases;
}

// Find doctors for a specialization
vector<pair<int,string>> find_doctors_by_specialization(sqlite3* db, const string &specialization) {
    vector<pair<int,string>> out; // (doctor_id, name)
    const char* q = "SELECT doctor_id, name FROM Doctors WHERE lower(specialization) = lower(?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) return out;
    sqlite3_bind_text(stmt, 1, specialization.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        out.emplace_back(id, name ? (const char*)name : "");
    }
    sqlite3_finalize(stmt);
    return out;
}

// Save patient and return patient_id
int save_patient(sqlite3* db, const string &name, int age, const string &gender, const string &contact) {
    const char* q = "INSERT INTO Patients (name, age, gender, contact) VALUES (?,?,?,?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,2, age);
    sqlite3_bind_text(stmt,3, gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4, contact.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(db);
}

// Save prescription (visit)
bool save_prescription(sqlite3* db, int patient_id, int disease_id, int doctor_id, const string &notes) {
    const char* q = "INSERT INTO Prescriptions (patient_id, disease_id, doctor_id, notes) VALUES (?,?,?,?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt,1, patient_id);
    sqlite3_bind_int(stmt,2, disease_id);
    sqlite3_bind_int(stmt,3, doctor_id);
    sqlite3_bind_text(stmt,4, notes.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

int main() {
    cout << "=== Smart Healthcare DBMS (Disease Prediction) ===\n";

    sqlite3* db = nullptr;
    int rc = sqlite3_open("health.db", &db);
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    if (rc) {
        cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    // Try apply schema.sql if exists
    ifstream f("schema.sql");
    if (f.good()) {
        cout << "Applying schema.sql ...\n";
        if (!apply_sql_file(db, "schema.sql")) {
            cerr << "Failed applying schema.sql\n";
            // continue anyway
        } else {
            cout << "Schema applied / sample data loaded.\n";
        }
    } else {
        cout << "schema.sql not found — ensure DB exists or provide schema.sql to initialize.\n";
    }

    // Load symptoms map
    auto symptom_map = load_symptoms(db); // map lowercase -> id
    if (symptom_map.empty()) {
        cerr << "No symptoms found in DB. Please ensure schema.sql was applied.\n";
    }

    // Main loop: get patient and symptoms, predict
    while (true) {
        cout << "\nMenu:\n1) New patient visit (enter symptoms)\n2) Exit\nChoice: ";
        int choice; if (!(cin >> choice)) break;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (choice == 2) break;
        if (choice != 1) continue;

        // Patient details
        string pname, gender, contact;
        int age;
        cout << "Patient name: ";
        getline(cin, pname);
        cout << "Age: ";
        string age_s; getline(cin, age_s);
        try { age = stoi(age_s); } catch(...) { age = 0; }
        cout << "Gender: "; getline(cin, gender);
        cout << "Contact: "; getline(cin, contact);

        int patient_id = save_patient(db, pname, age, gender, contact);
        if (patient_id < 0) {
            cerr << "Failed saving patient. Aborting visit.\n";
            continue;
        }
        cout << "Saved patient id = " << patient_id << "\n";

        cout << "Enter symptoms (comma separated). Example: fever, cough, headache\nSymptoms: ";
        string sins; getline(cin, sins);
        auto input_symptoms = split_and_trim(sins);
        // map to symptom_ids (lowercase)
        vector<int> input_ids;
        for (auto &s : input_symptoms) {
            string lower = to_lower_copy(s);
            if (symptom_map.count(lower)) input_ids.push_back(symptom_map[lower]);
            else {
                cout << "Symptom '" << s << "' not in DB. Do you want to add it? (y/n): ";
                char c; cin >> c; cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (c=='y' || c=='Y') {
                    // insert symptom
                    const char* qins = "INSERT INTO Symptoms (symptom_name) VALUES (?);";
                    sqlite3_stmt* st;
                    if (sqlite3_prepare_v2(db, qins, -1, &st, nullptr) == SQLITE_OK) {
                        sqlite3_bind_text(st,1, s.c_str(), -1, SQLITE_TRANSIENT);
                        if (sqlite3_step(st) == SQLITE_DONE) {
                            int new_id = (int)sqlite3_last_insert_rowid(db);
                            symptom_map[to_lower_copy(s)] = new_id;
                            input_ids.push_back(new_id);
                            cout << "Added symptom '"<<s<<"' with id " << new_id << "\n";
                        } else {
                            cout << "Failed to insert symptom\n";
                        }
                    }
                    sqlite3_finalize(st);
                } else {
                    cout << "Skipping symptom '"<<s<<"'\n";
                }
            }
        }

        if (input_ids.empty()) {
            cout << "No valid symptoms given. Aborting.\n";
            continue;
        }

        // Load diseases with symptom lists
        auto diseases = load_diseases_with_symptoms(db);

        // Prediction: compute match score = (#matching symptom ids) / (total disease symptoms) * 100
        struct Hit { int disease_id; string disease_name; string specialization; double score; int match_count; int total_symptoms; };
        vector<Hit> hits;
        for (auto &d : diseases) {
            int total = (int)d.symptom_ids.size();
            if (total == 0) continue;
            int matches = 0;
            for (int sid : d.symptom_ids) {
                if (find(input_ids.begin(), input_ids.end(), sid) != input_ids.end()) matches++;
            }
            double score = (100.0 * matches) / total;
            if (matches > 0) { // only show diseases with at least one matching symptom
                hits.push_back({d.id, d.name, d.specialization, score, matches, total});
            }
        }

        // sort by score descending then match_count
        sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) {
            if (a.score != b.score) return a.score > b.score;
            return a.match_count > b.match_count;
        });

        if (hits.empty()) {
            cout << "No probable disease found for given symptoms in DB.\n";
            continue;
        }

        // Show top suggestions (top 5)
        cout << "\nProbable Diseases (Ranked by Symptom Match):\n";
        cout << "==============================================================\n";
        cout << left << setw(5) << "No."
             << setw(25) << "Disease"
             << setw(25) << "Specialization"
             << setw(10) << "Score%"
             << setw(10) << "Matches" << endl;
        cout << "--------------------------------------------------------------\n";

        int shown = min((int)hits.size(), 5);
        for (int i = 0; i < shown; i++) {
            cout << left << setw(5) << (i + 1)
                 << setw(25) << hits[i].disease_name
                 << setw(25) << hits[i].specialization
                 << setw(10) << fixed << setprecision(1) << hits[i].score
                 << setw(10) << hits[i].match_count << endl;
        }
        cout << "==============================================================\n";


        // Let user pick one disease to save prescription for (or skip)
        cout << "\nEnter the number of disease to record prescription (0 to skip): ";
        int pick = 0; if (!(cin >> pick)) { cin.clear(); cin.ignore(); pick = 0; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (pick <= 0 || pick > shown) {
            cout << "Skipping saving prescription.\n";
            continue;
        }
        auto chosen = hits[pick-1];

        // Recommend doctors for that specialization
        auto doctors = find_doctors_by_specialization(db, chosen.specialization);
        if (doctors.empty()) {
            cout << "No doctors found for specialization '" << chosen.specialization << "'.\n";
            cout << "You can still save prescription with doctor_id = 0 (unknown).\n";
        } else {
            cout << "Available doctors for " << chosen.specialization << ":\n";
            for (size_t i=0;i<doctors.size();++i) {
                cout << i+1 << ") " << doctors[i].second << " (ID=" << doctors[i].first << ")\n";
            }
        }

        cout << "Choose doctor number (0 to enter doctor_id manually): ";
        int docpick = 0; if (!(cin >> docpick)) { cin.clear(); cin.ignore(); docpick = 0; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        int doctor_id = 0;
        if (docpick >= 1 && docpick <= (int)doctors.size()) doctor_id = doctors[docpick-1].first;
        else if (docpick == 0) {
            cout << "Enter doctor_id (0 = unknown): ";
            string dids; getline(cin, dids);
            try { doctor_id = stoi(dids); } catch(...) { doctor_id = 0; }
        }

        cout << "Enter brief notes/prescription: ";
        string notes; getline(cin, notes);

        if (save_prescription(db, patient_id, chosen.disease_id, doctor_id, notes)) {
            cout << "Prescription saved successfully.\n";
        } else {
            cout << "Failed to save prescription.\n";
        }
    }

    sqlite3_close(db);
    cout << "Goodbye!\n";
    return 0;
}
