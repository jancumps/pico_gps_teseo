# C++ library for ST Teseo GPS - Raspberry Pico / RP2040 port

C++ driver for the ST Teseo-LIV3 GPS module.  
Port for Rapberry Pico / RP2040, with examples.  

![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/jancumps/pico_gps_teseo/nightly.yml)
![GitHub Release](https://img.shields.io/github/v/release/jancumps/pico_gps_teseo)
  
to checkout the full project, *including submodules*:  

```
git clone https://github.com/jancumps/pico_gps_teseo.git --recursive
```

goals:
- Teseo lib code does not need to know what the target microcontroller is.
- Teseo lib code does not need to know if the project uses I2C or UART
- controller and protocol functionality is provided by the user's project code. It has to plug in a reader and writer function.
- lean, for embedded evelopment

1: [Pico and I2C support](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-1-pico-and-i2c-support?CommentId=a0dfd5e9-20a5-4ae6-8b1d-723620f2db3f)  
2: [Dynamic GPS configuration (and some other things) ](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-2-dynamic-gps-configuration-and-some-other-things)  

Raspberry Pico specific code sits in the port/pico folder.  
Latest development binaries are available on the [nightly release](https://github.com/jancumps/pico_gps_teseo/releases/tag/nightly_development).