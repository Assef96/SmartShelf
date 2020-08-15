<?php

header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

if (isset($_POST['unitId']) && isset($_POST['productId'])) {
    $unit_id = $_POST['unitId'];
    $product_id = $_POST['productId'];

    require_once("db_connect.php");
    
    $sql = "UPDATE units SET product_id = '$product_id' WHERE id = '$unit_id'";
    $result = mysqli_query($conn, $sql);
    
    // Check for succesfull execution of query
    if ($result) {
        // successfully inserted 
        $response["success"] = 1;
        $response["message"] = "Product successfully set.";
    } else {
        // Failed to insert data in database
        $response["success"] = 0;
        $response["message"] = "Something has been wrong";
    }
} else {
    // If required parameter is missing
    $response["success"] = 0;
    $response["message"] = "Parameter(s) are missing. Please check the request";
}
// Show JSON response
echo json_encode($response);
?>