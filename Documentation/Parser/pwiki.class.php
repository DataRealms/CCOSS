<?php
/*
 * This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 * PHPediaWiki (Pwiki), ported from SxWiki.
 * Primarily developed by en:User:Atyndall, for a full list of
 *  developers, see http://pwiki.sf.net/docs/Pwiki:Developers
 * Documentation can be found at http://pwiki.sf.net/
 * Changelog for this version can be found at 
 *  http://pwiki.sf.net/cl/$Pwiki_Version
 * A line written like "@See [[Link]]" means see documentation page
 *  of that name on the documentation website. E.g. "@See [[Home]]"
 *  means go to http://pwiki.sf.net/sdocs/0.1.2/ and search for Home.
 */

// @See [[Var:Pwiki Version]]
$Pwiki_Version = '0.1.2';

// @See [[class Pwiki]]
class Pwiki
{
  // @See [[Var:configFile]] and [[Configuration file]]
  var $configFile='config.include.php';

  /*
   * These are PHPediaWiki's default varibles, 
   * do not change them, instead override them 
   * using a configuration file and $configFile.
   */
  
  // @See [[Var:editsPerMinute]]
  var $editsPerMinute=10;

  // @See [[Var:username]] or [[Var:password]]
  var $username='';
  var $password='';
 
  // @See [[Var:mailto]]
  var $mailto=true;

  // @See [[Var:emailadd]]
  var $emailadd='';

  // @See [[Var:autoLogin]]
  var $autoLogin=true;

  // @See [[Var:loggedIn]]
  var $_loggedIn=false;
  
  // @See [[Var:forcelogin]]
  var $forcelogin=true;
  
  // @See [[Var:forcecheckrun]]
  var $_forcecheckrun=true;
  
  // @See [[Var:runpage]]
  var $runpage='run';
 
  // @See [[Var:runregex]]
  var $runregex='/(yes|enable|true)/i';
  
  // @See [[Var:message]]
  var $message="There has been an error in Pwiki.\n\nThis automated message was sent by a bot running the Pwiki framework.";

  // @See [[Var:subject]]
  var $subject='Bot error';

  // @See [[Var:url]]
  var $url = '';

  // @See [[Var:wikipedia]]
  var $wikipedia='en';

  // @See [[Var:_baseURL]]
  var $_baseURL;

  // @See [[Var:autoDie]]
  var $autoDie=true;

  // @See [[Var:verbosity]] and [[Var:logverbosity]] and [[Verbosity levels]]
  var $verbosity=1;
  var $logverbosity=4;
  
  // @See [[Var:logerrors]]
  var $logerrors="1";

  // @See [[Var:logfile]]
  var $logfile='';

  // @See [[Var:ch]]
  var $_ch=NULL;

  // @See [[Var:tempDir]]
  var $tempDir='';

  // @See [[Var:cookiejar]]
  var $_cookiejar=NULL;

  // @See [[Var:maxlag]]
  var $maxlag=5;

  // @See [[Var:proxyaddr]]
  var $proxyaddr=NULL;

  // @See [[Var:retries]]
  var $retries=3;

  // @See [[Var:confRead]]  
  var $_confRead=false;

  // @See [[Var:run]]
  var $_run=true;

  // @See [[Var:errmessage]]
  var $errmessage = '[1] Unknown Error - See log output for details';

  // @See [[Var:posttalkpage]]
  var $posttalkpage = true;

  // @See [[Var:ownerusername]]
  var $ownerusername = '';

  // @See [[Var:errpostsum]]
  var $errpostsum = "Posting to owner's talk page, informing about error.";

  // Disabled. No documentation available.
  //var $filename = basename(__FILE__);
  
  // @See [[Var:postermsg]]
  var $postermsg = '
== PHPediaWiki error ==
Hello, this is your bot speaking. Just letting you know that I have suffered an error and will need your help to restart my self. Please access the computer I am running on and restart my script by typing <code>./filename</code> or <code>php filename</code>. For your convenience, the error I received has been posted below. Thankyou.';

  // @See [[Var:apiedit]]
  var $apiedit = true;

  /*
   * These are the PHPpediaWiki functions,
   * their documentation can be found in
   * [[Category:Functions]].
   */

  // @See [[Func:Pwiki]]
  function Pwiki($wikipedia=NULL)
  {
  	$this->_debug(1,"Pwiki STARTED\n**************************************\n**************************************\n\n\n\n");
	$this->_readConf();
	if(isset($this->logfile)){
		ini_set("log_errors" , $this->logerrors);
		ini_set("error_log" , $this->logfile);
	}
	if (is_dir("/tmp")) {
      $this->tempDir="/tmp";
    } elseif (is_dir("C:\\")) {
      $wt="C:\\temp";
      if (is_dir($wt) || @mkdir($wt)) {
        $this->tempDir=$wt;
      }
    }
    if ($wikipedia) {
      $this->wikipedia=$wikipedia;
    }
  }

