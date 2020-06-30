<?php
    // Database configurations:
    $servername = "localhost";
    $username = "root";
    $password = "";
    $dbname = "shelf";
    
    // Connect to database:
    $conn = mysqli_connect($servername, $username, $password, $dbname);
    if (!$conn) {
        die("Connection failed: " . mysqli_connect_error());
    }
?>