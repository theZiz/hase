<?php
function escape_input($input)
{
	if (get_magic_quotes_gpc())
		return $input; //Already escaped
	else
		return mysql_real_escape_string($input);
}
?>
