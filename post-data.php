<?php
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

$servername = "localhost";

// REPLACE with your Database name
$dbname = "flyingor_esp_data";
// REPLACE with Database user
$username = "flyingor_esp_board";
// REPLACE with Database user password
$password = "Group4organ!";

// Keep this API Key value to be compatible with the ESP32 code provided in the project page. If you change this value, the ESP32 sketch needs to match
$api_key_value = "tPmAT5Ab3j7F9";

$api_key = $AmbientTemp = $AmbientHumidity = $ProbeTemp = $ProbeHumidity = $Pressure = $Altitude = $Ax = $Ay = $Az = $Light= $Latitude= $Longitude= $Battery= "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $AmbientTemp = test_input($_POST["AmbientTemp"]);
        $AmbientHumidity = test_input($_POST["AmbientHumidity"]);
        $ProbeTemp = test_input($_POST["ProbeTemp"]);
        $ProbeHumidity = test_input($_POST["ProbeHumidity"]);
        $Pressure = test_input($_POST["Pressure"]);
        $Altitude = test_input($_POST["Altitude"]);
        $Ax = test_input($_POST["Ax"]);
        $Ay = test_input($_POST["Ay"]);
        $Az = test_input($_POST["Az"]);
        $Light = test_input($_POST["Light"]);
        $Latitude = test_input($_POST["Latitude"]);
        $Longitude = test_input($_POST["Longitude"]); 
        $Battery = test_input($_POST["Battery"]);
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO Sensor (AmbientTemp, AmbientHumidity, ProbeTemp, ProbeHumidity, Pressure, Altitude, Ax, Ay, Az, Light,Latitude,Longitude, Battery)
        VALUES ('" . $AmbientTemp . "', '" . $AmbientHumidity . "', '" . $ProbeTemp . "',  '" . $ProbeHumidity . "', '" . $Pressure . "', '" . $Altitude . "', '" . $Ax . "', '" . $Ay . "', '" . $Az . "', '" . $Light . "', '" . $Latitude . "', '" . $Longitude . "', '" . $Battery . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }

}
else {
    echo "No data posted with HTTP POST. TEST";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
?>
