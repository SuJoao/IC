To build:
	cd src; make; cd ..

To test:
	cd test

	// exercise 1
	../bin/image_rgb_splitter <input ppm file> <output ppm file> <channel>

	// exercise 2 a)
	../bin/image_inverter <input ppm file> <output ppm file>

	// exercise 2 b)
	../bin/./image_mirror <input ppm file> <output ppm file> <mode>

    // exercise 2 c)
    ../bin/image_rotate <input ppm file> <output ppm file> <rotation>

    // exercise 2 d)
    ../bin/image_light_modifier <input ppm file> <output ppm file> <brightness factor>

    // exercise 3
    ../bin/golomb <m> <method> <number> <output_file>