  // @See [[Func:doSleep]]
  function _doSleep()
  {
  	$this->_debug(4,"Function _doSleep called.");
    // TODO: should be smarter, by using timers to count
    // how many edits it /really/ performs per minute.
    $epm=(int) $this->editsPerMinute;
    if ($epm<=0) {
      return NULL;
    }
    sleep((int) 60 / $epm);
    return true;
  }
 
  // @See [[Func:getBaseURL]]
  function _getBaseURL()
  {
    if ($this->url) {
      $this->_baseURL=$this->url;
    } elseif ($this->wikipedia) {
      $this->_baseURL="http://".strtolower($this->wikipedia).".wikipedia.org/w/";
    } else {
      $this->_error("Failed determining base URL!");
      return false;
    }
    return $this->_baseURL;
  }

  // @See [[Func:error]]
  function _error($message)
  {
    $this->_debug(0,'ERROR: '.$message);
    $message = $this->errmessage;
    if ($this->autoDie) {
	  $this->_debug(1,"Pwiki TERMINATED\n**************************************\n**************************************\n\n\n\n");
	if($this->mailto){
		$headers = "From: ". $this->username . '@' . $_SERVER['SERVER_ADDR'] . "\r\n". "Reply-To: " . $this->emailadd . "\r\n" . 'X-Mailer: PHP/' . phpversion();
		mail($this->emailadd, $this->subject, $this->message."\n\n Error Message: $message", $headers);
		}
	if($this->posttalkpage){
	  $this->putPage('User talk:'.$this->ownerusername, $this->errpostsum, $this->postermsg."\n\n<pre>\n$this->errmessage\n</pre>\n~~~~", false, false, true);
	}
      exit(1);
    }
    return true;
  }

  // @See [[Func:exit]]
  function _exit($message)
  {
    $this->_debug(0,'EXIT: '.$message);
    $this->_debug(1,"Pwiki EXITED\n**************************************\n**************************************\n\n\n\n");
    return true;
    exit(0);
  }

  // @See [[Func:debug]]
  function _debug($level,$message)
  {
	date_default_timezone_set('America/Phoenix');
  	$fd = '[' . date('Y-m-d h:i:sA') . '] Pwiki: ' . trim($message) . "\n";
	/*	if (isset($this->logfile) && $this->logverbosity>=$level) {
  		$fh = fopen($this->logfile, 'a');
		fwrite($fh, $fd);
  		fclose($fh);
		}*/
    if ($this->verbosity>=$level) {
  		print($fd);
    	return true;
    }
    return NULL;
  }

  // @See [[Func:login]]
  function login($username=NULL, $password=NULL)
  {
  	$this->_debug(4,"Function login called.");
    $this->_autoexec("login");
    if (!isset($username, $password) && isset($this->username, $this->password)) {
      $username=$this->username;
      $password=$this->password;
    } else {
    	return false;
    }
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    if ($this->forcecheckrun && !$this->checkRun()) {
    	return false;
    }
    $request=
      $url.'api.php?'.
      'action=login&'.
      'lgname='.urlencode($this->username).'&'.
      'lgpassword='.urlencode($this->password).'&'.
      'format=php';
    $params="wpName=".$this->username."&".
      "wpPassword=".$this->password."&".
      "wpLoginattempt=true";
    $sxLgInput=$this->_curlDo($request,$params);
    $sxLgI = unserialize($sxLgInput);
    $result = $sxLgI['login']['result'];
    if ($result == "NeedToWait") {
    	$waittime = $sxLgI['login']['wait'] + 1;
    	$this->_debug(2,"login: Login failed: NeedToWait. Sleeping for wait time ($waittime seconds).");
    	sleep($waittime);
    	unset($sxLgInput, $sxLgI, $result);
    	$sxLgInput=$this->_curlDo($request,$params);
    	$sxLgI = unserialize($sxLgInput);
    	$result = $sxLgI['login']['result'];
    	return $sxLgI['login']['lgtoken'] && $sxLgI['login']['lguserid'];	
//< blinky*mar, 10 may 2011 19:38:40 -0300 - take care of the SendToken thingy
    } elseif ($result == "NeedToken") {
        $this->_debug(2,"login: Login failed: NeedToken. Sending token");
        $params.="&lgtoken=".$sxLgI['login']['token'];
        unset($sxLgInput, $sxLgI, $result);
        $sxLgInput=$this->_curlDo($request,$params);
        $sxLgI = unserialize($sxLgInput);
        $result = $sxLgI['login']['result'];
        if  ($result != "Success") {
            $this->_debug(2,"login: Output: $sxLgInput");
            $this->_error("login: Login failed: $result");
        } else {
            $this->_debug(1,"login: Logged in as ".$this->username.".");
            $this->_loggedIn=true;
        }
        return $sxLgI['login']['lgtoken'] && $sxLgI['login']['lguserid'];
//>
    } elseif ($result != "Success") {
      $this->_error("login: Login failed: $result");
      $this->_debug(2,"login: Output: $sxLgInput");
    } else {
      $this->_debug(1,"login: Logged in as ".$this->username.".");
      $this->_loggedIn=true;
    }
    return $sxLgI['login']['lgtoken'] && $sxLgI['login']['lguserid'];
  }

