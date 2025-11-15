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

	// exercise 4
	../bin/wav_lossless_enc <input wav sample> <output conpressed file> [flags]

	flags:
	-b <block_size>   Block size for encoding (default: 1024)
	-p <order>        Predictor order 0-3 (default: 1)
	-m <method>       Negative handling method:
						'zigzag', 'sign_magnitude'
						(default: zigzag)
	-gd               Use dynamic Golomb m (default)
	-gs <m_value>     Use static Golomb m value

	../bin/wav_lossless_dec <input compressed file> <output wav sample>


