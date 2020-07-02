<?php
 
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

//Creating Array for JSON response
$response = array();

// Connect to database:
require_once("db_connect.php");

$sql = "SELECT units.id, products.name, products.price
        FROM units
        INNER JOIN products ON units.product_id=products.id";
$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));


// Check for succesfull execution of query and no results found
if (mysqli_num_rows($result) > 0) {
    
    $response["units"] = array();
 
	// While loop to store all the returned response in variable
    while ($row = mysqli_fetch_array($result)) {
        $unit = array();
        $unit["id"] = $row["id"];
        $unit["name"] = $row["name"];
        $unit["price"] = intval($row["price"]);
        array_push($response["units"], $unit);
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