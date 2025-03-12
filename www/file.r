server {
    # Server Basics
    host 127.0.0.1;
    listen 8080 7878;
    server_name example.com;
    root /var/www/html;
    index index.html index.php;
    autoindex off;
    client_max_body_size 10000000;

    # Error Pages
    error_page 404 /errors/404.html;
    error_page 500 /errors/500.html;
    error_page 403 /errors/403.html;

    # Upload Handling
    upload_path /var/www/uploads; # Directory for file uploads (POST requests)
    upload_max_size 100000000;

    # Location for Static Files
    location / {
        root /var/www/html;
        index index.html;
        allow_methods GET POST;
    }

    # Location for Images
    location /images/ {
        root /var/www/media;
        allow_methods GET;
    }

    # Location for Uploads
    location /uploads/ {
        root /var/www/uploads;
        allow_methods GET POST;
        autoindex on;
    }

    # Location for CGI
    location /cgi-bin/ {
        cgi_extension .php .pl;
        cgi_path /usr/bin/php-cgi;
        cgi_timeout 30s;
        allow_methods POST;
    }

    # Redirects
    location /old-path {
        redirect /new-path;
    }

    # Access Restrictions
    location /private/ {
        deny 192.168.1.1;
        allow all;
    }

    # Custom Error Responses
    location /errors/ {
        root /var/www/errors;
        allow_methods GET;
    }

    # Specific Location with POST Restriction
    location /submit {
        allow_methods POST;
        upload_path /var/www/uploads;
    }

    # Fallback for Undefined Routes
    location / {
        return 404;
    }
}
