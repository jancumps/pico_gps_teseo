#include <string>
#include <ranges>

#include "teseo_communicate.h"
#include "reset.h"
#include "teseo.h"
// for debug messages
#include <stdio.h>
#include "pico/stdlib.h"

#include "nmea.h"

teseo::teseo gps;
// for the container that will hold multy-line replies, 
// you can use an array, vector, list, ...
// based on what your architecture prefers or requires.
// vector size is a suggestion. STL will allocate at least NMEA_MAX_REPLIES
//std::vector<std::string> replies(NMEA_MAX_REPLIES); 
std::array<std::string, NMEA_MAX_REPLIES> replies; 
uint count; // intentionally uninitialised
bool valid; // intentionally uninitialised

// these array will receive parsed objects. 
// this example will perform aggregations and calculations over the containers.
std::array<nmea::gsv, NMEA_MAX_REPLIES> gsv_set = {}; 
std::array<nmea::gsv_sat, NMEA_MAX_REPLIES * 4> sat_set = {}; 

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

void print_talker(const nmea::nmea::talker_id& talker_id) {
    switch (talker_id) {
    case nmea::nmea::gps:
        printf("gps");
        break;
    case nmea::nmea::glonass:
        printf("glonass");
        break;
    case nmea::nmea::galileo:
        printf("galileo");
        break;
    default:
        printf("other");
        break;          
    }
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
    for(auto&& r: std::ranges::subrange(gsv_set.begin() + count, gsv_set.end())) {
        r = {}; // clean out unused tail of the container
    }
    return index;
}

size_t count_constellations(const nmea::nmea::talker_id source) {
    size_t i =  std::count_if(gsv_set.begin(), gsv_set.end(),              
                           [source](const auto& o){ return (o.source == source); });
    return i;
}

int main() {
    init();

    gps.getWriteCallback().set([](const std::string& s) -> void { write(s); });
    gps.getReadCallback().set([](std::string& s) -> void { read(s); });
    gps.getResetCallback().set([]() -> void { reset(); });

    /*
    when the teseo is preset for i2c according to AN5203,
    init is not required, and you can cut 4s 10ms from the startup sequence
    https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf
    */
    gps.init();    
    
    while (true) {
        size_t count; // intentionally uninitialised
        printf("+-- start --+\r\n");
	    size_t reply_count = retrieve_gsv();

        count = count_constellations(nmea::nmea::gps);
        print_talker(nmea::nmea::gps);
        printf(" count: %i\r\n", count);
        count = count_constellations(nmea::nmea::glonass);
        print_talker(nmea::nmea::glonass);
        printf(" count: %i\r\n", count);

        for(auto o : gsv_set | std::views::filter([](const auto& s){ return s.source != nmea::nmea::notset;})) {
            auto satellites = o.sats | std::views::filter([](const nmea::gsv_sat& s){ return s.prn != 0;});
            print_talker(o.source);
            printf(" sat id: ");
            for (const auto& s : satellites) {
              printf(" %i", s.prn);
            }
            printf(". \r\n");
        }

        printf("+--  end  --+\r\n\r\n");
        sleep_ms(1000);
    }
}