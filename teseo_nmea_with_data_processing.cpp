#include <string>
#include <ranges>
#include <span>

#include "teseo_communicate.h"
#include "reset.h"
#include "teseo.h"
// for debug messages
#include <stdio.h>
#include "pico/stdlib.h"

#include "nmea.h"

teseo::teseo gps;
// for the container that will hold multy-line replies, 
// you can use an array, vector, C array
// based on what your architecture prefers or requires.
// vector size is a suggestion. STL will allocate at least NMEA_MAX_REPLIES
//std::vector<std::string> replies(NMEA_MAX_REPLIES); 
std::array<std::string, NMEA_MAX_REPLIES> replies; // for multi-reply commands
uint count; // intentionally uninitialised
std::string reply; // for single reply commands
bool valid; // intentionally uninitialised

// this array will receive parsed objects. 
// this example will perform aggregations and calculations over the containers.
std::array<nmea::gsv, NMEA_MAX_REPLIES> gsv_set = {}; 
nmea::rmc rmc;

void print_t(const nmea::time_t& t) {
    printf("%02i:%02i:%02i.%03i", 
        (int)(t.hours().count()), (int)(t.minutes().count()),
        (int)(t.seconds().count()), (int)(t.subseconds().count()));
}

void print_d(const std::chrono::year_month_day& t) {
    printf("%4i-%02i-%02i", 
        t.year(), t.month(),
        t.day());
}

void print_talker(const nmea::talker_id& talker_id) {
    switch (talker_id) {
    case nmea::talker_id::gps:
        printf("gps");
        break;
    case nmea::talker_id::glonass:
        printf("glonass");
        break;
    case nmea::talker_id::galileo:
        printf("galileo");
        break;
    default:
        printf("other");
        break;          
    }
}

bool retrieve_rmc() {
    bool valid; // intentionally uninitialised
    valid = gps.ask_rmc(reply);
    if (!valid) { return false; }
    valid = nmea::rmc::from_data(reply, rmc);
    return valid;
}

size_t retrieve_gsv() {
    unsigned int count; // intentionally uninitialised
    valid = gps.ask_gsv(replies, count);
    if (!valid) { return 0; }
    size_t index = 0;
	for(const auto& r : std::ranges::subrange(replies.begin(), replies.begin() + count)) {
        valid = nmea::gsv::from_data(r, gsv_set[index]);
        if (!valid) {
            break;
        }
        index++;
	}

    // clean out unused tail of the container
    std::for_each( gsv_set.begin() + count, gsv_set.end(), [](auto& o){ o = {}; }); 

    return index;    
}

size_t count_constellations(const nmea::talker_id source) {
    size_t i =  std::count_if(gsv_set.begin(), gsv_set.end(),              
                           [source](const auto& o){ return (o.source == source); });
    return i;
}

int main() {
    initialize();

    gps.writer().set([](const std::string& s) -> void { write(s); });
    gps.reader().set([](std::string& s) -> void { read(s); });
    gps.resetter().set([]() -> void { reset(); });

    /*
    when the teseo is preset for i2c according to AN5203,
    init is not required, and you can cut 4s 10ms from the startup sequence
    https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf
    */
    gps.initialize();    
    
    while (true) {
        size_t count; // intentionally uninitialised
        printf("+-- start --+\r\n");

	    // gather data
        retrieve_rmc();
        retrieve_gsv();

        //print rmc info
        printf("time: ");
        print_t(rmc.t);
        printf("\r\nstatus: %s\r\nlat(n) lon(e): %f %f\r\ndate: ", 
            rmc.valid? "active" : "void", rmc.lat, rmc.lon);
        print_d(rmc.d);
        printf("\r\n");


        // aggregate and print data for GPS and GLONASS
        count = count_constellations(nmea::talker_id::gps);
        print_talker(nmea::talker_id::gps);
        printf(" count: %i\r\n", count);
        count = count_constellations(nmea::talker_id::glonass);
        print_talker(nmea::talker_id::glonass);
        printf(" count: %i\r\n", count);

        // print satellites
        for(auto o : gsv_set | std::views::filter([](const auto& s){ return s.source != nmea::talker_id::notset;})) {
            print_talker(o.source);
            printf(" sat id: ");
            for (const auto& s : o.sats | std::views::filter([](const nmea::gsv_sat& s){ return s.prn != 0;})) {
                printf(" %i", s.prn);
            }
            printf(". \r\n");
        }

        // print all satellites accross constellations with their attributes
        for(auto o : gsv_set | std::views::filter([](const auto& s){ return s.source != nmea::talker_id::notset;})) {
            for (const auto& s : o.sats | std::views::filter([](const nmea::gsv_sat& s){ return s.prn != 0;})) {
                printf("sat id: %i, elev: %i, azim: %i, snr: %i, source: ", s.prn, s.elev, s.azim, s.snr);
                print_talker(o.source);
                printf(".\r\n");
            }
        }

        printf("+--  end  --+\r\n\r\n");
        sleep_ms(1000);
    }
}