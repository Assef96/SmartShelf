<?php
 
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

//Creating Array for JSON response
$response = array();

// Connect to database:
require_once("db_connect.php");

$sql = "SELECT id, status
        FROM sensors";
$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));


// Check for succesfull execution of query and no results found
if (mysqli_num_rows($result) > 0) {
    
    $response["sensors"] = array();
 
	// While loop to store all the returned response in variable
    while ($row = mysqli_fetch_array($result)) {
        $sensor = array();
        $sensor["id"] = $row["id"];
        $sensor["status"] = $row["status"];
        array_push($response["sensors"], $sensor);
    }
    $response["success"] = 1;
    echo json_encode($response);
}	
else 
{
	$response["success"] = 0;
    $response["message"] = "No data found";
    echo json_encode($response);
}

mysqli_close($conn);
?>