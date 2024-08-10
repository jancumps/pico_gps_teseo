#include <string>

#include "teseo_communicate.h"
#include "reset.h"
#include "teseo.h"
// for debug messages
#include <stdio.h>
#include "pico/stdlib.h"

#include "nmea.h"

teseo::teseo gps;
std::vector<std::string> replies(NMEA_MAX_REPLIES); 
// vector size is a suggestion. STL will allocate at least NMEA_MAX_REPLIES
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

void retrieve_gsv() {
    valid = gps.ask_gsv(replies, count);
    if (!valid) { return; }
    unsigned int index = 0;
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

    return;
}


size_t count_constellations(const nmea::nmea::talker_id source) {
    size_t i =  std::count_if(gsv_set.begin(), gsv_set.end(),              
                           [source](const auto& o){ return (o.source == source); });
    return i;
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
    
    size_t count; // intentionally uninitialised
    while (true) {
        printf("+-- start --+\r\n");
	    retrieve_gsv();

        count = count_constellations(nmea::nmea::gps);
        printf("count of gps: %i\r\n", count);
        count = count_constellations(nmea::nmea::glonass);
        printf("count of glonass: %i\r\n", count);

        printf("+--  end  --+\r\n\r\n");
        sleep_ms(1000);
    }
}