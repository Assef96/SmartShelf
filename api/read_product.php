<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

if (isset($_GET["id"])) {
    require_once("db_connect.php");

    $id = $_GET["id"];
    $sql = "SELECT * FROM products WHERE id = '$id'";
    $result = mysqli_query($conn, $sql) or die(mysqli_error($conn));
    
    if (mysqli_num_rows($result) > 0) {
        $row = mysqli_fetch_assoc($result);
        $response["product"] = $row;
        $response["success"] = 1;
        echo json_encode($response);
    }	
    else 
    {
        $response["success"] = 0;
        $response["message"] = "No data found";
        echo json_encode($response);
    }
    
}
else {
    // If required parameter is missing
    $response["success"] = 0;
    $response["message"] = "Parameter(s) are missing. Please check the request";
    echo json_encode($response);
}

?> 