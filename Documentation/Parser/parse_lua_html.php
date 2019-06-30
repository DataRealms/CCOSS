<?php

require_once("_inc.php");


////////////////////////////////////////////////////////////////////////////////////
// Parse the C++ headers

// find all classes in a header files
function preprocess_header($dir, $file)
{
	// the first thing we do is split up the file into comment blocks
	$lines = explode("\n", file_get_contents("$dir/$file"));
	$classindex = -1;
	$code[0] = "";
	
	foreach($lines as $line)
	{
		// assume that all class declaration comments start with "// Class:" or "// $type class:"
		if(preg_match("/Class: /", $line) == 1 || 
			preg_match("/Abstract class: /", $line) == 1 ||
			preg_match("/Abstract Class: /", $line) == 1 ||
			preg_match("/Concrete class: /", $line) == 1 ||
			preg_match("/Concrete Class: /", $line) == 1)
		{
			$temp = explode(" ", chop($line));
			$len = count($temp);
			
			// check the $temp vector length for these oddly specific lengths
			if ($len == 5 || $len == 13)
			{
				// add a comment line so the CPP_Class constructor can process the output
				$classindex++;
				$code[$classindex] = "//////////////////////////////////////////////////////////////////////////////////////////\n";
			}
		}
		
		// skip lines before the first class declaration
		if ($classindex < 0)
			continue;
		
		$code[$classindex] .= $line."\n";
	}
	
	return $code;
}

// loop through all of our files and create any class found
$dir = "../../Entities";
if ($dh = opendir("$dir"))
{
	while (($file = readdir($dh)) !== false)
	{
		if (preg_match("/\.h\$/", $file))
		{
			$code = preprocess_header($dir, $file);
			foreach($code as $data)
				$headers[] = new CPP_Class($data);
		}
	}
	
	closedir($dh);
}

$dir = "../../Managers";
if ($dh = opendir("$dir"))
{
	while (($file = readdir($dh)) !== false)
	{
		if (preg_match("/\.h\$/", $file))
		{
			$code = preprocess_header($dir, $file);
			foreach($code as $data)
				$headers[] = new CPP_Class($data);
		}
	}
	
	closedir($dh);
}

$dir = "../../System";
if ($dh = opendir("$dir"))
{
	while (($file = readdir($dh)) !== false)
	{
		if (preg_match("/\.h\$/", $file))
		{
			$code = preprocess_header($dir, $file);
			foreach($code as $data)
				$headers[] = new CPP_Class($data);
		}
	}
	
	closedir($dh);
}

$dir = "../../Menus";
if ($dh = opendir("$dir"))
{
	while (($file = readdir($dh)) !== false)
	{
		if (preg_match("/\.h\$/", $file))
		{
			$code = preprocess_header($dir, $file);
			foreach($code as $data)
				$headers[] = new CPP_Class($data);
		}
	}
	
	closedir($dh);
}

// which headers had classes?
foreach($headers as $header)
{
	if ($header->getClassBlock())
		$cppClasses[] = $header;
}

// Search for a specific cpp class
function FindCPPClass($classes, $className)
{
	foreach($classes as $class)
	{
		if ($class->getClassBlock()->name == $className)
			return $class;
	}
	return null;
}


