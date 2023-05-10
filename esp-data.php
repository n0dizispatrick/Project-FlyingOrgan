<?php
header('Content-Type: application/json');

$servername = "localhost";
$dbname = "flyingor_esp_data";
$username = "flyingor_esp_board";
$password = "Group4organ!";


$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT id, AmbientTemp, AmbientTemp, AmbientHumidity, ProbeTemp, ProbeHumidity, Pressure, Altitude, Ax, Ay, Az, Light, Latitude, Longitude, Battery, reading_time FROM Sensor ORDER BY id DESC";

$data = array();
if ($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        $row_id = $row["id"];
        $row_AmbientTemp = $row["AmbientTemp"];
        $row_AmbientHumidity = $row["AmbientHumidity"];
        $row_ProbeTemp = $row["ProbeTemp"]; 
        $row_ProbeHumidity = $row["ProbeHumidity"]; 
        $row_Pressure = $row["Pressure"];
        $row_Altitude = $row["Altitude"]; 
        $row_Ax = $row["Ax"]; 
        $row_Ay = $row["Ay"];
        $row_Az = $row["Az"]; 

        $row_Light = $row["Light"];
        $row_Latitude = $row["Latitude"];
        $row_Longitude = $row["Longitude"];
        $row_Battery = $row["Battery"];
        $reading_time[$i] = date("Y-m-d h:i:sa", strtotime("$reading + 1 hours"));
        $row_reading_time = $row["reading_time"];
        

        $data[] = array(
            'id' => $row_id,
            'AmbientTemp' => $row_AmbientTemp,
            'AmbientHumidity' => $row_AmbientHumidity,
            'ProbeTemp' => $row_ProbeTemp,
            'ProbeHumidity' => $row_ProbeHumidity,
            'Pressure' => $row_Pressure,
            'Altitude' => $row_Altitude,
            'Ax' => $row_Ax,
            'Ay' => $row_Ay,
            'Az' => $row_Az,
            'Light' => $row_Light,
            'Latitude' => $row_Latitude,
            'Longitude' => $row_Longitude,
            'Battery' => $row_Battery,
            'reading_time' => $row_reading_time
        );
    }
    $result->free();
}

$conn->close();

echo json_encode($data);
?>
