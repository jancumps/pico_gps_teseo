#include "teseo.h"
#include <cassert>
namespace teseo {

nmea_rr teseo::gpgll("$PSTMNMEAREQUEST,100000,0\n\r", "$GPGLL,");
nmea_rr teseo::gpgsv("$PSTMNMEAREQUEST,80000,0\n\r", "$GPGSV,");
nmea_rr teseo::gprmc("$PSTMNMEAREQUEST,40,0\n\r", "$GPRMC,");


/*
when the teseo is preset for i2c according to AN5203,
init is not required, and you can cut 4s 10ms from the startup sequence
https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf
*/
 void teseo::init() {
    assert(writer.armed());
    assert(reader.armed());
    assert(resetter.armed());

    std::string s;

    resetter.call();

    // stop the engine
    write("$PSTMGPSSUSPEND\n\r");
    // do {
    //     read(s);            
    // }
    // while((s.find("$PSTMGPSSUSPENDED*") == std::string::npos)); // command successful

    // reset the UART message list
    write("$PSTMCFGMSGL,0,1,0,0\n\r");
    // do {
    //     read(s);            
    // }
    // while((s.find("$PSTMCFGMSGLOK*") == std::string::npos)); // command successful

    // reset the I2C message list
    write("$PSTMCFGMSGL,3,1,0,0\n\r");
    // do {
    //     read(s);            
    // }
    // while((s.find("$PSTMCFGMSGLOK*") == std::string::npos)); // command successful

    // disable the eco-ing message
    write("$PSTMSETPAR,1227,1,2\n\r");
    // do {
    //     read(s);            
    // }
    // while((s.find("$PSTMSETPAROK") == std::string::npos)); // command successful 

    write("$PSTMGPSRESTART\n\r");
    do {
        read(s);            
    }
    // TODO validate if I2C is OK with checking for empty
    while(((s.length()) && s.find("$PSTMGPSRESTART") == std::string::npos)); // command successful
}

void teseo::write(const std::string& s) {
    assert(writer.armed());
    writer.call(s);
}

void teseo::read(std::string& s) {
    assert(reader.armed());
    reader.call(s);
}

bool teseo::ask_nmea(const nmea_rr& command, std::string& s, uint retries) {
    bool retval; // intentionally not initialised
    uint tries; // intentionally not initialised
    for (tries = 0; tries <= retries; tries++){
        write(command.first);
        read(s);
        retval = s.starts_with(command.second);
        if (retval) {
            break;
        }
    }
    return retval;
}

bool teseo::ask_nmea_multiple(const nmea_rr& command, std::vector<std::string>& strings, uint retries) {
    return false; // TODO implement
}

bool teseo::ask_gpgll(std::string& s, uint retries) {
    return ask_nmea(gpgll, s, retries);
}

// TODO: ggsv returns multiple lines
bool teseo::ask_gpgsv(std::vector<std::string>& strings, uint retries) {
    return ask_nmea_multiple(gprmc, strings, retries);
}

bool teseo::ask_gprmc(std::string& s, uint retries) {
    return ask_nmea(gpgsv, s, retries);
}

} // namespace teseo