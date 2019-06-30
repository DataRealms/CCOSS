<?php

/**
* @desc This contains the data of a single bound element
*/
class Lua_Binding
{
	// Type of bound class; abstract or not etc
	public $type;
	// Name of the class bound in Lua
	public $luaName;
	// Name of the cpp class it's bound to
	public $cppName;
	// Name of the class this is owned by
	public $ownerName;
	// General description of this binding
	public $description;
	// Argument descriptions
	public $arguments;
	// Return value description
	public $returns;
	
	function __construct($type, $luaName, $cppName, $ownerName)
	{
		$this->type = $type;
		$this->luaName = $luaName;
		$this->cppName = $cppName;
		$this->ownerName = $ownerName;
	}

	/**
	* @desc Set the general description
	*/
	function setDescription($description)
	{
		$this->description = $description;
	}

	/**
	* @desc Add an argument description
	*/
	function addArgument($newArgument)
	{
		$this->arguments[] = $newArgument;
	}

	/**
	* @desc Set the return value description
	*/
	function setReturns($returns)
	{
		$this->returns = $returns;
	}
}

/**
* @desc This contains the data of a bound class in Lua
*/
class Lua_Class
{
	// Type of bound class; abstract or not etc
	public $type;
	// Name of the class bound in Lua
	public $luaName;
	// Name of the cpp class it's bound to
	public $cppName;
	// The Lua name of the parent class, if any
	public $parentName;
	// Information about all the member bindings
	public $bindings;
	// 2D array of all the enumerations associated with this class
	public $enums;
	
	function __construct($type, $luaName, $cppName, $parentName)
	{
		$this->type = $type;
		$this->luaName = $luaName;
		$this->cppName = $cppName;
		$this->parentName = $parentName;
	}

	/**
	* @desc Parse a line of binding; may be a class binding or any member within
	*/
	function parseLine($bindingLine)
	{
		
	}

	/**
	* @desc Add a binding
	*/
	function addBinding($newBinding)
	{
		$this->bindings[] = $newBinding;
	}

	/**
	* @desc Add an enum
	*/
	function addEnum(&$newEnum, $enumName)
	{
		$this->enums[$enumName] = $newEnum;
	}

	/**
	* @desc Add an enum element
	*/
	function addEnumElement($enumName, $enumElementNumber, $enumElementName)
	{
		$this->enums[$enumName][$enumElementNumber] = $enumElementName;
	}

	/**
	* @desc Get the bindings
	*/
	function getBindings()
	{
		return $this->bindings;
	}
	
	/**
	* @desc Get the enums
	*/
	function getEnums()
	{
		return $this->enums;
	}
}

?>