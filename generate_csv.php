<?php
// Retrieve the JSON data from esp-data.php
$json_data = file_get_contents('https://flyingorgan.com/esp-data.php');

// Decode the JSON data into an array
$data = json_decode($json_data, true);

$headers = array('ID', 'Ambient Temperature', 'Ambient Humidity', 'Probe Temperature', 'Probe Humidity', 'Pressure', 'Altitude', 'X Axis Acceleration', 'Y Axis Acceleration', 'Z Axis Acceleration', 'Light', 'Longitude', 'Latitude', 'Battery Voltage', 'Reading Time');

$filename = 'data_' . date('Y-m-d_H-i-s') . '.csv';

// Open a temporary file to write the CSV data
$csv_file = tmpfile();

// Write the headers to the CSV file
fputcsv($csv_file, $headers);

// Write the CSV data to the file
foreach ($data as $row) {
    fputcsv($csv_file, $row);
}

// Set the HTTP headers to indicate that a CSV file is being downloaded
header('Content-Type: text/csv');
header('Content-Disposition: attachment; filename="data.csv"');
//header('Content-Disposition: attachment; filename="' . $filename . '"');

// Rewind the file pointer to the beginning of the file
rewind($csv_file);

// Output the file data to the response stream
fpassthru($csv_file);
