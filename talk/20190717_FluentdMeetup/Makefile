IMAGES = img/overview.png img/comp.png img/kafka.png img/lamp.png img/lamp-log.png img/lamp-conn.png

slide.pdf: slide.html $(IMAGES)
	wkhtmltopdf -O Landscape $< $@

img/comp.png: dat/comp.gpi dat/comp.dat
	gnuplot dat/comp.gpi > $@

img/%.png: dat/%.dia
	dia -e $@ $^

clean:
	rm -f slide.pdf
	rm -f $(IMAGES)
