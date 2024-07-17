#ifndef TESEO_H_
#define TESEO_H_

#include <string>
#include "callbackmanager.h"
// std::pair
#include <utility> 

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
      \param cmd const nmea_rr reference. 
      \param s std::string reference. 
      \param retries int. Default: 1.
      \returns bool true if valid reply

      Send NMEA request to the Teseo. Validate and Return the repy. Retry to get a valid reply
    */    
    bool ask_nmea(const nmea_rr& command, std::string& s, uint retries = 0);

    //! get GPGLL request to the Teseo and read reply
    /*!
      \param s std::string reference. 
      \param retries int. Default: 1.
      \returns bool true if valid reply

      Send request for GPGLL data to the Teseo. Retrieve the repy.
    */    
    bool ask_gpgll(std::string& s, uint retries = 0);


    //! get GPRMC request to the Teseo and read reply
    /*!
      \param s std::string reference. 
      \param retries int. Default: 1.
      \returns bool true if valid reply

      Send request for RPRMC data to the Teseo. Retrieve the repy.
    */    
    bool ask_gprmc(std::string& s, uint retries = 0);

private:

    //! command to retrieve GPGLL data
    static nmea_rr gpgll;
    //! command to retrieve GPRMC data
    static nmea_rr gprmc;
    //! callback manager for writing to the Teseo
    Callback<void, const std::string&> writer;
    //! callback manager for reading from the Teseo
    Callback<void, std::string&> reader;
    //! callback manager for resetting the Teseo
    Callback<void> resetter;

};

} // namespace teseo

#endif // TESEO_H_

