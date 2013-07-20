# Overview

The evaluation board is the small, successful program that spawns many large successful programs.

# Build Environment

1. Set up an ARM build environment (out of scope for this document).
   * CodeSourcery Lite 2010 was used as of this writing http://www.codesourcery.com
1. Create build tree

        mkdir quickstart-dev
        cd quickstart-dev

# Source

1. Download StellarisWare http://www.ti.com/tool/SW-LM3S#orderoptions
   * Expand it as
    ```
quickstart-dev/StellarisWare
```
1. Download FreeRTOS http://sourceforge.net/projects/freertos/files/FreeRTOS/
   * Version 6.1.0 used as of this writing.
   * Expand it as
    ```
quickstart-dev/FreeRTOS
```
1. Clone the git repositories

        git clone https://github.com/gvb/quickstart.git
        git clone https://github.com/gvb/lwip.git
        git clone https://github.com/gvb/lwip-contrib.git

# Build

    pushd StellarisWare; make; popd;
    pushd quickstart; make; popd;
