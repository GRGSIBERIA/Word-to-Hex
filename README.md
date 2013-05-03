Word-to-Hex
===========

My local program. I need the tool for nes. The tool exchanges word to hexagon by nametable.

# Usage

1. Compile the word to hex and the return maker.

    gcc word_to_hex.c -o wth.exe
    gcc return_maker.c -o rtmk.exe

2. Use the word to hex. You'll gives 1st option is required.

    wth input_file output_file

You don't gives the 2nd options, default output_file name is 'a.str'.

3. You gives to make string file by the word to hex. 1st option is required.

    rtmk input_file output_file

# Meaning Special Codes

I defined special codes for NES.
NULL, Return, Page Skip, Space, 4 special codes.

Each code using address,
NULL is 0x00,
Space is 0x40,
Return is 0x80,
Page Skip is 0xC0.

but, the Word to hex exported string file don't use the Page Skip.
It's 0xC0 is error code.
If would you not use the return maker,
you'll not use 0xC0.