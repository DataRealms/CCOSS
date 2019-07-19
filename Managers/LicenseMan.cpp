//////////////////////////////////////////////////////////////////////////////////////////
// File:            LicenseMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the LicenseMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "LicenseMan.h"

#include <string>
#include <curl/curl.h>
#include "System/MD5/md5.h"
#include <cstring>

namespace RTE
{

const string LicenseMan::m_ClassName = "LicenseMan";

//const string LicenseMan::m_sPPPBaseURL = "https://www.plimus.com/jsp/validateKey.jsp";
const string LicenseMan::m_sPPPBaseURL = "http://licensing.datarealms.com/licensing/";
const string LicenseMan::m_sProductID = "48144";
const string LicenseMan::m_sWatermark = "XXWATERMARKPLACEHOLDERWATERMARKPLACEHOLDERWATERMARKPLACEHOLDERXX";
// This is the same as main dev box hostname just to make it easy to use web browser for manual reset (xor won't touch if chars are the same)
#define XORMACHINEKEY "daeltabox"
#define NUMTOBYTE(x) ((x >='A' && x <= 'F') ? x - 'A' + 0xA : x - '0') // for HEX to BYTE


#ifndef __OPEN_SOURCE_EDITION
//////////////////////////////////////////////////////////////////////////////////////////
// EXECryptor stuff for win32

#ifdef WIN32
#include "EXECryptor.h"
#else // WIN32
#define CRYPT_START
#define CRYPT_END
#endif // WIN32

#endif

/////////////////////////////
// TURN OPTIMIZATIONS OFF
// This is so the EXECryptor markers don't get mangled by the optimizer

#pragma optimize("", off)


//////////////////////////////////////////////////////////////////////////////////////////
// Function for getting the hostname of the machine running. If this isn't the final relese
// this just returns ""

#ifdef WIN32
#include <winsock.h>

string GetMachineHostName()
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

// TODO: REMOVE
//    return "";

    // Add wsock32.lib to linker options!

    WSADATA WSAData;

    // Initialize winsock dll
    if(int result = WSAStartup(MAKEWORD(1, 0), &WSAData))
    {
        // Error handling
        if (result == WSASYSNOTREADY)
            return "";
        else if (result == WSAVERNOTSUPPORTED)
            return "";
        else if (result == WSAEINPROGRESS)
            return "";
        else if (result == WSAEPROCLIM)
            return "";
        else if (result == WSAEFAULT)
            return "";
        else
            return "";
    }

    // Get local host name
    char szHostName[256] = "";
    if(gethostname(szHostName, sizeof(szHostName) - 1))
    {
        // Error -> call 'WSAGetLastError()'
        return "";
    }

    // Cleanup winsock
    WSACleanup();

    return string(szHostName);

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}

#else // WIN32

string GetMachineHostName()
{
    return "";
}

#endif // WIN32


//////////////////////////////////////////////////////////////////////////////////////////
// Callback function for the libcURL http request, this puts together the data returned
// from the http request.

string g_XMLBuffer;

size_t CollectData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    string tempString;
    register size_t actualsize = size * nmemb;

    if ((FILE *)stream == NULL)
    {
        tempString.assign((char *)ptr, actualsize);
        g_XMLBuffer += tempString;
    }
    else
        fwrite(ptr, size, nmemb, (FILE *)stream);

    return actualsize;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this LicenseMan, effectively
//                  resetting the members of this abstraction level only.

void LicenseMan::Clear()
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    m_pLibCURL = 0;
    m_LicenseEmail.erase();
    m_LastLicenseEmail.erase();
    m_LicenseKey.erase();
    m_LastLicenseKey.erase();
    m_HostName.erase();
    m_ValidationAttempted = false;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the LicenseMan object ready for use.

