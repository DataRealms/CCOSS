#ifndef _RTELICENSEMAN_
#define _RTELICENSEMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            LicenseMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the LicenseMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

//#include <list>
//#include <map>
//#include <string>
//using namespace std;

#include "DDTTools.h"
#include "Singleton.h"
#define g_LicenseMan LicenseMan::Instance()

#include "Serializable.h"

typedef void CURL;

// DISABLE DRM EVERYWHERE
#define NODRM

#ifndef __OPEN_SOURCE_EDITION

// EXEcryptor start and end strong code encryption markers.
// These need to be used in UNOPTIMIZED functions!
#if defined(WIN32)

#define CRYPT_START  { \
	__asm _emit 0EBh \
	__asm _emit 006h \
	__asm _emit 0EBh \
	__asm _emit 0FCh \
	__asm _emit 0EBh \
	__asm _emit 0FCh \
	__asm _emit 0FFh \
	__asm _emit 0F8h; \
        {

#define CRYPT_END  } \
	__asm _emit 0EBh \
	__asm _emit 006h \
	__asm _emit 0EBh \
	__asm _emit 0FCh \
	__asm _emit 0EBh \
	__asm _emit 0FCh \
	__asm _emit 0FFh \
	__asm _emit 0FFh; \
	}
#else // defined(WIN32)
// [CHRISK] What to do with this on other platforms
#define CRYPT_START
#define CRYPT_END
#endif // defined(WIN32)

#endif

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           LicenseMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of the DRM scheme, using Plimus' server side
//                  'Piracy Protection' system.
// Parent(s):       Singleton, Serializable?
// Class history:   11/27/2007 LicenseMan created.

class LicenseMan:
    public Singleton<LicenseMan>//,
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


enum OfflineResult
{
    OFFLINEINVALID = 0
};

enum ServerResult
{
    SUCCESS = 0,
    // This is success, but with old plimus key. User should be instructed to make new key online.
    DEPRECATEDKEY,
    INVALIDEMAIL,
    FAILEDCONNECTION,
    INVALIDXML,
    MAXCOUNT,
    INVALIDKEY,
    EMAILMISMATCH,
    INVALIDPRODUCT,
    EXPIREDKEY,
    INVALIDMACHINE,
    UNKNOWNERROR
};


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     LicenseMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a LicenseMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    LicenseMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~LicenseMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a LicenseMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~LicenseMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the LicenseMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this LicenseMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the LicenseMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire LicenseMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the LicenseMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLicenseEmail
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the license email to use. It will NOT replace any email already set
//                  since startup, only if there isn't one yet, and it checks out in
//                  formatting. If OK, it will be used next time Validate is called.
//                  Only when Validate is called after this returns true will
//                  HasValidatedLicense return true.
// Arguments:       The license email string to use for validation, or "" if not registered.
// Return value:    Whether the new email is the first and had good formatting.

    bool SetLicenseEmail(std::string licenseEmail);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLastLicenseEmail
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the last license email entry. Shold only be used by whomever reads
//                  it from the non-obfuscated file. No validation will be done other than
//                  formatting check. Also, if there already is a valid current email
//                  registered, the last license key will be set to that, regardless of what
//                  is passed here.
// Arguments:       The last valid license email string.
// Return value:    Whether the email has ok formatting and has been set.

    bool SetLastLicenseEmail(std::string lastLicenseEmail);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the license key to use. It will NOT replace any key already set
//                  since startup, only if there isn't one yet, and it checks out in
//                  formatting. If OK, it will be used next time Validate is called.
//                  Only when Validate is called after this returns true will
//                  HasValidatedLicense return true.
// Arguments:       The license key string to use for validation, or "" if not registered.
// Return value:    Whether the new key is the first and had good formatting.

    bool SetLicenseKey(std::string licenseKey);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLastLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the last license key entry. Shold only be used by whomever reads
