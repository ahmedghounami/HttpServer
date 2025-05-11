#!/usr/bin/php-cgi
<?php
// Start the session
session_start();

// Check if reset is requested
if (isset($_GET['reset']) && $_GET['reset'] == 1) {
    session_unset(); // Unset all session variables
    session_destroy(); // Destroy the session
    
    // Display reset confirmation message
    echo "<html><body>";
    echo "<h1>Session Reset</h1>";
    echo "<p>Your session has been reset. You will be redirected shortly.</p>";
    echo "<meta http-equiv='refresh' content='2;url=cookie.php'>"; // Redirect after 2 seconds
    echo "</body></html>";
    exit; // Exit to prevent further processing
}

// Set default session data if not set
if (!isset($_SESSION["username"])) {
    $_SESSION["username"] = "guest_" . rand(100, 999);
}

// Set the response header for HTML content

// Output HTML content
echo "<html><body>";
echo "<h1>Session Info</h1>";
echo "<p>Session ID: " . session_id() . "</p>";
echo "<p>Username: " . $_SESSION["username"] . "</p>";
echo "<p><a href='?reset=1'>Reset Session</a></p>";
echo "</body></html>";
// while(true){}
?>
