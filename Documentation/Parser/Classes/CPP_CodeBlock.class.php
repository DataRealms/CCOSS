<?php

/**
* @desc An intermediary to collect and parse all of the information for a block
*/
class CPP_CodeBlock
{
	// the type of code block
	public $type;
	
	// name of the block
	public $name;
	
	// all fields found
	public $fields;
	
	// the code between this block and the next
	public $code;
	
	// raw data
	public $raw;
	
	/**
	* @desc Parse out our code block
	*/
	function __construct($data, $code)
	{
		//print $data;
		
		$this->raw = $data . "\n" . $code;
		
		// flatten out code lines
		$code = str_replace("\n", "", $code);
		$this->code= $code;
		
		$lines = explode("\n", $data);
		
//		if (preg_match("/\/\/\//", $lines[1]))
//			var_dump($lines);
		
//		print($lines[1] . "\n");
		
		// look for type in line two		
		list($type, $name) = explode(":", $lines[1]);
		$this->type = trim(substr($type, 3));
		$this->name = trim($name);
		
		/*if (preg_match("/Class/", $this->type))
		{
			print($this->type . ", " . $this->name . "\n");
			print(chop($lines[1])."\n");
		}*/

		$lastkey = "";
		$currentValue = "";
		foreach($lines as $line)
		{
			$key = substr($line, 3, 17);
			$value = substr($line, 20);
			
			$key = trim($key);
			$value = trim($value);
			
			if(strstr($key, ":"))
			{
				if(!empty($currentKey))
				{
					$this->fields[$currentKey] = $currentValue;
					$currentValue = "";
				}
					
				$currentKey = str_replace(":", "", $key);
			}
			
			$append = " ";
			
			// append a newline if first letter is capital
			if(strtoupper(substr($value, 0, 1)) == substr($value, 0, 1))
				$append = "\n";
			
			if(!strstr($value, '//'))
				$currentValue .= $append . $value;
		}
		$this->fields[$currentKey] = $currentValue;
	}
}


/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Actor to be identical to another, by deep copy.
// Arguments:       A reference to the Actor to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Actor &reference);

becomes


//////////////////////////////////////////////////////////////////////////////////////////
// $type:  $name
//////////////////////////////////////////////////////////////////////////////////////////
// $fields[Description]:     $value
// $fields[Arguments]:       $arguments
// $fields[Return value]:    $return

	$code
*/

?>