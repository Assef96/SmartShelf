<?php

header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json; charset=UTF-8");

//Creating Array for JSON response
$response = array();
$response["success"] = 0;
 
// Check if we got the field from the user
if (isset($_POST["lamp"]))
    $command = "lamp";
elseif (isset($_POST["fan"]))
    $command = "fan";
elseif (isset($_POST["buzzer"]))
$command = "buzzer";
else {
        $response["message"] = "No parameter is set on request.";
        echo json_encode($response);
    }
$status = $_POST[$command];

require_once("db_connect.php");
$result = mysqli_query($conn, "UPDATE commands SET $command = $status WHERE id = 1");
if ($result) {
    $response["success"] = 1;
    $response["message"] = "'$command' was set to '$status'.";
}
else {
    $response["message"] = "Error in updating database.";
}
echo json_encode($response);

?>