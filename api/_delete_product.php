<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");
//Creating Array for JSON response
$response = array();
 
// Check if we got the field from the user
if (isset($_POST['productId']) && isset($_POST['name'])) {
    $price = $_POST['price'];
    $name = $_POST['name'];
    $image = $_FILES['image']['name'];
    if(isset($_POST['sold']))
    $sold = 1;
    else
    $sold = 0;

    $target = "../img/".basename($image);

    // Connect to database:
    require_once("db_connect.php");
    
    $sql = "INSERT INTO products (sold, price, name, image) VALUES('$sold', '$price', '$name', '$image')";
    $result = mysqli_query($conn, $sql);
 
    if (move_uploaded_file($_FILES['image']['tmp_name'], $target)) {
        $response["image_message"] = "Image uploaded successfully";
    } else {
        $response["image_message"]  = "Failed to upload image";
    }

    // Check for succesfull execution of query
    if ($result) {
        // successfully inserted 
        $response["success"] = 1;
        $response["message"] = "Product successfully created.";
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