  // @See [[Func:autoexec]]
  function _autoexec($skip=NULL)
  {
  	$this->_debug(4,"Function _autoexec called.");
    if ($skip!='conf') {
      $this->_readConf();
    }
    if ($skip!='login') {
      $this->_autoLogin();
    }
    return NULL;
  }

  // @See [[Func:autoLogin]]
  function _autoLogin()
  {
  	$this->_debug(4,"Function _autoLogin called.");
    if ($this->autoLogin && !$this->_loggedIn) {
      return $this->login();
    }
    return NULL;
  }
 
  // @See [[Func:getPage]]
  function getPage($article)
  {
    $this->_debug(4,"Function getPage called.");
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    $this->_debug(2,"getPage: Retrieving source for $article");
    $params=array(
      'action'=>'query',
      'prop'=>'revisions',
      'titles'=>$article,
      'rvprop'=>'content'
    );
    $sxGetA = $this->callAPI($params);
    if (!$sxGetA) {
      $this->_error("getPage: Failed retrieving result for article $article.");
      return false;
    }
    $sxGetAID = $sxGetA['query']['pages'];
    $sxGetAID = array_shift($sxGetAID);
    $sxGetAID = array_shift($sxGetAID);
    $sxAText = $sxGetA['query']['pages'][$sxGetAID]['revisions'][0]["*"];
    return $sxAText;
  }
  
  // @See [[Func:getPageInfo]]
  function getPageInfo($article)
  { 
  	$this->_debug(4,"Function getPageInfo called.");
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    $this->_debug(2,"getPageInfo: Retrieving page info for $article");
    $params=array(
      'action'=>'query',
      'prop'=>'info',
      'titles'=>$article
    );
    $sxGetA = $this->callAPI($params);
    if (!$sxGetA) {
      $this->_error("getPageInfo: Failed retrieving result for article $article.");
      return false;
    }
    $sxGetA = $sxGetA['query']['pages'];
    list($key)=array_keys($sxGetA);
    return $sxGetA[$key];
  }

  // @See [[Func:curlDo]]
  function _curlDo($url,$params='')
  {
  	$this->_debug(4,"Function _curlDo called.");
    if (!$this->_ch) {
      $this->_ch=curl_init();
    }
    $ch=&$this->_ch;
    if (!$this->_cookiejar) {
      $this->_cookiejar=tempnam($this->tempDir,"Pwiki_cookiejar_");
    }
    curl_setopt($ch, CURLOPT_COOKIEJAR, $this->_cookiejar);
    curl_setopt($ch, CURLOPT_URL,$url);
    if($params) {
      curl_setopt($ch, CURLOPT_POST, true);
      curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
    } else {
      curl_setopt($ch, CURLOPT_POST, false);
    }
    curl_setopt($ch, CURLOPT_RETURNTRANSFER,1);

    // We do NOT want to execute this conditionally!
    //< blinky*mar, 10 may 2011 19:44:45 -0300 - cURL changed
    curl_setopt($ch, CURLOPT_PROXY,$this->proxyaddr);
    //>
    $attempts=$this->retries+1;
    do {
      $attempts--;
      $ret=curl_exec($ch);
    } while (curl_errno($ch) && $attempts);
    if (curl_errno($ch)) {
      $this->_error(curl_error($ch));
    }
    return $ret;
  }

  // @See [[Func:getUrl]]
  function getUrl($url) {
  	$this->_debug(4,"Function getUrl called.");
    return $this->_curlDo($url);
  }

