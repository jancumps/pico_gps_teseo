# C++ library for ST Teseo GPS

C++ driver for the ST Teseo-LIV3 GPS module.

goals:
- Teseo lib code does not need to know what the target microcontroller is.
- Teseo lib code does not need to know if the project uses I2C or UART
- controller and protocol functionality is provided by the user's project code. It has to plug in a reader and writer function.
- lean, for embedded evelopment

1: [Pico and I2C support](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-1-pico-and-i2c-support?CommentId=a0dfd5e9-20a5-4ae6-8b1d-723620f2db3f)  
2: [Dynamic GPS configuration (and some other things) ](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-2-dynamic-gps-configuration-and-some-other-things)  