//                  it from the non-obfuscated file. No validation will be done other than
//                  formatting check. Also, if there already is a valid current key
//                  registered, the last license key will be set to that, regardless of what
//                  is passed here.
// Arguments:       The last valid license key string.
// Return value:    Whether the key has ok formatting and has been set.

    bool SetLastLicenseKey(std::string lastLicenseKey);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLicenseEmail
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently registered license email string.
// Arguments:       None.
// Return value:    The currently registered license email string, or "" if not registered.

    std::string GetLicenseEmail() const { return m_LicenseEmail; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLastLicenseEmail
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the last valid license email used. It will be the same as the
//                  current one if it is valid, but will also remain after the current one
//                  is gone.
// Arguments:       None.
// Return value:    The last valid license email, or "" if none has been registered.

    std::string GetLastLicenseEmail() const { return m_LastLicenseEmail; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently registered license key string.
// Arguments:       None.
// Return value:    The currently registered license key string, or "" if not registered.

    std::string GetLicenseKey() const { return m_LicenseKey; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLastLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the last valid license key used. It will be the same as the
//                  current one if it is valid, but will also remain after the current one
//                  is gone.
// Arguments:       None.
// Return value:    The last valid license key, or "" if none has been registered.

    std::string GetLastLicenseKey() const { return m_LastLicenseKey; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMachineHostname
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the hostname of this machine, as determined on startup.
// Arguments:       None.
// Return value:    The hostname of this machine.

    std::string GetMachineHostname() const { return m_HostName; }


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
// Arguments:       The email address to register with.
//                  The string with the key to register with. Format: "XXX-XXXX-XXXX-XXXX"
// Return value:    The result of the registration. See ServerResult enum.

    ServerResult Register(std::string licenseEmail, std::string licenseKey);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Unregister
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unregisters this copy, freeing up a license use slot for the currently
//                  registered license key at the server. If not currently registered, this
//                  will do nothing and return success.
// Arguments:       None.
// Return value:    The result of the server communication. See ServerResult enum.

    ServerResult Unregister();


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
// Arguments:       None.
// Return value:    The result of the server communication. See ServerResult enum.

    ServerResult Validate();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasValidatedLicense
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this installation is currently registered with a
//                  license key, and that key has at least been attempted to be validated
//                  once against the server this runtime instance.
// Arguments:       None.
// Return value:    Has validated license key or not.

    bool HasValidatedLicense();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckEmailFormatting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a string with a license email to verify that it matches the
//                  required formatting: abcde@abcdef.abc.
// Arguments:       The string reference with the license email to check formatting of. If
//                  it can be fixed to match the formatting, it will be altered.
// Return value:    Whether the format ended up good or not.

    bool CheckEmailFormatting(std::string &licenseEmail);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckKeyFormatting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a string with a license key to verify that it matches the
//                  required formatting: XXX-XXXX-XXXX-XXXX, no numbers, all caps letters.
//                  If there are formatting errors like no dashes or not caps letter, they
//                  will be fixed and success returned.
// Arguments:       The string reference with the license key to check formatting of. If
//                  it can be fixed to match the rigid formatting, it will be altered.
// Return value:    Whether the format ended up good or not.

    bool CheckKeyFormatting(std::string &licenseKey);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckKeyValidityOffline
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a string with a formatted license key and accompanying email
//                  address to see if it can be validated without contacting the server.
// Arguments:       The string with the billing email used to obtain the key.
//                  The string with the correctly formatted key to check against the email.
// Return value:    The product ID if the key was valid, or 0 if invalid.

    char CheckKeyValidityOffline(const char * EMAIL, const char * KEY);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this LicenseMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

    void Update();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPPPXML
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually contacts the Plimus server and returns the XML response.
// Arguments:       The full URL to contact.
// Return value:    A string with the complete response from the http(s) request. If the
//                  request failed, or contact couldn't be made, "" is returned.

    std::string GetPPPXML(std::string contactURL);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ParseStatusCode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Parses and returns the status code string out of the PPP XML data.
// Arguments:       The string containing the full XML data returned from the Plimus server.
// Return value:    A string with only the status code, eg "SUCCESS", "ERROR_MAXCOUNT", etc.

    std::string ParseStatusCode(std::string plimusXML);


    // Member variables
    static const std::string m_ClassName;

    // Base of url for Plimus' license server: https://www.plimus.com/jsp/validateKey.jsp?action=[REGISTER|UNREGISTER|VALIDATE]&productId=99999&key=XXX-XXXX-XXXX-XXXX
    static const std::string m_sPPPBaseURL;
    // Product ID of this app on Plimus' server
    static const std::string m_sProductID;
    // Watermark placeholder that will be replaced with an acutal watermark on post-build automatic packaging
    static const std::string m_sWatermark;

    // libcURL handle
    CURL *m_pLibCURL;
    // Billing email associated with the license key
    std::string m_LicenseEmail;
    // The last billing email entered, if any. This will be the same as LicenseEmail if LicenseEmail is valid, but will also remain if LicenseEmail is erased
    std::string m_LastLicenseEmail;
    // License key: XXX-XXXX-XXXX-XXXX (all caps letters, no numbers). If size = 0, no license key has been entered/read
    std::string m_LicenseKey;
    // The last valid key entered, if any. This will be the same as LicenseKey if licensekey is valid, but will also remain if LicenseKey is erased
    std::string m_LastLicenseKey;
    // Name of the host machine, to id it
    std::string m_HostName;
    // Whether license key has at least been attempted to be validated this runtime instance
    bool m_ValidationAttempted;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this LicenseMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    LicenseMan(const LicenseMan &reference);
    LicenseMan & operator=(const LicenseMan &rhs);

};

} // namespace RTE

#endif // File