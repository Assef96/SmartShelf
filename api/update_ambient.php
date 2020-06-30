<?php

header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

$content = trim(file_get_contents("php://input"));
$contentType = $_SERVER["CONTENT_TYPE"];
$data = json_decode($content, true);

// Connect to database:
require_once("db_connect.php");


$temperature = $data["temperature"];
$humidity = $data["humidity"];
$light = $data["light"];
// $sql = "UPDATE units SET status= '$status' WHERE id = '$id'";
$sql = "INSERT INTO ambient (temperature, humidity, light, time) VALUES ('$temperature', '$humidity', '$light', now())";
$result = mysqli_query($conn, $sql);
if (!$result) {
    $response["success"] = 0;
    echo json_encode($response);
}
$response["success"] = 1;
echo json_encode($response);


mysqli_close($conn);
?>