  // @See [[Func:putPage]]
  function putPage($article, $editsum, $newtext, $minor=false, $botedit=true, $newsection=false)
  { 
    $this->_debug(4,"Function putPage called.");
    $this->_autoexec();
    if (!$url=$this->_getBaseURL()) {
        return false;
    }
    if ($this->forcecheckrun && !$this->checkRun()) {
    	return false;
    }
    if (!$this->_loggedIn && $this->forcelogin) {
		$this->_error("putPage: Could not login, not editing page.");
		return false;
    }
    if ($this->apiedit) {
      $this->_debug(3,"putPage: using API editing.");
      if ($minor) {
	$edittype = 'minor';
      } else {
	$edittype = 'notminor';
      }
      if ($botedit) {
	$bottype = '1';
      } else {
	$bottype = '0';
      }
      $params = array(
		    'action'=>'query',
		    'prop'=>'info',
		    'intoken'=>'edit',
		    'titles'=>$article,
		    'indexpageids'=>'1'
		    );
      $initials = $this->callAPI($params);
      $pageid = $initials['query']['pageids'][0];
      $token = $initials['query']['pages'][$pageid]['edittoken'];
      $postparams = array(
			'action'=>'edit',
			'title'=>$article,
			'text'=>$newtext,
			'token'=>$token,
			'summary'=>$editsum,
			$edittype=>'0',
			'bot'=>$bottype,
			);
      if ($newsection) {
	$postparams['section'] = 'new';
	$newsec = ' new section';
      } else {
	$newsec = NULL;
      }
      $this->_debug(3,"putPage: title: $article; token: $token; summary: $editsum;$newsec");
      $response = $this->callAPI($postparams);
      $result = $response['edit']['result'];
      $revid = $response['edit']['newrevid'];
      if ($result != 'Success') {
	$this->_debug(3,"putPage: Posting error: '$result'");
	$returnval = false;
      } else {
	$this->_debug(3,"putPage: Success posting revision id $revid");
	$returnval = true;
      }
    } else {
         $this->_debug(3,"putPage: using POST editing.");
	 $article = $this->toURL($article);
	 $postrequest = $url . 'index.php?action=edit&title=' . $article;
	 $DOMresults = $this->inputRetrieve($postrequest);
	 /* $this->_debug(3,"putPage: getting $postrequest");
	 $response = $this->_curlDo($postrequest);
	 $this->_debug(4,"putPage:got response:");
	 $this->_debug(4,$response);
	 $dom = true;
	 if ($dom) {
	   $dom = new DOMDocument();
	   $dom->loadHTML($response);
	   $input = $dom->getElementsByTagName( "input" );
	   foreach( $input as $tag ) {
	     $name = $dom->getAttribute('name');
	     $value = $dom->getAttribute('value');
	     $DOMresults[$name] = $value;
	     $this->_debug(4,"putPage: pulled dom $name with value $value");
	   }
	   }*/
	 $this->_debug(3,"starttime: {$starttime[1]}; edittime: {$edittime[1]}; wpAutoSummary: {$autosum[1]}; token: {$token[1]}");
	 if (!$starttime || !$edittime || !$autosum || !$token) {
	   $this->_error("Failed retrieving necessary data for editing!");
	   return false;
	 }
	 $postrequest = $url . 'index.php?title=' . $article . '&action=submit&maxlag=' . $this->maxlag;
	 if (!$botedit) {
	   $postrequest = $postrequest . "&bot=0";
	 }
	 $postData['wpEditToken'] = $DOMresults['wpEditToken'];
	 $postData['wpAutoSummary'] = $DOMresults['wpAutoSummary'];
	 $postData['wpStarttime'] = $DOMresults['wpStarttime'];
	 $postData['wpEdittime'] = $DOMresults['wpEdittime'];
	 $postData['wpScrolltop'] = '';
	 $postData['wpSection'] = '';
	 $postData['wpSummary'] = $editsum;
	 $postData['wpTextbox1'] = $newtext;
	 $postData['wpSave'] = 1;
	 if ($minor) {
	   $postData['wpMinoredit'] = 1;
	 }
	 $response = $this->_curlDo($postrequest,$postData);
	 $this->_debug(4,"putPage response:");
	 $this->_debug(4,$response);
	 if (preg_match('/^Waiting for (.*) seconds lagged/', $response)) {
	   $this->_error("Maxlag hit, not posted.");
	   $returnval = false;
	 } else {
	   $returnval = true;
	 }
    }
    $this->_doSleep();
    return $returnval;
  }

  // @See [[Func:getCat]]
  function getCat($categoryname, $ns='0', $limit='500', $sort='timestamp', $tsstart='1', $direction='asc')
  {
  	$this->_debug(4,"Function getCat called.");
    $fcat = array();
    if (!isset($categoryname)){
    	$this->_error('getCat: $categoryname is not set.');
    	return false;
    }
    
    $params=array(
      'action'=>'query',
      'list'=>'categorymembers',
      'cmtitle'=>'Category:'.$this->toURL($categoryname),
      'cmlimit'=>$limit,
      'cmsort'=>$sort,
      'cmnamespace'=>$ns,
      'cmstart'=>$tsstart,
      'cmdir'=>$direction
    );
    $finalcat = $this->callAPI($params);
    if(isset($finalcat['query-continue']['categorymembers']['cmcontinue'])) {
      $firstrun = 1;
      $catfrom = $finalcat['query-continue']['categorymembers']['cmcontinue'];
    } else {
      $firstrun = 0;
    }
    $i=1;
    while (!isset($finalcat['query']['categorymembers']) && $i <= 5) {
      $this->_debug(4,'$finalcat["query"]["categorymembers"] was not set, trying to access data again ($i == '."$i).");
      unset($finalcat);
      $finalcat = $this->callAPI($params);
      $i++;
    }
    foreach($finalcat['query']['categorymembers'] as $fcat_l) {
      array_push($fcat, $fcat_l);
    }
    $done=false;
    while(!$done) {
      if(
        isset($cat['query-continue']['categorymembers']['cmcontinue']) ||
        $firstrun == 1
      ) {
        $params['cmcontinue']=$catfrom;
        $cat = $this->callAPI($params);
        $catfrom = $cat['query-continue']['categorymembers']['cmcontinue'];
        $catfrom = urlencode($catfrom);
        foreach($cat['query']['categorymembers'] as $fcat_l) {
          array_push($fcat, $fcat_l);
        }
        $firstrun = 0;
      } else {
        $done = true;
      }
    }
    $result = array();
    foreach($fcat as $catmemb) {
      if ($catmemb['ns'] == $ns || $ns == "all") {
        $cret =  $catmemb['title'];
        $cret = ltrim($cret);
        $cret = rtrim($cret);
        array_push($result, $catmemb['title']);
      }
    }
    return $result;
  }
  
