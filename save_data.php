<?php
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "simoncu";

// Membuat koneksi ke database
$conn = new mysqli($servername, $username, $password, $dbname);

// Cek koneksi
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Cek apakah data POST ada
$temperature = isset($_POST['temperature']) ? $_POST['temperature'] : null;
$humidity = isset($_POST['humidity']) ? $_POST['humidity'] : null;
$lux = isset($_POST['lux']) ? $_POST['lux'] : null;
$status = isset($_POST['status']) ? $_POST['status'] : null;

// Debugging: Menampilkan data POST yang diterima
echo "<pre>";
print_r($_POST);
echo "</pre>";

if ($temperature && $humidity && $lux && $status) {
    // Menyusun query untuk memasukkan data ke dalam tabel
    $sql = "INSERT INTO sensor_dataa (temperature, humidity, lux, status)
    VALUES ('$temperature', '$humidity', '$lux', '$status')";

    // Menjalankan query
    if ($conn->query($sql) === TRUE) {
        echo "New record created successfully";
    } else {
        echo "Error: " . $sql . "<br>" . $conn->error;
    }
} else {
    echo "Data tidak lengkap atau tidak valid!";
}

$conn->close();
?>
