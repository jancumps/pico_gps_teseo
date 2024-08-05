#include <string>
// for std::find
#include <algorithm>

#include "teseo_communicate.h"
#include "teseo.h"
// for debug messages
#include <stdio.h>
#include "pico/stdlib.h"

#include "reset.h"

#include "nmea.h"

#include <format>

teseo::teseo gps;
std::string reply;
std::vector<std::string> replies(NMEA_MAX_REPLIES); 
// vector size is a suggestion. STL will allocate at least NMEA_MAX_REPLIES
uint count; // intentionally uninitialised
bool valid; // intentionally uninitialised

void test_gll() {
    valid = gps.ask_gll(reply);
    if (!valid) { return; }
    nmea::gll o;
    valid = nmea::gll::from_data(reply, o);
    printf("GLL source: %i. lat: %f lon: %f.\r\n", 
        o.source, o.lat, o.lon);
    return;
}

void test_gsv() {
    valid = gps.ask_gsv(replies, count);
    if (!valid) { return; }
	for(auto r : replies) {
		nmea::gsv o;
	    valid = nmea::gsv::from_data(r, o);
        printf("GSV source: %i.\r\n", 
            o.source);
	    for(const auto s : o.sats) {
            printf("sat prn: %i, elev: %i, azim: %i, snr: %i.\r\n", 
                s.prn, s.elev, s.azim, s.snr);
	    }
	}
    return;
}

void test_rmc() {
    valid = gps.ask_rmc(reply);
    if (!valid) { return; }
    nmea::rmc o;
    valid = nmea::rmc::from_data(reply, o);
    printf("RMC source: %i. lat: %f lon: %f.\r\n", 
        o.source, o.lat, o.lon);
    return;
}

int main() {
    init();

    gps.getWriteCallback().set([](const std::string& s) -> void {
        write(s);
    });
    gps.getReadCallback().set([](std::string& s) -> void {
        read(s);
    });
    gps.getResetCallback().set([]() -> void {
        reset();
    });

    /*
    when the teseo is preset for i2c according to AN5203,
    init is not required, and you can cut 4s 10ms from the startup sequence
    https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf
    */
    gps.init();
    
    while (true) {
    	test_gll();
	    test_gsv();
	    test_rmc();
        sleep_ms(1000);
    }
}