  // @See [[Func:randcatMembers]]
  function randcatMembers($cat, $ns='0', $limit='500')
  {
	$this->_debug(4,"Function randcatMembers called.");
	if ($limit == '0') {
	  $this->_debug(2,"randcatMembers: Limit must be above 0, function aborted.");
	  return false;
	}
	$stsparams=array(
		'action'=>'query',
		'list'=>'categorymembers',
    		'cmtitle'=>'Category:'.$cat,
		'cmlimit'=>'1',
   		'cmnamespace'=>$ns,
		'cmprop'=>'timestamp',
		'cmsort'=>'timestamp'
	);
  	$tsresult = $this->callAPI($stsparams);
	$ts = $tsresult['query']['categorymembers'][0]['timestamp'];
       	$etsparams=array(
		'action'=>'query',
		'list'=>'categorymembers',
    		'cmtitle'=>'Category:'.$cat,
		'cmlimit'=>$limit,
   		'cmnamespace'=>$ns,
		'cmprop'=>'timestamp',
		'cmsort'=>'timestamp',
		'cmdir'=>'desc'
	);
  	$etsresult = $this->callAPI($etsparams);
	$exceptioncatch = count($etsresult['query']['categorymembers']);
	$this->_debug(4,"randcatMembers: exceptioncatch is $exceptioncatch.");
	$ets = $etsresult['query']['categorymembers'][($limit-1)]['timestamp'];
	if ($exceptioncatch == 0) {
		  $this->_debug(3,"randcatMembers: categorymembers contains no pages, aborting.");
		  return false;
	} 
	while ($exceptioncatch != $limit) {
	  $this->_debug(4,"randcatMembers: Catching the exception, limit = $limit.");
	  $limit--;
	  $ets = $etsresult['query']['categorymembers'][($limit-1)]['timestamp'];
	}
	$this->_debug(4,"randcatMembers: beginning timestamp = $ts, end timestamp = $ets.");
	$tsinus = strtotime(substr($ts, 0, -1));
	$endtsinus = strtotime(substr($ets, 0, -1));
	$this->_debug(4,"randcatMembers: tsinus = $tsinus, endtsinus = $endtsinus.");
	$rand = rand($tsinus,$endtsinus); 
	$this->_debug(4,"randcatMembers: rand = $rand.");
        $randts = substr(date("c", $rand), 0, -6)."Z";
	$this->_debug(4,"randcatMembers: Randomly generated timestamp is $randts.");
	$result = $this->getCat($cat, $ns, $limit, $sort='timestamp', $tsstart=$randts);
    return $result;
  }
  
  // @See [[Func:getPrefix]]
  function getPrefix($prefixname)
  {
  	$this->_debug(4,"Function getPrefix called.");
    $o_prefixname = $prefixname;
    $result = array();
    $searchpf = '/^' . $o_prefixname . '/';
    $params=array(
      'action'=>'query',
      'list'=>'allpages',
      'apfrom'=>$prefixname,
      'aplimit'=>500
    );
    $finalpre = $this->callAPI($params);
    if(isset($finalpre['query-continue']['allpages']['apfrom'])) {
      $firstrun = "1";
      $prefart = urlencode($finalpre['query-continue']['allpages']['apfrom']);
    } else {
      $firstrun = "0";
    }
    foreach($finalpre['query']['allpages'] as $finalpre_l) {
      if(!preg_match($searchpf, $finalpre_l['title'])) {
        $done = 1;
      } else {
        array_push($result, $finalpre_l['title']);
      }
    }

    $done=false;
    while(!$done) {
      if(isset($pref['query-continue']['allpages']['apfrom']) || $firstrun == "1") {
        $params['apfrom']=$prefart;
        $pref = $this->callAPI($params);
        $prefart = urlencode($pref['query-continue']['allpages']['apfrom']);
        foreach($pref['query']['allpages'] as $pref_l) {
          if(!preg_match($searchpf, $pref_l['title'])) {
            $done = 1;
          } else {
            array_push($result, $pref_l['title']);
          }
        }
        echo ".";
        $firstrun = 0;
      } else {
        $done = true;
      }
    }
    return $result;
  }

