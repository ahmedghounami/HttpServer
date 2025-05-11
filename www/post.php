<?php
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
?>
<!DOCTYPE html>
<html>
<head>
    <script src="https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4"></script>
    <title>Send Raw File</title>
    <script>
        function updateHeader(text) {
            document.getElementById('header-text').innerText = text;
        }

        function updateStatus() {
            const fileInput = document.getElementById('file');
            updateHeader(fileInput.files.length > 0 ? "✅ File selected!" : "Select a file to upload:");
        }

        function sendFile() {
            const fileInput = document.getElementById('file');
            const file = fileInput.files[0];
            const statusMsg = document.getElementById('upload-status');
            const failureMsg = document.getElementById('failure-msg');

            if (!file) return false;

            const reader = new FileReader();

            reader.onload = function(e) {
                // Send actual filename via query parameter
                fetch(`http://localhost:9090/post.php?filename=${encodeURIComponent(file.name)}`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/octet-stream'
                    },
                    body: e.target.result
                })
                .then(response => {
                    if (response.ok) {
                        statusMsg.innerText = "✅ File uploaded successfully!";
                        statusMsg.classList.remove('hidden');
                        statusMsg.classList.add('bg-green-500', 'text-white', 'p-4', 'rounded-lg');
                        failureMsg.classList.add('hidden');
                    } else {
                        failureMsg.innerText = "❌ File upload failed. Please try again!";
                        failureMsg.classList.remove('hidden');
                        failureMsg.classList.add('bg-red-500', 'text-white', 'p-4', 'rounded-lg');
                        statusMsg.classList.add('hidden');
                    }
                });
            };

            reader.readAsArrayBuffer(file);
            return false;
        }
    </script>
</head>
<body class="bg-gradient-to-br text-white flex from-black via-[#02274a] to-[#021636] h-[100vh] w-screen justify-center items-center">
    <main class="flex flex-col items-center gap-8 w-[500px]">
        <h1 id="header-text" class="text-3xl font-bold text-center">Select a file to upload</h1>
        
        <form class="border rounded-3xl p-8 grid gap-4 w-full" onsubmit="return sendFile();">
            <input id="file" type="file" onchange="updateStatus()" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 text-white w-full" required>
            <input type="submit" value="Upload File" class="bg-white/10 backdrop-blur-2xl rounded-3xl p-4 cursor-pointer w-full">
        </form>

        <p id="upload-status" class="hidden font-semibold text-center">✅ File uploaded successfully!</p>
        <p id="failure-msg" class="hidden font-semibold text-center">❌ File upload failed. Please try again!</p>
    </main>
</body>
</html>
<?php
} else if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $rawData = file_get_contents('php://input');

    // Use original filename from GET parameter, or fall back to timestamped .bin
    $fileName = basename($_GET['filename'] ?? ('upload_' . time() . '.bin'));

    if (!empty($rawData)) {
        $uploadDir = __DIR__ . '/uploads';
        if (!is_dir($uploadDir)) {
            mkdir($uploadDir, 0777, true);
        }

        $filePath = $uploadDir . '/' . $fileName;
        if (file_put_contents($filePath, $rawData)) {
            echo "File '{$fileName}' uploaded successfully!";
        } else {
            echo "File upload failed.";
        }
    } else {
        echo "No data received.";
    }
}
?>