int LicenseMan::Create()
{
#ifndef NODRM
    // Init libcURL
    m_pLibCURL = curl_easy_init();
    if (!m_pLibCURL)
    {
        // This is OK for now, will try again later before contacting server
        return 0;
    }
    // This is just to establish contact with anything before loading a bunch of data on the heap;
    // it seems that if the first libcurl connection is done after everything else loads, curl_easy_perform freezes for a long time.
    GetPPPXML(m_sPPPBaseURL);

    // Try to get the hostname for machine ID
    m_HostName = GetMachineHostName();
#endif // NODRM
    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int LicenseMan::ReadProperty(std::string propName, Reader &reader)
{
//    if (propName == "AddEffect")
//        g_PresetMan.GetEntityPreset(reader);
//    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this LicenseMan with a Writer for
//                  later recreation with Create(Reader &reader);

int LicenseMan::Save(Writer &writer) const
{

// TODO: "Do this!")

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the LicenseMan object.

void LicenseMan::Destroy()
{
#ifndef NODRM
    // Clean up libcURL
    curl_easy_cleanup(m_pLibCURL);
#endif // NODRM
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLicenseEmail
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the license email to use. It will NOT replace any email already set
//                  since startup, only if there isn't one yet, and it checks out in
//                  formatting. If OK, it will be used next time Validate is called.

bool LicenseMan::SetLicenseEmail(string licenseEmail)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // If there already is a email, then don't set a new one, and don't use a badly formatted new one
    if (CheckEmailFormatting(m_LicenseEmail) || !CheckEmailFormatting(licenseEmail))
        return false;

    // Set the new email
    m_LastLicenseEmail = m_LicenseEmail = licenseEmail;
    // Not validated yet
    m_ValidationAttempted = false;

    return true;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLastLicenseEmail
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the last license email entry. Shold only be used by whomever reads
//                  it from the non-obfuscated file. No validation will be done other than
//                  formatting check. Also, if there already is a valid current email
//                  registered, the last license email will be set to that, regardless of what
//                  is passed here.

bool LicenseMan::SetLastLicenseEmail(string lastLicenseEmail)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // Override if there is a current email in use
    if (HasValidatedLicense())
    {
        m_LastLicenseEmail = m_LicenseEmail;
        return true;
    }

    // Check for formatting
    if (!CheckEmailFormatting(lastLicenseEmail))
        return false;

    // Set the last email
    m_LastLicenseEmail = lastLicenseEmail;
    return true;

#ifndef __OPEN_SOURCE_EDITION
    CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the license key to use. It will NOT replace any key already set
//                  since startup, only if there isn't one yet, and it checks out in
//                  formatting. If OK, it will be used next time Validate is called.

bool LicenseMan::SetLicenseKey(string licenseKey)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // If there already is a key, then don't set a new one, and don't use a badly formatted new one
    if (CheckKeyFormatting(m_LicenseKey) || !CheckKeyFormatting(licenseKey))
        return false;

    // Set the new key
    m_LastLicenseKey = m_LicenseKey = licenseKey;
    // Not validated yet
    m_ValidationAttempted = false;

    return true;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLastLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the last license key entry. Shold only be used by whomever reads
//                  it from the non-obfuscated file. No validation will be done other than
//                  formatting check. Also, if there already is a valid current key
//                  registered, the last license key will be set to that, regardless of what
//                  is passed here.

bool LicenseMan::SetLastLicenseKey(string lastLicenseKey)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // Override if there is a current key in use
    if (HasValidatedLicense())
    {
        m_LastLicenseKey = m_LicenseKey;
        return true;
    }

    // Check for formatting
    if (!CheckKeyFormatting(lastLicenseKey))
        return false;

    // Set the last key
    m_LastLicenseKey = lastLicenseKey;
    return true;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Register
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a key and does both (L)ocal validation check and attmepts to do it
//                  on the licensing (S)erver. If this copy is already registered, nothing
//                  will happen here and success will be returned. The results come out as follows:
//                  (? denotes failure to connect to server, 0 means positive invalidation of key)
//                  L S R
//                  1 1 1
//                  1 ? 1
//                  1 0 0
//                  0 1 1 - DEPRECATED KEY!
//                  0 ? 0
//                  0 0 0
//                  This is to allow play on computer temporarily without inet connection.
//                  ALSO, if key used to validate is of the old deprecated plimus system,
//                  then DEPRECATEDKEY is returned here. That is still a success code.

LicenseMan::ServerResult LicenseMan::Register(string licenseEmail, string licenseKey)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // Already registered and validated, don't bother contacting the server
    if (HasValidatedLicense())
        return SUCCESS;

    // Doesn't have a properly formatted email to register with
    if (!CheckEmailFormatting(licenseEmail))
        return INVALIDEMAIL;

    // Check and try to fix formatting errors in the new key
    if (!CheckKeyFormatting(licenseKey))
        return INVALIDKEY;

    // Try to validate the key offline first if we can
    char offlineResult = CheckKeyValidityOffline(licenseEmail.c_str(), licenseKey.c_str());

    // The server result that we will evaluate after all server activity has been attempted
    ServerResult serverResult = UNKNOWNERROR;

    // Re-init libCURL if it isn't running already
    if (!m_pLibCURL)
    {
        m_pLibCURL = curl_easy_init();
        // If still can't get it started, it means the conneciton is down and failed!
        if (!m_pLibCURL)
            serverResult = FAILEDCONNECTION;
    }

    // Assemble the URL
    string plimusURL = m_sPPPBaseURL + "?action=REGISTER" + "&productId=" + m_sProductID + "&email=" + licenseEmail + "&key=" + licenseKey;
    // Add the machine's host name if we could get it
    if (!m_HostName.empty())
//        plimusURL += "&uniqueMachineId=" + XORStrings(m_HostName, XORMACHINEKEY);
        plimusURL += "&uniqueMachineId=" + m_HostName;

    // Contact the server and get response about key registration
    string plimusXML = GetPPPXML(plimusURL);

    // Clean up libcURL to make sure it gets reset
//    curl_easy_cleanup(m_pLibCURL);
//    m_pLibCURL = 0;

    // We failed to contact the server!
    if (plimusXML.empty())
        serverResult = FAILEDCONNECTION;

    // Extract the status code from the returned XML
    string statusCode = ParseStatusCode(plimusXML);
    // The XML we got back didn't have the proper tags we were looking for
    if (statusCode.empty())
        serverResult = INVALIDXML;

    // Handle the different status codes
    if (statusCode == "SUCCESS")
        serverResult = SUCCESS;
    else if (statusCode == "ERROR_INVALIDKEY")
        serverResult = INVALIDKEY;
    else if (statusCode == "ERROR_EMAILMISMATCH")
        serverResult = EMAILMISMATCH;
    else if (statusCode == "ERROR_INVALIDPRODUCT")
        serverResult = INVALIDPRODUCT;
    else if (statusCode == "ERROR_MAXCOUNT")
        serverResult = MAXCOUNT;
    else if (statusCode == "ERROR_EXPIREDKEY")
        serverResult = EXPIREDKEY;
    else if (statusCode == "ERROR_INVALIDMACHINE")
        serverResult = INVALIDMACHINE;
    else
        serverResult = UNKNOWNERROR;

    // Now that we know both local and server validation results, we can figure out the overall result using the following truth table:
    // (? denotes failure to connect to server, 0 means positive invalidation of key)
    // L S R
    // 1 1 1
    // 1 ? 1
    // 1 0 0
    // 0 1 1
    // 0 ? 0
    // 0 0 0

    // Local validation was successful, go through the possibilites
    if (offlineResult != OFFLINEINVALID)
    {
        // Server validation was also either positively successful, or failed connection (possibly offline computer? - that's ok)
        if (serverResult == SUCCESS || serverResult == FAILEDCONNECTION || serverResult == INVALIDXML || serverResult == UNKNOWNERROR)
        {
            // Save the right email and key as the last good ones
            m_LastLicenseEmail = m_LicenseEmail = licenseEmail;
            m_LastLicenseKey = m_LicenseKey = licenseKey;
            // Validation attempt successful
            m_ValidationAttempted = true;
            // Report success with a current up-to-date key
            return SUCCESS;
        }
        // Server validation showed positively negative result, so this is NOT a valid key regardless of what local check says
        else
        {
            // Don't affect current registration status negatively, this was just an attempt for new key
            // Report failure to register
            return serverResult;
        }
    }
    // Local validation failed, but if server says OK then it's still good, but an old deprecated key
    else
    {
        // Server validation was positively successful despite faile local check, so that tells us this is is an old deprecated plimus key
        if (serverResult == SUCCESS)
        {
            // Save the right email and key as the last good ones
            m_LastLicenseEmail = m_LicenseEmail = licenseEmail;
            m_LastLicenseKey = m_LicenseKey = licenseKey;
            // Validation attempt successful
            m_ValidationAttempted = true;
            // Report success with an old deprecated key that should be updated
            return DEPRECATEDKEY;
        }
        // Server validation either failed to connect or showed positively negative result, which is a total fail
        else
        {
            // Don't affect current registration status negatively, this was just an attempt for new key
            // Report failure to validate, and the specific server error
            return serverResult;
        }
    }

    // Report unknown error
    return UNKNOWNERROR;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Unregister
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unregisters this copy, freeing up a license use slot for the currently
//                  registered license key at the server. If not currently registered, this
//                  will do nothing and return success.

LicenseMan::ServerResult LicenseMan::Unregister()
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

#ifdef NODRM
    return SUCCESS;
#endif // NODRM

/* Not applicable to unregistering
    // Doesn't have a properly formatted email to unregister
    if (!CheckEmailFormatting(m_LicenseEmail))
    {
        // Clear the license email
        m_LicenseEmail.erase();
        // Mark us as not validated on the current (now nonexistent) email
        m_ValidationAttempted = false;
        // Report invalid email
        return INVALIDEMAIL;
    }
*/
    // Don't have a key to unregister, don't bother contacting the server
    if (!CheckKeyFormatting(m_LicenseKey))
    {
        // Clear the license email and key
        m_LicenseEmail.erase();
        m_LicenseKey.erase();
        // Mark us as not validated on the current (now nonexistent) key
        m_ValidationAttempted = false;
        // Report success (we are now unregistered)
        return SUCCESS;
    }

    // Re-init libCURL if it isn't running already
    if (!m_pLibCURL)
    {
        m_pLibCURL = curl_easy_init();
        // If still can't get it started, it means the conneciton is down and failed!
        if (!m_pLibCURL)
            return FAILEDCONNECTION;
    }

    // Assemble the URL
    string plimusURL = m_sPPPBaseURL + "?action=UNREGISTER" + "&productId=" + m_sProductID + "&email=" + m_LicenseEmail + "&key=" + m_LicenseKey;
    // Add the machine's host name if we could get it
    if (!m_HostName.empty())
//        plimusURL += "&uniqueMachineId=" + XORStrings(m_HostName, XORMACHINEKEY);
        plimusURL += "&uniqueMachineId=" + m_HostName;

    // Contact the server and get response about key unregistration
    string plimusXML = GetPPPXML(plimusURL);

    // Clean up libcURL to make sure it gets reset
//    curl_easy_cleanup(m_pLibCURL);
//    m_pLibCURL = 0;

    // We failed to contact the server!
    if (plimusXML.empty())
        return FAILEDCONNECTION;

    // Extract the status code from the returned XML
    string statusCode = ParseStatusCode(plimusXML);
    // The XML we got back didn't have the proper tags we were looking for
    if (statusCode.empty())
        return INVALIDXML;

    // Handle the different status codes
    if (statusCode == "SUCCESS")
    {
        // Save the last valid email and key
        m_LastLicenseEmail = m_LicenseEmail;
        m_LastLicenseKey = m_LicenseKey;
        // Clear the license email and key
        m_LicenseEmail.erase();
        m_LicenseKey.erase();
        // Mark us as not validated on the current (now nonexistent) key
        m_ValidationAttempted = false;
        // Report success
        return SUCCESS;
    }
    else if (statusCode == "ERROR_INVALIDKEY" || statusCode == "ERROR_EMAILMISMATCH")
    {
        // Clear the license email and key
        m_LicenseEmail.erase();
        m_LicenseKey.erase();
        // Mark us as not registered anymore
        m_ValidationAttempted = false;
        // Report that we're now unregistered anyway
        return SUCCESS;
    }
    else if (statusCode == "ERROR_INVALIDPRODUCT")
        return INVALIDPRODUCT;
    else if (statusCode == "ERROR_INVALIDMACHINE")
        return INVALIDMACHINE;
    
    // Report unknown error
    return UNKNOWNERROR;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Validate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does both a local check and attempts a server check on the current key.
//                  The results come out as follows:
//                  (? denotes failure to connect to server, 0 means positive invalidation of key)
//                  L S R
//                  1 1 1
//                  1 ? 1
//                  1 0 0
//                  0 1 1 - DEPRECATED KEY!
//                  0 ? 0
//                  0 0 0
//                  This is to allow play on computer temporarily without inet connection.
//                  ALSO, if key used to validate is of the old deprecated plimus system,
//                  then DEPRECATEDKEY is returned here. That is still a success code.

LicenseMan::ServerResult LicenseMan::Validate()
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // Already registered and validated, don't bother contacting the server
    if (HasValidatedLicense())
        return SUCCESS;

    // Doesn't have a properly formatted email to validate
    if (!CheckEmailFormatting(m_LicenseEmail))
    {
        // Clear the license email
        m_LicenseEmail.erase();
        // Mark us as not validated on the current (now nonexistent) email
        m_ValidationAttempted = false;
        // Report invalid email
        return INVALIDEMAIL;
    }

    // Doesn't have a properly formatted key to validate
    if (!CheckKeyFormatting(m_LicenseKey))
    {
        // Clear the license key
        m_LicenseKey.erase();
        // Mark us as not validated on the current (now nonexistent) key
        m_ValidationAttempted = false;
        // Report invalid key
        return INVALIDKEY;
    }

    // Try to validate the key offline first if we can
    char offlineResult = CheckKeyValidityOffline(m_LicenseEmail.c_str(), m_LicenseKey.c_str());

    // The server result that we will evaluate after all server activity has been attempted
    ServerResult serverResult = UNKNOWNERROR;

    // Re-init libCURL if it isn't running already
    if (!m_pLibCURL)
    {
        m_pLibCURL = curl_easy_init();
        // If still can't get it started, it means the conneciton is down and failed!
        if (!m_pLibCURL)
            serverResult = FAILEDCONNECTION;
    }

    // Assemble the URL
    string plimusURL = m_sPPPBaseURL + "?action=VALIDATE" + "&productId=" + m_sProductID + "&email=" + m_LicenseEmail + "&key=" + m_LicenseKey;
    // Add the machine's host name if we could get it
    if (!m_HostName.empty())
//        plimusURL += "&uniqueMachineId=" + XORStrings(m_HostName, XORMACHINEKEY);
        plimusURL += "&uniqueMachineId=" + m_HostName;

    // Contact the server and get response about key validation
    string plimusXML = GetPPPXML(plimusURL);

    // Clean up libcURL to make sure it gets reset
//    curl_easy_cleanup(m_pLibCURL);
//    m_pLibCURL = 0;

    // We failed to contact the server!
    if (plimusXML.empty())
        serverResult = FAILEDCONNECTION;

    // Extract the status code from the returned XML
    string statusCode = ParseStatusCode(plimusXML);
    // The XML we got back didn't have the proper tags we were looking for
    if (statusCode.empty())
        serverResult = INVALIDXML;

    // Handle the different status codes
    if (statusCode == "SUCCESS")
        serverResult = SUCCESS;
    else if (statusCode == "ERROR_INVALIDKEY")
        serverResult = INVALIDKEY;
    else if (statusCode == "ERROR_EMAILMISMATCH")
        serverResult = EMAILMISMATCH;
    else if (statusCode == "ERROR_INVALIDPRODUCT")
        serverResult = INVALIDPRODUCT;
    else if (statusCode == "ERROR_MAXCOUNT")
        serverResult = MAXCOUNT;
    else if (statusCode == "ERROR_EXPIREDKEY")
        serverResult = EXPIREDKEY;
    else if (statusCode == "ERROR_INVALIDMACHINE")
        serverResult = INVALIDMACHINE;
    else
        serverResult = UNKNOWNERROR;

    // Now that we know both local and server validation results, we can figure out the overall result using the following truth table:
    // (? denotes failure to connect to server, 0 means positive invalidation of key)
    // L S R
    // 1 1 1
    // 1 ? 1
    // 1 0 0
    // 0 1 1
    // 0 ? 0
    // 0 0 0

    // Local validation was successful, go through the possibilites
    if (offlineResult != OFFLINEINVALID)
    {
        // Server validation was also either positively successful, or failed connection (possibly offline computer? - that's ok)
        if (serverResult == SUCCESS || serverResult == FAILEDCONNECTION || serverResult == INVALIDXML || serverResult == UNKNOWNERROR)
        {
            // Save the right email and key as the last good ones
            m_LastLicenseEmail = m_LicenseEmail;
            m_LastLicenseKey = m_LicenseKey;
            // Validation attempt successful
            m_ValidationAttempted = true;
            // Report success with a current up-to-date key
            return SUCCESS;
        }
        // Server validation showed positively negative result, so this is NOT a valid key regardless of what local check says
        else
        {
            // Clear the license email and key
//            m_LicenseEmail.erase();
            m_LicenseKey.erase();
            // Mark us as not registered anymore
            m_ValidationAttempted = false;
            // Report failure to validate
            return serverResult;
        }
    }
    // Local validation failed, but if server says OK then it's still good, but an old deprecated key
    else
    {
        // Server validation was positively successful despite faile local check, so that tells us this is is an old deprecated plimus key
        if (serverResult == SUCCESS)
        {
            // Save the right email and key as the last good ones
            m_LastLicenseEmail = m_LicenseEmail;
            m_LastLicenseKey = m_LicenseKey;
            // Validation attempt successful
            m_ValidationAttempted = true;
            // Report success with an old deprecated key that should be updated
            return DEPRECATEDKEY;
        }
        // Server validation either failed to connect or showed positively negative result, which is a total fail
        else
        {
            // Clear the license email and key
//            m_LicenseEmail.erase();
            m_LicenseKey.erase();
            // Mark us as not registered anymore
            m_ValidationAttempted = false;
            // Report failure to validate, and the specific server error
            return serverResult;
        }
    }

    // Report unknown error
    return UNKNOWNERROR;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasValidatedLicense
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether this installation is currently registered with a
//                  license key, and that key has at least been attempted to be validated
//                  once against the server this runtime instance.

bool LicenseMan::HasValidatedLicense()
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

#ifdef _DEBUG
    return true;
#endif //_DEBUG

#ifdef NODRM
    return true;
#endif //_DEBUG

    return m_ValidationAttempted && CheckKeyFormatting(m_LicenseKey);

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckEmailFormatting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a string with a license email to verify that it matches the
//                  required formatting: abcde@abcdef.abc, all lowercase letters.

bool LicenseMan::CheckEmailFormatting(std::string &licenseEmail)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // Remove any illegal chars, could be junk at the beginning or end of a copy-paste
    int badCharIndex = 0;
    while ((badCharIndex = licenseEmail.find_first_not_of(".@!#$%*/?|^{}`~&'+-=_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")) != string::npos)
        licenseEmail.erase(badCharIndex, 1);

    // See if there are either minimum required number of chars (a@b.cd)
    if (licenseEmail.size() < 6)
        return false;

    // See if there are any hopeless characters
    if (licenseEmail.find_first_not_of(".@!#$%*/?|^{}`~&'+-=_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != string::npos)
        return false;

    // Make sure we have a '@' and at least one '.' too
    if (licenseEmail.find_first_of("@") == string::npos || licenseEmail.find_first_of(".") == string::npos)
        return false;

    // See if there are any letters that just need to be lowercased
    if (licenseEmail.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != string::npos)
    {
        // Lowercase all letters
        for (string::iterator itr = licenseEmail.begin(); itr != licenseEmail.end(); ++itr)
        {
            // Only adjust letters
//            if ((*itr) != '-')
                (*itr) = tolower(*itr);
        }
    }

    // Report that the license email string is now well formatted!
    return true;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckKeyFormatting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a string with a license key to verify that it matches the
//                  required formatting: XXX-XXXX-XXXX-XXXX, no numbers, all caps letters.
//                  If there are formatting errors like no dashes or not caps letter, they
//                  will be fixed and success returned.

bool LicenseMan::CheckKeyFormatting(string &licenseKey)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    // Remove any illegal chars, could be junk at the beginning or end of a copy-paste
    int badCharIndex = 0;
    while ((badCharIndex = licenseKey.find_first_not_of("-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")) != string::npos)
        licenseKey.erase(badCharIndex, 1);

    // See if there are either minimum required number of chars (without the dashes), or the max possible (with the dashes)
    if (!(licenseKey.size() == 15 || licenseKey.size() == 18))
        return false;

    // Check for the dashes
    if (licenseKey.size() != 18 || licenseKey[3] != '-' || licenseKey[8] != '-' || licenseKey[13] != '-')
    {
        // If the dashes are not present, see if they are just missing in an notherwise OK key that can be corrected
        if (licenseKey.size() == 15 && licenseKey.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == string::npos)
        {
            // Put the dashes in there
            licenseKey.insert(3, "-");
            licenseKey.insert(8, "-");
            licenseKey.insert(13, "-");
        }
        else
            return false;
    }

    // See if there are any hopeless characters now
    if (licenseKey.find_first_not_of("-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != string::npos)
        return false;

    // See if there are any letters that just need to be capitalized
    if (licenseKey.find_first_of("abcdefghijklmnopqrstuvwxyz") != string::npos)
    {
        // Capitalize all letters
        for (string::iterator itr = licenseKey.begin(); itr != licenseKey.end(); ++itr)
        {
            // Only adjust letters
            if ((*itr) != '-')
                (*itr) = toupper(*itr);
        }
    }

    // Report that the license key string is now well formatted!
    return true;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckKeyValidityOffline
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a string with a formatted license key and accompanying email
//                  address to see if it can be validated without contacting the server.

char LicenseMan::CheckKeyValidityOffline(const char * EMAIL, const char * KEY)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

	// Variables
	md5_state_t state;
	md5_byte_t digest[16];
	unsigned char rnd[2];
	char hex_output[64];
 	char buffer[64];
	char sum_output[4*2 + 1];
	char productID;
	int sum;

    return KEY[0];

	memset (buffer,0,64);
	memset (hex_output,0,64);
	memset (sum_output,0,9);
	// MD5 hash of email addy
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *) EMAIL, strlen(EMAIL));
	md5_finish(&state, digest);

	// Reduce MD5 to 4 bytes, by XORing
	digest[0]=digest[ 0]^digest[ 1]^digest[ 2]^digest[ 3];
	digest[1]=digest[ 4]^digest[ 5]^digest[ 6]^digest[ 7];
	digest[2]=digest[ 8]^digest[ 9]^digest[10]^digest[11];
	digest[3]=digest[12]^digest[13]^digest[14]^digest[15];

	// Get key's PID
	productID=KEY[0];
	
	// Get key's RANDOM NUMBER
	rnd[0]=NUMTOBYTE(KEY[14])*0x10 + NUMTOBYTE(KEY[15]);
	rnd[1]=NUMTOBYTE(KEY[16])*0x10 + NUMTOBYTE(KEY[17]);
	
	// XOR MD5 with our RANDOM NUMBER
	digest[0]=digest[0]^rnd[0];
	digest[1]=digest[1]^rnd[1];
	digest[2]=digest[2]^rnd[0];
	digest[3]=digest[3]^rnd[1];
	
	// Calculate a checksum
	sum=productID+digest[0]+digest[1]+digest[2]+digest[3]+rnd[0]+rnd[1];
	sum=sum%255;

	// Format MD5
    sprintf(hex_output, "%02X%", digest[0]);
    sprintf(hex_output+2, "%02X%", digest[1]);
    sprintf(hex_output+4, "-%02X%", digest[2]);
    sprintf(hex_output+7, "%02X%", digest[3]);

	// Format RANDOM NUMBER
    sprintf(hex_output + 9, "-%02X", rnd[0]);
	sprintf(hex_output + 12, "%02X", rnd[1]);

	// Format CHECKSUM
	sprintf(sum_output, "%02X", sum);

	// Build final key
	buffer[0]=productID;
	strcat(buffer, sum_output);
	strcat(buffer, "-");
	strcat(buffer, hex_output);
	
	// Compare
	if (strcmp(KEY, buffer)==0)
		return productID;
	else
		return OFFLINEINVALID;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this LicenseMan. Supposed to be done every frame
//                  before drawing.

void LicenseMan::Update()
{
    
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPPPXML
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually contacts the Plimus server and returns the XML response.

string LicenseMan::GetPPPXML(string contactURL)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

#ifdef NODRM
    return "";
#endif // NODRM

    if (contactURL.empty())
        return "";

    ////////////////////////////////
    // Set the libcURL options, see http://curl.haxx.se/libcurl/c/curl_easy_setopt.html for reference

    // Use a fresh connection
//    curl_easy_setopt(m_pLibCURL, CURLOPT_FORBID_REUSE, 1);

    // Timeout after 5 secs if can't connect
    curl_easy_setopt(m_pLibCURL, CURLOPT_TIMEOUT, 5);
    // Do not report progress
    curl_easy_setopt(m_pLibCURL, CURLOPT_NOPROGRESS, 1);
    // Set option to write to string
    curl_easy_setopt(m_pLibCURL, CURLOPT_WRITEDATA, 0);
    // Set the callback function that will handle the incoming data and put it into a string
    curl_easy_setopt(m_pLibCURL, CURLOPT_WRITEFUNCTION, CollectData);
    curl_easy_setopt(m_pLibCURL, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(m_pLibCURL, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(m_pLibCURL, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)");
    // This disables peer-side checking of license from the server in the HTTPS/SSL connection
    curl_easy_setopt(m_pLibCURL, CURLOPT_SSL_VERIFYPEER, 0);
    // The URL to call
    curl_easy_setopt(m_pLibCURL, CURLOPT_URL, contactURL.c_str());

    // Reset the buffer which will be filled with return data
    g_XMLBuffer.erase();
    // Go HTTPS request, go!
    CURLcode result = curl_easy_perform(m_pLibCURL);

// TODO: some kind of error handling on the result?

    // This will be empty if there was an error
    return g_XMLBuffer;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ParseStatusCode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Parses and returns the status code string out of the PPP XML data.

string LicenseMan::ParseStatusCode(string plimusXML)
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    if (plimusXML.empty())
        return "";

    // Find the the open and close tags for the status code
    string openTag("<status>");
    string closeTag("</status>");
    int statusStartPos = plimusXML.find(openTag);
    int statusEndPos = plimusXML.find(closeTag);

    // Couldn't find the tags
    if (statusStartPos == string::npos || statusEndPos == string::npos)
        return "";

    // Add the length of the open tag to get the actual start of the status code
    statusStartPos += openTag.size();
    // Now cut out the status code between the tags
    string statusCode = plimusXML.substr(statusStartPos, statusEndPos - statusStartPos);

    return statusCode;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}

#ifndef __OPEN_SOURCE_EDITION
/////////////////////////////
// TURN OPTIMIZATIONS ON
// This is so the EXECryptor markers don't get mangled by the optimizer

#pragma optimize("", on)

#endif
} // namespace RTE
