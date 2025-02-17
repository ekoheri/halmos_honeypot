Halmos Honeypot
===============
Honeypot adalah sistem keamanan yang dirancang untuk meniru layanan atau jaringan asli dengan tujuan memancing, mendeteksi, dan menganalisis serangan siber. Sistem ini bertindak sebagai umpan yang menarik peretas agar mengungkap teknik, alat, atau eksploitasi yang mereka gunakan tanpa membahayakan infrastruktur utama. Honeypot dapat digunakan untuk penelitian keamanan, meningkatkan pertahanan jaringan, serta mengidentifikasi ancaman sebelum menyerang sistem yang sebenarnya.

Pada repository ini tersedia prototype service honeypot, yang bisa anda manfaatkan untuk mengamankan web server anda dari serangan cyber seperti misalnya SQL Injection, XSS, dll. Dimana layanan Honeypot ini dilengkapi dengan AI sederhana untuk mendeteksi adanya serangan tersebut (SQL Injection, XSS, dll). 

A. Prasyarat :
--------------
1. Untuk menguji honeypot ini, anda perlu menyediakan web server dan interpreter terlebih dahulu. Misalnya Apache sebagai layanan web server, dan PHP yang berperan sebagai server side script.

2. Program AI yang disertakan pada aplikasi ini menggunakan bahasa Python. Jadi anda perlu menyediakan interpreter Python terlebih dahulu sebelum meng-install halmos_honeypot

B. Langkah Instalasi program
----------------------------
1. Masuk ke folder home, atau folder yang lain

cd home

2. clone source code dari github

sudo git clone https://github.com/ekoheri/halmos_honeypot.git halmos_honeypot

3. Masuk ke folder halmos_honeypot

cd halmos_honeypot

4. Install library CURL

sudo apt update && sudo apt install libcurl4-openssl-dev

5. Kompilasi dan install program

sudo make install

6. Masuk ke folder bin dan jalankan program

cd bin

./halmos_honeypot

C. Untuk menghentikan service halmos_honeypot
---------------------------------------------
1. Panggil fungsi grep

ps aux | grep halmos_honeypot | grep -v grep

2. Matikan program dengan perintah kill

kill nomor PID

contoh : kill 4594, jika kebetulan PID-nya nomor 4594

D. Konfigurasi honeypot
-----------------------
Setelah servioce halmos_honeypot ter-install, anda bisa melakukan konfigurasi jika dibutuhkan. Langkah-langkah konfigurasi sbb.
1. Masuk ke folder /etc/halmos

cd /etc/halmos/

2. Buka file halmos_h

nano halmos_honeypot.conf

Pada file konfigurasi tersebut, isinya seperti ini
-
server_name = 127.0.0.1

server_port = 8080

web_server_name = http://localhost/halmos

web_server_port = 80

max_event = 64

python_version = python3.10

python_directory = /home/test_ai/


Penjelasannya adalah sbb
-
1. server_name : adalah alamat (IP Address) server honeypot kita. Pada konfigurasi dicontohkan 127.0.0.1 (localhost)
2. server port : adalah port server honeypot kita. Secara default nilainya adalah 8080. tetapi bisa anda ganti, selama port-nya tidak digunakan aplikasi lain.
3. web_server_name : adalah web server kita yang asli. Bisa Apache atau Nginx  atau yang lain. Secara default nama domain web server-nya adalah http://localhost/halmos. Folder halmos ini, bisa anda akses pada bagian folder /var/www/html/halmos/. Gunakan perintah cd /var/www/html/halmos/ untuk mengakses folder DocumentRoot-nya serta memodifikasi HTML atau CSS atau yang lain.
4. web_server_port : adalah port web server kita yang asli. Biasanya default web server itu port-nya 80 untuk HTTP, atau 443 untuk HTTPS
5. max_events : adalah jumlah maksimum antrian proses koneksi yang diijinkan pada waktu yang sama. Anda bisa merubah angka max_events ini jika dirasa server anda mampu menangani proses koneksi diwaktu yang sama. Jika diwaktu yang sama ternyata terdapat koneksi yang belum selesai diproses sebanyak 64, maka untuk koneksi selanjutnya tidak akan dilayani, sampai ada beberapa proses yang sudah selesai dilayani. Tujuan dari pembatasan ini supaya tidak terjadi over load kinerja server, sekaligus untuk menghindari praktek Denial of Service (DoS).
6. python_version : adalah versi interpreter python untuk menjalankan program AI. DImana AI initugasnya adalah untuk mendeteksi jika ada serangan cyber (SQL injection, XSS, dll). 
7. python_directory : adalah directory (folder) dimana program AI itu disimpan. Default-nya, program AI itu diletakkan pada folder /home/test_ai/. Silahkan masuk ke folder tsb dengan perintah cd /home/test_ai/
8. python_script : adalah nama file program AI-nya. Default nama filenya adalah script_ai.py. Silahkan anda buka contoh sederhana AI tersebut. Anda boleh memodifikasi programnya atau menambahkan dataset untuk supaya AI bisa lebih banyak mendeteksi pola-pola serangan cyber-nya.

E. File LOG
-----------
Aplikasi halmos_honeypot ini dilengkapi dengan file LOG, yang bisa anda akses pada folder /var/log/halmos_honeypot/. 
1. Silahkan masuk ke folder tersebut

cd /var/log/halmos_honeypot/

2. Lalu silahkan anda lihat file log-nya

ls- l
