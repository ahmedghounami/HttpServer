<?php
session_start(); // Start the session at the top

// Check if the request is a POST or GET
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Handle form submission
    $name = isset($_POST['name']) ? $_POST['name'] : '';
    $email = isset($_POST['email']) ? $_POST['email'] : '';

    // Store in session
    $_SESSION['name'] = $name;
    $_SESSION['email'] = $email;

    // Store in cookies with metadata (expires in 1 hour)
    setcookie('cookie_name', $name, time() + 3600, '/');  // 1 hour expiration, available for the entire site
    setcookie('cookie_email', $email, time() + 3600, '/');  // 1 hour expiration, available for the entire site

    // Set the content type as JSON
    header('Content-Type: application/json');

    // Return a JSON response
    echo json_encode([
        "status" => "success",
        "message" => "Received and stored name: $name, email: $email"
    ]);
} else {
    // Display session and cookie data if available
    $sessionData = '';
    if (isset($_SESSION['name']) && isset($_SESSION['email'])) {
        $sessionData = "<p class='text-green-300 text-sm text-center'>Session: {$_SESSION['name']} - {$_SESSION['email']}</p>";
    }

    $cookieData = '';
    if (isset($_COOKIE['cookie_name']) && isset($_COOKIE['cookie_email'])) {
        $cookieData = "<p class='text-green-300 text-sm text-center'>Cookie: {$_COOKIE['cookie_name']} - {$_COOKIE['cookie_email']}</p>";
    }

    // Respond with the HTML form if no form data is received
    echo '
    <html>
    <head>
        <script src="https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4"></script>
        <title>Submit Your Information</title>
    </head>
    <body class="bg-gradient-to-br text-white flex from-black via-[#02274a] to-[#021636] h-[100vh] w-screen justify-center items-center">
        <main class="flex flex-col items-center gap-8 w-[500px]">
            <h1 class="text-3xl font-bold text-center">Enter Your Information</h1>
            ' . $sessionData . '
            ' . $cookieData . '
            <form method="POST" class="border rounded-3xl p-8 grid gap-4 w-full">
                <label for="name" class="text-white">Name:</label>
                <input type="text" id="name" name="name" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 text-white w-full" required><br><br>
                
                <label for="email" class="text-white">Email:</label>
                <input type="text" id="email" name="email" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 text-white w-full" required><br><br>
                
                <input type="submit" value="Submit" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 cursor-pointer w-full">
            </form>
        </main>
    </body>
    </html>';
}
?>
