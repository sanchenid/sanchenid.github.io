<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Weather Monitoring Dashboard</title>
    <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #eeeeee;
        }

        .header {
            background-color: #4113c0;
            color: white;
            padding: 10px 0;
            text-align: center;
        }

        .container {
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            padding: 20px;
        }

        .card {
            background: rgb(252, 252, 252);
            border-radius: 8px;
            box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);
            margin: 10px;
            padding: 20px;
            width: 250px;
            text-align: center;
        }

        .card h3 {
            margin-bottom: 10px;
        }

        .card .icon {
            font-size: 50px;
            margin-bottom: 15px;
        }

        /* Button Styles */
        .btn {
            padding: 10px 20px;
            background-color: #4CAF50;
            /* Green */
            color: rgb(120, 121, 175);
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            margin-top: 20px;
            transition: background-color 0.3s;
        }

        .btn:hover {
            background-color: #4645a0;
            /* Darker green on hover */
        }

        .card .value {
            font-size: 24px;
            font-weight: bold;
            color: #333;
        }

        .footer {
            background-color: #333;
            color: white;
            text-align: center;
            padding: 10px 0;
            position: fixed;
            bottom: 0;
            width: 100%;
        }

        /* Specific icon colors */
        .temperature-icon {
            color: #FF6347;
            /* Tomato */
        }

        .humidity-icon {
            color: #1E90FF;
            /* Dodger Blue */
        }

        .light-icon {
            color: #FFD700;
            /* Gold */
        }

        .weather-icon {
            color: #32CD32;
            /* Lime Green */
        }

        .wind-icon {
            color: #00CED1;
            /* Dark Turquoise */
        }

        .gass-icon {
            color: #000000;
            /* Dark Turquoise */
        }

        .tabs {
            display: flex;
            justify-content: center;
            margin-top: 10px;
        }

        .tab {
            padding: 10px 20px;
            margin: 0 5px;
            text-decoration: none;
            color: white;
            background-color: #4c7acf;
            border-radius: 5px;
            transition: background-color 0.3s ease;
        }

        .tab:hover {
            background-color: #4049b1;
        }

        .tab.active {
            background-color: #993112;
            font-weight: bold;
        }
    </style>
</head>

<body>

    <div class="header">
        <h1>DASHBOARD MONITORING CUACA</h1>
        <h4>Proyek Keteknikan Kelompok 3</h4>
        <h2>Institut Teknologi Nasional Yogyakarta</h2>
        <nav class="tabs">
            <a href="Simoncu13.html" class="tab">Home</a>
            <a href="tentangkami.html" class="tab">About us</a>
            <a href="info.html" class="tab">More</a>
            <a href="galeri.html" class="tab">Galery</a>
            <a href="blog.html" class="tab">Article</a>
        </nav>
    </div>

    <div class="container">
        <!--Tombol Card-->
        <!-- Temperature Card -->
        <div class="card">
            <i class="material-icons icon temperature-icon">thermostat</i>
            <h3>Temperature</h3>
            <div class="value" id="tempValue">--°C</div>
        </div>

        <!-- Humidity Card -->
        <div class="card">
            <i class="material-icons icon humidity-icon">water_drop</i>
            <h3>Humidity</h3>
            <div class="value" id="humidityValue">--%</div>
        </div>

        <!-- Light Intensity Card -->
        <div class="card">
            <i class="material-icons icon light-icon">wb_sunny</i>
            <h3>Light Intensity</h3>
            <div class="value" id="luxValue">-- Lx</div>
        </div>

        <!-- Weather Status Card -->
        <div class="card">
            <i class="material-icons icon weather-icon">cloud</i>
            <h3>Weather Status</h3>
            <div class="value" id="weatherStatus">--</div>
        </div>

        <!-- Wind Speed Card -->
        <div class="card">
            <i class="material-icons icon wind-icon">speed</i>
            <h3>Wind Speed</h3>
            <div class="value" id="windSpeed">-- km/h</div>
        </div>

        <!-- co card -->
        <div class="card">
            <i class="material-icons icon gass-icon">CO</i>
            <h3>CO gass</h3>
            <div class="value" id="Pollution">-- ppm</div>
        </div>
        <!-- co2 Card -->
        <div class="card">
            <i class="material-icons icon gass-icon">co2</i>
            <h3>CO2 Gass</h3>
            <div class="value" id="co2Value">-- ppm</div>
        </div>
        <!-- Air Pressure Card -->
        <div class="card">
            <i class="material-icons icon">filter_drama</i>
            <h3>Air Pressure</h3>
            <div class="value" id="airPressureValue">-- atm</div>
        </div>
        <!-- attitute -->
        <div class="card">
            <i class="material-icons icon">M</i>
            <h3>attitude</h3>
            <div class="value" id="airPressureValue">-- meter</div>
        </div>


    </div>


    <script>
        // Simulate data from sensors
        // Replace this with actual data from your ESP32 using AJAX or WebSocket
        setInterval(() => {
            document.getElementById("tempValue").textContent = "25°C";  // Example temperature value
            document.getElementById("humidityValue").textContent = "60%";  // Example humidity value
            document.getElementById("luxValue").textContent = "500 Lx";  // Example light intensity
            document.getElementById("weatherStatus").textContent = "Cerah";  // Example weather status
            document.getElementById("windSpeed").textContent = "15 km/h";  // Example wind speed
        }, 2000);  // Update data every 2 seconds
    </script>

</body>

</html>
