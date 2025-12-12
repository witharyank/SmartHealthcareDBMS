-- ===========================================================
-- SMART HEALTHCARE DBMS - SCHEMA FILE (SAFE VERSION)
-- ===========================================================

-- Enable foreign key constraints
PRAGMA foreign_keys = ON;

-- ===========================================================
-- TABLES
-- ===========================================================

-- 1. Patients Table
CREATE TABLE IF NOT EXISTS Patients (
                                        patient_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                        name TEXT NOT NULL,
                                        age INTEGER,
                                        gender TEXT,
                                        contact TEXT,
                                        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 2. Symptoms Table
CREATE TABLE IF NOT EXISTS Symptoms (
                                        symptom_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                        symptom_name TEXT UNIQUE NOT NULL
);

-- 3. Diseases Table
CREATE TABLE IF NOT EXISTS Diseases (
                                        disease_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                        disease_name TEXT UNIQUE NOT NULL,
                                        specialization TEXT NOT NULL
);

-- 4. Mapping Table (Disease–Symptoms)
CREATE TABLE IF NOT EXISTS DiseaseSymptoms (
                                               id INTEGER PRIMARY KEY AUTOINCREMENT,
                                               disease_id INTEGER,
                                               symptom_id INTEGER,
                                               FOREIGN KEY (disease_id) REFERENCES Diseases(disease_id),
    FOREIGN KEY (symptom_id) REFERENCES Symptoms(symptom_id)
    );

-- 5. Doctors Table
CREATE TABLE IF NOT EXISTS Doctors (
                                       doctor_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                       name TEXT NOT NULL,
                                       specialization TEXT NOT NULL,
                                       contact TEXT
);

-- 6. Prescriptions Table
CREATE TABLE IF NOT EXISTS Prescriptions (
                                             prescription_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                             patient_id INTEGER,
                                             disease_id INTEGER,
                                             doctor_id INTEGER,
                                             notes TEXT,
                                             created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                                             FOREIGN KEY (patient_id) REFERENCES Patients(patient_id),
    FOREIGN KEY (disease_id) REFERENCES Diseases(disease_id),
    FOREIGN KEY (doctor_id) REFERENCES Doctors(doctor_id)
    );

-- ===========================================================
-- SAMPLE DATA (SAFE INSERTS)
-- ===========================================================

-- Symptoms
INSERT INTO Symptoms (symptom_name)
SELECT 'fever' WHERE NOT EXISTS (SELECT 1 FROM Symptoms WHERE symptom_name = 'fever');
INSERT INTO Symptoms (symptom_name)
SELECT 'cough' WHERE NOT EXISTS (SELECT 1 FROM Symptoms WHERE symptom_name = 'cough');
INSERT INTO Symptoms (symptom_name)
SELECT 'headache' WHERE NOT EXISTS (SELECT 1 FROM Symptoms WHERE symptom_name = 'headache');
INSERT INTO Symptoms (symptom_name)
SELECT 'fatigue' WHERE NOT EXISTS (SELECT 1 FROM Symptoms WHERE symptom_name = 'fatigue');
INSERT INTO Symptoms (symptom_name)
SELECT 'vomiting' WHERE NOT EXISTS (SELECT 1 FROM Symptoms WHERE symptom_name = 'vomiting');
INSERT INTO Symptoms (symptom_name)
SELECT 'body pain' WHERE NOT EXISTS (SELECT 1 FROM Symptoms WHERE symptom_name = 'body pain');

-- Diseases
INSERT INTO Diseases (disease_name, specialization)
SELECT 'Influenza', 'General Physician'
    WHERE NOT EXISTS (SELECT 1 FROM Diseases WHERE disease_name = 'Influenza');

INSERT INTO Diseases (disease_name, specialization)
SELECT 'Typhoid', 'Infectious Disease'
    WHERE NOT EXISTS (SELECT 1 FROM Diseases WHERE disease_name = 'Typhoid');

INSERT INTO Diseases (disease_name, specialization)
SELECT 'Dengue', 'Infectious Disease'
    WHERE NOT EXISTS (SELECT 1 FROM Diseases WHERE disease_name = 'Dengue');

INSERT INTO Diseases (disease_name, specialization)
SELECT 'Migraine', 'Neurologist'
    WHERE NOT EXISTS (SELECT 1 FROM Diseases WHERE disease_name = 'Migraine');

INSERT INTO Diseases (disease_name, specialization)
SELECT 'Common Cold', 'General Physician'
    WHERE NOT EXISTS (SELECT 1 FROM Diseases WHERE disease_name = 'Common Cold');

-- Doctors
INSERT INTO Doctors (name, specialization, contact)
SELECT 'Dr. Anita Sharma', 'General Physician', '+91-9876500001'
    WHERE NOT EXISTS (SELECT 1 FROM Doctors WHERE name = 'Dr. Anita Sharma');

INSERT INTO Doctors (name, specialization, contact)
SELECT 'Dr. Rahul Verma', 'Infectious Disease', '+91-9876500002'
    WHERE NOT EXISTS (SELECT 1 FROM Doctors WHERE name = 'Dr. Rahul Verma');

INSERT INTO Doctors (name, specialization, contact)
SELECT 'Dr. Meera Joshi', 'Neurologist', '+91-9876500003'
    WHERE NOT EXISTS (SELECT 1 FROM Doctors WHERE name = 'Dr. Meera Joshi');

-- Disease–Symptoms Mapping
-- Influenza
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Influenza'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='fever')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Influenza')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='fever')
);
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Influenza'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='cough')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Influenza')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='cough')
);
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Influenza'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='fatigue')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Influenza')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='fatigue')
);

-- Typhoid
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Typhoid'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='fever')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Typhoid')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='fever')
);
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Typhoid'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='headache')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Typhoid')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='headache')
);

-- Dengue
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Dengue'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='fever')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Dengue')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='fever')
);
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Dengue'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='body pain')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Dengue')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='body pain')
);

-- Migraine
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Migraine'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='headache')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Migraine')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='headache')
);

-- Common Cold
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Common Cold'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='cough')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Common Cold')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='cough')
);
INSERT INTO DiseaseSymptoms (disease_id, symptom_id)
SELECT (SELECT disease_id FROM Diseases WHERE disease_name='Common Cold'),
       (SELECT symptom_id FROM Symptoms WHERE symptom_name='headache')
    WHERE NOT EXISTS (
  SELECT 1 FROM DiseaseSymptoms WHERE disease_id=(SELECT disease_id FROM Diseases WHERE disease_name='Common Cold')
  AND symptom_id=(SELECT symptom_id FROM Symptoms WHERE symptom_name='headache')
);

-- ===========================================================
-- END OF FILE
-- ===========================================================
