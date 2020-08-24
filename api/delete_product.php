<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");
//Creating Array for JSON response
$response = array();
 
// Check if we got the field from the user
if (isset($_POST['id'])) {
    $id = $_POST['id'];

    // Connect to database:
    require_once("db_connect.php");
    
    $sql = "DELETE FROM products WHERE id = {$id}";
    $result = mysqli_query($conn, $sql);
 
    // Check for succesfull execution of query
    if ($result) {
        // successfully inserted 
        $response["success"] = 1;
        $response["message"] = "Product successfully removed.";
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