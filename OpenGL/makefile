LDLIBS=-lGL -lGLEW -lglfw -lm

spinningcube_withlight: spinningcube_withlight_SKEL.cpp textfile.c
	gcc $^ $(LDLIBS) -o $@

clean:
	rm -f *.o *~

cleanall: clean
	rm -f spinningcube_withlight