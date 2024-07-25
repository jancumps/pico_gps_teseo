#include "teseo.h"
#include <cassert>
namespace teseo {

nmea_rr teseo::gll("$PSTMNMEAREQUEST,100000,0\n\r", "$PSTMNMEAREQUEST,100000,0");
nmea_rr teseo::gsv("$PSTMNMEAREQUEST,80000,0\n\r", "$PSTMNMEAREQUEST,80000,0");
nmea_rr teseo::gsa("$PSTMNMEAREQUEST,4,0\n\r", "$PSTMNMEAREQUEST,4,0");
nmea_rr teseo::gga("$PSTMNMEAREQUEST,2,0\n\r", "$PSTMNMEAREQUEST,2,0");
nmea_rr teseo::rmc("$PSTMNMEAREQUEST,40,0\n\r", "$PSTMNMEAREQUEST,40,0");
nmea_rr teseo::vtg("$PSTMNMEAREQUEST,10,0\n\r", "$PSTMNMEAREQUEST,10,0");

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

bool teseo::parse_multiline_reply(std::vector<std::string> & strings, const std::string s, uint& count, const nmea_rr& command) {
    std::size_t maxelements = strings.size(); // at this moment, don't support growing the array (embedded)
    std::size_t string_index = 0;
    std::size_t vector_index; // intentionally uninitialised
    std::string substring;
    bool valid = false;

    // TODO: current implementation will reply false if there are more answers than strings.size()
    // it stores all valid replies up to that point. the remaining ones are discarded.
    // In the future, I may add a parameter with the max count (default = 0: use strings.size()) 
    // and rely on the user to provide a container that's big enough for that max count (can assert that)
    
    for(vector_index = 0; vector_index < maxelements; vector_index++) {
        std::size_t new_string_index = s.find("\r\n", string_index);
        if (new_string_index == std::string::npos) {// exhausted. This should be the status string
#ifdef __GNUC__ // this requires a recent version of GCC.
#if __GNUC_PREREQ(10,0)
            valid = s.substr(string_index, s.length() - string_index).starts_with(command.second);
#else
            valid = (s.substr(string_index, s.length() - string_index).find(command.second)) != std::string::npos;
#endif
#endif
            break;
        }
        assert(vector_index < maxelements);
        strings[vector_index] = s.substr(string_index, (new_string_index + 2) - string_index); // include the separator
        string_index = new_string_index + 2; // skip the separator
    }
    count = vector_index; // report the number of retrieved data lines.
    std::for_each(strings.begin() + count, strings.end(), [](auto &discard) { 
        discard = std::string(); });
    return valid;
}

void teseo::write(const std::string& s) {
    assert(writer.armed());
    writer.call(s);
}

void teseo::read(std::string& s) {
    assert(reader.armed());
    reader.call(s);
}

bool teseo::ask_nmea(const nmea_rr& command, std::string& s) {
    bool retval; // intentionally not initialised
    uint count;
    write(command.first);
    read(s);
    retval = parse_multiline_reply(single_line_parser, s, count, command);
    s = single_line_parser[0];    
    return retval;
}

bool teseo::ask_nmea_multiple(const nmea_rr& command, std::vector<std::string>& strings, uint& count) {
    uint retval; // intentionally not initialised
    std::string s;
    write(command.first);
    read(s);
    retval = parse_multiline_reply(strings, s, count, command);
    return retval;
}

bool teseo::ask_gll(std::string& s) {
    return ask_nmea(gll, s);
}

bool teseo::ask_gsv(std::vector<std::string>& strings, uint& count) {
    return ask_nmea_multiple(gsv, strings, count);
}

bool teseo::ask_gsa(std::vector<std::string>& strings, uint& count) {
    return ask_nmea_multiple(gsa, strings, count);
}

bool teseo::ask_gga(std::string& s) {
    return ask_nmea(gga, s);
}

bool teseo::ask_rmc(std::string& s) {
    return ask_nmea(rmc, s);
}

bool teseo::ask_vtg(std::string& s) {
    return ask_nmea(vtg, s);
}

} // namespace teseo