  // @See [[Func:lastEdited]]
  function lastEdited($article)
  {
  	$this->_debug(4,"Function lastEdited called.");
    $params=array(
      'action'=>'query',
      'prop'=>'revisions',
      'titles'=>$article,
      'rvprop'=>'user|comment',
      'rvlimit'=>1
    );
    $sxGetA = $this->callAPI($params);
    $sxGetAID = $sxGetA;
    $sxGetAID = array_shift($sxGetAID);
    $sxGetAID = array_shift($sxGetAID);
    $sxGetAID = array_shift($sxGetAID);
    $sxGetAID = array_shift($sxGetAID);
    $sxAText = array();
    $sxAText['user'] = $sxGetA['query']['pages'][$sxGetAID]['revisions'][0]['user'];
    $sxAText['editsum'] = $sxGetA['query']['pages'][$sxGetAID]['revisions'][0]['comment'];
    return $sxAText;
  }
 
  // @See [[Func:callAPI]]
  function callAPI($params)
  {
  	$this->_debug(4,"Function callAPI called.");
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    if (is_array($params)) {
      $p='';
      foreach($params as $k=>$v) {
        if ($k=='format') {
          continue;
        }
        $p.='&'.urlencode($k).'='.urlencode($v);
      }
      $params=substr($p,1);
    }
    $params.='&format=php';
    $result=$this->_curlDo($url.'api.php',$params);
    $result=unserialize($result);
    //< blinky*mar, 10 may 2011 19:48:18 -0300 - check if $result['error'] is set before using it
    if (isset($result['error']) && $result['error']) {
    //>
      $this->_error("callAPI: API error [".$result['error']['code']."]: ".$result['error']['info']);
      return false;
    }
    return $result;
  }

  // @See [[Func:getTransclusion]]
  function getTransclusion($templatename, $ns, $limit='500')
  {
  	$this->_debug(4,"Function getTransclusion called.");
    $params=array(
      'action'=>'query',
      'list'=>'embedded',
      'eititle'=>$templatename,
      'eilimit'=>$limit
    );
    $result=$this->callAPI($params);
    $pages = array();
    $oresult = $result;
    foreach ($result['query']['embeddedin'] as $single_result) {
      if ($single_result['ns'] == $ns && $ns != "all") {
        array_push($pages, $single_result['title']);
      } else {
        array_push($pages, $single_result['title']);
      }
    }
    $done=false;
    while (!$done) {
      if(isset($result['query-continue']['embeddedin']['eicontinue'])) {
        $params['eicontinue']=
          $result['query-continue']['embeddedin']['eicontinue'];
        $result=$this->callAPI($params);
        foreach ($result['query']['embeddedin'] as $single_result) {
          if ($single_result['ns'] == $ns && $ns != "all") {
            array_push($pages, $single_result['title']);
          } else {
            array_push($pages, $single_result['title']);
          }
        }
      } else {
        $done = true;
      }
    }
    return $pages;
  }

  // @See [[Func:blockUser]]
  function blockUser($user, $expiry, $reason, $ao, $acb, $autoblock, $emailban)
  {
  	$this->_debug(4,"Function blockUser called.");
    $this->_autoexec();
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    if (!$this->_loggedIn && $this->forcelogin) {
		$this->_error("blockUser: Could not login, not blocking user.");
		return false;
	}
    $euser = urlencode($user);
    $postrequest = $url . 'index.php?title=Special:Blockip/' . $euser;
    $DOMresults = $this->inputRetrieve($postrequest);
    //preg_match('/\<input name\=\"wpEditToken\" type\=\"hidden\" value\=\"(.*)\" \/\>/i', $response, $token);
//     >>>
    if (!$token) {
      $this->_error("blockUser: Failed retrieving token for blocking!");
      $this->_debug(2,$response);
      return false;
    }
    $token=$token[1];
    $this->_debug(2,"blockUser: Retrieved token for blocking: $token");
    $this->_debug(3,$response);
    $postrequest = $url . 'index.php?title=Special:Blockip/' . $euser . '&action=submit&maxlag=' . $this->maxlag;
    $postData['wpEditToken'] = urlencode($DOMresults['wpEditToken']);
    $postData['wpBlockAddress'] = $euser;
    $postData['wpBlockOther'] = urlencode($expiry);
    $postData['wpBlockReason'] = urlencode($reason);
    $postData['wpBlock'] = "Block";
    $postData['wpBlockExpiry']='other';
    $postData['wpBlockReasonList']='other';
    if ($ao != null) {
      $postData['wpAnonOnly'] = $ao;
    }
    if ($acb != null) {
      $postData['wpCreateAccount'] = $acb;
    }
    if ($autoblock != null) {
      $postData['wpEnableAutoblock'] = $autoblock;
    }
    if ($emailban != null) {
      $postData['wpEmailBan'] = $emailban;
    }
    $this->_debug(3,"\$postData: ".$this->_varDump($postData));
    $response=$this->_curlDo($postrequest,$postData);
    if (preg_match('/^Waiting for (.*) seconds lagged/', $response)) {
      $this->_error("blockUser: Maxlag hit, not posted.");
      return false;
    }
    $this->_debug(3,'blockUser: '.$response);
    $this->_doSleep();
    return true;
  }
  