// Search for a specific code block of a specific cpp class
function FindCPPBlock($classes, $className, $memberName)
{
	foreach($classes as $class)
	{
		if ($class->getClassBlock()->name == $className)
		{
			foreach($class->getBlocks() as $block)
			{
				if ($block->name == $memberName)
					return $block;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////
// Parse the LuaMan for all the lua bindings

// Read and split up the binding file into lines
$luaBindingLines =  explode("\n", file_get_contents("../../Managers/LuaMan.cpp"));

// Find the beginning of the actual bindings
foreach($luaBindingLines as $lineIndex => $luaLine)
{
	if (preg_match("/module\(/", $luaLine))
	{
		// Remove the start of the bindings and commence parsing them
		unset($luaBindingLines[$lineIndex]);
		break;
	}
	// Remove all the stuff before the actual bindings
	else
		unset($luaBindingLines[$lineIndex]);
}

// Re-index the lines to shift away all teh ones that were removed
$luaBindingLines = array_values($luaBindingLines);

$currentClass = null;
$currentEnum = (array)null;
// Go through each of the lines and build the actual bindings
foreach($luaBindingLines as $luaLine)
{
	$matches = (array)null;
	
	// Ignore empty, useless, and comment lines
	if (strlen($luaLine) == 1 || preg_match("/^\s*(\/\/|\/\*|\*\/|\[|\]|$)/", $luaLine))
		continue;
	// Global function binding
	else if (preg_match("/^\s*def\(/", $luaLine, $matches))
	{
		
	}
	// Generic class binding
	else if (preg_match("/\s*class_<((?:\w*:*)(\w*) *,* *(\w*))>\(\"(.*)\"\)/", $luaLine, $matches))
	{
		// Extract all the names from the binding
		$luaName = $matches[4];
		$cppName = "";
		$parentName = "";
		// Figure out if it was a complex name and extract the simple class name without namespaces etc
		if (preg_match("/[\:,]/", $matches[1]))
		{
			$cppName = $matches[2];
			$parentName = $matches[3];
		}
		else
		{
			$cppName = $matches[1];
		}
		
		if ($cppName == "")
		{
			if ($matches[4] == "Activity")
				$cppName = "Activity";
			else if ($matches[4] == "GameActivity")
				$cppName = "GameActivity";
			else
				print("Found unknown class string ".$matches[0]."\n");
		}
		
		// Create the lua class instance that represents this binding
		$luaClasses[] = $currentClass = new Lua_Class("Generic", $luaName, $cppName, $parentName);
		// Close off any open member enum bindings
		$currentEnum = (array)null;
	}
	// Abstract class binding
	else if (preg_match("/\s*ABSTRACTLUABINDING\( *(\w*)[ ,]*(\w*) *\)/", $luaLine, $matches))
	{
		// Extract all the names from the binding
		$luaName = $cppName = $matches[1];
		$parentName = $matches[2];
		// Create the lua class instance that represents this binding
		$luaClasses[] = $currentClass = new Lua_Class("Abstract", $luaName, $cppName, $parentName);
		// Add some of the baseline bindings associated with all abstract classes
		$currentClass->addBinding(new Lua_Binding("Member Property", "ClassName", "GetClassName", $luaName));
		$globalBindings[] = new Lua_Binding("Global Function", "To" . $luaName, "", "Global");
		// Close off any open member enum bindings
		$currentEnum = (array)null;
	}
	// Concrete RTE class binding
	else if (preg_match("/\s*CONCRETELUABINDING\( *(\w*)[ ,]*(\w*) *\)/", $luaLine, $matches))
	{
		// Extract all the names from the binding
		$luaName = $cppName = $matches[1];
		$parentName = $matches[2];
		// Create the lua class instance that represents this binding
		$luaClasses[] = $currentClass = new Lua_Class("Concrete", $luaName, $cppName, $parentName);
		// Add some of the baseline bindings associated with all abstract classes
		$currentClass->addBinding(new Lua_Binding("Member Property", "Clone", "Clone", $luaName));
		$currentClass->addBinding(new Lua_Binding("Member Property", "ClassName", "GetClassName", $luaName));
		$globalBindings[] = new Lua_Binding("Global Function", "Create" . $luaName, "", "Global");
		$globalBindings[] = new Lua_Binding("Global Function", "To" . $luaName, "", "Global");
		// Close off any open member enum bindings
		$currentEnum = (array)null;
	}
	// Shouldn't ever encounter a member binding def outside a class binding
	else if ($currentClass == null)
		continue;
	// Member function binding
	else if (preg_match("/\s*\.def\( *(.*) *\)/", $luaLine, $matches))
	{
		// If thre are quotes, it's a normal function binding
		if (preg_match("/[\"]/", $matches[1]))
		{		
// TODO: make work with the crazy argument-specified bindings, eg:
//.def("ForceBounds", (bool (SceneMan::*)(int &, int &))&SceneMan::ForceBounds)etc
			preg_match("/^\"(\w*)\"[ ,]*\&\w*\:\:(\w*)/", $matches[1], $matches);			
			if (count($matches) > 0)
			{
				$luaName = $matches[1];
				$cppName = $matches[2];
				$ownerName = $currentClass->luaName;
				$currentClass->addBinding(new Lua_Binding("Member Function", $luaName, $cppName, $ownerName));
			}
		}
		// Special binding, like a constructor or operator overload
		else
		{
// TODO! Handle constructors and operator overload bindings
		}
		$currentEnum = (array)null;
	}
	// Member var binding
	else if (preg_match("/\s*\.def_readwrite\(/", $luaLine, $matches))
	{
// TODO! Handle this
	}
	// Member property binding
	else if (preg_match("/^\s*\.property\( *(\"(\w*)\"[ ,]*\&\w*\:\:(\w*).*) *\)/", $luaLine, $matches))
	{
		$luaName = $matches[2];
		$cppName = $matches[3];
		$ownerName = $currentClass->luaName;
		// Determine whether it's a read-only, or read/write property (whether there's both gets AND sets accessors bound)
		if (preg_match("/&.*&/", $matches[1]))
			$currentClass->addBinding(new Lua_Binding("Member Property", $luaName, $cppName, $ownerName));
		else
			$currentClass->addBinding(new Lua_Binding("Member Read-Only Property", $luaName, $cppName, $ownerName));
	}
	// Member enumeration binding
	else if (preg_match("/\s*\.enum_\( *\"(.*)\" *\)/", $luaLine, $matches))
	{
		$currentEnum = (array)null;
		$currentClass->addEnum($currentEnum, $matches[1]);
	}
	// Enumeration element binding
	else if (preg_match("/\s*value\( *\"(.*)\"[ ,]*(\d*) *\)/", $luaLine, $matches))
	{
		if ($currentEnum == null)
			continue;
		// Add the enumeration mapping to the current enum
		$currentEnum[$matches[2]] = $matches[1];
	}
}


////////////////////////////////////////////////////////////////////////////////
// Build the HTML

// build up our patterns for replacement
foreach($luaClasses as $class)
{
	$patternsLength[] = strlen($class->luaName);
	$patterns[] = "/\s*(" . $class->luaName . ")\z/";
}

// sort by length (so we replace SceneLayer before Scene)
array_multisort($patternsLength, $patterns);
$patterns = array_reverse($patterns);

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

// build up our index html
$entity = "";
$generic = "";

// process each class
foreach($luaClasses as $luaClass)
{
	$luaName = $luaClass->luaName;
	
	// Find the corresponding CPP class
	$cppClass = FindCPPClass($cppClasses, $luaClass->cppName);
	
	// If we couldn't find the cpp class, then go onto next one
	if ($cppClass == null)
	{
		print("ERROR: Couldn't find C++ class that " . $luaName . " is bound to, ie: " . $luaClass->cppName . "!\n");
		continue;
	}

	// open up the file for writing, save to index
	$fp = fopen("_outLua/$luaName.html", "w+");
	if ($luaClass->parentName == "")
		$generic .= "<a href=\"$luaName.html\">$luaName</a><br />";
	else
		$entity .= "<a href=\"$luaName.html\">$luaName</a><br />";
	
	// start with type and name of class, don't type if it's a manager
	if (!preg_match("/Manager/", $luaName))
		fwrite($fp, "<h2>$luaClass->type Class:</h2>");
	fwrite($fp, "<h1>$luaName</h1>");

	// Description
	fwrite($fp, "<h3>" . linkNames($cppClass->getClassBlock()->fields["Description"]) . "</h3>");

	// Parent class, if any
	if (strlen($luaClass->parentName) > 0)
		fwrite($fp, "<h3>Parent: " . nl2br(linkNames($luaClass->parentName)) . "</h3>");
	else
		fwrite($fp, "<p>Has no parent class.</p>");

	// No bindings, then move onto next
//	if (!$luaClass->getBindings())
//		continue;

	// Go through each lua binding, find the cpp member it's bound to, and print out its data
	foreach($luaClass->getBindings() as $binding)
	{
		foreach($cppClass->getBlocks() as $block)
		{
/*
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
*/
			if ($block->name == $binding->cppName)
			{
				// Write out all the data:
				// Type of binding
				fwrite($fp, "<br/><h3>" . preg_replace("/Member /", "", $binding->type) . ":</h3>");
				// Name of binding
				fwrite($fp, "<h2>$binding->luaName</h2>");
				// Code declaration
//				fwrite($fp, "<p><em>$block->code</em></p>");

				if (preg_match("/Property/", $binding->type))
				{
//					fwrite($fp, "<p><strong>Description:</strong><br/>");
					fwrite($fp, "<p>");
					
					// Output only the fields relevant to properties
					foreach($block->fields as $key => $value)
					{
						if ($key == "Description" || $key == "Arguments" || $key == "Return value")
							if (!preg_match("/None/", $value))
								fwrite($fp, linkNames(preg_replace("/Gets t/", "T", preg_replace("/Gets a/", "A", preg_replace("/Returns a/", "A", $value)))));
					}
					
					fwrite($fp, "</p>");
				}
				else
				{
					// Output each field normally
					foreach($block->fields as $key => $value)
					{
			//			fwrite($fp, "<p><strong>$key</strong>" . nl2br(linkNames($value)) . "</p>");
			
						//	Description
						if ($key == "Description")
//							fwrite($fp, "<p><strong>$key</strong>" . ":" . nl2br(linkNames($value)) . "</p>");
							fwrite($fp, "<p>" . linkNames($value) . "</p>");
			
						//	Arguments
						if ($key == "Arguments")
							fwrite($fp, "<p><strong>$key:</strong>" . nl2br(linkNames($value)) . "</p>");
							
						//	Return value
						if ($key == "Return value")
							fwrite($fp, "<p><strong>$key:</strong>" . nl2br(linkNames($value)) . "</p>");
					}
				}
			}
		}
	}

	fclose($fp);
}

// spit out our index file
$fp = fopen("_outLua/index.html", "w+");
fwrite($fp, "<H3>Entity</H3>");
fwrite($fp, $entity);
fwrite($fp, "<BR><H3>Generic</H3>");
fwrite($fp, $generic);
fclose($fp);

?>