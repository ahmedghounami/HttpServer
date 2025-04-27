<?php
$targetDir = "/Users/hboudar/goinfre/";

if (!file_exists($targetDir)) {
	mkdir($targetDir, 0777, true);
}

$messageType = "";
$message = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
	$filename = basename($_POST["filename"]);
	$filePath = $targetDir . $filename;

	if (empty($filename)) {
		$messageType = "error";
		$message = "Please enter a file name.";
	} else if (!file_exists($filePath)) {
		$messageType = "error";
		$message = "File not found: " . htmlspecialchars($filename);
	} else {
		if (unlink($filePath)) {
			$messageType = "success";
			$message = "File deleted successfully: " . htmlspecialchars($filename);
		} else {
			$messageType = "error";
			$message = "Failed to delete the file: " . htmlspecialchars($filename);
		}
	}
}

?>

<!DOCTYPE html>
<html lang="en">

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Delete file by name</title>
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
						<span class="text-xl font-semibold text-gray-800">Delete File by Name</span>
					</div>
				</div>
			</div>
		</nav>

		<!-- Main Content -->
		<div class="flex-grow flex items-center justify-center py-12 px-4 sm:px-6 lg:px-8">
			<div class="max-w-md w-full">
				<div class="bg-white rounded-2xl shadow-xl overflow-hidden">
					<!-- Gradient Header -->
					<div class="bg-gradient-to-r from-red-500 to-red-600 p-8">
						<h2 class="text-3xl font-bold text-white">
							Delete File
						</h2>
						<p class="mt-2 text-red-100">
							Enter the name of the file you want to delete
						</p>
					</div>

					<!-- Delete Form -->
					<div class="p-8">
						<form action="" method="POST" class="space-y-6">
							<div>
								<label for="filename" class="block text-sm font-medium text-gray-700">File Name:</label>
								<input type="text" id="filename" name="filename" placeholder="File name" required
									class="mt-1 block w-full border border-gray-300 rounded-md shadow-sm py-2 px-3 focus:outline-none focus:ring-2 focus:ring-red-500 focus:border-red-500 sm:text-sm">
							</div>
							<button type="submit"
								class="w-full flex justify-center py-2.5 px-4 border border-transparent rounded-lg shadow-sm text-sm font-medium text-white bg-red-600 hover:bg-red-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-red-500 transition-all duration-150 hover:shadow-lg">
								Delete
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