<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

// Connect to database:
require_once("db_connect.php");

$sql = "SELECT * FROM products";
$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));

if (mysqli_num_rows($result) > 0) {
    
    $response["products"] = array();
 
	// While loop to store all the returned response in variable
    while ($row = mysqli_fetch_array($result)) {
        $product = array();
        $product["id"] = $row["id"];
        $product["name"] = $row["name"];
        $product["price"] = $row["price"]; 
        $product["image"] = $row["image"];
        array_push($response["products"], $product);
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