<?php

require_once("_inc.php");

// loop through all of our files and create a class for them
$dir = "_headers";
if ($dh = opendir("$dir"))
{
	while (($file = readdir($dh)) !== false)
		if(preg_match("/\.h\$/", $file))
			$headers[] = new CPP_Class(file_get_contents("$dir/$file"));
			
    closedir($dh);
}

// build up our index html
$index = "";

// which headers had classes?
foreach($headers as $header)
	if($header->getClassBlock())
		$classes[] = $header;

// build up our patterns for replacement
foreach($classes as $class)
{
	$patternsLength[] = strlen($class->getClassBlock()->name);
	$patterns[] = "/\s(" . $class->getClassBlock()->name . ")/";
}

// sort by length (so we replace SceneLayer before Scene)
array_multisort($patternsLength, $patterns);
$patterns = array_reverse($patterns);

var_dump($patterns);

// callback for replacement
function linkNames_callback($matches)
{
	$name = $matches[1];
	return " <a href=\"$name.html\">$name</a>";
}

// automagically link names
function linkNames($text)
{
	global $patterns;
	return @preg_replace_callback($patterns, "linkNames_callback", $text);
}

// process each class
foreach($classes as $class)
{
	$name = $class->getClassBlock()->name;
		
	// open up the file for writing, save to index
	$fp = fopen("_out/$name.html", "w+");
	$index .= "<a href=\"$name.html\">$name</a><br />";
	
	// start with name of class
	fwrite($fp, "<h1>$name</h1>");
	
	// process each code block
	foreach($class->getBlocks() as $block)
	{
		// Only include interesting methods, avoid boilerplate stuff
		if ($block->type != "Method" && $block->type != "Virtual method")
			continue;
		
		// Exclude boilerplate stuff that aren't bound to LUA
		if ($block->name == "Create" ||
			$block->name == "Destroy" ||
			$block->name == "Save" ||
			$block->name == "ReadProperty" ||
			$block->name == "GetClass" ||
			$block->name == "Update" ||
			$block->name == "Draw" ||
			$block->name == "DrawHUD")
			continue;
	
//		// type and name of block
//		fwrite($fp, "<h2>$block->type - $block->name</h2>");
		// Name of block
		fwrite($fp, "<h2>$block->name</h2>");
		// Declaration	
		fwrite($fp, "<p><em>$block->code</em></p>");
		
		// output each field
		foreach($block->fields as $key => $value)
		{
//			fwrite($fp, "<p><strong>$key</strong>" . nl2br(linkNames($value)) . "</p>");

			//	Description
			if ($key == "Description")
				fwrite($fp, "<p><strong>$key</strong>" . nl2br(linkNames($value)) . "</p>");

			//	Arguments
			if ($key == "Arguments")
				fwrite($fp, "<p><strong>$key</strong>" . nl2br(linkNames($value)) . "</p>");
				
			//	Return value
			if ($key == "Return value")
				fwrite($fp, "<p><strong>$key</strong>" . nl2br(linkNames($value)) . "</p>");
		}

		// end with declaration
//		fwrite($fp, "<h3>Declaration</h3>");
//		fwrite($fp, "<p><em>$block->code</em></p>");
	}
	
	fclose($fp);
}

// spit out our index file
$fp = fopen("_out/_index.html", "w+");
fwrite($fp, $index);
fclose($fp);



?>