<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");
//Creating Array for JSON response
$response = array();
 
// Check if we got the field from the user
if (isset($_GET['id']) && isset($_POST['price']) && isset($_POST['name'])) {
    $price = $_POST['price'];
    $name = $_POST['name'];
    $id = $_GET['id'];
    $image = $_FILES['image']['name'];
    
    if($image != "")
    {
        $target = "../img/".basename($image);
        if (move_uploaded_file($_FILES['image']['tmp_name'], $target)) {
            $response["image_message"] = "Image uploaded successfully";
        } else {
            $response["image_message"]  = "Failed to upload image";
        }
    }
    
    // Connect to database:
    require_once("db_connect.php");
    
    if($id != 0) // edit an existing product
    {
        
        if($image == "")
            $sql = "UPDATE products SET price = '$price', name = '$name' WHERE id = '$id'";
        else
            $sql = "UPDATE products SET price = '$price', name = '$name', image = '$image' WHERE id = '$id'";
    }
    else // create new product
    {
        if($image == "")
            $image = "placeholder.png";
        $sql = "INSERT INTO products (price, name, image) VALUES('$price', '$name', '$image')";
    }
    $result = mysqli_query($conn, $sql);
    
    // Check for succesfull execution of query
    if ($result) {
        // successfully inserted 
        $response["success"] = 1;
        $response["message"] = "Product successfully created or edited.";
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