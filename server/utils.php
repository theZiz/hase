<?php
function escape_input($connection, $input)
{
	if (get_magic_quotes_gpc())
		return $input; //Already escaped
	else
		return mysqli_real_escape_string($connection, $input);
}
?>
