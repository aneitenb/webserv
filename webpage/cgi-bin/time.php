#!/usr/bin/env php
<?php echo "Content-Type: text/html\r\n\r\n"; ?>
<!DOCTYPE html>
<html>
<head>
	<title>Time</title>
</head>
<body>
	<?php
	if ($_SERVER['PATH_INFO'] == "/debug") {
		echo "<h2>Environment Variables:</h2><ul>";
		foreach ($_SERVER as $var => $val) {
			if ($var != 'argv' and $var != 'argc')
				echo "<li><strong>" . $var . "</strong>: " . $val . "</li>";
		}
		echo "</ul><br><br>";
	}
	$queries = array();
	parse_str($_SERVER['QUERY_STRING'], $queries);
	$timezone = $queries['timezone'] ?? "Europe/Helsinki";
	if (in_array($timezone, DateTimeZone::listIdentifiers())) {
		date_default_timezone_set($timezone);
		echo "Current time in " . $timezone . ": " . date("H:i:s") . "<br>";
	} else {
		echo "Timezone " . $timezone . " is not a recognized timezone";
	}
	?>
</body>
</html>