  // @See [[Func:varDump]]
  function _varDump($var)
  {
    ob_start();
    var_dump($var);
    return ob_get_clean();
  }
  
  // @See [[Func:modRollback]]
  function modRollback($user, $reason, $action)
  {
  	$this->_debug(4,"Function modRollback called.");
    $this->_autoexec();
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    if (!$this->_loggedIn && $this->forcelogin) {
		$this->_error("modRollback: Could not login, not modifying rollback rights.");
		return false;
	}
    $user2 = urlencode($user);
    $postrequest = $url . 'index.php?title=Special:Userrights&user=' . $user2;
    /*$response = $this->_curlDo($postrequest);
     preg_match('/\<input name\=\"wpEditToken\" type\=\"hidden\" value\=\"(.*)\" \/\>/i', $response, $token);*/
    $DOMresults = $this->inputRetrieve($postrequest);
    $postrequest = $url . 'index.php?title=Special:Userrights&user=' . $user2 . '&maxlag=' . $this->maxlag;
    $postData['wpEditToken'] = $DOMresults['wpEditToken'];
    $postData['user'] = $user;
    switch ($action) {
      case "add":
        $postData['available[]'] = "rollbacker";
        $postData['removable[]'] = "";
        break;
      case "del":
        $postData['removable[]'] = "rollbacker";
        $postData['available[]'] = "";
        break;
    }
    $postData['user-reason'] = $reason;
    $postData['saveusergroups'] = "Save User Groups";
    $response = $this->_curlDo($postrequest,$postData);
    if (preg_match('/^Waiting for (.*) seconds lagged/', $response)) {
      $this->_error("modRollback: Maxlag hit, not posted.");
      return false;
    }
    $this->_doSleep();
    return true;
  }

  // @See [[Func:unblockUser]]
  function unblockUser($user, $reason)
  {
  	$this->_debug(4,"Function unblockUser called.");
    $this->_autoexec();
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    if (!$this->_loggedIn && $this->forcelogin) {
		$this->_error("unblockUser: Could not login, not unblocking user.");
		return false;
	}
    $postrequest = $url . 'index.php?title=Special:Ipblocklist&action=unblock&ip=' . urlencode($user);
    /*$response = $this->_curlDo($postrequest);
     preg_match('/\<input name\=\"wpEditToken\" type\=\"hidden\" value\=\"(.*)\" \/\>/i', $response, $token);*/
    $DOMresults = $this->inputRetrieve($postrequest);
    $postrequest = $url . 'index.php?title=Special:Ipblocklist&action=submit'  . '&action=submit&maxlag=' . $this->maxlag;
    $postData['wpEditToken'] = $DOMresults['wpEditToken'];
    $postData['wpUnblockAddress'] = $user;
    $postData['wpUnblockReason'] = $reason;
    $postData['wpBlock'] = "Unblock";
    $response = $this->_curlDo($postrequest,$postData);
    if (preg_match('/^Waiting for (.*) seconds lagged/', $response)) {
      $this->_error("unblockUser: Maxlag hit, not posted.");
      return false;
    }
    $this->_doSleep();
    return $response;
  }

  // @See [[Func:isBlocked]]
  function isBlocked($user,$moreinfo=false)
  {
  	$this->_debug(4,"Function isBlocked called.");
    if (!$url=$this->_getBaseURL()) {
      return false;
    } else {
  		$user=urlencode($user);
  		$params=array(
  			'action'=>'query',
  			'list'=>'blocks',
  			'bkusers'=>$user,
  			'bklimit'=>'1',
  			'bkdir'=>'newer'
  			);
  		$blocklog = $this->callAPI($params);
  		if (!$moreinfo) {
			if(isset($blocklog['query']['blocks'][0])){
				$this->_debug(3,"isBlocked: $user is blocked.");
  				return true;
  			} else {
  				$this->_debug(3,"isBlocked: $user is not blocked.");
  				return false;
  			};
  		} else {
  			$this->_debug(3,"isBlocked: Information array has been returned.");
  		   	return $blocklog['query']['blocks'][0];
    	};
    };
  }
  
