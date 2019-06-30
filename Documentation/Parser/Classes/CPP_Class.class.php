<?php

/**
* @desc This is actually better thought of a class to hold the contents of a header file
*/
class CPP_Class
{
	public $name;
	private $blocks;
	
	function __construct($data)
	{
		// the first thing we do is split up the file into comment blocks
		$lines = explode("\n", $data);
		
		$commentBlocks = array();
		$buffer = "";
		$code = "";
		$linecount = 0;
		
		foreach($lines as $line)
		{
			// skip blank lines
			if(strlen($line) == 1)
				continue;			
			
			// Skip comments before the first proper comment line ///////////////
			if ($linecount == 0 && !preg_match("/^\/\/\//", $line))
				continue;
			
			// if this line starts with //, add to current comment block
			if(preg_match("/^\/\//", $line))
			{
				$buffer .= $line . "\n";
				if(chop($line) == str_repeat('/', 90))
					$linecount++;
			}
			// otherwise reset
			else
			{
				// fill our code
				$code .= $line;

				// stop filling code and save block if we hit a semicolon or function start
				if(preg_match("/[{;]/", $line))
				{
					// only save the block if we had two full comment lines (///////, etc)
					if($linecount == 2)
						$this->blocks[] = new CPP_CodeBlock($buffer, $code);
					
					$linecount = 0;
					$buffer = "";
					$code = "";
				}
			}
		}
	}
	
	/**
	* @desc Get our class info
	*/
	function getClassBlock()
	{
		if(count($this->blocks) == 0)
			return null;
		
		foreach($this->blocks as $block)
		{
/*
			if (preg_match("/SettingsMan/", $block->name))
			{
				print("Searching for class block, found mention of: " . $block->name . "\n");
				var_dump($this->blocks);
			}
*/
			if($block->type == "Class" ||
				$block->type == "Abstract class" || $block->type == "Abstract Class" ||
				$block->type == "Concrete class" || $block->type == "Concrete Class")
			{
				return $block;
			}
		}
	}
	
	function printClassBlock()
	{
		if(count($this->blocks) == 0)
			return null;
		
		foreach($this->blocks as $block)
		{
			print(".".$block->type." \t ".$block->name."\n");
		}
	}
	
	/**
	* @desc Get all of our blocks
	*/
	function getBlocks()
	{
		return $this->blocks;
	}
	
}

?>