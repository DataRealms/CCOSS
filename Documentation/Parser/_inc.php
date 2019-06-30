<?php

// autoload our classes
function __autoload($class)
{
	require_once("Classes/" . $class . ".class.php");
}

?>