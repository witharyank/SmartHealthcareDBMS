# SmartHealthcareDBMS

**Smart Healthcare DBMS** is a lightweight C++ console application built with CMake/CLion that predicts probable diseases from patient symptoms using an SQLite database and records patient visits/prescriptions.

---

## Key Features

* Simple symptom-based disease prediction (match percentage by symptom overlap)
* Stores patients, diseases, symptoms, doctors and prescriptions in SQLite
* Allows adding new symptoms interactively
* Save patient visits and prescriptions to the database
* CLI driven: easy to run on Windows/macOS/Linux

---

## Project Structure

```
SmartHealthcareDBMS/
├─ CMakeLists.txt
├─ main.cpp           # Main application (disease prediction + DB CRUD)
├─ schema.sql         # SQL schema + optional sample data
├─ health.db          # SQLite database (generated or provided)
└─ .idea/             # CLion IDE settings (NOT needed in repo)
```

> Note: `health.db` may be created by applying `schema.sql` if present. You can remove `health.db` from version control and keep only `schema.sql`.

---

## Prerequisites

* C++ compiler with C++11 support or later
* CMake (3.10+ recommended)
* CLion (optional) or any editor
* SQLite3 development library (headers & linker) installed

On Windows you can use the SQLite amalgamation and link the `sqlite3.lib` provided with the dev package. On Linux/macOS install `libsqlite3-dev` (or equivalent) via your package manager.

---

## Build (CLion)

1. Open the project folder in CLion.
2. Configure toolchain (compiler) if needed.
3. Build the project (Build ▶ Build Project).
4. Run the resulting executable from CLion's run configuration or terminal.

## Build (Command Line)

```powershell
# from project root (Windows PowerShell)
mkdir build
cd build
cmake ..
cmake --build .
# run the executable (path depends on generator) e.g.:
./SmartHealthcareDBMS.exe
```

On Linux/macOS, use the usual `mkdir build && cd build && cmake .. && make` sequence.

---

## Initialize Database

If `health.db` is not present, the program will attempt to apply `schema.sql` if it exists in the project root. `schema.sql` should contain table definitions and any sample data you want to preload (Symptoms, Diseases, DiseaseSymptoms, Doctors, etc.).

Example simple schema skeleton (add to `schema.sql` as needed):

```sql
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Symptoms (
  symptom_id INTEGER PRIMARY KEY AUTOINCREMENT,
  symptom_name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS Diseases (
  disease_id INTEGER PRIMARY KEY AUTOINCREMENT,
  disease_name TEXT NOT NULL,
  specialization TEXT
);

CREATE TABLE IF NOT EXISTS DiseaseSymptoms (
  disease_id INTEGER,
  symptom_id INTEGER,
  FOREIGN KEY(disease_id) REFERENCES Diseases(disease_id),
  FOREIGN KEY(symptom_id) REFERENCES Symptoms(symptom_id)
);

CREATE TABLE IF NOT EXISTS Doctors (
  doctor_id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  specialization TEXT
);

CREATE TABLE IF NOT EXISTS Patients (
  patient_id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT,
  age INTEGER,
  gender TEXT,
  contact TEXT
);

CREATE TABLE IF NOT EXISTS Prescriptions (
  prescription_id INTEGER PRIMARY KEY AUTOINCREMENT,
  patient_id INTEGER,
  disease_id INTEGER,
  doctor_id INTEGER,
  notes TEXT,
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY(patient_id) REFERENCES Patients(patient_id),
  FOREIGN KEY(disease_id) REFERENCES Diseases(disease_id),
  FOREIGN KEY(doctor_id) REFERENCES Doctors(doctor_id)
);
```

---

## Usage (Runtime)

1. Run the executable.
2. Choose **New patient visit (enter symptoms)** from the menu.
3. Enter patient details (name, age, gender, contact).
4. Enter comma-separated symptoms (e.g. `fever, cough, headache`).
5. The app computes probable diseases ranked by percentage match and allows you to save a prescription linked to a doctor.

### Notes on adding unknown symptoms

If you enter a symptom not present in the database, the app prompts whether to add it. If you confirm, it inserts the symptom and maps it for the current visit.

---

## Example Session

```
=== Smart Healthcare DBMS (Disease Prediction) ===

Menu:
1) New patient visit (enter symptoms)
2) Exit
Choice: 1

Patient name: John Doe
Age: 34
Gender: Male
Contact: 123456789
Enter symptoms (comma separated). Example: fever, cough, headache
Symptoms: fever, cough

Probable Diseases (Ranked by Symptom Match):
No.  Disease                   Specialization           Score%    Matches
1    Common Cold               General Medicine         100.0     2
2    Influenza                 General Medicine         50.0      1

Enter the number of disease to record prescription (0 to skip): 1
Available doctors for General Medicine:
1) Dr. A (ID=2)
Choose doctor number (0 to enter doctor_id manually): 1
Enter brief notes/prescription: Rest, fluids, paracetamol
Prescription saved successfully.
```

---

## .gitignore (recommended)

```
# CLion / IntelliJ
.idea/
*.iml

# CMake
cmake-build-*/
CMakeFiles/
CMakeCache.txt
*.cmake

# SQLite DB
*.db

# OS
Thumbs.db
.DS_Store
```

---

## Contributing

Contributions are welcome. Please open issues or pull requests. Suggested improvements:

* Improve prediction algorithm (weighted symptoms, bayesian approach)
* Add unit tests
* Add web UI or REST API
* Secure/persist sensitive patient data appropriately (encryption, access control)

---

## License

Include your preferred license (MIT, Apache-2.0, etc.). By default you can add an `LICENSE` file with MIT contents.

---

If you want, I can also generate a `README.md` file directly in your repository, create the `.gitignore`, or craft a short `LICENSE` file. Which would you like me to add next?
