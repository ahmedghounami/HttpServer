<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Index</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com/" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Bungee+Spice&family=Lilita+One&display=swap" rel="stylesheet">
    <link href="file.css" rel="stylesheet">
</head>

<body
    class="bg-gradient-to-br text-white flex from-black via-[#02274a] to-[#021636] h-screen w-screen relative text-center justify-center ">
  //sent post request to get the data
    <script>
        fetch('https://api.waifu.pics/sfw/waifu')
            .then(response => response.json())
            .then(data => {
                const imageUrl = data.url;
                const imageElement = document.createElement('img');
                imageElement.src = imageUrl;
                imageElement.alt = 'Waifu Image';
                imageElement.className = 'rounded-lg shadow-lg';
                document.body.appendChild(imageElement);
            })
            .catch(error => console.error('Error fetching the image:', error));
    </script>
</body>
</html>