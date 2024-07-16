#include "teseo.h"
#include <cassert>
namespace teseo {

const std::string teseo::gpgll_msg = std::string("$PSTMNMEAREQUEST,100000,0\n\r");

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

    // reset the i2c message list
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
    while((s.find("$PSTMGPSRESTART") == std::string::npos)); // command successful 

}

void teseo::write(const std::string& s) {
    assert(writer.armed());
    writer.call(s);
}

void teseo::read(std::string& s) {
    assert(reader.armed());
    reader.call(s);
}

void teseo::ask_gpgll(std::string& s) {
    write(gpgll_msg);
    read(s);
    // TODO validate
    // assert( s.length() == 0 || std::count(s.begin(), s.end(), '$') == 2); //if I get a reply, is it valid?

}

} // namespace teseo