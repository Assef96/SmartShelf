<?php
 
/****************************************************/
/************** Created by : Vivek Gupta ************/
/***************     www.vsgupta.in     *************/
/***************     www.iotmonk.com     *************/
/****************************************************/ 

header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

//Creating Array for JSON response
$response = array();

// Connect to database:
require_once("db_connect.php");

// $result = mysqli_query($conn, "SELECT *FROM commands") or die(mysqli_error($conn));

$sql = "SELECT * FROM commands ORDER BY id DESC LIMIT 0, 1";
$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));

// Check for succesfull execution of query and no results found

if ($result) {
    $row = mysqli_fetch_assoc($result);
    $response["commands"] = $row;
    $response["success"] = 1;
}
else {
    $response["success"] = 0;
}
 
    // Show JSON response
    echo json_encode($response);

?>