  // @See [[Func:fromURL]]
  function fromURL($string)
  {
  	$this->_debug(4,"Function fromURL called.");
    return str_replace("_"," ",urldecode($string));
  }

  // @See [[Func:toURL]]
  function toURL($string)
  {
  	$this->_debug(4,"Function toURL called.");
    return
      str_replace("%2F","/",
        str_replace("%3A",":",
          urlencode(
            str_replace(" ","_",$string)
          )
        )
      );
  }

  // @See [[Func:destory]]
  function destroy()
  {
    if ($this->_ch) {
      curl_close($this->_ch);
      unset($this->_ch);
    }
    if ($this->_cookiejar) {
      @unlink($this->_cookiejar);
      unset($this->_cookiejar);
    }
  }

  // @See [[Func:destruct]]
  function __destruct()
  {
    $this->destroy();
  }

  // @See [[Func:readConfigurationFile]]
  function readConfigurationFile()
  {
    $this->_debug(4,"Function readConfigurationFile called.");
    $file = $this->configFile;
    if (!isset($this->configFile) || !is_file($file) || !is_readable($file)) {
      $this->_debug(2,"readConfigurationFile: Configuration file \"$file\" doesn't exist, cannot be read or \$configFile is not set.");
      return false;
    }
    require_once $file;
        $this->_debug(4,"readConfigurationFile: Reading varibles.");
    foreach($config as $conf) {
      $key = array_search($conf,$config);
      $this->$key = $conf;
      $this->_debug(4,"readConfigurationFile: $key read.");
    }
    return true;
  }

  
  // @See [[Func:readConf]]
  function _readConf()
  {
    $this->_debug(4,"Function _readConf called.");
    if ($this->_confRead) {
      return NULL;
    }
    $this->_confRead=true;
    return $this->readConfigurationFile();
  }

  // @See [[Func:isPageProtected]]
  function isPageProtected($pageName)
  {
  	$this->_debug(4,"Function isPageProtected called.");
    $this->_debug(2,"isPageProtected: Checking whether page $pageName is protected.");
    if (!$url=$this->_getBaseURL()) {
      return false;
    }
    $url=substr($url,0,-2)."wiki/";
    $content=$this->getURL($url.$this->toURL($pageName));
    if (!($pos=strpos($content,"</head>"))) {
      $this->_error("isPageProtected: Improperly formatted page source in PwikiPro::isPageProtected()!");
      return NULL;
    }
    $content=substr($content,0,$pos);
    if(preg_match("/wgRestrictionEdit[=\s\"\[]+sysop/",$content)) {
      return true;
    } else {
      return false;
    }
  }
  
  // @See [[Func:checkRun]]
  function checkRun()
  {
	$this->_debug(4,"Function checkRun called.");
	if (!$url=$this->_getBaseURL()) {
      return false;
    }
  	$this->_autoexec("login");
  	$username = $this->username;
  	$emailadd = $this->emailadd;
  	$page = 'User:'.$username.'/'.$this->runpage;
	$this->_debug(1,"checkRun: Checking $page.");
	if (!preg_match($this->runregex,$this->getPage($page)) || $this->isBlocked($username)) {
		$this->_error("checkRun: $page has been edited or $username is blocked.");
		return false;
	} else {
		$this->_debug(2,"checkRun: $page has not been edited or and $username is not blocked.");
		return true;      
	}
  }
  
  // @See [[Func:wikifyimplodeArray]]
  function wikifyimplodeArray($array, $after=', ') {
  	$this->_debug(4,"Function wikifyimplodeArray called.");
    while (!isset($array)) {
      $this->_debug(4,'$array was not set, returning NULL.');
      return NULL;
    }
    foreach($array as $item){
        $output .= "[[$item]]$after";
    }
    return $output;
  }
  
  // @See [[Func:wrapimplodeArray]]
  function wrapimplodeArray($around=NULL, $after=', ', $array) {
    $this->_debug(4,"Function wrapimplodeArray called.");
    while (!isset($array)) {
      $this->_debug(4,'$array was not set, returning NULL.');
      return NULL;
    }
    foreach($array as $item){
        $output .= "$around$item$around$after";
    }
    return $output;
  }

  // @See [[Func:inputRetrieve]]
  function inputRetrieve($url) {
    //$dom = true;
    //    if ($dom) {
    $response = $this->_curlDo($url);
    $this->_debug(4,"function inputRetrieve called.");
    $dom = new DOMDocument();
    $dom->loadHTML($response);
    $input = $dom->getElementsByTagName( "input" );
    foreach( $input as $tag ) {
      $name = $dom->getAttribute('name');
      $value = $dom->getAttribute('value');
      $DOMresults[$name] = $value;
      $this->_debug(4,"inputRetrieve: pulled dom $name with value $value");
    }
    return $DOMresults;
  }

}
?>
