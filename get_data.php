<?php
header('Content-Type: application/json');

// Koneksi ke database
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "iot_weather";

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Ambil data terbaru dari tabel
$sql = "SELECT * FROM sensor_dataa ORDER BY timestamp DESC LIMIT 1";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    echo json_encode([
        "temperature" => $row["temperature"],
        "humidity" => $row["humidity"],
        "lux" => $row["lux"],
        "status" => $row["status"],
        "windSpeed" => $row["wind_speed"],
        "co" => $row["co"],
        "co2" => $row["co2"],
        "airPressure" => $row["air_pressure"]
    ]);
} else {
    echo json_encode(["error" => "No data found"]);
}

$conn->close();
?>
