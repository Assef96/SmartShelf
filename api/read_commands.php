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

	// // Storing the returned array in response
    // $response["commands"] = array();
 
	// // While loop to store all the returned response in variable
    // while ($row = mysqli_fetch_array($result)) {
    //     // temperoary user array
    //     $commands = array();
    //     $commands["id"] = $row["id"];
    //     $commands["lamp1"] = (int)$row["lamp1"];
    //     $commands["lamp2"] = (int)$row["lamp2"];
    //     $commands["buzzer"] = (int)$row["buzzer"];
	// 	// Push all the items 
    //     array_push($response["commands"], $commands);
    // }
    // On success
    
 
    // Show JSON response
    echo json_encode($response);

?>