#ifndef TESEO_H_
#define TESEO_H_

#include <string>
#include "callbackmanager.h"
// std::pair
#include <utility> 
#include <vector>

namespace teseo {

/**
 * A std::pair to hold a NMEA command and its reply signature validation string
*/
typedef const std::pair<const std::string, const std::string> nmea_rr;

//! Driver class for ST Teseo IC.
/*!
  Understands the Teseo command set and replies. 
  For the communication, it relies on I2C or UART functions that the user has to provide.  
  Callbacks are required for:  
  - writing comms protocol  
  - reading comms protocol  
  - resetting the Teseo (optional, see init() documentation)  
 */
class teseo {
public:

    //! constructor.
    teseo() : single_line_parser(2) {}

    //! expose the callback manager for writing to Teseo.
    /*!
      The developer has to register the logic for writing to the device.  
      Callback parameter: const std::string reference with data to be written to Teseo.  
      This can be a C style function, an object method, a static class object method, or lambda code.  

      Example code:
      @code
      // device specific I2C writer function. Classic C style.
      void write(const std::string& s) {
        i2c_write_blocking(i2c_default, I2C_ADDR, reinterpret_cast<const uint8_t*>(s.c_str()), s.length() +1, false);
        return;
      }

      teseo::teseo gps;
      // register that write() function as the handler for writing to the Teseo.
      gps.getWriteCallback().set([](const std::string& s) -> void {
        write(s);
      });
      @endcode
    */
    inline Callback<void, const std::string&>& getWriteCallback() {
        return writer;
    }

    //! expose the callback manager for reading from Teseo
    /*!
      The developer has to register the logic for reading from the device.  
      Callback parameter: std::string reference where the data returned by the Teseo will be stored.  
      For instructions on how to register your handler, check the documentation of getWriteCallback().
    */
    inline Callback<void, std::string&>& getReadCallback() {
        return reader;
    }

    //! expose the callback manager for resetting the Teseo
    /*!
      The developer has to register the logic that resets the device. It is optional. See the init() documentation.  
      The handler has to pull reset low then high. Then it needs to wait 4 seconds to allow the Teseo to boot.  
      Callback parameter: none.  
      For instructions on how to register your handler, check the documentation of getWriteCallback().
    */
    inline Callback<void>& getResetCallback() {
        return resetter;
    }

    //! configure the Teseo for use as a position sensor (optional).
    /*!
    init() is used for dynamic configuration of the Teseo.  
    Precondition (asserted): the handlers have to be set by the developer before calling init().  
    Optional. When the Teseo is preset for i2c according to AN5203,
    init is not required, and developer can cut the 4s 10ms from the startup sequence,
    that are consumed during the reset sequence.  
    https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf  
    In that case, the developer doesn't need to provide a resetter callback handler.
    */
    void init();

    //! utility to parse a multiline Teseo reply into separate strings
    /*!
      \param strings std::vector<qtd::string> reference will get the individual strings.  
      \param s constant std::string reference string to be parsed.  
      \param count uint reference gets count of strings parsed.  
      \param command nmea_rr const reference used to validate the status line.  
      \returns  bool true if valid reply 

      split a big Teseo reply in its individual strings. The separator is "\r\n"
    */
    static bool parse_multiline_reply(std::vector<std::string> & strings, const std::string s, uint& count, const nmea_rr& command) ;

    //! write command to the Teseo
    /*!
      \param s constant std::string reference.  

      Write command to the Teseo by invoking the provided callback handler.  
      Precondition (asserted): the handler has to be set by the developer before first use.     
    */
    void write(const std::string& s);
    
    //! read data from the Teseo
    /*!
      \param s std::string reference. 

      Read replies from the Teseo by invoking the provided callback handler.  
      Precondition (asserted): the handler has to be set by the developer before first use.     
    */    
    void read(std::string& s);

    //! send NMEA request to the Teseo and return reply
    /*!
      \param command const nmea_rr reference holds the NMEA command.   
      \param s std::string reference gets the reply.  
      \returns bool true if valid reply

      Send NMEA request to the Teseo. Validate and Return the repy.
    */    
    bool ask_nmea(const nmea_rr& command, std::string& s);

    //! send NMEA request to the Teseo and return multi line reply
    /*!
      \param command const nmea_rr reference holds the NMEA command.   
      \param strings std::vector<std::string> reference gets the replies. 
      \param count uint reference count of strings parsed.  
      \returns  bool true if valid reply 

      Send NMEA request that expects more than 1 reply to the Teseo. Validate and Return the repies.
    */    
    bool ask_nmea_multiple(const nmea_rr& command, std::vector<std::string>& strings, uint& count);

    //! get GLL request to the Teseo and read reply
    /*!
      \param s std::string reference gets the reply.  
      \returns bool true if valid reply  

      Send request for GLL data to the Teseo. Retrieve the repy.
    */    
    bool ask_gll(std::string& s);

    //! get GSV request to the Teseo and read reply
    /*!
      \param strings std::vector<std::string> reference gets the reply. 
      \param count int reference gets count of replies. 
      \returns boold true if validated.

      Send request for GSV data to the Teseo. Retrieve the replies.
    */    
    bool ask_gsv(std::vector<std::string>& strings, uint& count);

    //! get GSA request to the Teseo and read reply
    /*!
      \param strings std::vector<std::string> reference gets the reply. 
      \param count int reference gets count of replies. 
      \returns boold true if validated.

      Send request for GSA data to the Teseo. Retrieve the replies.
    */    
    bool ask_gsa(std::vector<std::string>& strings, uint& count);

    //! get RMC request to the Teseo and read reply
    /*!
      \param s std::string reference gets the reply.  
      \returns bool true if valid reply  

      Send request for RMC data to the Teseo. Retrieve the repy.
    */    
    bool ask_rmc(std::string& s);

    //! get GGA request to the Teseo and read reply
    /*!
      \param s std::string reference gets the reply.  
      \returns bool true if valid reply  

      Send request for GGA data to the Teseo. Retrieve the repy.
    */    
    bool ask_gga(std::string& s);

    //! get VTG request to the Teseo and read reply
    /*!
      \param s std::string reference gets the reply.  
      \returns bool true if valid reply  

      Send request for VTG data to the Teseo. Retrieve the repy.
    */    
    bool ask_vtg(std::string& s);

private:

    //! command to retrieve GLL data
    static nmea_rr gll;
    //! command to retrieve GSV data
    static nmea_rr gsv;
    //! command to retrieve GSA data
    static nmea_rr gsa;
    //! command to retrieve GGA data
    static nmea_rr gga;
    //! command to retrieve RMC data
    static nmea_rr rmc;
    //! command to retrieve VTG data
    static nmea_rr vtg;
    //! callback manager for writing to the Teseo
    Callback<void, const std::string&> writer;
    //! callback manager for reading from the Teseo
    Callback<void, std::string&> reader;
    //! callback manager for resetting the Teseo
    Callback<void> resetter;
    //! every single line NMEA command has two lines. reply and status
    std::vector<std::string> single_line_parser;

};

} // namespace teseo

#endif // TESEO_H_

