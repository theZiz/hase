<html>
<body>

<?php 
// In PHP kleiner als 4.1.0 sollten Sie $HTTP_POST_FILES anstatt 
// $_FILES verwenden.

echo '<pre>';
if (move_uploaded_file($_FILES['userfile']['tmp_name'], $uploadfile)) {
    echo "Datei ist valide und wurde erfolgreich hochgeladen.\n";
} else {
    echo "Möglicherweise eine Dateiupload-Attacke!\n";
}

echo 'Weitere Debugging Informationen:';
print_r($_FILES);

print "</pre>";

?>
</body>
</html>
