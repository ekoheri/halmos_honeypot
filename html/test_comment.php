<?php
    $nama = $_POST['nama'];
    $email = $_POST['email'];
    $message = $_POST['message'];

    echo "<h1>Komentar yang kamu isikan</h1>\n";
    echo "<p>Nama : $nama</p>\n";
    echo "<p>Email : $email</p>\n";
    echo "<p>Komentar : $message</p>\n";
?>
