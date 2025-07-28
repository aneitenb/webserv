#!/usr/bin/env php
<?php echo "Content-Type: text/html\r\n\r\n"; ?>
<!DOCTYPE html>
<html>
<head>
	<title>Time</title>
</head>
<body>
	<?php
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
