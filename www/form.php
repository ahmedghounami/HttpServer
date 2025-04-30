<?php

$name = "";
$email = "";
$message = "";
$messageType = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    if (isset($_POST["name"]) && isset($_POST["email"])) {
        $name = htmlspecialchars($_POST['name']);
        $email = htmlspecialchars($_POST['email']);

        $messageType = "success";
        $message = "Thank you, $name! Your email address ($email) has been received.";
    } else {
        $messageType = "error";
        $message = "Please fill in all the fields.";
    }
}

if ($_SERVER["REQUEST_METHOD"] == "GET" && isset($_GET['message'])) {
    $message = htmlspecialchars($_GET['message']);
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Contact Form PHP</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link rel="icon" type="image/png" href="./icon.png" />
</head>

<body class="bg-gray-50 min-h-screen">
    <div class="min-h-screen flex flex-col">
        <!-- Nav placeholder for consistency -->
        <nav class="bg-white shadow-sm">
            <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
                <div class="flex justify-between h-16">
                    <div class="flex items-center">
                        <span class="text-xl font-semibold text-gray-800">Contact Form PHP</span>
                    </div>
                </div>
            </div>
        </nav>

        <!-- Main Content -->
        <div class="flex-grow flex items-center justify-center py-12 px-4 sm:px-6 lg:px-8">
            <div class="max-w-md w-full">
                <div class="bg-white rounded-2xl shadow-xl overflow-hidden">
                    <!-- Gradient Header -->
                    <div class="bg-gradient-to-r from-blue-500 to-blue-600 p-8">
                        <h2 class="text-3xl font-bold text-white">
                            Contact Form
                        </h2>
                        <p class="mt-2 text-blue-100">
                            Fill in your details to contact us
                        </p>
                    </div>

                    <!-- Contact Form -->
                    <div class="p-8">
                        <form method="POST" action="" class="space-y-6">
                            <div>
                                <label for="name" class="block text-sm font-medium text-gray-700">Name:</label>
                                <input type="text" id="name" name="name" required
                                    placeholder="Enter your name here"
                                    class="mt-1 block w-full border border-gray-300 rounded-md shadow-sm py-2 px-3 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-blue-500 sm:text-sm">
                            </div>
                            <div>
                                <label for="email" class="block text-sm font-medium text-gray-700">Email:</label>
                                <input type="email" id="email" name="email" required
                                    placeholder="Enter your email address"
                                    class="mt-1 block w-full border border-gray-300 rounded-md shadow-sm py-2 px-3 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-blue-500 sm:text-sm">
                            </div>
                            <button type="submit"
                                class="w-full flex justify-center py-2.5 px-4 border border-transparent rounded-lg shadow-sm text-sm font-medium text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 transition-all duration-150 hover:shadow-lg">
                                Submit
                            </button>
                        </form>

                        <?php if (!empty($message)): ?>
                            <div class="mt-4">
                                <div
                                    class="rounded-lg p-4 <?php echo $messageType === 'success' ? 'bg-green-50 text-green-800' : 'bg-red-50 text-red-800'; ?>">
                                    <div class="flex">
                                        <div class="flex-shrink-0">
                                            <?php if ($messageType === 'success'): ?>
                                                <svg class="h-5 w-5 text-green-400" fill="none" stroke="currentColor"
                                                    viewBox="0 0 24 24">
                                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2"
                                                        d="M5 13l4 4L19 7" />
                                                </svg>
                                            <?php else: ?>
                                                <svg class="h-5 w-5 text-red-400" fill="none" stroke="currentColor"
                                                    viewBox="0 0 24 24">
                                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2"
                                                        d="M6 18L18 6M6 6l12 12" />
                                                </svg>
                                            <?php endif; ?>
                                        </div>
                                        <div class="ml-3">
                                            <p class="text-sm"><?php echo htmlspecialchars($message); ?></p>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        <?php endif; ?>
                    </div>
                </div>
            </div>
        </div>
    </div>
</body>

</html>
