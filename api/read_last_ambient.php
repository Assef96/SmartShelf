<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

require_once("db_connect.php");

$sql = "SELECT * FROM ambient ORDER BY id DESC LIMIT 0, 1";
$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));

if (mysqli_num_rows($result) > 0) {
    $row = mysqli_fetch_assoc($result);
    $response["ambient"] = $row;
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