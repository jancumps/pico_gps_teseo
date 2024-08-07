// https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf


/*! @mainpage C++ library for ST Teseo GPS
*
* @authors Jan Cumps
*
* @section MAIN C++ driver for the ST Teseo-LIV3 GPS module 
* goals:
* - Teseo lib code does not need to know what the target microcontroller is.
* - Teseo lib code does not need to know if the project uses I2C or UART
* - controller and protocol functionality is provided by the user's project code. It has to plug in a reader and writer function.
* - lean, for embedded evelopment
*
* articles on element14:  
* 1: [Pico and I2C support](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-1-pico-and-i2c-support?CommentId=a0dfd5e9-20a5-4ae6-8b1d-723620f2db3f)  
* 2: [Dynamic GPS configuration (and some other things) ](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-2-dynamic-gps-configuration-and-some-other-things)  
*
* example:
* -# teseo_reply_response.cpp
* -# teseo_with_nmea_parse.cpp
*/

#include <string>
// for std::find
#include <algorithm>

#include "teseo_communicate.h"
#include "teseo.h"
// for debug messages
#include <stdio.h>
#include "pico/stdlib.h"
#include "reset.h"

teseo::teseo gps;
std::string reply;
std::vector<std::string> replies(NMEA_MAX_REPLIES); 
// vector size is a suggestion. STL will allocate at least NMEA_MAX_REPLIES


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
    
    uint count; // intentionally uninitialised
    bool valid; // intentionally uninitialised

    while (true) {
        printf("+-- start --+\r\n");
        
        valid = gps.ask_gll(reply);
        printf("GLL valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        valid = gps.ask_gsv(replies, count);
        printf("GSV valid: %s. count: %u.\r\n", valid ? "yes" : "no", count);
        std::for_each(replies.begin(), replies.begin() + count, [](auto &s) { 
            printf(s.c_str()); });

        valid = gps.ask_gsa(replies, count);
        printf("GSA valid: %s. count: %u.\r\n", valid ? "yes" : "no", count);
        std::for_each(replies.begin(), replies.begin() + count, [](auto &s) { 
            printf(s.c_str()); });

        valid = gps.ask_gga(reply);
        printf("GGA valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        valid = gps.ask_vtg(reply);
        printf("VTG valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        valid = gps.ask_rmc(reply);
        printf("RMC valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        /* commented out because this is a slow command. It's here to show
        how to call a custom multi line command.
        Uncomment if you want to test this.
        */
        // // example: execute a custom command that returns multiple lines
        // printf("DUMPALMANAC command will take a while to reply.\r\n");
        // teseo::nmea_rr almanac("$PSTMDUMPALMANAC\n\r", "$PSTMDUMPALMANAC");
        // valid = gps.ask_nmea_multiple(almanac, replies, count);
        // printf("DUMPALMANAC valid: %s. count: %u.\r\n", valid ? "yes" : "no", count);
        // std::for_each(replies.begin(), replies.begin() + count, [](auto &s) { 
        //     printf(s.c_str()); });

        printf("+--  end  --+\r\n\r\n");
        sleep_ms(1000);
    }
}