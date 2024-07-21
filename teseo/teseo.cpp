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

bool teseo::parse_multiline_reply(std::vector<std::string> & strings, const std::string s, uint& count) {
    std::size_t maxelements = strings.size(); // at this moment, don't support growing the array (embedded)
    std::size_t string_index = 0;
    std::size_t vector_index; // intentionally uninitialised
    std::string substring;
    
    for(vector_index = 0; vector_index < maxelements; vector_index++) {
        // TODO check for maxelements (assert will do for now)
        assert(vector_index < maxelements);
        std::size_t new_string_index = s.find("\r\n", string_index);
        if (new_string_index == std::string::npos) {// exhausted
            // TODO maybe validate if the remaining string is the correct confirmation?
            break;
        }
        strings[vector_index] = s.substr(string_index, (new_string_index + 2) - string_index); // include the separator
        string_index = new_string_index + 2; // skip the separator
    }
    count = vector_index;
    return vector_index > 0;
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
    uint count;
    for (tries = 0; tries <= retries; tries++){
        write(command.first);
        read(s);
        retval = s.starts_with(command.second);
        if (retval) {
            break;
        }
    }
    retval = parse_multiline_reply(single_line_parser, s, count);
    s = single_line_parser[0];    
    return retval;
}

bool teseo::ask_nmea_multiple(const nmea_rr& command, std::vector<std::string>& strings, uint& count) {
#ifdef GPS_OVER_I2C
//    assert(false); // "not tested with I2C yet"
#endif
    uint retval; // intentionally not initialised
    std::string s;
    write(command.first);
    read(s);
    retval = parse_multiline_reply(strings, s, count);
    return retval;
}

bool teseo::ask_gpgll(std::string& s, uint retries) {
    return ask_nmea(gpgll, s, retries);
}

bool teseo::ask_gpgsv(std::vector<std::string>& strings, uint& count) {
    return ask_nmea_multiple(gpgsv, strings, count);
}

bool teseo::ask_gprmc(std::string& s, uint retries) {
    return ask_nmea(gprmc, s, retries);
}

} // namespace teseo